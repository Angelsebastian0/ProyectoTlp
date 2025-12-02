.text
.globl _GetThreadId@4
_GetThreadId@4:
    jmp _GetCurrentThreadId@0

.section .idata$5
.globl __imp__GetThreadId@4
__imp__GetThreadId@4:
    .long _GetThreadId@4
