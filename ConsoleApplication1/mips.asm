.data
small     :  .word	49
big       :  .word	17
z         :  .word	97
i         :  .space	4
j         :  .space	4
op        :  .space	4
__str0    :  .asciiz	"please input op"
__str1    :  .asciiz	"please input op"
__str2    :  .asciiz	"please input questiontimes"
__str3    :  .asciiz	"please input num"
.text
	j	main

fun:
	mov  $fp, $sp
	addi  $sp, $sp,  -4
	sw  $ra, 0($sp)
	addi  $sp, $sp,  -4
	sw  $a0, 0($sp)
	addi  $sp, $sp,  -4
	sw  $a1, 0($sp)
	addi  $sp, $sp,  -4
	sw  $a2, 0($sp)
	addi  $sp, $sp,  -4
	sw  $a3, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s0, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s1, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s2, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s3, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s4, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s5, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s6, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s7, 0($sp)
	addi  $sp, $sp,  -4
	addi  $sp, $sp,  -80
	addi  $sp, $sp,  -4
	lw  $s0, 0($fp)
	addi  $s1, $0,  26
	slt  $t0, $s0, $s1
	bne  $t0, $0, LABEL_0
	add  $a0, $0, $0
	j  RETURN_0
LABEL_0:
	lw  $s0, -140($fp)
	addi  $s1, $0,  4
	mult  $s0, $s1
	mflo  $s2
	addi  $sp, $sp,  -4
	sw  $s2, 0($sp)
	lw  $s0, 0($sp)
	addi  $sp, $sp,  4
	add  $s2, $0, $s0
	sw  $s2, -140($fp)
	lw  $t0, 0($fp)
	addi  $t1, $0,  -60
	sub  $t1, $t1,  $t0
	add  $t1, $fp,  $t1
	lw  $s0, 0($t1)
	add  $s2, $0, $s0
	addi  $sp, $sp,  -4
	sw  $s2, 0($sp)
	lw  $s0, -140($fp)
	lw  $s1, 0($sp)
	addi  $sp, $sp,  4
	mult  $s0, $s1
	mflo  $s2
	addi  $sp, $sp,  -4
	sw  $s2, 0($sp)
	lw  $s0, 0($sp)
	addi  $sp, $sp,  4
	add  $s2, $0, $s0
	lw  $t0, -140($fp)
	addi  $t1, $0,  -60
	sub  $t1, $t1,  $t0
	add  $t1, $fp,  $t1
	sw  $s2, 0($t1)
	la  $t0, op
	lw  $s0, 0($t0)
	addi  $s1, $0,  43
	bne  $s0, $s1, LABEL_2
	la  $t0, big
	lw  $s0, 0($t0)
	add  $s2, $0, $s0
	sw  $s2, -56($fp)
	j	LABEL_1
LABEL_2:
	la  $t0, op
	lw  $s0, 0($t0)
	addi  $s1, $0,  45
	bne  $s0, $s1, LABEL_3
	la  $t0, small
	lw  $s0, 0($t0)
	add  $s2, $0, $s0
	sw  $s2, -56($fp)
	j	LABEL_1
LABEL_3:
	la  $t0, op
	lw  $s0, 0($t0)
	addi  $s1, $0,  42
	bne  $s0, $s1, LABEL_4
	add  $a0, $0, $0
	j  RETURN_0
	j	LABEL_1
LABEL_4:
	nop
LABEL_1:
	addi  $s0, $0,  48
	lw  $s1, -56($fp)
	add  $s2, $s0, $s1
	addi  $sp, $sp,  -4
	sw  $s2, 0($sp)
	lw  $s0, 0($sp)
	addi  $sp, $sp,  4
	lw  $s1, 0($fp)
	add  $s2, $s0, $s1
	addi  $sp, $sp,  -4
	sw  $s2, 0($sp)
	li  $v0, 4
	la  $a0, __str0
	syscall
	lw  $s1, 0($sp)
	addi  $sp, $sp,  4
	li  $v0, 1
	add  $a0, $s1, $0
	syscall
	lw  $s0, 0($fp)
	addi  $s1, $0,  1
	slt  $t0, $s1, $s0
	beq  $t0, $0, LABEL_5
	lw  $s0, 0($fp)
	addi  $s1, $0,  1
	sub  $s2, $s0, $s1
	addi  $sp, $sp,  -4
	sw  $s2, 0($sp)
	lw  $s0, 4($fp)
	addi  $sp, $sp,  -4
	sw  $s0, 0£¨$sp£©
	lw  $s0, 0($sp)
	addi  $sp, $sp,  4
	addi  $sp, $sp,  -4
	sw  $s0, 0£¨$sp£©
	jal	fun
LABEL_5:
	add  $a0, $0, $0
	j  RETURN_0
RETURN_0:
	lw  $ra, -4($fp)
	lw  $a2, -16($fp)
	lw  $a3, -20($fp)
	lw  $s0, -24($fp)
	lw  $s1, -28($fp)
	lw  $s2, -32($fp)
	lw  $s3, -36($fp)
	lw  $s4, -40($fp)
	lw  $s5, -44($fp)
	lw  $s6, -48($fp)
	lw  $s7, -52($fp)
	lw  $sp, -8($fp)
	lw  $fp, -12($fp)
	jr  $ra

inputchar:
	mov  $fp, $sp
	addi  $sp, $sp,  -4
	sw  $ra, 0($sp)
	addi  $sp, $sp,  -4
	sw  $a0, 0($sp)
	addi  $sp, $sp,  -4
	sw  $a1, 0($sp)
	addi  $sp, $sp,  -4
	sw  $a2, 0($sp)
	addi  $sp, $sp,  -4
	sw  $a3, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s0, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s1, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s2, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s3, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s4, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s5, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s6, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s7, 0($sp)
	addi  $sp, $sp,  -4
	addi  $sp, $sp,  -4
	addi  $sp, $sp,  -4
	addi  $sp, $sp,  -4
	li  $v0, 4
	la  $a0, __str1
	syscall
	li  $v0, 5
	syscall
	add  $s2, $v0, $0
	sw  $s2, -56($fp)
	li  $v0, 5
	syscall
	add  $s2, $v0, $0
	sw  $s2, -60($fp)
	li  $v0, 5
	syscall
	add  $s2, $v0, $0
	sw  $s2, -64($fp)
	li  $v0, 5
	syscall
	add  $s2, $v0, $0
	sw  $s2, -68($fp)
	lw  $s0, -56($fp)
	addi  $s1, $0,  43
	bne  $s0, $s1, LABEL_6
	lw  $s0, -56($fp)
	add  $s2, $0, $s0
	la  $t0, op
	lw  $s0, 0($t0)
	j	LABEL_7
LABEL_6:
	lw  $s0, -56($fp)
	addi  $s1, $0,  45
	bne  $s0, $s1, LABEL_9
	lw  $s0, -56($fp)
	add  $s2, $0, $s0
	la  $t0, op
	lw  $s0, 0($t0)
	j	LABEL_8
LABEL_9:
	nop
	addi  $s0, $0,  42
	add  $s2, $0, $s0
	la  $t0, op
	lw  $s0, 0($t0)
LABEL_7:
	addi  $s0, $0,  1
	add  $a0, $s0, $0
	j  RETURN_1
RETURN_1:
	lw  $ra, -4($fp)
	lw  $a2, -16($fp)
	lw  $a3, -20($fp)
	lw  $s0, -24($fp)
	lw  $s1, -28($fp)
	lw  $s2, -32($fp)
	lw  $s3, -36($fp)
	lw  $s4, -40($fp)
	lw  $s5, -44($fp)
	lw  $s6, -48($fp)
	lw  $s7, -52($fp)
	lw  $sp, -8($fp)
	lw  $fp, -12($fp)
	jr  $ra

