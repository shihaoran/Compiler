.data
__str0    :  .asciiz	"please input a num"
__str1    :  .asciiz	"please 1"
__str2    :  .asciiz	"please 2"
__str3    :  .asciiz	"please 3"
.text
	j	main

main:
	move  $fp, $sp
	addi  $sp, $sp,  -52
	addi  $sp, $sp,  -4
	li  $v0, 4
	la  $a0, __str0
	syscall
	li  $v0, 5
	syscall
	add  $s2, $v0, $0
	sw  $s2, -56($fp)
	addi  $s0, $0,  2
	lw  $s1, -56($fp)
	add  $s2, $s0, $s1
	addi  $sp, $sp,  -4
	sw  $s2, 0($sp)
	lw  $s0, 0($sp)
	addi  $sp, $sp,  4
	addi  $s1, $0,  1
	addi  $sp, $sp,  -4
	bne  $s0, $s1, LABEL_1
	li  $v0, 4
	la  $a0, __str1
	syscall
	j	LABEL_0
LABEL_1:
	lw  $s0, 0($sp)
	addi  $sp, $sp,  4
	addi  $s1, $0,  2
	addi  $sp, $sp,  -4
	bne  $s0, $s1, LABEL_2
	li  $v0, 4
	la  $a0, __str2
	syscall
	j	LABEL_0
LABEL_2:
	lw  $s0, 0($sp)
	addi  $sp, $sp,  4
	addi  $s1, $0,  3
	addi  $sp, $sp,  -4
	bne  $s0, $s1, LABEL_3
	li  $v0, 4
	la  $a0, __str3
	syscall
	j	LABEL_0
LABEL_3:
	nop
LABEL_0:
	nop
RETURN_0:
	nop
