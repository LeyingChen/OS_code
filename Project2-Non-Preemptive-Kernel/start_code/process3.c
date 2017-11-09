#include "common.h"
#include "syslib.h"
#include "util.h"

uint64_t pro_time = 4294967295u;
//int flag = 0;
char * display_pro = "process switch: cycles per yield = ";

void _start(void)
{
	uint64_t curtime;
	curtime  = get_timer();
	/* need student add */
	if(curtime < pro_time){
		//print_str(0, 2, "first time");
		pro_time = get_timer();
		yield();
		exit();
	} else {
		print_str(0, 2, display_pro);
		print_int(strlen(display_pro)+1, 2, (int)(curtime - pro_time));
		exit(); 
	}
}
