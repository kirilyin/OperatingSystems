#include <gtest/gtest.h>
#include "common.h"
#include <sstream>
#include <vector>
#include <cmath>
#include <limits>

std::string capture_cout(std::function<void()> func) {
    std::ostringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    func();
    std::cout.rdbuf(old);
    return buffer.str();
}

std::string run_with_input(const std::string& input, std::function<void()> func) {
    std::istringstream in(input);
    std::ostringstream out;
    std::streambuf* old_in = std::cin.rdbuf(in.rdbuf());
    std::streambuf* old_out = std::cout.rdbuf(out.rdbuf());

    func();

    std::cin.rdbuf(old_in);
    std::cout.rdbuf(old_out);
    return out.str();
}

TEST(InputArraySize, ValidPositive) {
    int n = -1;
    run_with_input("15\n", [&n]() { inputArraySize(n); });
    EXPECT_EQ(n, 15);
}

TEST(InputArraySize, Zero) {
    int n = -1;
    run_with_input("0\n", [&n]() { inputArraySize(n); });
    EXPECT_EQ(n, 0);
}

TEST(InputArraySize, Negative) {
    int n = 10;
    std::string output = run_with_input("-5\n", [&n]() { inputArraySize(n); });
    EXPECT_EQ(n, -5);
    EXPECT_NE(output.find("Invalid array size."), std::string::npos);
}

TEST(FillArray, ManualInput) {
    std::vector<int> arr(3);
    run_with_input("0\n1 2 3\n", [&arr]() { fillArray(arr); });
    EXPECT_EQ(arr, std::vector<int>({ 1, 2, 3 }));
}

TEST(FillArray, RandomFill) {
    std::vector<int> arr(10);
    run_with_input("1\n", [&arr]() { fillArray(arr); });
    for (int x : arr) {
        EXPECT_GE(x, 0);
        EXPECT_LT(x, 100);
    }
}

TEST(InputDelay, NormalValue) {
    int delay = -1;
    run_with_input("500\n", [&delay]() { inputDelay(delay); });
    EXPECT_EQ(delay, 500);
}

TEST(Worker7, ContainsMultiples) {
    std::vector<int> arr = { 1, 5, 10, 7, 15, -5, 0 };
    std::string output = capture_cout([&arr]() { worker7(&arr); });
    EXPECT_NE(output.find("5"), std::string::npos);
    EXPECT_NE(output.find("10"), std::string::npos);
    EXPECT_NE(output.find("15"), std::string::npos);
    EXPECT_NE(output.find("-5"), std::string::npos);
    EXPECT_NE(output.find("0"), std::string::npos);
}

TEST(Worker7, NoMultiples) {
    std::vector<int> arr = { 1, 2, 3, 4, 6 };
    std::string output = capture_cout([&arr]() { worker7(&arr); });
    EXPECT_EQ(output, "Worker: numbers divisible by 5: \n");
}

TEST(Worker7, EmptyArray) {
    std::vector<int> arr;
    std::string output = capture_cout([&arr]() { worker7(&arr); });
    EXPECT_EQ(output, "Worker: numbers divisible by 5: \n");
}

TEST(Worker9, FloorZero) {
    std::vector<int> arr = { 0, 1, -1, 0, 5 };
    std::string output = run_with_input("0.7\n", [&arr]() { worker9(&arr); });

    EXPECT_NE(output.find("0"), std::string::npos);

    std::string marker = "Worker: count of numbers divisible by the integer part of the entered number: ";
    size_t pos = output.find(marker);
    ASSERT_NE(pos, std::string::npos);

    std::string rest = output.substr(pos + marker.length());
    std::istringstream iss(rest);
    int count;
    iss >> count;

    EXPECT_EQ(count, 2);
}

TEST(Worker9, PositiveFloor) {
    std::vector<int> arr = { 10, 20, 3, 30, -10, 0 };
    std::string output = run_with_input("5.9\n", [&arr]() { worker9(&arr); });

    EXPECT_NE(output.find("5"), std::string::npos);

    std::string marker = "Worker: count of numbers divisible by the integer part of the entered number: ";
    size_t pos = output.find(marker);
    ASSERT_NE(pos, std::string::npos);

    std::string rest = output.substr(pos + marker.length());
    std::istringstream iss(rest);
    int count;
    iss >> count;

    EXPECT_EQ(count, 5);
}

TEST(Worker9, NegativeFloor) {
    std::vector<int> arr = { 9, -8, 12, 0, 4, -4 };
    std::string output = run_with_input("-4.7\n", [&arr]() { worker9(&arr); });

    EXPECT_NE(output.find("-5"), std::string::npos);

    std::string marker = "Worker: count of numbers divisible by the integer part of the entered number: ";
    size_t pos = output.find(marker);
    ASSERT_NE(pos, std::string::npos);

    std::string rest = output.substr(pos + marker.length());
    std::istringstream iss(rest);
    int count;
    iss >> count;
}

TEST(Worker9, NegativeFloorFixed) {
    std::vector<int> arr = { -10, 10, 0, 15, -15, 7 };
    std::string output = run_with_input("-5.0\n", [&arr]() { worker9(&arr); });

    EXPECT_NE(output.find("-5"), std::string::npos);

    std::string marker = "Worker: count of numbers divisible by the integer part of the entered number: ";
    size_t pos = output.find(marker);
    ASSERT_NE(pos, std::string::npos);

    std::string rest = output.substr(pos + marker.length());
    std::istringstream iss(rest);
    int count;
    iss >> count;

    EXPECT_EQ(count, 5);
}

TEST(Worker9, LargeFloor) {
    std::vector<int> arr = { 100, 200, 300 };
    std::string output = run_with_input("100.0\n", [&arr]() { worker9(&arr); });

    EXPECT_NE(output.find("100"), std::string::npos);

    std::string marker = "Worker: count of numbers divisible by the integer part of the entered number: ";
    size_t pos = output.find(marker);
    ASSERT_NE(pos, std::string::npos);

    std::string rest = output.substr(pos + marker.length());
    std::istringstream iss(rest);
    int count;
    iss >> count;

    EXPECT_EQ(count, 3);
}

TEST(Worker9, DivisionByZeroCase) {
    std::vector<int> arr = { 0, 0, 1, 0 };
    std::string output = run_with_input("0.0\n", [&arr]() { worker9(&arr); });

    EXPECT_NE(output.find("0"), std::string::npos);

    std::string marker = "Worker: count of numbers divisible by the integer part of the entered number: ";
    size_t pos = output.find(marker);
    ASSERT_NE(pos, std::string::npos);

    std::string rest = output.substr(pos + marker.length());
    std::istringstream iss(rest);
    int count;
    iss >> count;

    EXPECT_EQ(count, 3);
}