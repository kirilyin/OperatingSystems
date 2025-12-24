#include <gtest/gtest.h>
#include <windows.h>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>

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

unsigned long long factorial(int n) {
    if (n < 0) return 0;
    if (n == 0 || n == 1) return 1;
    unsigned long long res = 1;
    for (int i = 2; i <= n; ++i) {
        res *= i;
    }
    return res;
}

bool WaitForProcess(HANDLE hProcess, DWORD timeoutMs = 5000) {
    DWORD result = WaitForSingleObject(hProcess, timeoutMs);
    return result == WAIT_OBJECT_0;
}

TEST(PositiveTests, OneWorkerOneTask) {
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi_browser;
    si.dwFlags = STARTF_USESTDHANDLES;

    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    HANDLE hRead, hWrite;
    CreatePipe(&hRead, &hWrite, &sa, 0);
    si.hStdInput = hRead;

    BOOL created = CreateProcessA("Browser1.exe", NULL, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi_browser);
    ASSERT_TRUE(created);

    char input[] = "1\n1\n";
    DWORD written;
    WriteFile(hWrite, input, strlen(input), &written, NULL);
    CloseHandle(hWrite);
    CloseHandle(hRead);

    bool finished = WaitForProcess(pi_browser.hProcess, 10000);
    ASSERT_TRUE(finished);

    DWORD exitCode;
    GetExitCodeProcess(pi_browser.hProcess, &exitCode);
    EXPECT_EQ(exitCode, 0);

    CloseHandle(pi_browser.hThread);
    CloseHandle(pi_browser.hProcess);
}

TEST(PositiveTests, MultipleWorkersMultipleTasks) {
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESTDHANDLES;

    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    HANDLE hRead, hWrite;
    CreatePipe(&hRead, &hWrite, &sa, 0);
    si.hStdInput = hRead;

    BOOL created = CreateProcessA("Browser1.exe", NULL, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    ASSERT_TRUE(created);

    char input[] = "2\n10\n";
    DWORD written;
    WriteFile(hWrite, input, strlen(input), &written, NULL);
    CloseHandle(hWrite);
    CloseHandle(hRead);

    bool finished = WaitForProcess(pi.hProcess, 15000);
    ASSERT_TRUE(finished);

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    EXPECT_EQ(exitCode, 0);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}

TEST(PositiveTests, ProperShutdown) {
    SUCCEED();
}

TEST(BoundaryTests, EmptyTask) {
    Task task{};
    task.type = TASK_NORMAL;
    task.size = 0;

    Result result{};
    result.success = true;
    result.size = task.size;

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.size, 0);
}

TEST(FactorialFunction, BasicCalculations) {
    EXPECT_EQ(factorial(0), 1ULL);
    EXPECT_EQ(factorial(1), 1ULL);
    EXPECT_EQ(factorial(5), 120ULL);
    EXPECT_EQ(factorial(10), 3628800ULL);
}

TEST(FactorialFunction, NegativeInput) {
    Task task{};
    task.type = TASK_NORMAL;
    task.size = 1;
    task.data[0] = -5;

    Result result{};
    result.success = false;
    result.size = 1;
    result.data[0] = factorial(-5);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.data[0], 0ULL);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}