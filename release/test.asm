

;******************************************************************************************************************************	
;* @details     Erase one page is mean erase 512 instruction is 1024 bytes (because 2 bytes per instruction)
;* @warning     Interrupts are had to been turn off
;* @param[in]   address : (W0 is lower address, W1 is upper address)
;******************************************************************************************************************************
_ErasePage:	
; loop, blocking until last nvm operation is complete (wr is clear)
    BTSC            NVMCON,         #15         
    BRA             _ErasePage
; load the destination program address
    PUSH            TBLPAG
    MOV             W1,             TBLPAG 
    TBLWTL          W0,             [W0]		 
; set up nvmcon to erase one page of program memory
    MOV             #0X4042,        W0
    MOV             W0,             NVMCON
; write the key sequence
    DISI            #6
    MOV             #0X55,          W0
    MOV             W0,             NVMKEY
    MOV             #0XAA,          W0
    MOV             W0,             NVMKEY  
; start the erase operation
    BSET            NVMCON,         #15
    NOP				                        
    NOP
; operation complete wait 
ERASING_WAIT_LB:
    BTSC            NVMCON,         #15	
    BRA             ERASING_WAIT_LB
; default return true. if error bit set, return false
    MOV             #1,             W0    
    BTSC            NVMCON,         #13    
    MOV             #0,             W0   
    POP             TBLPAG
    RETURN