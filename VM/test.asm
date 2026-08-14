// push constant 12
@12
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop local 5
@5
D=A
@LCL
D=D+M
@R13
M=D
@SP
AM=M-1
D=M
@R13
A=M
M=D
// push constant 10
@10
D=A
@SP
A=M
M=D
@SP
M=M+1
