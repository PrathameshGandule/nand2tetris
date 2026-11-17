
    // Set screen pointer to SCREEN (RAM[16384])
    @SCREEN
    D=A
    @ptr
    M=D

    // There are 8192 screen words
    @8192
    D=A
    @count
    M=D

(FILL)
    @ptr
    A=M
    M=-1

    @ptr
    M=M+1

    @count
    MD=M-1

    @FILL
    D;JGT

(END)
    @END
    0;JMP
