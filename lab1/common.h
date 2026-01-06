#pragma once

#include <windows.h>
#include <iostream>
#include <vector>
#include <ctime>

void inputArraySize(int& n);
void fillArray(std::vector<int>& arr);
void inputDelay(int& delay);
void printCompletionMessage();

bool runThreadWithSuspend(const std::vector<int>& arr, int delay);
bool runThreadExWithSuspended(const std::vector<int>& arr, int delay);

unsigned __stdcall worker7(void* param);
unsigned __stdcall worker9(void* param);