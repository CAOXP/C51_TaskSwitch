/*
1)  超级可以移植性，与CPU无关，几乎任何支持C语言编程的CPU都可以用！（本文仅仅以51单片机为例而已，但实际上可以任意移植）
2)  小之又小， 原理很简单，一看就懂。
3)  省之又省， 可以说对RAM和ROM省到极致。
4)  取protothread之精华，将定时器与状态机和伪线程语法融合到一个框架，任务函数可以有两种写法。
5)  基于定时器触发，调度效率高，最大化减少无效的代码运行时间。
***********************************************************/  

#include <STC12C2052AD.H>
#include <stdio.h>

/****小小调度器开始**********************************************/
#define MAXTASKS 3
volatile unsigned char timers[MAXTASKS];
#define _SS static unsigned char _lc=0; switch(_lc){default: 
#define _EE ;}; _lc=0; return 255;
#define WaitX(tickets)  do {_lc=(__LINE__+((__LINE__%256)==0))%256; return tickets ;} while(0); case (__LINE__+((__LINE__%256)==0))%256: 

#define RunTask(TaskName,TaskID) do { if (timers[TaskID]==0) timers[TaskID]=TaskName(); }  while(0); 
#define RunTaskA(TaskName,TaskID) { if (timers[TaskID]==0) {timers[TaskID]=TaskName(); continue;} }   //前面的任务优先保证执行

#define CallSub(SubTaskName) do {unsigned char currdt; _lc=(__LINE__+((__LINE__%256)==0))%256; return 0; case (__LINE__+((__LINE__%256)==0))%256:  currdt=SubTaskName(); if(currdt!=255) return currdt;} while(0);
#define InitTasks() {unsigned char i; for(i=MAXTASKS;i>0 ;i--) timers[i-1]=0; }
#define UpdateTimers() {unsigned char i; for(i=MAXTASKS;i>0 ;i--){if((timers[i-1]!=0)&&(timers[i-1]!=255)) timers[i-1]--;}}

#define SEM unsigned int 
//初始化信号量
#define InitSem(sem) sem=0;
//等待信号量
#define WaitSem(sem) do{ sem=1; WaitX(0); if (sem>0) return 1;} while(0);
//等待信号量或定时器溢出， 定时器tickets 最大为0xFFFE
#define WaitSemX(sem,tickets)  do { sem=tickets+1; WaitX(0); if(sem>1){ sem--;  return 1;} } while(0);
//发送信号量
#define SendSem(sem)  do {sem=0;} while(0);

/*****小小调度器结束*******************************************************/


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
		以下需要根据不同的芯片进行不同的配置
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
    TL0=0Xff;   //10ms 重装
    TH0=0XDB;	//b7;    

    UpdateTimers();

    RunTask(task0,0);//任务0具有精确按时获得执行的权限，要求：task0每次执行消耗时间<0.5个 ticket
}


void main()
{
	InitT0();
	InitTasks(); //初始化任务，实际上是给timers清零
	while(1)
	{
		//RunTask(task0,0);
		RunTaskA(task1,1);//任务1具有比任务2高的运行权限                   
		RunTaskA(task2,2);//任务2具有低的运行权限     
		/*
			以上任务带优先级，1.优先级不得重复。
							  2.数值不得高于任务数量
		*/              
	}
}




/*

－－－－－－－－－－－－－－－－－－以下为说明－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－

小小调度器任务函数的写法主要注意的，主要有三点：

1） 任务函数内部变量，建议都用静态局部变量来定义。
2） 任务函数内不能用switch语句。
3） 任务函数内，不能用return语句。 因为return已经被赋予任务延时的特定意义。（这是返回型任务函数版本的一个强制要求）

这三点，并不会明显造成写程序的不方便。
－－－－－－－－－－－－－－－－－－－－－－－－－－－
从裸奔到使用OS操作系统或调度系统的代价主要有：

硬件资源代价（对RAM和ROM的消耗），
学习代价（学会其原理，并掌握其用法），
移植代价（往不同cpu上移植的工作量），
效率代价（使用调度系统后带来的额外cpu负担），
商业代价（版权费用），
稳定性代价（是否引入潜在不稳定因素，或者增大bug跟踪调试工作量）。

从这几方面来讲，应用小小调度器的代价，都是非常小的。
1） 硬件资源代价： 前面的优化版本已经说明问题。keil下，本例程ram消耗 : 22字节，rom消耗126字节.
2） 学习代价： 小小调度器总共只有十多行代码，如果我们做一个简单的解释说明，理解起来其实是很快的。我相信学习时间比其他调度系统要短。
3） 移植代价： 几乎没有什么移植工作量，对于各种cpu,几乎是通吃。
4） 效率代价： 我们一直在努力优化，相信调度效率已经不低了。比如任务切换时间，应该是可以做到uS级别，甚至亚uS级别。
5） 商业代价： 小小本调度器为免费使用，无需支付任何费用。
6） 稳定性代价：小小调度器本质上仅仅是几个宏而已，未涉及任何对内部寄存器或堆栈的操作，避免了引入不稳定风险因素，所有操作都在可预见，可把控的前提下进行。
－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－－
*/

