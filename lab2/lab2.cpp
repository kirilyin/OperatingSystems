#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

void CheckError(BOOL result, const std::string& message) {
    if (!result) {
        DWORD error = GetLastError();
        std::cerr << message << " Error: " << error << std::endl;
        ExitProcess(error);
    }
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");

    if (argc == 1) {
        int n;
        std::cout << "Введите размер массива: ";
        std::cin >> n;
        if (n <= 0) {
            std::cerr << "Некорректный размер массива\n";
            return 1;
        }

        std::vector<int> arr(n);
        std::cout << "Заполнить массив случайно? (1 - да, 0 - нет): ";
        int choice;
        std::cin >> choice;

        if (choice) {
            srand((unsigned)time(nullptr));
            for (int i = 0; i < n; i++) {
                arr[i] = rand() % 100;
            }
        }
        else {
            std::cout << "Введите элементы: ";
            for (int i = 0; i < n; i++) {
                std::cin >> arr[i];
            }
        }

        int A, B;
        std::cout << "Введите диапазон [A, B]:\nA: ";
        std::cin >> A;
        std::cout << "B: ";
        std::cin >> B;

        HANDLE hPipe1Read, hPipe1Write, hPipe2Read, hPipe2Write;
        SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

        CheckError(CreatePipe(&hPipe1Read, &hPipe1Write, &saAttr, 0), "CreatePipe1 failed.");
        CheckError(CreatePipe(&hPipe2Read, &hPipe2Write, &saAttr, 0), "CreatePipe2 failed.");

        STARTUPINFO si = { sizeof(STARTUPINFO) };
        PROCESS_INFORMATION pi = { 0 };
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = hPipe1Read;
        si.hStdOutput = hPipe2Write;
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);

        std::string cmdLine = std::string(exePath) + " child";

        CheckError(CreateProcessA(NULL, &cmdLine[0], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi),
            "CreateProcess failed.");

        CloseHandle(hPipe1Read);
        CloseHandle(hPipe2Write);

        DWORD bytesWritten;
        CheckError(WriteFile(hPipe1Write, &n, sizeof(n), &bytesWritten, NULL), "Write array size failed.");
        for (int i = 0; i < n; i++) {
            CheckError(WriteFile(hPipe1Write, &arr[i], sizeof(int), &bytesWritten, NULL), "Write array element failed.");
        }
        CheckError(WriteFile(hPipe1Write, &A, sizeof(A), &bytesWritten, NULL), "Write A failed.");
        CheckError(WriteFile(hPipe1Write, &B, sizeof(B), &bytesWritten, NULL), "Write B failed.");

        CloseHandle(hPipe1Write);

        int result;
        DWORD bytesRead;
        CheckError(ReadFile(hPipe2Read, &result, sizeof(result), &bytesRead, NULL), "Read result failed.");

        WaitForSingleObject(pi.hProcess, INFINITE);

        std::cout << "Количество элементов в диапазоне [" << A << ", " << B << "]: " << result << std::endl;

        CloseHandle(hPipe2Read);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

    }
    else if (argc == 2 && std::string(argv[1]) == "child") {
        int n;
        DWORD bytesRead;
        CheckError(ReadFile(GetStdHandle(STD_INPUT_HANDLE), &n, sizeof(n), &bytesRead, NULL), "Read array size failed.");
        if (n <= 0) {
            std::cerr << "Некорректный размер массива\n";
            return 1;
        }

        std::vector<int> arr(n);
        for (int i = 0; i < n; i++) {
            CheckError(ReadFile(GetStdHandle(STD_INPUT_HANDLE), &arr[i], sizeof(int), &bytesRead, NULL), "Read array element failed.");
        }

        int A, B;
        CheckError(ReadFile(GetStdHandle(STD_INPUT_HANDLE), &A, sizeof(A), &bytesRead, NULL), "Read A failed.");
        CheckError(ReadFile(GetStdHandle(STD_INPUT_HANDLE), &B, sizeof(B), &bytesRead, NULL), "Read B failed.");

        int count = 0;
        for (int x : arr) {
            if (x >= A && x <= B) {
                count++;
            }
        }

        DWORD bytesWritten;
        CheckError(WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), &count, sizeof(count), &bytesWritten, NULL), "Write result failed.");

    }
    else {
        std::cerr << "Некорректные аргументы командной строки\n";
        return 1;
    }

    return 0;
}