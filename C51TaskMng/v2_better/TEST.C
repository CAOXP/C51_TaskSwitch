/*
1)  ����������ֲ�ԣ���CPU�޹أ������κ�֧��C���Ա�̵�CPU�������ã������Ľ�����51��Ƭ��Ϊ�����ѣ���ʵ���Ͽ���������ֲ��
2)  С֮��С�� ԭ���ܼ򵥣�һ���Ͷ���
3)  ʡ֮��ʡ�� ����˵��RAM��ROMʡ�����¡�
4)  ȡprotothread֮����������ʱ����״̬����α�߳��﷨�ںϵ�һ����ܣ�����������������д����
5)  ���ڶ�ʱ������������Ч�ʸߣ���󻯼�����Ч�Ĵ�������ʱ�䡣
***********************************************************/  

#include <STC12C2052AD.H>
#include <stdio.h>

/****СС��������ʼ**********************************************/
#define MAXTASKS 3
volatile unsigned char timers[MAXTASKS];
#define _SS static unsigned char _lc=0; switch(_lc){default: 
#define _EE ;}; _lc=0; return 255;
#define WaitX(tickets)  do {_lc=(__LINE__+((__LINE__%256)==0))%256; return tickets ;} while(0); case (__LINE__+((__LINE__%256)==0))%256: 

#define RunTask(TaskName,TaskID) do { if (timers[TaskID]==0) timers[TaskID]=TaskName(); }  while(0); 
#define RunTaskA(TaskName,TaskID) { if (timers[TaskID]==0) {timers[TaskID]=TaskName(); continue;} }   //ǰ����������ȱ�ִ֤��

#define CallSub(SubTaskName) do {unsigned char currdt; _lc=(__LINE__+((__LINE__%256)==0))%256; return 0; case (__LINE__+((__LINE__%256)==0))%256:  currdt=SubTaskName(); if(currdt!=255) return currdt;} while(0);
#define InitTasks() {unsigned char i; for(i=MAXTASKS;i>0 ;i--) timers[i-1]=0; }
#define UpdateTimers() {unsigned char i; for(i=MAXTASKS;i>0 ;i--){if((timers[i-1]!=0)&&(timers[i-1]!=255)) timers[i-1]--;}}

#define SEM unsigned int 
//��ʼ���ź���
#define InitSem(sem) sem=0;
//�ȴ��ź���
#define WaitSem(sem) do{ sem=1; WaitX(0); if (sem>0) return 1;} while(0);
//�ȴ��ź�����ʱ������� ��ʱ��tickets ���Ϊ0xFFFE
#define WaitSemX(sem,tickets)  do { sem=tickets+1; WaitX(0); if(sem>1){ sem--;  return 1;} } while(0);
//�����ź���
#define SendSem(sem)  do {sem=0;} while(0);

/*****СС����������*******************************************************/


sbit LED1 = P2 ^ 1;
sbit LED2 = P2 ^ 2;

sbit LED0 = P2 ^ 5;

unsigned char task0()
{
    _SS
    while(1)
    {
        WaitX(50);
        LED0 = !LED0;
    }
    _EE
}

unsigned char  task1()
{
    _SS
    while(1)
    {
        WaitX(100);
        LED1 = !LED1;
    }
    _EE
}

unsigned char  task2()
{
    _SS
    while(1)
    {
        WaitX(100);
        LED2 = !LED2;
    }
    _EE
}








/*
		������Ҫ���ݲ�ͬ��оƬ���в�ͬ������
*/



void InitT0()
{
    TMOD = 0x21;
    IE |= 0x82;  // 12t
    TL0 = 0Xff;
    TH0 = 0XDB;
    TR0 = 1;
}

void INTT0(void) interrupt 1 using 1
{
    TL0=0Xff;   //10ms ��װ
    TH0=0XDB;	//b7;    

    UpdateTimers();

    RunTask(task0,0);//����0���о�ȷ��ʱ���ִ�е�Ȩ�ޣ�Ҫ��task0ÿ��ִ������ʱ��<0.5�� ticket
}


void main()
{
	InitT0();
	InitTasks(); //��ʼ������ʵ�����Ǹ�timers����
	while(1)
	{
		//RunTask(task0,0);
		RunTaskA(task1,1);//����1���б�����2�ߵ�����Ȩ��                   
		RunTaskA(task2,2);//����2���е͵�����Ȩ��     
		/*
			������������ȼ���1.���ȼ������ظ���
							  2.��ֵ���ø�����������
		*/              
	}
}




/*

����������������������������������������Ϊ˵������������������������������������������������������������������������

СС��������������д����Ҫע��ģ���Ҫ�����㣺

1�� �������ڲ����������鶼�þ�̬�ֲ����������塣
2�� �������ڲ�����switch��䡣
3�� �������ڣ�������return��䡣 ��Ϊreturn�Ѿ�������������ʱ���ض����塣�����Ƿ������������汾��һ��ǿ��Ҫ��

�����㣬�������������д����Ĳ����㡣
������������������������������������������������������
���㱼��ʹ��OS����ϵͳ�����ϵͳ�Ĵ�����Ҫ�У�

Ӳ����Դ���ۣ���RAM��ROM�����ģ���
ѧϰ���ۣ�ѧ����ԭ�������������÷�����
��ֲ���ۣ�����ͬcpu����ֲ�Ĺ���������
Ч�ʴ��ۣ�ʹ�õ���ϵͳ������Ķ���cpu��������
��ҵ���ۣ���Ȩ���ã���
�ȶ��Դ��ۣ��Ƿ�����Ǳ�ڲ��ȶ����أ���������bug���ٵ��Թ���������

���⼸����������Ӧ��СС�������Ĵ��ۣ����Ƿǳ�С�ġ�
1�� Ӳ����Դ���ۣ� ǰ����Ż��汾�Ѿ�˵�����⡣keil�£�������ram���� : 22�ֽڣ�rom����126�ֽ�.
2�� ѧϰ���ۣ� СС�������ܹ�ֻ��ʮ���д��룬���������һ���򵥵Ľ���˵��������������ʵ�Ǻܿ�ġ�������ѧϰʱ�����������ϵͳҪ�̡�
3�� ��ֲ���ۣ� ����û��ʲô��ֲ�����������ڸ���cpu,������ͨ�ԡ�
4�� Ч�ʴ��ۣ� ����һֱ��Ŭ���Ż������ŵ���Ч���Ѿ������ˡ����������л�ʱ�䣬Ӧ���ǿ�������uS����������uS����
5�� ��ҵ���ۣ� СС��������Ϊ���ʹ�ã�����֧���κη��á�
6�� �ȶ��Դ��ۣ�СС�����������Ͻ����Ǽ�������ѣ�δ�漰�κζ��ڲ��Ĵ������ջ�Ĳ��������������벻�ȶ��������أ����в������ڿ�Ԥ�����ɰѿص�ǰ���½��С�
����������������������������������������������������������������������������������������������������������������������������������������������������������������
*/
