#ifndef __DEVICE_INFO_READ__
#define __DEVICE_INFO_READ__
/*
内存信息结构体
*/
typedef struct {
	long long used;//已经使用
	long long free;//闲置
	long long buff;//闲置
	long long cached;//已经缓存
}MemoryInfo_t;
/*
CPU信息结构体
*/
typedef struct {
	float usr;//用户CPU时间
	float sys;//系统CPU时间
	float nic;//优雅CPU时间
	float idle; //CPU空闲时间
	float io;//I/O等待
	float irq;//硬中断
	float sirq;//软中断
}CPUInfo_t;
/*
设备信息结构体
*/
typedef struct {
	CPUInfo_t nCPU;//CPU
	MemoryInfo_t nMemory;//内存
}DeviceInfo_t;
 

/*
	读取当前进程CPU占用情况
	参数：进程的名称
	返回值：该进程当前CPU占用率
*/
float GetProcessCpuRate(char *process_name);
/*
	读取当设备状态
	参数：设备信息存储结构体
*/
void GetDeviceInfo(DeviceInfo_t *device_info);
/*监测系统的资源并写日志*/
void MonitorDeviceInfo();
#endif
