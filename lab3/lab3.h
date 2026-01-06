#ifndef LAB3_H
#define LAB3_H

#include <windows.h>
#include <vector>

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

DWORD WINAPI markerThread(LPVOID param);

class MarkerSystem {
public:
    MarkerSystem(int size, int num_threads, bool silent = false);
    ~MarkerSystem();

    void start();
    void waitForAllBlocked();
    std::vector<int> getArray() const;
    void terminateMarker(int num);
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
    std::vector<int> active;
    bool silent;
};

#endif