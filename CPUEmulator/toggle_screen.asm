
    // Initialize screen state: 0 = white, 1 = black
    @state
    M=0

(LOOP)
    // Wait for a key press
    @KBD
    D=M
    @LOOP
    D;JEQ

    // Toggle state: 0 -> 1, 1 -> 0
    @state
    M=!M

    // Choose black or white
    @state
    D=M
    @WHITE
    D;JEQ

    // Fill screen with black (all bits = 1)
    @SCREEN
    D=A
    @ptr
    M=D

    @8192
    D=A
    @count
    M=D

(BLACK_LOOP)
    @ptr
    A=M
    M=-1

    @ptr
    M=M+1

    @count
    MD=M-1

    @BLACK_LOOP
    D;JGT

    @WAIT_RELEASE
    0;JMP

(WHITE)
    // Fill screen with white (all bits = 0)
    @SCREEN
    D=A
    @ptr
    M=D

    @8192
    D=A
    @count
    M=D

(WHITE_LOOP)
    @ptr
    A=M
    M=0

    @ptr
    M=M+1

    @count
    MD=M-1

    @WHITE_LOOP
    D;JGT

(WAIT_RELEASE)
    // Wait until all keys are released
    @KBD
    D=M
    @WAIT_RELEASE
    D;JNE

    @LOOP
    0;JMP
