/*
   kernel.c
   the start of kernel
   */

#include "common.h"
#include "kernel.h"
#include "scheduler.h"
#include "th.h"
#include "util.h"
#include "queue.h"

#include "tasks.c"

volatile pcb_t *current_running;

queue_t ready_queue, blocked_queue;
pcb_t *ready_arr[NUM_TASKS];
pcb_t *blocked_arr[NUM_TASKS];


/*
   this function is the entry point for the kernel
   It must be the first function in the file
   */

#define PORT3f8 0xbfe48000

 void printnum(unsigned long long n)
 {
   int i,j;
   unsigned char a[40];
   unsigned long port = PORT3f8;
   i=10000;
   while(i--);

   i = 0;
   do {
   a[i] = n % 16;
   n = n / 16;
   i++;
   }while(n);

  for (j=i-1;j>=0;j--) {
   if (a[j]>=10) {
      *(unsigned char*)port = 'a' + a[j] - 10;
    }else{
	*(unsigned char*)port = '0' + a[j];
   }
  }
  printstr("\r\n");
}

void _stat(void){
	/* some scheduler queue initialize */
	struct queue ready_q, blocked_q;

	ready_queue = &ready_q;
	ready_queue->pcbs = ready_arr;
	ready_queue->capacity = NUM_TASKS;
	queue_init(ready_queue);

	blocked_queue = &blocked_q;
	blocked_queue->pcbs = blocked_arr;
	blocked_queue->capacity = NUM_TASKS;
	queue_init(blocked_queue);

	clear_screen(0, 0, 30, 24);
	
	/* Initialize the PCBs and the ready queue */
	int stackp = STACK_MIN;
	int processp;
	pcb_t pcbs[NUM_TASKS];
	pcb_t *curpro = &pcbs[0];

	ASSERT(STACK_MIN + NUM_TASKS * STACK_SIZE <= STACK_MAX);

	for(processp = 0; processp < NUM_TASKS; processp++){
		struct task_info *taskp = task[processp];
		stackp += STACK_SIZE;
		curpro->pid = processp;
		//curpro->iskernel = (taskp->task_type == KERNEL_THREAD)? 1 : 0;
		curpro->state = PROCESS_READY;
		curpro->s0 = 0;
		curpro->s1 = 0;
		curpro->s2 = 0;
		curpro->s3 = 0;
		curpro->s4 = 0;
		curpro->s5 = 0;
		curpro->s6 = 0;
		curpro->s7 = 0;
		curpro->fp = stackp;
		curpro->sp = stackp;
		curpro->ra = taskp->entry_point;

		queue_push(ready_queue, curpro);
		curpro++;
	}

	/*Schedule the first task */
	scheduler_count = 0;
	scheduler_entry();

	/*We shouldn't ever get here */
	ASSERT(0);
}
