#include <windows.h> // task 7
#include <process.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

DWORD WINAPI worker(LPVOID param) {
    std::vector<int>* vec = static_cast<std::vector<int>*>(param);
    std::cout << "Worker: числа, кратные 5: ";
    for (int x : *vec) {
        if (x % 5 == 0) std::cout << x << " ";
    }
    std::cout << std::endl;
    return 0;
}


int main() {
    setlocale(LC_ALL, "Russian");

    int n;
    std::cout << "Введите размер массива: ";
    std::cin >> n;

    std::vector<int> arr(n);
    std::cout << "Заполнить массив случайно? (1 - да, 0 - нет): ";
    int choice; std::cin >> choice;

    if (choice) {
        srand((unsigned)time(nullptr));
        for (int i = 0; i < n; i++) arr[i] = rand() % 100;
    }
    else {
        std::cout << "Введите элементы: ";
        for (int i = 0; i < n; i++) std::cin >> arr[i];
    }

    std::cout << "Введите задержку (мс): ";
    int delay; std::cin >> delay;


    std::cout << "\n=== Создание потока через CreateThread ===\n";

    HANDLE hThread = CreateThread(
        nullptr,
        0,
        worker,
        &arr,
        0,
        nullptr
    );
    if (hThread == NULL) {
        std::cerr << "Ошибка создания потока\n";
        return -1;
    }
    SuspendThread(hThread);
    Sleep(delay);
    ResumeThread(hThread);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    std::cout << "\nРабота завершена.\n";
    return 0;
}
