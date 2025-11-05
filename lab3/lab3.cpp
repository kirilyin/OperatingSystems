#include <windows.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

struct ThreadParams {
    int num;
    int size;
    int* arr;
    CRITICAL_SECTION* cs;
    HANDLE startEvent;
    HANDLE continueEvent;
    HANDLE blockEvent;
    HANDLE terminateEvent;
    bool silent;
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

        if (!p->silent) {
            std::cout << "Marker " << p->num << " marked " << count << " blocked at " << blockIndex << std::endl;
        }
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
            return 0;
        }
    }

    return 0;
}

class MarkerSystem {
public:
    MarkerSystem(int size, int num_threads, bool silent = false);
    ~MarkerSystem();

    void start();
    void waitForAllBlocked();
    std::vector<int> getArray() const;
    void terminateMarker(int num); // num from 1 to num_threads
    void continueAllActive();
    bool hasActive() const;
    std::vector<int> getActiveMarkers() const;

private:
    int array_size;
    int num_threads;
    int* arr;
    mutable CRITICAL_SECTION cs;
    HANDLE startEvent;
    std::vector<HANDLE> continueEvents;
    std::vector<HANDLE> blockEvents;
    std::vector<HANDLE> terminateEvents;
    std::vector<HANDLE> threadHandles;
    std::vector<ThreadParams*> threadParams;
    std::vector<int> active; // indices 0 to num_threads-1
    bool silent;
};

MarkerSystem::MarkerSystem(int s, int n, bool sil) : array_size(s), num_threads(n), silent(sil) {
    arr = new int[s];
    for (int i = 0; i < s; ++i) {
        arr[i] = 0;
    }

    InitializeCriticalSection(&cs);

    startEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    continueEvents.resize(n);
    blockEvents.resize(n);
    terminateEvents.resize(n);
    threadHandles.resize(n);
    threadParams.resize(n);
    active.resize(n);

    for (int i = 0; i < n; ++i) {
        continueEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        blockEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        terminateEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        threadParams[i] = new ThreadParams{
            i + 1,
            s,
            arr,
            &cs,
            startEvent,
            continueEvents[i],
            blockEvents[i],
            terminateEvents[i],
            silent
        };
        threadHandles[i] = CreateThread(NULL, 0, markerThread, threadParams[i], 0, NULL);
        active[i] = i;
    }

}
std::vector<int> MarkerSystem::getActiveMarkers() const {
    std::vector<int> markers;
    for (int idx : active) {
        markers.push_back(idx + 1);
    }
    return markers;
}

MarkerSystem::~MarkerSystem() {
    for (int i = 0; i < num_threads; ++i) {
        SetEvent(terminateEvents[i]);
    }
    WaitForMultipleObjects(num_threads, threadHandles.data(), TRUE, INFINITE);

    for (size_t i = 0; i < threadHandles.size(); ++i) {
        CloseHandle(threadHandles[i]);
    }

    for (auto p : threadParams) {
        delete p;
    }

    DeleteCriticalSection(&cs);
    CloseHandle(startEvent);
    for (size_t i = 0; i < continueEvents.size(); ++i) {
        CloseHandle(continueEvents[i]);
        CloseHandle(blockEvents[i]);
        CloseHandle(terminateEvents[i]);
    }
    delete[] arr;
}

void MarkerSystem::start() {
    SetEvent(startEvent);
}

void MarkerSystem::waitForAllBlocked() {
    DWORD n_active = static_cast<DWORD>(active.size());
    HANDLE* waitBlocks = new HANDLE[n_active];
    for (DWORD j = 0; j < n_active; ++j) {
        waitBlocks[j] = blockEvents[active[j]];
    }
    WaitForMultipleObjects(n_active, waitBlocks, TRUE, INFINITE);
    delete[] waitBlocks;
}

std::vector<int> MarkerSystem::getArray() const {
    std::vector<int> copy;
    EnterCriticalSection(&cs);
    copy.assign(arr, arr + array_size);
    LeaveCriticalSection(&cs);
    return copy;
}

void MarkerSystem::terminateMarker(int num) {
    int index = num - 1;
    auto it = std::find(active.begin(), active.end(), index);
    if (it == active.end()) return;
    SetEvent(terminateEvents[index]);
    WaitForSingleObject(threadHandles[index], INFINITE);
    active.erase(it);
}

void MarkerSystem::continueAllActive() {
    for (int j : active) {
        SetEvent(continueEvents[j]);
    }
}

bool MarkerSystem::hasActive() const {
    return !active.empty();
}
