    MOV             W1,             TBLPAG 
    MOV             #0X4042,        W0
    MOV             W0,             NVMCON

ORIGIN 0x10
    MOV             #0X55,          W0
    MOV             W0,             NVMKEY
    MOV             #0XAA,          W0
    MOV             W0,             NVMKEY  