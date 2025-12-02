.text

.globl _GetThreadId@4
_GetThreadId@4:
    jmp _GetCurrentThreadId@0

.globl _InitializeConditionVariable@4
_InitializeConditionVariable@4:
    ret $4

.globl _WakeConditionVariable@4
_WakeConditionVariable@4:
    ret $4

.globl _WakeAllConditionVariable@4
_WakeAllConditionVariable@4:
    ret $4

.globl _SleepConditionVariableCS@12
_SleepConditionVariableCS@12:
    movl $1, %eax
    ret $12

.globl _InitializeSRWLock@4
_InitializeSRWLock@4:
    ret $4

.globl _AcquireSRWLockExclusive@4
_AcquireSRWLockExclusive@4:
    ret $4

.globl _ReleaseSRWLockExclusive@4
_ReleaseSRWLockExclusive@4:
    ret $4

.section .idata$5,"w"
.globl __imp__GetThreadId@4
__imp__GetThreadId@4: .long _GetThreadId@4

.globl __imp__InitializeConditionVariable@4
__imp__InitializeConditionVariable@4: .long _InitializeConditionVariable@4

.globl __imp__WakeConditionVariable@4
__imp__WakeConditionVariable@4: .long _WakeConditionVariable@4

.globl __imp__WakeAllConditionVariable@4
__imp__WakeAllConditionVariable@4: .long _WakeAllConditionVariable@4

.globl __imp__SleepConditionVariableCS@12
__imp__SleepConditionVariableCS@12: .long _SleepConditionVariableCS@12

.globl __imp__InitializeSRWLock@4
__imp__InitializeSRWLock@4: .long _InitializeSRWLock@4

.globl __imp__AcquireSRWLockExclusive@4
__imp__AcquireSRWLockExclusive@4: .long _AcquireSRWLockExclusive@4

.globl __imp__ReleaseSRWLockExclusive@4
__imp__ReleaseSRWLockExclusive@4: .long _ReleaseSRWLockExclusive@4
