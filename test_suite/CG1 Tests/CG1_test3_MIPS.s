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
li $v0, 4
la $a0, _newline_
syscall 
li $t0, 6
li $t1, 7
li $t2, 5
add $t1, $t1, $t2
sub $t0, $t0, $t1
li $v0, 1
la $a0, 0($t0)
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
