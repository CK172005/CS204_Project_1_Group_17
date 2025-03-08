target: sub x4, x5, x6       
main: 
    beq x5, x6, target   
    add x1, x2, x3 
    lw x1 10 x5
    lui x5 0b10101010101010101010
    auipc x7 3
    jal x1 func1
    lui x8 0xff
andi x5, x6, 10  
func1:
