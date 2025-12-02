#include <windows.h>

// Stubs para funciones de Vista+ que no existen en XP
// Estas funciones son llamadas por libstdc++ pero podemos dar implementaciones vacías
// ya que nuestro código no usa threading real

VOID WINAPI InitializeConditionVariable(PCONDITION_VARIABLE cv) {
    if (cv) cv->Ptr = NULL;
}

VOID WINAPI WakeConditionVariable(PCONDITION_VARIABLE cv) {
    (void)cv;
}

VOID WINAPI WakeAllConditionVariable(PCONDITION_VARIABLE cv) {
    (void)cv;
}

BOOL WINAPI SleepConditionVariableCS(PCONDITION_VARIABLE cv, PCRITICAL_SECTION cs, DWORD ms) {
    (void)cv;
    (void)cs;
    Sleep(ms == INFINITE ? 1 : ms);
    return TRUE;
}

VOID WINAPI InitializeSRWLock(PSRWLOCK lock) {
    if (lock) lock->Ptr = NULL;
}

VOID WINAPI AcquireSRWLockExclusive(PSRWLOCK lock) {
    (void)lock;
}

VOID WINAPI ReleaseSRWLockExclusive(PSRWLOCK lock) {
    (void)lock;
}

VOID WINAPI AcquireSRWLockShared(PSRWLOCK lock) {
    (void)lock;
}

VOID WINAPI ReleaseSRWLockShared(PSRWLOCK lock) {
    (void)lock;
}

DWORD WINAPI GetThreadId(HANDLE hThread) {
    (void)hThread;
    return GetCurrentThreadId();
}
