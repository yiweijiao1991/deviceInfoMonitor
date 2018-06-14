
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdarg.h>
#include <sys/time.h>
//LOG的存储路径
#define  LOG_PATH    "/home/wintone/deviceInfoLog/"

//创建文件夹
static int createDir(char *fileName)
{
	if((fileName == NULL) || (strlen(fileName) == 0)){
		return -1;
	}
	char	*tag = NULL;
	char	buf[1000],path[1000];
	int	flag = 0;
	int	ret;
	
	for(tag=fileName;*tag;tag++)
	{
		if (*tag=='/')
		{
			memset(buf, 0, 1000);
			memset(path, 0, 1000);
			strcpy(buf,fileName);
			buf[strlen(fileName)-strlen(tag)+1]='\0';
			strcpy(path,buf);
			if (access(path,0)==-1)
			{
				ret = mkdir(path, S_IRWXU);
				if(ret < 0)
				{
					return -1;
				}else{
					flag = 1;
				}
			}
		}
	}
	if(flag == 1){
		return 0;
	}else{
		return 1;
	}
}

//写文件
static int write_file(char *chFileName, char *chData, int nSize)
{
	if((chFileName == NULL)
		|| (chData == NULL)
		|| (nSize <= 0))
		return -1;
	
	// 自动创建目录
	if(createDir(chFileName) == -1)
		return -1;
	
	// 保存文件
	FILE *file;
	file = fopen(chFileName, "a+");
	if(file == NULL)
		return -1;
printf("%s",chData);
	fwrite(chData, nSize, 1, file);
	fclose(file);
	
	return 0;
}
//写设备信息日志
int DeviceInfoLogWrite(const char *pFormat, ...)
{	
	char		chSavePath[256] = {0};
	char		loginfo[1024] = {0};

	va_list		args;
	time_t 		tt;
	struct tm 	*t;
	int		nRet;
	time(&tt);
	t = gmtime(&tt); 

	sprintf(loginfo, "%02d:%02d:%02d ", t->tm_hour, t->tm_min, t->tm_sec);
	va_start(args, pFormat);
	vsprintf(loginfo+strlen(loginfo),pFormat,args);
	va_end(args);
	sprintf(loginfo+strlen(loginfo), "\n");
	memset(chSavePath, 0, sizeof(chSavePath));
	sprintf(chSavePath, "%s%d%02d%02d.log", LOG_PATH,t->tm_year + 1900, t->tm_mon+1, t->tm_mday);
	nRet = write_file(chSavePath, loginfo, strlen(loginfo));
	
	return nRet;
	
}

