/* Author(s): <Your name here>
 * COS 318, Fall 2013: Project 3 Pre-emptive Scheduler
 * Implementation of the process scheduler for the kernel.
 */

#include "common.h"
#include "interrupt.h"
#include "queue.h"
#include "printf.h"
#include "scheduler.h"
#include "util.h"
#include "syslib.h"

#define NON_PRI 0

pcb_t *current_running;
node_t ready_queue;
node_t sleep_wait_queue;
// more variables...
volatile uint64_t time_elapsed;

void debug_print(){
    printf(10, 1, "qwq");
}

/* wake up sleeping processes whose deadlines have passed */
void check_sleeping(){
    pcb_t *sleeping_task;
    uint64_t current_time = get_timer();
  
    while (!is_empty(&sleep_wait_queue)) {
        sleeping_task = (pcb_t *)peek(&sleep_wait_queue);
        if(sleeping_task->deadline <= current_time * 1000){
            sleeping_task->status = READY;
            enqueue(&ready_queue, dequeue(&sleep_wait_queue));
        } else {
            return;
        }
    }
}

/* Round-robin scheduling: Save current_running before preempting */
void put_current_running(){
    current_running->status = READY;
    enqueue(&ready_queue, (node_t*)current_running);
}

/* Change current_running to the next task */
void scheduler(){
     ASSERT(disable_count);
     check_sleeping(); // wake up sleeping processes
     while (is_empty(&ready_queue)){
          leave_critical();
          enter_critical();
          check_sleeping();
     }
     if(NON_PRI){
          current_running = (pcb_t *) dequeue(&ready_queue);
     } else {
          node_t *item;
          pcb_t* temp;
          temp = (pcb_t*)peek(&ready_queue);
          for(item = peek(&ready_queue); item && item!=&ready_queue; item = item->next){
               if(temp->priority < ((pcb_t *)item)->priority)
                    temp = (pcb_t *)item;
          }
          temp->node.prev->next = temp->node.next;
          temp->node.next->prev = temp->node.prev;
          temp->node.next = NULL;
          temp->node.prev = NULL;
          current_running = temp;
          if(do_getpriority() > 10)
               current_running->priority -= 10;
          else
               do_setpriority(10*(5-current_running->pid));
     }
     //current_running->status = RUNNING;
     ASSERT(NULL != current_running);
     ++current_running->entry_count;
}

int lte_deadline(node_t *a, node_t *b) {
     pcb_t *x = (pcb_t *)a;
     pcb_t *y = (pcb_t *)b;

     if (x->deadline <= y->deadline) {
          return 1;
     } else {
          return 0;
     }
}

void do_sleep(int milliseconds){
     ASSERT(!disable_count);

     enter_critical();

     uint64_t deadline;
     deadline = time_elapsed*1000 + milliseconds;
     current_running->status = SLEEPING;
     current_running->deadline = deadline;
     
     enqueue_sort(&sleep_wait_queue, (node_t *)current_running, (node_lte)&lte_deadline);
     //sort needed
     scheduler_entry();
//     leave_critical();
}

void do_yield(){
     enter_critical();
     //printf(10, 1 ,"do_yield");
     put_current_running();
     scheduler_entry();
}

void do_exit(){
     enter_critical();
     current_running->status = EXITED;
     scheduler_entry();
     /* No need for leave_critical() since scheduler_entry() never returns */
}

void block(node_t * wait_queue){
     ASSERT(disable_count);
     current_running->status = BLOCKED;
     enqueue(wait_queue, (node_t *) current_running);
     scheduler_entry();
     enter_critical();
}

void unblock(pcb_t * task){
     ASSERT(disable_count);
     task->status = READY;
     enqueue(&ready_queue, (node_t *) task);
}

pid_t do_getpid(){
     pid_t pid;
     enter_critical();
     pid = current_running->pid;
     leave_critical();
     return pid;
}

uint64_t do_gettimeofday(void){
     return time_elapsed;
}

priority_t do_getpriority(){
     return current_running->priority;
}


void do_setpriority(priority_t priority){
     current_running->priority = priority;
}

uint64_t get_timer(void) {
     return do_gettimeofday();
}
