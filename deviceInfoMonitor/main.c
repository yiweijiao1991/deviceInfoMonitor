#include <stdio.h>
#include <unistd.h>  
#include "include/deviceInfoRead.h"
/*
	主函数:
	循环定时的读取CPU信息 进程信息保存到日志中
*/
int main()
{
	sleep(60);
	while(1)
	{
		MonitorDeviceInfo();//cpu memory test
		sleep(5);
	}
	
	return 0;
}
