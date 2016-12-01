.data
a         :  .space	4
b         :  .space	4
__str0    :  .asciiz	"please input a num\0"
__str1    :  .asciiz	"please 1\n"
__str2    :  .asciiz	"please 2\n"
__str3    :  .asciiz	"please 3\n"
__str4    :  .asciiz	"none 123\n"
__str5    :  .asciiz	"please input a char\0"
__str6    :  .asciiz	"please b\n"
__str7    :  .asciiz	"please c\n"
__str8    :  .asciiz	"please d\n"
__str9    :  .asciiz	"none bcd\n"
.text
	j	main

main:
	move  $fp, $sp
	addi  $sp, $sp,  -52
	li  $v0, 4
	la  $a0, __str0
	syscall
	li  $v0, 5
	syscall
	add  $s2, $v0, $0
	la  $t0, a
	sw  $s2, 0($t0)
	addi  $s0, $0,  0
	la  $t0, a
	lw  $s1, 0($t0)
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
	li  $v0, 4
	la  $a0, __str4
	syscall
LABEL_0:
	li  $v0, 4
	la  $a0, __str5
	syscall
	li  $v0, 12
	syscall
	add  $s2, $v0, $0
	la  $t0, b
	sw  $s2, 0($t0)
	addi  $s0, $0,  0
	la  $t0, b
	lw  $s1, 0($t0)
	add  $s2, $s0, $s1
	addi  $sp, $sp,  -4
	sw  $s2, 0($sp)
	lw  $s0, 0($sp)
	addi  $sp, $sp,  4
	addi  $s1, $0,  98
	addi  $sp, $sp,  -4
	bne  $s0, $s1, LABEL_5
	li  $v0, 4
	la  $a0, __str6
	syscall
	j	LABEL_4
LABEL_5:
	lw  $s0, 0($sp)
	addi  $sp, $sp,  4
	addi  $s1, $0,  99
	addi  $sp, $sp,  -4
	bne  $s0, $s1, LABEL_6
	li  $v0, 4
	la  $a0, __str7
	syscall
	j	LABEL_4
LABEL_6:
	lw  $s0, 0($sp)
	addi  $sp, $sp,  4
	addi  $s1, $0,  100
	addi  $sp, $sp,  -4
	bne  $s0, $s1, LABEL_7
	li  $v0, 4
	la  $a0, __str8
	syscall
	j	LABEL_4
LABEL_7:
	nop
	li  $v0, 4
	la  $a0, __str9
	syscall
LABEL_4:
	nop
RETURN_0:
	nop
