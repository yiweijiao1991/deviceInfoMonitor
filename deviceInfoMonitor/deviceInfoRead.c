#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>  
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <ctype.h>
#include <fcntl.h>
#include <wait.h>
#include <time.h>
#include "deviceInfoRead.h"
#include "include/log.h"

//读取设备信息时间间隔单位秒
#define READ_DEVICEINFO_TIMEINTERVAL  30
//写日志频率
#define WRITE_LOG_TIMEINTERVAL 1800

typedef struct
{
	float cpumax;
	float cpumin;
	double cpusum;
}CpuInfo_t;//系统一共消耗的CPU资源
typedef struct
{
	long long   memorymax;
	long long   memorymin;
	long long memorysum;
}memoryInfo_t;//内存使用情况
typedef struct{
	CpuInfo_t ipcinfo;
	CpuInfo_t sqliteinfo;
	CpuInfo_t systemcpu;
	memoryInfo_t memoryinfo;
}deviceInfoStatistics_t;

//默认值
deviceInfoStatistics_t m_deviceInfoStatistics_model =
{
	.ipcinfo = {0,100,0},
	.sqliteinfo = {0,100,0},
	.systemcpu = {0,100,0},
	.memoryinfo = {0,99999,0}
};


/*
重写的popen函数 该函数不会拷贝父进程的栈空间，
功能参照popen
*/
static FILE *mypopen(char *cmd,char type)  
{  
	int pipefd[2];           //
	int pid_t;               //  
	if(type !='r' && type != 'w')  
	{  
	    printf("myopen() flag error/n");  
	    return NULL;  
	}  
	if(pipe(pipefd)<0)        // 
	{  
	    printf("myopen() pipe create error/n");  
	    return NULL;  
	}    
	pid_t=vfork();             //

	if(pid_t < 0)   
	    return NULL;  

	if(0 == pid_t)            //
	{  
	    if(type == 'r')  
	    {  
		close(pipefd[0]);               //
		dup2(pipefd[1],STDOUT_FILENO);  //
		close(pipefd[1]);         

	    }  
	    else{  
		close(pipefd[1]);  
		dup2(pipefd[0],STDIN_FILENO);  
		close(pipefd[0]);  
	    }  
 		execl("/bin/sh", "sh", "-c",cmd, (char*)0);  
		_exit(127);      
	}  	  
	wait(0);                                //
	if(type=='r'){  
	    close(pipefd[1]);  
	    return fdopen(pipefd[0],"r");   //
	}else{  
	    close(pipefd[0]);  
	    return fdopen(pipefd[1],"w");  
	}  
}  
/*
读取现在CPU的总使用情况
参数：CPU信息结构体
*/
static void GetCPUInfo(CPUInfo_t *cpu_info)
{
	char cpu_string[100] = {0};//读取到的CPU情况字符串
	char cmd_cpu[] ="top -b -n1| head -n 2 | grep CPU |awk '{print($2,$4,$6,$8,$10,$12,$14);}'";//cpu查询命令
	
	FILE *pp = mypopen(cmd_cpu, 'r'); //建立管道  
	if (pp != NULL)  
	{  
		if(fgets(cpu_string, sizeof(cpu_string), pp))
		{
			sscanf(cpu_string,"%f%% %f%% %f%% %f%% %f%% %f%% %f%%",&cpu_info->usr,&cpu_info->sys,&cpu_info->nic,&cpu_info->idle,&cpu_info->io,&cpu_info->irq,&cpu_info->sirq);
		}		
		pclose(pp); //关闭管道	
	} 
	return ;
}
/*
读取现在内存的总使用情况
参数：内存信息结构体
*/
static void GetMemoryInfo(MemoryInfo_t *memory_info)
{
	char memory_string[100] = {0};//读取到的内存情况字符串
	char cmd_cpu[] ="top -b -n1 | head -n 1 | awk '{print($2,$4,$8,$10);}'";//cpu查询命令	
	FILE *pp = mypopen(cmd_cpu, 'r'); //建立管道  
	if (pp != NULL)  
	{  
		if(fgets(memory_string, sizeof(memory_string), pp))
		{
			//printf("get str %s",memory_string);
			sscanf(memory_string,"%lldK %lldK %lldK %lldK",&memory_info->used,&memory_info->free,&memory_info->buff,&memory_info->cached);
		}		
		pclose(pp); //关闭管道	
	} 
	return ;
}
/****************************
			外部接口
*****************************/
/*
	读取当设备状态
	参数：设备信息存储结构体
*/
void GetDeviceInfo(DeviceInfo_t *device_info)
{
	GetCPUInfo(&(device_info->nCPU));//读取CPU信息
	GetMemoryInfo(&(device_info->nMemory));//读取内存信息
	return ;
}
/*
	读取当前进程CPU占用情况
	参数：进程的名称
	返回值：该进程当前CPU占用率
*/
float GetProcessCpuRate(char *process_name)
{

	float cpurate = 0;//CPU使用率
	char cpu_string[100] = {0};//读取到的CPU情况字符串
	char cmd[100] ={0};//查询命令
	
	if(strcmp(process_name,"ipc_app") == 0)//查询ipc_app命令 该命令特殊 cpu在第8个参数
		sprintf(cmd,"top -b -n1 | grep %s| awk '{print($8);}'",process_name);//XXXX就是你要监控的进程名
	else
		sprintf(cmd,"top -b -n1 | grep %s| awk '{print($9);}'",process_name);//XXXX就是你要监控的进程名

	FILE *pp = mypopen(cmd, 'r'); //建立管道  
	if (pp != NULL)  
	{  
		if(fgets(cpu_string, sizeof(cpu_string), pp))
		{
			cpurate =  atof(cpu_string);//字符串转成浮点型数字
		}		
		pclose(pp); //关闭管道	
	} 
	return cpurate;
}

