#include "common.h"
#include "scheduler.h"
#include "util.h"

uint64_t time;
char *display_str = "thread switch: cycles per do_yield = ";

void thread4(void){
	time = get_timer();
	do_yield();
	do_exit();
}

void thread5(void){
	time = get_timer() - time;
	print_str(0, 1, display_str);
	print_int(strlen(display_str)+1, 1, (int)time);
	do_exit();
}
