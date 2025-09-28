global _start

section .text
_start:
    mov eax, 32
    mov ecx, 10
    add eax, ecx
    push rax

    ;call syscall to terminate properly
    mov eax, 60     ; sys_exit
    pop rdi         ; exit code should be 42
    syscall


