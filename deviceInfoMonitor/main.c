#include <stdio.h>
#include <unistd.h>  
#include "include/deviceInfoRead.h"
/*
	������:
	ѭ����ʱ�Ķ�ȡCPU��Ϣ ������Ϣ���浽��־��
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
