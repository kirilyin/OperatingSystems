#include <windows.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

struct ThreadParams {
    int num;
    int size;
    int* arr;
    CRITICAL_SECTION* cs;
    HANDLE startEvent;
    HANDLE continueEvent;
    HANDLE blockEvent;
    HANDLE terminateEvent;
};

DWORD WINAPI markerThread(LPVOID param) {
    ThreadParams* p = static_cast<ThreadParams*>(param);
    WaitForSingleObject(p->startEvent, INFINITE);

    srand(p->num);

    int count = 0;

    while (true) {
        bool blocked = false;
        int blockIndex = -1;

        while (!blocked) {
            int index = rand() % p->size;

            EnterCriticalSection(p->cs);
            if (p->arr[index] == 0) {
                Sleep(5);
                p->arr[index] = p->num;
                Sleep(5);
                count++;
            }
            else {
                blocked = true;
                blockIndex = index;
            }
            LeaveCriticalSection(p->cs);
        }

        std::cout << "Marker " << p->num << " marked " << count << " blocked at " << blockIndex << std::endl;
        SetEvent(p->blockEvent);

        HANDLE waits[2] = { p->continueEvent, p->terminateEvent };
        DWORD res = WaitForMultipleObjects(2, waits, FALSE, INFINITE);

        if (res == WAIT_OBJECT_0 + 1) { 
            EnterCriticalSection(p->cs);
            for (int i = 0; i < p->size; ++i) {
                if (p->arr[i] == p->num) {
                    p->arr[i] = 0;
                }
            }
            LeaveCriticalSection(p->cs);
            delete p;
            return 0;
        }
    }

    return 0;
}

int main() {
    int size;
    std::cout << "Enter array size: ";
    std::cin >> size;

    int* arr = new int[size];
    for (int i = 0; i < size; ++i) {
        arr[i] = 0;
    }

    int N;
    std::cout << "Enter number of marker threads: ";
    std::cin >> N;

    CRITICAL_SECTION cs;
    InitializeCriticalSection(&cs);

    HANDLE startEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    HANDLE* continueEvents = new HANDLE[N];
    HANDLE* blockEvents = new HANDLE[N];
    HANDLE* terminateEvents = new HANDLE[N];
    HANDLE* threads = new HANDLE[N];

    std::vector<int> active;

    for (int i = 0; i < N; ++i) {
        continueEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        blockEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        terminateEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        ThreadParams* p = new ThreadParams{
            i + 1,
            size,
            arr,
            &cs,
            startEvent,
            continueEvents[i],
            blockEvents[i],
            terminateEvents[i]
        };
        threads[i] = CreateThread(NULL, 0, markerThread, p, 0, NULL);
        active.push_back(i);
    }

    SetEvent(startEvent);

    while (!active.empty()) {
        DWORD n_active = static_cast<DWORD>(active.size());
        HANDLE* waitBlocks = new HANDLE[n_active];
        for (DWORD j = 0; j < n_active; ++j) {
            waitBlocks[j] = blockEvents[active[j]];
        }
        WaitForMultipleObjects(n_active, waitBlocks, TRUE, INFINITE);
        delete[] waitBlocks;

        std::cout << "Current array: ";
        for (int i = 0; i < size; ++i) {
            std::cout << arr[i] << " ";
        }
        std::cout << std::endl;

        int k;
        std::cout << "Enter marker number to terminate: ";
        std::cin >> k;
        int indexTerminate = k - 1;

        SetEvent(terminateEvents[indexTerminate]);

        WaitForSingleObject(threads[indexTerminate], INFINITE);

        active.erase(std::remove(active.begin(), active.end(), indexTerminate), active.end());

        std::cout << "Array after termination: ";
        for (int i = 0; i < size; ++i) {
            std::cout << arr[i] << " ";
        }
        std::cout << std::endl;

        for (int j : active) {
            SetEvent(continueEvents[j]);
        }
    }

    DeleteCriticalSection(&cs);
    CloseHandle(startEvent);
    for (int i = 0; i < N; ++i) {
        CloseHandle(continueEvents[i]);
        CloseHandle(blockEvents[i]);
        CloseHandle(terminateEvents[i]);
        CloseHandle(threads[i]);
    }
    delete[] continueEvents;
    delete[] blockEvents;
    delete[] terminateEvents;
    delete[] threads;
    delete[] arr;

    return 0;
}