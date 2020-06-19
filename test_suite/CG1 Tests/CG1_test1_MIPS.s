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
li $t0, 4
li $t1, 5
sne $t0, $t0, $t1
li $t1, 6
li $t2, 8
seq $t1, $t1, $t2
bnez $t0, .L0
bnez $t1, .L0
li $t0, 0
b .L1
.L0: 
li $t0, 1
.L1: 
beqz $t0, .L2_else
li $t1, 1
li $v0, 1
la $a0, 0($t1)
syscall 
li $v0, 4
la $a0, _newline_
syscall 
li $t2, 2
li $t3, 6
li $t4, 3
mulo $t3, $t3, $t4
li $t4, 97
sub $t3, $t3, $t4
add $t2, $t2, $t3
li $v0, 1
la $a0, 0($t2)
syscall 
li $v0, 4
la $a0, _newline_
syscall 
b .L3_ifElseDone
.L2_else: 
li $t3, 0
li $v0, 1
la $a0, 0($t3)
syscall 
li $v0, 4
la $a0, _newline_
syscall 
.L3_ifElseDone: 
.L4_while: 
li $t4, 8
li $t5, 3
slt $t4, $t4, $t5
beqz $t4, .L5_whileDone
li $t5, 8
li $v0, 1
la $a0, 0($t5)
syscall 
b .L4_while
.L5_whileDone: 
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
