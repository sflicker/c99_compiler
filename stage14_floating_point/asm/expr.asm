global _start

; calculates 2+3*4 => 14

section .text
_start:
    mov eax, 2                ; load 2 and push
    push rax
    mov eax, 3                ; load 3 and push
    push rax
    mov eax, 4                ; load 4 and push
    push rax                  ; pop two (4 & 3)
    pop rcx
    pop rax
    imul eax, ecx             ; multiply 3*4 and push
    push rax
    pop rcx                   ; pop two (12 and 2)
    pop rax
    add eax, ecx              ; add and push
    push rax

    ;call syscall to terminate properly
    mov eax, 60     ; sys_exit
    pop rdi         ; exit code should be 14
    syscall