input:
	mov  $fp, $sp
	addi  $sp, $sp,  -4
	sw  $ra, 0($sp)
	addi  $sp, $sp,  -4
	sw  $a0, 0($sp)
	addi  $sp, $sp,  -4
	sw  $a1, 0($sp)
	addi  $sp, $sp,  -4
	sw  $a2, 0($sp)
	addi  $sp, $sp,  -4
	sw  $a3, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s0, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s1, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s2, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s3, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s4, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s5, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s6, 0($sp)
	addi  $sp, $sp,  -4
	sw  $s7, 0($sp)
	addi  $sp, $sp,  -4
	addi  $t0, $0,  1
	sw  $t0, 0($sp)
	addi  $sp, $sp,  -4
	addi  $sp, $sp,  -4
	addi  $sp, $sp,  -400
	addi  $sp, $sp,  -4
	addi  $s0, $0,  0
	add  $s2, $0, $s0
	sw  $s2, -64($fp)
	li  $v0, 4
	la  $a0, __str2
	syscall
	li  $v0, 5
	syscall
	add  $s2, $v0, $0
	sw  $s2, -60($fp)
	li  $v0, 4
	la  $a0, __str3
	syscall
	li  $v0, 5
	syscall
	add  $s2, $v0, $0
	sw  $s2, -64($fp)
	lw  $s1, -60($fp)
	li  $v0, 1
	add  $a0, $s1, $0
	syscall
LABEL_12:
	lw  $s0, -60($fp)
	addi  $s1, $0,  0
	slt  $t0, $s1, $s0
	beq  $t0, $0, LABEL_10
	jal	inputchar
	la  $t0, op
	lw  $s0, 0($t0)
	addi  $s1, $0,  42
	beq  $s0, $s1, LABEL_11
	lw  $s0, -64($fp)
	add  $a0, $s0, $0
	j  RETURN_2
LABEL_11:
	lw  $s0, -60($fp)
	addi  $s1, $0,  1
	sub  $s2, $s0, $s1
	addi  $sp, $sp,  -4
	sw  $s2, 0($sp)
	lw  $s0, 0($sp)
	addi  $sp, $sp,  4
	add  $s2, $0, $s0
	sw  $s2, -60($fp)
	j	LABEL_12
LABEL_10:
	lw  $s0, -64($fp)
	add  $a0, $s0, $0
	j  RETURN_2
RETURN_2:
	lw  $ra, -4($fp)
	lw  $a2, -16($fp)
	lw  $a3, -20($fp)
	lw  $s0, -24($fp)
	lw  $s1, -28($fp)
	lw  $s2, -32($fp)
	lw  $s3, -36($fp)
	lw  $s4, -40($fp)
	lw  $s5, -44($fp)
	lw  $s6, -48($fp)
	lw  $s7, -52($fp)
	lw  $sp, -8($fp)
	lw  $fp, -12($fp)
	jr  $ra

main:
	mov  $fp, $sp
	addi  $sp, $sp,  -4
	addi  $t0, $0,  117
	sw  $t0, 0($sp)
	addi  $sp, $sp,  -400
	addi  $sp, $sp,  -4
	jal	input
	add  $s0, $a0, $0
	add  $s2, $0, $s0
	addi  $sp, $sp,  -4
	sw  $s2, 0($sp)
	lw  $s0, 0($sp)
	addi  $sp, $sp,  4
	add  $s2, $0, $s0
	sw  $s2, -460($fp)
	lw  $s0, -280($fp)
	add  $s2, $0, $s0
	addi  $sp, $sp,  -4
	sw  $s2, 0($sp)
	lw  $s0, 0($sp)
	addi  $sp, $sp,  4
	addi  $sp, $sp,  -4
	sw  $s0, 0£¨$sp£©
	lw  $s0, -460($fp)
	addi  $sp, $sp,  -4
	sw  $s0, 0£¨$sp£©
	jal	fun
	add  $a0, $0, $0
	j  RETURN_3
RETURN_3
	nop
