#include "mtype.h"
#include "main.h"
#include "mlib.h"
#include "task.h"

#define TASK_NUM_LIMIT  8
#define TASK_STACK_SIZE 256
#define PRESERVE_REGSET 32+2

#define TASK_READY	0x0
#define TASK_WAIT_READ	0x1
#define TASK_WAIT_WRITE	0x2

#define IT_FORK 	0x1
#define IT_GETPID 	0x2
#define IT_SYSCALL	0x8
#define IT_SYSTICK	0x9

void syscall();
int fork();
int getpid();
RegSet *to_user_mode(RegSet *stack);
void counter(int t, char *s)
{
	int i;
	while(1)
	{
		i=t;
		while(i--);
		mputs(s);
	}
}
void second_task()
{
	mputo("PID:",getpid());
	counter(1000,"ABCDEFGHIJK\n");
	while(1);
}
void init_task()
{
	mputs("THIS IS INIT_TASK\n");
	if( !fork() ) second_task();
	syscall();
	counter(1000,"abcdefghijk\n");
}
RegSet *set_task(uint *task_stack, void (*start)() )
{
	RegSet *rs= (RegSet *)(task_stack+TASK_STACK_SIZE-PRESERVE_REGSET);
	rs->state = TASK_READY;
	rs->pc = (uint)start;
	return rs;
}

int main()
{
	mputs("THIS IS MAIN\n");
	uint task_number =0;
	uint task_currnet=0;
	//mputo("TASK CURRENT ADDR: ",(int)&task_currnet);

	uint task_stack[TASK_NUM_LIMIT][TASK_STACK_SIZE];
	RegSet *rs[TASK_NUM_LIMIT];

	rs[task_number]=set_task(task_stack[task_number],&init_task);
	task_number++;

	SYSTICK_RELOAD_VALUE=5000;
	SYSTICK_CONTROL=SYSTICK_CONTROL_ENABLE|SYSTICK_CONTROL_CONTINUOUS;

	while(1)
	{
		rs[task_currnet] = to_user_mode(rs[task_currnet]);
		rs[task_currnet]->state=TASK_READY;
		switch(rs[task_currnet]->it_number)
		{
			case IT_FORK:
				if(task_number==TASK_NUM_LIMIT)
				{
					rs[task_currnet]->r0=-1;
				}
				else
				{
					uint used=(task_stack[task_currnet])+
						(TASK_STACK_SIZE)-(uint *)(rs[task_currnet]);
					rs[task_number]=(RegSet *)
						(task_stack[task_number]+(TASK_STACK_SIZE)-used);
					imemcpy(rs[task_number],rs[task_currnet],used);

					rs[task_currnet]->r0=task_number;
					rs[task_number]->r0=0;
					task_number++;
				}
				break;
			case IT_GETPID:
				rs[task_currnet]->r0=task_currnet;
				break;
			case IT_SYSCALL:
				break;
			case IT_SYSTICK:
				break;
		}
		while(TASK_READY!=rs[
			task_currnet=(task_currnet+1>=task_number)?0:task_currnet+1]->state);
	}

	//while(1);

	//int n;
	//while(n!=9)
	//{
	//	mputs("\n\n\n");
	//	mputs("PROGRAM START\n");
	//	mputs(">> 1: qsort\n");
	//	mputs(">> 2: gcd\n");
	//	mputs(">> 3: bcd\n");
	//	mputs(">> 9: go\n");
	//	n=mgetn();
	//	switch(n)
	//	{
	//		case 1: qsort_task(); break;
	//		case 2: gcd_task();   break;
	//		case 3: bcd_task();   break;
	//	}
	//}

	//while(1);
	//return 0;
}
