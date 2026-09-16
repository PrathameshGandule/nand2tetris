// push constant 10
@10
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop local 0
@0
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
// push constant 21
@21
D=A
@SP
A=M
M=D
@SP
M=M+1
// push constant 22
@22
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop argument 2
@2
D=A
@ARG
D=D+M
@R13
M=D
@SP
AM=M-1
D=M
@R13
A=M
M=D
// pop argument 8
@8
D=A
@ARG
D=D+M
@R13
M=D
@SP
AM=M-1
D=M
@R13
A=M
M=D
// add
@SP
AM=M-1
D=M
A=A-1
M=D+M
// sub
@SP
AM=M-1
D=M
A=A-1
M=M-D
// push constant 36
@36
D=A
@SP
A=M
M=D
@SP
M=M+1
// pop this 8
@8
D=A
@THIS
D=D+M
@R13
M=D
@SP
AM=M-1
D=M
@R13
A=M
M=D
// label LOOP
(LOOP)
// function mult 2
(mult)
@SP
A=M
M=0
@SP
M=M+1
@SP
A=M
M=0
@SP
M=M+1
// label LOOP
(mult$LOOP)
