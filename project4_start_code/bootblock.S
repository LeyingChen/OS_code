	.data
		os_size: .word 1
	.text
	.globl main

main:
	# check the offset of main
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	#need add code
	#read kernel
	#move the parameters
	li $4, 0xa0800200
	li $5, 0x200
	#li $8, 0xa0800002
	lw $6, os_size
	sll $6, $6, 9
	#jump to read_disk_functions 0x8007b1a8 
	jal 0x8007b1a8
	#jump to kernel 0xa080026c
	jal 0xa0800200
	jr $31
