#ifndef __DEVICE_INFO_READ__
#define __DEVICE_INFO_READ__
/*
�ڴ���Ϣ�ṹ��
*/
typedef struct {
	long long used;//�Ѿ�ʹ��
	long long free;//����
	long long buff;//����
	long long cached;//�Ѿ�����
}MemoryInfo_t;
/*
CPU��Ϣ�ṹ��
*/
typedef struct {
	float usr;//�û�CPUʱ��
	float sys;//ϵͳCPUʱ��
	float nic;//����CPUʱ��
	float idle; //CPU����ʱ��
	float io;//I/O�ȴ�
	float irq;//Ӳ�ж�
	float sirq;//���ж�
}CPUInfo_t;
/*
�豸��Ϣ�ṹ��
*/
typedef struct {
	CPUInfo_t nCPU;//CPU
	MemoryInfo_t nMemory;//�ڴ�
}DeviceInfo_t;
 

/*
	��ȡ��ǰ����CPUռ�����
	���������̵�����
	����ֵ���ý��̵�ǰCPUռ����
*/
float GetProcessCpuRate(char *process_name);
/*
	��ȡ���豸״̬
	�������豸��Ϣ�洢�ṹ��
*/
void GetDeviceInfo(DeviceInfo_t *device_info);
/*���ϵͳ����Դ��д��־*/
void MonitorDeviceInfo();
#endif
