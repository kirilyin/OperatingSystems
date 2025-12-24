#include <windows.h>
#include <iostream>
#include <string>

const int TASK_NORMAL = 0;
const int TASK_SHUTDOWN = 1;
const int MAX_ARRAY_SIZE = 100;

struct Task {
    int type;
    int size;
    int data[MAX_ARRAY_SIZE];
};

struct Result {
    bool success;
    int size;
    unsigned long long data[MAX_ARRAY_SIZE];
};

std::string PipeNameIn(int id) {
    return "\\\\.\\pipe\\worker_in_" + std::to_string(id);
}

std::string PipeNameOut(int id) {
    return "\\\\.\\pipe\\worker_out_" + std::to_string(id);
}

void Log(const std::string& message) {
    HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, L"LogAccessMutex");
    if (hMutex == NULL) {
        std::cout << message << std::endl;
        return;
    }

    WaitForSingleObject(hMutex, INFINITE);
    std::cout << message << std::endl;
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
}

unsigned long long factorial(int n) {
    if (n < 0) return 0;
    if (n == 0 || n == 1) return 1;
    unsigned long long res = 1;
    for (int i = 2; i <= n; ++i) {
        res *= i;
    }
    return res;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Error: worker ID not provided\n";
        return 1;
    }

    int id = atoi(argv[1]);

    HANDLE hPipeIn = CreateFileA(
        PipeNameIn(id).c_str(),
        GENERIC_READ,
        0, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);

    HANDLE hPipeOut = CreateFileA(
        PipeNameOut(id).c_str(),
        GENERIC_WRITE,
        0, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);

    if (hPipeIn == INVALID_HANDLE_VALUE || hPipeOut == INVALID_HANDLE_VALUE) {
        Log("Worker " + std::to_string(id) + ": failed to connect to pipes");
        return 1;
    }

    Log("Worker " + std::to_string(id) + " started and connected.");

    while (true) {
        Task task{};
        DWORD bytesRead;
        if (!ReadFile(hPipeIn, &task, sizeof(task), &bytesRead, NULL) || bytesRead != sizeof(task)) {
            Log("Worker " + std::to_string(id) + ": error reading task");
            break;
        }

        if (task.type == TASK_SHUTDOWN) {
            Log("Worker " + std::to_string(id) + " received shutdown command.");
            break;
        }

        Result result{};
        result.success = true;
        result.size = task.size;

        for (int i = 0; i < task.size; ++i) {
            int num = task.data[i];
            if (num < 0) {
                result.success = false;
                result.data[i] = 0;
            }
            else {
                result.data[i] = factorial(num);
            }
        }

        DWORD bytesWritten;
        if (!WriteFile(hPipeOut, &result, sizeof(result), &bytesWritten, NULL) ||
            bytesWritten != sizeof(result)) {
            Log("Worker " + std::to_string(id) + ": error sending result");
        }
    }

    CloseHandle(hPipeIn);
    CloseHandle(hPipeOut);
    Log("Worker " + std::to_string(id) + " finished.");
    return 0;
}