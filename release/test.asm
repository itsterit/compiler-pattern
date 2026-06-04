origin 0x0000
initialization:
    mov r0, #10
    mov r1, #01

origin 0x0020
executable_loop:
    sub r0, r0, #1
    cbz r0, infinite_loop

origin 0x0040
infinite_loop:
