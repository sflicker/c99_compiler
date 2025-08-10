section .data

A: dd 1,2,42,3,4,5,6,7,8,9

section .text
global _start

_start:
            push rbp            ; stack += 8 (depth now 8)
            mov rbp, rsp
            lea rax, [rel A]    ; load address of A
            push rax            ; push address of A
            mov eax, 2          ; build index
            push rax
            pop rcx
            imul rcx, 4         ; scale index
            pop rax
            add rax, rcx        ; add base and index
            mov eax, [rax]      ; copy value from address to reg preparing for exit
            push rax            ; push to stack

            ;call syscall to terminate properly
            mov eax, 60     ; sys_exit
            pop rdi         ; exit code which in this case is actually the program results and should be 42
            syscall

