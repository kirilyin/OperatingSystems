#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

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

std::string PipeNameIn(int id) { return "\\\\.\\pipe\\worker_in_" + std::to_string(id); }
std::string PipeNameOut(int id) { return "\\\\.\\pipe\\worker_out_" + std::to_string(id); }

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

bool sendTask(HANDLE hPipeIn, const Task& task) {
    DWORD written;
    return WriteFile(hPipeIn, &task, sizeof(task), &written, NULL) && written == sizeof(task);
}

bool receiveResult(HANDLE hPipeOut, Result& result) {
    DWORD readBytes;
    return ReadFile(hPipeOut, &result, sizeof(result), &readBytes, NULL) && readBytes == sizeof(result);
}

int main() {
    HANDLE hMutex = CreateMutexW(nullptr, FALSE, L"LogAccessMutex");
    if (hMutex == NULL) {
        std::cout << "Error creating mutex\n";
        return 1;
    }

    int N;
    std::cout << "Enter number of workers (N): ";
    std::cin >> N;
    if (N <= 0) {
        Log("Error: N must be positive");
        CloseHandle(hMutex);
        return 1;
    }
    std::cin.ignore(); 

    std::vector<HANDLE> pipesIn(N), pipesOut(N);
    std::vector<HANDLE> processHandles(N);

    for (int id = 0; id < N; ++id) {
        pipesIn[id] = CreateNamedPipeA(PipeNameIn(id).c_str(),
            PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1, 4096, 4096, 0, NULL);

        pipesOut[id] = CreateNamedPipeA(PipeNameOut(id).c_str(),
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
        if (!CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
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
    Log("All " + std::to_string(N) + " workers started and connected.");

    std::cout << "Enter : size num1 num2 ... numN\n";

    std::vector<Task> tasks;
    std::string line;
    int taskNumber = 1;

    while (true) {
        std::cout << "Task " << taskNumber << ": ";
        std::getline(std::cin, line);

        if (line.empty()) {
            std::cout << "Empty line received — finishing task input.\n";
            break;
        }

        std::istringstream iss(line);
        Task task{};
        task.type = TASK_NORMAL;

        if (!(iss >> task.size)) {
            std::cout << "Error: invalid format (size missing)\n";
            continue;
        }

        if (task.size < 0 || task.size > MAX_ARRAY_SIZE) {
            std::cout << "Error: size must be between 0 and " << MAX_ARRAY_SIZE << "\n";
            continue;
        }

        bool valid = true;
        for (int i = 0; i < task.size; ++i) {
            if (!(iss >> task.data[i])) {
                std::cout << "Error: not enough numbers for given size\n";
                valid = false;
                task.size = i;
                break;
            }
        }

        if (!valid) continue;

        tasks.push_back(task);
        ++taskNumber;
    }

    int nextWorker = 0;
    for (size_t i = 0; i < tasks.size(); ++i) {
        int workerId = nextWorker;
        nextWorker = (nextWorker + 1) % N;

        const Task& task = tasks[i];

        if (!sendTask(pipesIn[workerId], task)) {
            Log("Error sending task " + std::to_string(i + 1) + " to worker " + std::to_string(workerId));
            continue;
        }

        Result result{};
        if (!receiveResult(pipesOut[workerId], result)) {
            Log("Error receiving result for task " + std::to_string(i + 1));
            continue;
        }

        std::string inputStr = "Task " + std::to_string(i + 1) + " -> Worker " + std::to_string(workerId) + " Input:";
        if (task.size == 0) {
            inputStr += " <empty>";
        } else {
            for (int j = 0; j < task.size; ++j) {
                inputStr += " " + std::to_string(task.data[j]);
            }
        }

        std::string factStr = "  Factorials:";
        if (result.size == 0) {
            factStr += " <empty>";
        } else {
            for (int j = 0; j < result.size; ++j) {
                factStr += " " + std::to_string(result.data[j]);
            }
        }

        Log(inputStr);
        Log(factStr);
        if (!result.success) {
            Log("  (contains negative numbers -> marked as error)");
        }
        Log("");
    }

    Log("Sending shutdown commands to all workers...");
    for (int id = 0; id < N; ++id) {
        Task shutdown{};
        shutdown.type = TASK_SHUTDOWN;
        sendTask(pipesIn[id], shutdown);
    }

    WaitForMultipleObjects(N, processHandles.data(), TRUE, INFINITE);

    // Cleanup
    for (int i = 0; i < N; ++i) {
        CloseHandle(pipesIn[i]);
        CloseHandle(pipesOut[i]);
        CloseHandle(processHandles[i]);
    }

    Log("Browser finished. All workers terminated cleanly.");
    CloseHandle(hMutex);
    return 0;
}