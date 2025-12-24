#include <windows.h>
#include <iostream>
#include <vector>
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
    if (hMutex == NULL) return;

    WaitForSingleObject(hMutex, INFINITE);
    std::cout << message << std::endl;
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
}

int main() {
    HANDLE hMutex = CreateMutexW(nullptr, FALSE, L"LogAccessMutex");
    if (hMutex == NULL) {
        std::cout << "Error creating mutex\n";
        return 1;
    }

    int N, M;
    std::cout << "N (number of workers): ";
    std::cin >> N;
    std::cout << "M (number of tasks, M >= N): ";
    std::cin >> M;

    if (M < N) {
        Log("Error: M must be >= N");
        CloseHandle(hMutex);
        return 1;
    }

    std::vector<HANDLE> pipesIn(N), pipesOut(N);
    std::vector<HANDLE> processHandles(N);

    for (int id = 0; id < N; ++id) {
        pipesIn[id] = CreateNamedPipeA(
            PipeNameIn(id).c_str(),
            PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1, 4096, 4096, 0, NULL);

        pipesOut[id] = CreateNamedPipeA(
            PipeNameOut(id).c_str(),
            PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1, 4096, 4096, 0, NULL);

        if (pipesIn[id] == INVALID_HANDLE_VALUE || pipesOut[id] == INVALID_HANDLE_VALUE) {
            Log("Error creating pipes for worker " + std::to_string(id));
            CloseHandle(hMutex);
            return 1;
        }

        char cmdLine[256];
        sprintf_s(cmdLine, "Worker1.exe %d", id);

        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi;

        BOOL created = CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        if (!created) {
            Log("Failed to start Worker " + std::to_string(id));
            CloseHandle(hMutex);
            return 1;
        }

        CloseHandle(pi.hThread);
        processHandles[id] = pi.hProcess;
    }

    for (int id = 0; id < N; ++id) {
        ConnectNamedPipe(pipesIn[id], NULL);
        ConnectNamedPipe(pipesOut[id], NULL);
    }

    Log("All workers started and connected.");

    int nextWorker = 0;
    for (int taskNum = 1; taskNum <= M; ++taskNum) {
        int workerId = nextWorker;
        nextWorker = (nextWorker + 1) % N;

        Task task{};
        task.type = TASK_NORMAL;
        task.size = 8 + (taskNum % 5);
        for (int i = 0; i < task.size; ++i) {
            task.data[i] = 1 + (i + taskNum) % 20;
        }

        DWORD written;
        if (!WriteFile(pipesIn[workerId], &task, sizeof(task), &written, NULL) || written != sizeof(task)) {
            Log("Error sending task to worker " + std::to_string(workerId));
            continue;
        }

        Result result{};
        DWORD readBytes;
        if (!ReadFile(pipesOut[workerId], &result, sizeof(result), &readBytes, NULL) ||
            readBytes != sizeof(result) || !result.success || result.size != task.size) {
            Log("Error receiving result from worker " + std::to_string(workerId));
        }
        else {
            std::string inputStr = "  Input:  ";
            for (int i = 0; i < task.size; ++i) inputStr += std::to_string(task.data[i]) + " ";

            std::string factStr = "  Factorials: ";
            for (int i = 0; i < result.size; ++i) factStr += std::to_string(result.data[i]) + " ";

            Log("Task " + std::to_string(taskNum) + " completed by worker " + std::to_string(workerId) + ":");
            Log(inputStr);
            Log(factStr);
            Log("");
        }
    }

    Log("Sending shutdown command to workers...");
    for (int id = 0; id < N; ++id) {
        Task shutdown{};
        shutdown.type = TASK_SHUTDOWN;
        shutdown.size = 0;
        DWORD written;
        WriteFile(pipesIn[id], &shutdown, sizeof(shutdown), &written, NULL);
    }

    WaitForMultipleObjects(N, processHandles.data(), TRUE, INFINITE);

    for (int i = 0; i < N; ++i) {
        CloseHandle(pipesIn[i]);
        CloseHandle(pipesOut[i]);
        CloseHandle(processHandles[i]);
    }

    Log("Browser finished.");
    CloseHandle(hMutex);
    return 0;
}