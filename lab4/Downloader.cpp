#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstdio>

bool isPrime(int n) {
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6)
        if (n % i == 0 || n % (i + 2) == 0) return false;
    return true;
}

int countPrimes() {
    int cnt = 0;
    for (int i = 2; i <= 10000; ++i)
        if (isPrime(i)) ++cnt;
    return cnt;
}

int main(int argc, char* argv[]) {
    srand((unsigned)time(nullptr) ^ GetCurrentProcessId());

    int fileId = (argc > 1) ? atoi(argv[1]) : 1;
    char fileName[64];
    sprintf(fileName, "file_%03d.dat", fileId);

    HANDLE hSem = OpenSemaphoreA(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, FALSE, "DownloadSlots");
    HANDLE hMutex = OpenMutexA(SYNCHRONIZE | MUTEX_MODIFY_STATE, FALSE, "LogAccessMutex");
    HANDLE hEvent = OpenEventA(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, "BrowserClosingEvent");

    if (!hSem || !hMutex || !hEvent) {
        std::cout << "[PID: " << GetCurrentProcessId() << "] Error connecting to browser\n";
        return 1;
    }

    DWORD pid = GetCurrentProcessId();
    bool slotTaken = false;
    HANDLE waits[2] = { hSem, hEvent };

    DWORD result = WaitForMultipleObjects(2, waits, FALSE, INFINITE);
    if (result == WAIT_OBJECT_0 + 1) {
        WaitForSingleObject(hMutex, INFINITE);
        std::cout << "[PID: " << pid << "] Download of '" << fileName << "' interrupted by browser closing.\n";
        ReleaseMutex(hMutex);
        goto end;
    }

    slotTaken = true;

    WaitForSingleObject(hMutex, INFINITE);
    std::cout << "[PID: " << pid << "] Connection established. Starting download of '" << fileName << "'...\n";
    ReleaseMutex(hMutex);

    int totalSleep = 1000 + rand() % 2001;
    int interval = 100;
    bool interrupted = false;

    while (totalSleep > 0 && !interrupted) {
        DWORD waitRes = WaitForSingleObject(hEvent, min(interval, totalSleep));
        if (waitRes == WAIT_OBJECT_0) {
            interrupted = true;
        }
        else {
            totalSleep -= interval;
        }
    }

    if (interrupted) {
        WaitForSingleObject(hMutex, INFINITE);
        std::cout << "[PID: " << pid << "] Download of '" << fileName << "' interrupted by browser closing.\n";
        ReleaseMutex(hMutex);
        goto end;
    }

    int primes = countPrimes();

    WaitForSingleObject(hMutex, INFINITE);
    std::cout << "[PID: " << pid << "] File '" << fileName << "' processed successfully. "
        << "Found " << primes << " prime numbers (1..10000).\n";
    ReleaseMutex(hMutex);

    ReleaseSemaphore(hSem, 1, nullptr);

end:
    CloseHandle(hSem);
    CloseHandle(hMutex);
    CloseHandle(hEvent);
    return 0;
}