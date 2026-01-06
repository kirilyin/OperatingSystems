#include "common.h"

int main()
{
    setlocale(LC_ALL, "Russian");

    int n;
    inputArraySize(n);
    if (n < 0) return 0;

    std::vector<int> arr(n);
    fillArray(arr);

    int delay;
    inputDelay(delay);

    runThreadExWithSuspended(arr, delay);

    printCompletionMessage();
    return 0;
}