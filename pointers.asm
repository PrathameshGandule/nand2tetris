// Program: PointerDemo.asm
// Starting at the address stored in R0,
// sets the first R1 words to –1
// i = 0
@i
M=0
(LOOP)
// if (i == R1) goto END
@i
D=M
@R1
D=D-M
@END
D;JEQ
// RAM[R0 + i] = -1
@R0
D=M
@i	
A=D+M
M=-1
// i = i + 1
@i
M=M+1
// goto LOOP
@LOOP
0;JMP
(END)
@END
0;JMP