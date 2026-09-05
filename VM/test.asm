// push local 2
@2
D=A
@LCL
A=D+A
D=M
@SP
A=M
M=D
@SP
M=M+1
// push static 5
@_test.5
D=M
@SP
A=M
M=D
@SP
M=M+1
// push static 6
@_test.6
D=M
@SP
A=M
M=D
@SP
M=M+1
// push static 9
@_test.9
D=M
@SP
A=M
M=D
@SP
M=M+1
// push pointer 1
@THAT
D=M
@SP
A=M
M=D
@SP
M=M+1
// pop argument 89
@89
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
// eq
@SP
AM=M-1
D=M
A=A-1
D=M-D
@_test_EQ_TRUE_0
D;JEQ
@SP
A=M-1
M=0
@_test_EQ_END_0
0;JMP
(_test_EQ_TRUE_0)
@SP
A=M-1
M=-1
(_test_EQ_END_0)
// eq
@SP
AM=M-1
D=M
A=A-1
D=M-D
@_test_EQ_TRUE_1
D;JEQ
@SP
A=M-1
M=0
@_test_EQ_END_1
0;JMP
(_test_EQ_TRUE_1)
@SP
A=M-1
M=-1
(_test_EQ_END_1)
// gt
@SP
AM=M-1
D=M
A=A-1
D=M-D
@_test_GT_TRUE_2
D;JGT
@SP
A=M-1
M=0
@_test_GT_END_2
0;JMP
(_test_GT_TRUE_2)
@SP
A=M-1
M=-1
(_test_GT_END_2)
// neg
@SP
A=M-1
M=-M
// label LOOP
(LOOP)
// goto LOOP
@LOOP
0;JMP
// push static 8
@_test.8
D=M
@SP
A=M
M=D
@SP
M=M+1
// if-goto LOOP
@SP
AM=M-1
D=M
@LOOP
D;JNE
// push constant 5
@5
D=A
@SP
A=M
M=D
@SP
M=M+1
