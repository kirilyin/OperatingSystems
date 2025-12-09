#include <windows.h>
#include <iostream>
#include <vector>

int main() {
    int N, M;
    std::cout << "N : ";
    std::cin >> N;
    std::cout << "M (M > N): ";
    std::cin >> M;

    if (M <= N) {
        std::cout << "Error ((M > N))!\n";
        return 1;
    }

    HANDLE hSemaphore = CreateSemaphoreW(nullptr, N, N, L"DownloadSlots");
    HANDLE hMutex = CreateMutexW(nullptr, FALSE, L"LogAccessMutex");
    HANDLE hClosingEvent = CreateEventW(nullptr, TRUE, FALSE, L"BrowserClosingEvent");

    if (!hSemaphore || !hMutex || !hClosingEvent) {
        std::cout << "Error creating sinhronisation\n";
        return 1;
    }

    std::vector<PROCESS_INFORMATION> processes;
    processes.reserve(M);

    std::cout << "\nBrowser is running. Press Enter to close...\n";

    std::cout << "Start " << M << " tasks for downloading...\n";

    for (int i = 1; i <= M; ++i)
    {
        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi = {};

        wchar_t cmdLine[100];
        wsprintfW(cmdLine, L"Downloader.exe %d", i);

        BOOL ok = CreateProcessW(
            nullptr,
            cmdLine,
            nullptr, nullptr,
            FALSE, 0,
            nullptr, nullptr,
            &si, &pi
        );

        if (ok)
        {
            CloseHandle(pi.hThread);
            processes.push_back(pi);
        }
        else
        {
            std::cout << "Cant start Downloader " << i
                << " (error " << GetLastError() << ")\n";
        }
    }

    std::cin.ignore(10000, '\n');
    std::cin.get();

    std::cout << "Browser is closing. Sending termination signal to all downloads...\n";
    SetEvent(hClosingEvent);

    if (!processes.empty())
    {
        std::vector<HANDLE> handles;
        for (const auto& p : processes) handles.push_back(p.hProcess);

        WaitForMultipleObjects((DWORD)handles.size(), handles.data(), TRUE, INFINITE);

        for (const auto& p : processes) CloseHandle(p.hProcess);
    }

    CloseHandle(hSemaphore);
    CloseHandle(hMutex);
    CloseHandle(hClosingEvent);

    std::cout << "Browser closed.\n";
    return 0;
}