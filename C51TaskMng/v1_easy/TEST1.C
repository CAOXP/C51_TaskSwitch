/*
1)  超级可以移植性，与CPU无关，几乎任何支持C语言编程的CPU都可以用！（本文仅仅以51单片机为例而已，但实际上可以任意移植）
2)  小之又小， 原理很简单，一看就懂。
3)  省之又省， 可以说对RAM和ROM省到极致。
4)  取protothread之精华，将定时器与状态机和伪线程语法融合到一个框架，任务函数可以有两种写法。
5)  基于定时器触发，调度效率高，最大化减少无效的代码运行时间。
***********************************************************/  
#include <STC12C2052AD.H>
#include <stdio.h>


/*****************小小调度器部分开始********************************************/ 
#define  _SS   static char lc=0; switch(lc){   case 0: lc=0;
#define  _EE   }; lc=0; 
#define  WaitX(a,b)  settimer(&lc,__LINE__,a,b); return ; case __LINE__:
struct TASK {
  char td;
  void (*fp)();
};
#define MAXTASKS 5
struct TASK tasks[MAXTASKS];

//设置定时器
void settimer(char *lc,char  line,char  tmrid,int d)
{
  *lc=line;
  tasks[tmrid].td=d;
}

//逻辑定时器处理，在定时器中断里调用
void dectimers() 
{ 
	unsigned char i;   
	for (i=0;i<MAXTASKS;i++){ 
	   if (tasks[i].td>0)  tasks[i].td--;  
	}
}
//任务调度函数，在main里面运行
void runtasks() 
{
	unsigned char i;    
	for(i=0;i<MAXTASKS;i++)        
	{   
		if (tasks[i].fp!=0)
		{    
			if (tasks[i].td==0)
			{
				tasks[i].td=-1;  
				tasks[i].fp();
			}  
		}         
	}
}
/****************小小调度器部分结束*******************************************************/


sbit KEY = P3^2;
unsigned char code numtab[16]={0x24,0x6F,0xE0,0x62,0x2B,0x32,0x30,0x67,0x20,0x22,0x21,0x38,0xB4,0x68,0xB0,0xB1};


//初始化定时器
void InitT0()
{
        TMOD = 0x21;
        IE |= 0x82;  // 12t
        TL0=0Xff;
        TH0=0Xb7;
        TR0 = 1;
}
//定时器中断
void INTT0(void) interrupt 1 using 1
{
        TL0=0Xff;    //10ms 重装
        TH0=0Xb7;
        dectimers();
}



//任务一，状态机写法
sbit LED1= P2^4;  
void ontimer0()
{ 
  LED1=!LED1;  // LED1引脚接在发光管负极，LED1=0 为亮，LED1=1为灭。

  //重装定时器
  if (LED1) tasks[0].td=45;  //450mS 灭
  else tasks[0].td=5;  //50ms  亮
}

//任务二，状态机写法
char keycount=0;
void task1()
{
	if(KEY==0) 
	{
	   keycount++;
	}
	else
	{
	    keycount=0;
	}
	//重装定时器
	tasks[1].td=5;
}


//任务三，伪线程写法
void  task2()
{
	static char i;
	_SS
	
	while(1){
	
	for(i=0;i<=9;i++){   //从0--9快速显示，间隔200mS
	   WaitX(2,20);         //    等待200mS,实际是设置定时器2为200mS
	   P1=numtab[i];
	}
	for(i=0;i<=9;i++){ //从0--9慢速显示，间隔500mS
	   WaitX(2,50);       //    等待500mS,实际是设置定时器2为500mS
	   P1=numtab[i];
	}
	}
	
	_EE
}


const char * pte=23;
char * const pdt=23;


void main()
{
        unsigned char         KeyNum;

        P1 = 0xff;         //关显示

        InitT0();

        KEY =1;                                //按键IO口
        KeyNum=0;                        //按下次数

    //装载任务:
        tasks[0].fp=ontimer0; 
        tasks[1].fp=task1; 
        tasks[2].fp=task2; 

    //循环调度
        while(1)
		{
         runtasks();
        // clr_wdt();
        }
}