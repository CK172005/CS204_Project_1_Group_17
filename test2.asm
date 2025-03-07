.data
a1: .byte 16
name: .asciz "lauda"
arr: .word -10 0x1 0b101 11
.text
addi a0 x0 0x100
slli a0 a0 20
addi a1 a0 1
addi a2 a1 1
.data
num3: .half 33