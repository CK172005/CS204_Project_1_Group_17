.data 
n: .word 10
.text
addi x5 x0 0x100
lw x5 0 x5
addi x6 x0 1
jal x1 fibbo
jal x0 exit
fibbo:
addi x2 x2 -8
sw x5 4 x2
sw x1 0 x2
beq x5 x0 base_case
beq x5 x6 base_case1
addi x5 x5 -1
jal x1 fibbo
lw x5 4 x2
addi x2 x2 -4
sw x7 0 x2
addi x5 x5 -2
jal x1 fibbo
lw x8 0 x2
addi x2 x2 4
add x7 x7 x8
lw x1 0 x2
addi x2 x2 8
jalr x0 x1 0
base_case:
addi x7 x0 0
addi x2 x2 8
jalr x0 x1 0
base_case1:
addi x7 x0 1
addi x2 x2 8
jalr x0 x1 0
exit:
