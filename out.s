.data
_ret: .asciiz "\n"
_prompt: .asciiz "Enter an integer:"
.globl main
.text
read:
	li $v0,4
	la $a0,_prompt
	syscall
	li $v0,5
	syscall
	jr $ra

write:
	li $v0,1
	syscall
	li $v0,4
	la $a0,_ret
	syscall
	move $v0,$0
	jr $ra
fact:
	sw $a0,-4($fp)
	lw $t0,-4($fp)
	sw $t0,-8($fp)
	li $t0,1
	sw $t0,-12($fp)
	lw $t0,-8($fp)
	lw $t1,-12($fp)
	beq $t0,$t1,label0
	j label1
label0:
	lw $t0,-4($fp)
	sw $t0,-16($fp)
	lw $v0,-16($fp)
	jr $ra
	j label2
label1:
	lw $t0,-4($fp)
	sw $t0,-24($fp)
	lw $t0,-4($fp)
	sw $t0,-36($fp)
	li $t0,1
	sw $t0,-40($fp)
	lw $t0,-36($fp)
	lw $t1,-40($fp)
	sub $t2,$t0,$t1
	sw $t2,-32($fp)
	lw $a0,-32($fp)
	addi $sp,$sp,-4
	sw $ra,0($sp)
	addi $sp,$sp,-4
	sw $fp,0($sp)
	move $fp,$sp
	subu $sp,$sp,40
	jal fact
	move $sp,$fp
	lw $fp,0($sp)
	addi $sp,$sp,4
	lw $ra,0($sp)
	addi $sp,$sp,4
	sw $v0,-28($fp)
	lw $t0,-24($fp)
	lw $t1,-28($fp)
	mul $t2,$t0,$t1
	sw $t2,-20($fp)
	lw $v0,-20($fp)
	jr $ra
label2:
main:
	move $fp,$sp
	subu $sp,$sp,32
	addi $sp,$sp,-4
	sw $ra,0($sp)
	jal read
	lw $ra,0($sp)
	addi $sp,$sp,4
	sw $v0,-4($fp)
	lw $t0,-4($fp)
	sw $t0,-8($fp)
	lw $t0,-8($fp)
	sw $t0,-12($fp)
	li $t0,1
	sw $t0,-16($fp)
	lw $t0,-12($fp)
	lw $t1,-16($fp)
	bgt $t0,$t1,label3
	j label4
label3:
	lw $a0,-8($fp)
	addi $sp,$sp,-4
	sw $ra,0($sp)
	addi $sp,$sp,-4
	sw $fp,0($sp)
	move $fp,$sp
	subu $sp,$sp,40
	jal fact
	move $sp,$fp
	lw $fp,0($sp)
	addi $sp,$sp,4
	lw $ra,0($sp)
	addi $sp,$sp,4
	sw $v0,-20($fp)
	lw $t0,-20($fp)
	sw $t0,-24($fp)
	j label5
label4:
	li $t0,1
	sw $t0,-28($fp)
	lw $t0,-28($fp)
	sw $t0,-24($fp)
label5:
	lw $a0,-24($fp)
	addi $sp,$sp,-4
	sw $ra,0($sp)
	jal write
	lw $ra,0($sp)
	addi $sp,$sp,4
	li $t0,0
	sw $t0,-32($fp)
	lw $v0,-32($fp)
	jr $ra
