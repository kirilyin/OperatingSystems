#include <gtest/gtest.h>
#include <windows.h>
#include <string>
#include <fstream>

void RunBrowser(const std::string& input, int N = 1) {
    std::ofstream inputFile("temp_input.txt");
    inputFile << N << "\n";
    inputFile << input;
    inputFile.close();

    std::string cmd = "Browser1.exe < temp_input.txt > test_output.txt 2>&1";
    system(cmd.c_str());

    remove("temp_input.txt");
}

bool OutputContains(const std::string& needle) {
    std::ifstream file("test_output.txt");
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        if (line.find(needle) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// ====================== ТЕСТЫ ======================

TEST(PositiveTests, OneWorkerOneTask) {
    std::string input =
        "3 5 2 8\n"
        "\n";

    RunBrowser(input, 1);

    EXPECT_TRUE(OutputContains("Input: 5 2 8"));
    EXPECT_TRUE(OutputContains("Factorials: 120 2 40320"));
}

TEST(PositiveTests, MultipleWorkersMultipleTasks) {
    std::string input =
        "2 4 7\n"
        "1 10\n"
        "0\n"
        "3 3 3 3\n"
        "\n";

    RunBrowser(input, 2);

    EXPECT_TRUE(OutputContains("All 2 workers started"));
    EXPECT_TRUE(OutputContains("All workers terminated cleanly."));
}

TEST(PositiveTests, CleanShutdown) {
    std::string input = "1 5\n\n";
    RunBrowser(input, 1);

    EXPECT_TRUE(OutputContains("Sending shutdown commands"));
    EXPECT_TRUE(OutputContains("Worker 0 received shutdown command"));
}

TEST(EdgeTests, EmptyTask) {
    std::string input = "0\n\n";
    RunBrowser(input, 1);

    EXPECT_TRUE(OutputContains("Input: <empty>"));
    EXPECT_TRUE(OutputContains("Factorials: <empty>"));
}

TEST(EdgeTests, NegativeNumbers) {
    std::string input = "3 -1 5 -10\n\n";
    RunBrowser(input, 1);

    EXPECT_TRUE(OutputContains("contains negative numbers -> marked as error"));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}