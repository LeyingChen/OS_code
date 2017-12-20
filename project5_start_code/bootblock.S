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

#	li $4, 0xa0800200
	lui $4, 0xa080
	ori $4, $4, 0x200
	li $5, 0x200
	lw $6, os_size
	sll $6, $6, 9
	jal 0x8007b1a8
	nop
	jal 0xa0800200
	nop
	jr $31	
