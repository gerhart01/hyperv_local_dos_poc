
.CODE

ArchEnableVmx PROC
    push rax			
    xor rax,rax			
    mov rax,cr4
    or rax,02000h		
    mov cr4,rax
    pop rax				
    ret
ArchEnableVmx ENDP

ArchChangeCr3 PROC
    push rax
    mov rax, cr3
    mov cr3, rax
    pop rax
    ret
ArchChangeCr3 ENDP

END