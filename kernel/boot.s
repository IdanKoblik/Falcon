section .multiboot_header
header_start:
   dd 0xe85250d6 ; Magic number (multiboot2)
   dd 0 ; Arch 0
   dd header_end - header_start ; Header len

   ; Checksum
   dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

   ; End tag
   dw 0
   dw 0
   dd 8
header_end:

section .text
[bits 32]
global _start

_start:
    mov esp, stack_top

    push ebx
    push eax

    extern kmain
    call kmain
.halt:
    hlt
    jmp .halt

section .bss
align 16
stack_bottom:
    resb 16384  ; 16 KB stack
stack_top:
