.data 
_newline_: 
.asciiz "\n" 
.text 
.globl main 
main: 
addiu $sp, $sp, -40
sw $ra, 36($sp)
sw $fp, 32($sp)
sw $s0, 28($sp)
sw $s1, 24($sp)
sw $s2, 20($sp)
sw $s3, 16($sp)
sw $s4, 12($sp)
sw $s5, 8($sp)
sw $s6, 4($sp)
sw $s7, 0($sp)
addiu $fp, $sp, 40
li $t0, 98
li $t1, 50
sle $t0, $t0, $t1
li $t1, 1
li $t2, 1
seq $t1, $t1, $t2
beqz $t0, .L0
beqz $t1, .L0
li $t0, 1
b .L1
.L0: 
li $t0, 0
.L1: 
beqz $t0, .L2_else
li $t1, 1
li $v0, 1
la $a0, 0($t1)
syscall 
b .L3_ifElseDone
.L2_else: 
li $t2, 98
li $v0, 1
la $a0, 0($t2)
syscall 
.L3_ifElseDone: 
li $v0, 4
la $a0, _newline_
syscall 
li $t3, 1
li $t4, 2
li $t5, 3
li $t6, 4
li $t7, 5
addiu $sp, $sp, -4
sw $t0, 0($sp)
li $t0, 6
addiu $sp, $sp, -4
sw $t1, 0($sp)
li $t1, 7
addiu $sp, $sp, -4
sw $t2, 0($sp)
li $t2, 8
addiu $sp, $sp, -4
sw $t3, 0($sp)
li $t3, 9
addiu $sp, $sp, -4
sw $t4, 0($sp)
li $t4, 10
add $t3, $t3, $t4
lw $t4, 0($sp)
addiu $sp, $sp, 4
add $t2, $t2, $t3
lw $t3, 0($sp)
addiu $sp, $sp, 4
add $t1, $t1, $t2
lw $t2, 0($sp)
addiu $sp, $sp, 4
add $t0, $t0, $t1
lw $t1, 0($sp)
addiu $sp, $sp, 4
add $t7, $t7, $t0
lw $t0, 0($sp)
addiu $sp, $sp, 4
add $t6, $t6, $t7
add $t5, $t5, $t6
add $t4, $t4, $t5
add $t3, $t3, $t4
li $v0, 1
la $a0, 0($t3)
syscall 
li $v0, 4
la $a0, _newline_
syscall 
lw $s7, 0($sp)
lw $s6, 4($sp)
lw $s5, 8($sp)
lw $s4, 12($sp)
lw $s3, 16($sp)
lw $s2, 20($sp)
lw $s1, 24($sp)
lw $s0, 28($sp)
lw $fp, 32($sp)
lw $ra, 36($sp)
addiu $sp, $sp, 40
jr $ra
