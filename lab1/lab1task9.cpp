#include <windows.h> // task 9
#include <process.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

DWORD WINAPI worker(LPVOID param) {
    std::vector<int>* vec = static_cast<std::vector<int>*>(param);
    double d;
    std::cout << "����� ������������ �����: ";
    std::cin >> d;
    int i = floor(d);
    std::cout << "\nWorker: �����, ������� ����� ����� �������������: ";
    int k = 0;
    for (int x : *vec) {
        if (x % i == 0) k++;
    }
    std::cout << k << " ";
    std::cout << std::endl;
    return 0;
}


int main() {
    setlocale(LC_ALL, "Russian");

    int n;
    std::cout << "������� ������ �������: ";
    std::cin >> n;

    std::vector<int> arr(n);
    std::cout << "��������� ������ ��������? (1 - ��, 0 - ���): ";
    int choice; std::cin >> choice;

    if (choice) {
        srand((unsigned)time(nullptr));
        for (int i = 0; i < n; i++) arr[i] = rand() % 100;
    }
    else {
        std::cout << "������� ��������: ";
        for (int i = 0; i < n; i++) std::cin >> arr[i];
    }

    std::cout << "������� �������� (��): ";
    int delay; std::cin >> delay;

    std::cout << "\n=== �������� ������ ����� CreateThread ===\n";

    HANDLE hThread = CreateThread(
        nullptr,             
        0,                   
        worker,        
        &arr,              
        0,   
        nullptr             
    );
    if (hThread == NULL) {
        std::cerr << "������ �������� ������\n";
        return -1; 
    }
    SuspendThread(hThread);
    Sleep(delay);
    ResumeThread(hThread); 
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    std::cout << "\n������ ���������.\n";
    return 0;
}
