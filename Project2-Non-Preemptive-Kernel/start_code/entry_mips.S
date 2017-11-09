#include "regs.h"
	.text
	.align	2
	.globl  get_timer	
	.ent	get_timer
	.type	get_timer, @function
get_timer:
	mfc0 $2,$9
        nop
        j       ra
        nop
	.align	2
	.globl  kernel_entry	
	.ent	kernel_entry
	.type	kernel_entry, @function
kernel_entry:
	addiu sp, sp, -24
	sw ra, 0(sp)
	bnez $4, 1f
	nop

	jal do_yield
	nop
	beqz $0,2f
	nop

1:
	jal do_exit
	nop
2:
	lw ra, 0(sp)
	addiu sp, sp, 24
	jr ra
	nop

	.align	2
	.globl  scheduler_entry	
	.ent	scheduler_entry
	.type	scheduler_entry, @function
scheduler_entry:
	# call scheduler, which will set new current process
	# need student add
	
	jal scheduler
	
	la t0, current_running
	lw t1, (t0)
	
	lw s0, 8(t1)
	lw s1, 12(t1)
	lw s2, 16(t1)
	lw s3, 20(t1)
	lw s4, 24(t1)
	lw s5, 28(t1)
	lw s6, 32(t1)
	lw s7, 36(t1)
	lw fp, 40(t1)
	lw sp, 44(t1)
	lw ra, 48(t1)

	

	.align	2
	.globl  save_pcb	
	.ent	save_pcb
	.type	save_pcb, @function
save_pcb:
	# save the pcb of the currently running process
	# need student add

	la t0, current_running
	lw t1, (t0)

	sw s0, 8(t1)
	sw s1, 12(t1)
	sw s2, 16(t1)
	sw s3, 20(t1)
	sw s4, 24(t1)
	sw s5, 28(t1)
	sw s6, 32(t1)
	sw s7, 36(t1)
	sw fp, 40(t1)

	addiu sp, sp, 24
	sw sp, 44(t1)
	addiu sp, sp, -24

	lw t2, 16(sp)
	sw t2, 48(t1)

	jr ra
