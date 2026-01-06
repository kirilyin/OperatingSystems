#define _CRT_SECURE_NO_WARNINGS

#include "common.h"
#include <process.h>
#include <cmath>
#include <cstdlib>

unsigned __stdcall worker7(void* param)
{
    std::vector<int>* vec = static_cast<std::vector<int>*>(param);
    std::cout << "Worker: numbers divisible by 5: ";
    for (int x : *vec)
    {
        if (x % 5 == 0)
            std::cout << x << " ";
    }
    std::cout << std::endl;
    return 0;
}

unsigned __stdcall worker9(void* param)
{
    std::vector<int>* vec = static_cast<std::vector<int>*>(param);
    double d;
    std::cout << "Enter a floating-point number: ";
    std::cin >> d;

    int i = static_cast<int>(std::floor(d));
    std::cout << i << std::endl;

    std::cout << "\nWorker: count of numbers divisible by the integer part of the entered number: ";
    int k = 0;
    if (i == 0)
    {
        for (int x : *vec)
            if (x == 0) ++k;
    }
    else
    {
        for (int x : *vec)
            if (x % i == 0) ++k;
    }
    std::cout << k << std::endl;
    return 0;
}

void inputArraySize(int& n)
{
    std::cout << "Enter the size of the array: ";
    std::cin >> n;
    if (n < 0)
        std::cout << "Invalid array size.\n";
}

void fillArray(std::vector<int>& arr)
{
    std::cout << "Fill the array randomly? (1 - yes, 0 - no): ";
    int choice;
    std::cin >> choice;

    if (choice)
    {
        srand(static_cast<unsigned>(time(nullptr)));
        for (size_t i = 0; i < arr.size(); ++i)
            arr[i] = rand() % 100;
    }
    else
    {
        std::cout << "Enter the elements: ";
        for (size_t i = 0; i < arr.size(); ++i)
            std::cin >> arr[i];
    }
}

void inputDelay(int& delay)
{
    std::cout << "Enter the delay (ms): ";
    std::cin >> delay;
}

void printCompletionMessage()
{
    std::cout << "\nWork completed.\n";
}

bool runThreadWithSuspend(const std::vector<int>& arr, int delay)
{
    HANDLE hThread = CreateThread(
        nullptr,
        0,
        (LPTHREAD_START_ROUTINE)worker7,
        const_cast<std::vector<int>*>(&arr),
        0,
        nullptr
    );

    if (hThread == nullptr)
    {
        std::cerr << "Error creating thread with CreateThread\n";
        return false;
    }

    SuspendThread(hThread);
    Sleep(delay);
    ResumeThread(hThread);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    return true;
}

bool runThreadExWithSuspended(const std::vector<int>& arr, int delay)
{
    unsigned threadId;
    HANDLE hThread = (HANDLE)_beginthreadex(
        nullptr,
        0,
        worker9,
        const_cast<std::vector<int>*>(&arr),
        CREATE_SUSPENDED,
        &threadId
    );

    if (hThread == nullptr)
    {
        std::cerr << "Error creating thread with _beginthreadex\n";
        return false;
    }

    Sleep(delay);
    ResumeThread(hThread);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    return true;
}