/*监测系统的资源并写日志*/
void MonitorDeviceInfo()
{
	//ipc_app
	float ipc_cpu = 0;
	//sqlite
	float sqlite_cpu = 0;
	//system_cpu
	float system_cpu = 0;
	//memory
	long long memory = 0;	
	static int rounds = 0;
	static deviceInfoStatistics_t deviceInfoStatistics = {{0,100,0},{0,100,0},{0,100,0},{0,99999,0}};
	static int firsttime = 1; //first time start
	DeviceInfo_t deviceInfo;//设备信息结构体

	time_t nowTime;		//当前时间
	static time_t lastTimeGetInfo;
	static 	time_t lastTimeWriteLog;

	if(firsttime)
	{
		lastTimeGetInfo = time(NULL);
		lastTimeWriteLog = time(NULL);
		nowTime = time(NULL);
		firsttime = 0;
	}

	nowTime = time(NULL);

	if((nowTime - lastTimeGetInfo) >=  READ_DEVICEINFO_TIMEINTERVAL || (nowTime - lastTimeGetInfo) < 0)
	{
		
		memset(&deviceInfo,0,sizeof(DeviceInfo_t));
		rounds++;
		//进程检测
		//ipc_app进程
		ipc_cpu = GetProcessCpuRate("ipc_app");
		if(deviceInfoStatistics.ipcinfo.cpumax< ipc_cpu)
			deviceInfoStatistics.ipcinfo.cpumax = ipc_cpu;
		if(deviceInfoStatistics.ipcinfo.cpumin > ipc_cpu)
			deviceInfoStatistics.ipcinfo.cpumin = ipc_cpu;
		deviceInfoStatistics.ipcinfo.cpusum+=ipc_cpu;

		//sqlite进程
		sqlite_cpu = GetProcessCpuRate("sqliteControl");
		if(deviceInfoStatistics.sqliteinfo.cpumax < sqlite_cpu)
			deviceInfoStatistics.sqliteinfo.cpumax = sqlite_cpu;
		if(deviceInfoStatistics.sqliteinfo.cpumin > sqlite_cpu)
			deviceInfoStatistics.sqliteinfo.cpumin = sqlite_cpu;
		deviceInfoStatistics.sqliteinfo.cpusum+=sqlite_cpu;

		//系统信息检测
		GetDeviceInfo(&deviceInfo);
		//memory
		memory = deviceInfo.nMemory.free+deviceInfo.nMemory.cached+deviceInfo.nMemory.buff;
		if(deviceInfoStatistics.memoryinfo.memorymax < memory)
			deviceInfoStatistics.memoryinfo.memorymax = memory;
		if(deviceInfoStatistics.memoryinfo.memorymin > memory)
			deviceInfoStatistics.memoryinfo.memorymin = memory;
		deviceInfoStatistics.memoryinfo.memorysum+=memory;
		//system_cpu
		system_cpu = deviceInfo.nCPU.usr+deviceInfo.nCPU.sys;
		if(deviceInfoStatistics.systemcpu.cpumax < system_cpu)
			deviceInfoStatistics.systemcpu.cpumax = system_cpu;
		if(deviceInfoStatistics.systemcpu.cpumin > system_cpu)
			deviceInfoStatistics.systemcpu.cpumin = system_cpu;
		deviceInfoStatistics.systemcpu.cpusum+=system_cpu;

		lastTimeGetInfo = nowTime;

		if((nowTime - lastTimeWriteLog) >=  WRITE_LOG_TIMEINTERVAL || (nowTime - lastTimeWriteLog) < 0)
		{
			//写日志[]内部依次最大值最小值平均值	
			DeviceInfoLogWrite("[now,max,min,aver] ipc_app[%4.1ff,%4.1ff,%4.1ff,%4.1ff] sqliteControl[%4.1ff,%4.1ff,%4.1ff,%4.1ff] memoryInfo[%6lld,%6lld,%6lld,%6lld] CPUInfo[%4.1f,%4.1f,%4.1f,%4.1f],{memoryInfoDetailed: used:%6lldK,free:%6lldK,buff:%6lldK,cached:%6lldK},{CPUInfoDetailed: usr:%4.1f,sys:%4.1f,nic:%4.1f,idle:%4.1f,io:%4.1f,irq:%4.1f,sirq:%4.1f}"\
							,ipc_cpu,deviceInfoStatistics.ipcinfo.cpumax,deviceInfoStatistics.ipcinfo.cpumin,deviceInfoStatistics.ipcinfo.cpusum/rounds \
							,sqlite_cpu,deviceInfoStatistics.sqliteinfo.cpumax,deviceInfoStatistics.sqliteinfo.cpumin,deviceInfoStatistics.sqliteinfo.cpusum/rounds \
							,memory,deviceInfoStatistics.memoryinfo.memorymax,deviceInfoStatistics.memoryinfo.memorymin,deviceInfoStatistics.memoryinfo.memorysum/rounds \
							,system_cpu,deviceInfoStatistics.systemcpu.cpumax,deviceInfoStatistics.systemcpu.cpumin,deviceInfoStatistics.systemcpu.cpusum/rounds \
							,deviceInfo.nMemory.used,deviceInfo.nMemory.free,deviceInfo.nMemory.buff,deviceInfo.nMemory.cached \
							,deviceInfo.nCPU.usr,deviceInfo.nCPU.sys,deviceInfo.nCPU.nic,deviceInfo.nCPU.idle,deviceInfo.nCPU.io,deviceInfo.nCPU.irq,deviceInfo.nCPU.sirq);					
			rounds = 0;
			deviceInfoStatistics = m_deviceInfoStatistics_model;				
			lastTimeWriteLog = nowTime;
		}

	}
	return ;
}
