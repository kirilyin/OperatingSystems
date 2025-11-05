#include <gtest/gtest.h>
#include "lab3.cpp" 
#include <thread>
#include <chrono>

// ======== 义耱 1.1 ========
TEST(MarkerSystemTest, SingleThreadMarking_FailsIfNotAllMarked) {
    MarkerSystem sys(10, 1, true);
    sys.start();
    sys.waitForAllBlocked();

    auto arr = sys.getArray();

    bool allMarkedWithOne = true;
    for (int val : arr) {
        if (val != 1) {
            allMarkedWithOne = false;
            break;
        }
    }

    EXPECT_FALSE(allMarkedWithOne);
}

// ======== 义耱 1.2 ========
TEST(MarkerSystemTest, CleanupAfterTermination) {
    MarkerSystem sys(10, 1, true);
    sys.start();
    sys.waitForAllBlocked();
    sys.terminateMarker(1);

    auto arr = sys.getArray();

    for (int val : arr) {
        EXPECT_EQ(val, 0);
    }
}

// ======== 义耱 2.1 ========
TEST(MarkerSystemTest, NoDataRaceWithMultipleMarkers) {
    MarkerSystem sys(20, 10, true);
    sys.start();
    sys.waitForAllBlocked();

    auto arr = sys.getArray();

    for (int val : arr) {
        EXPECT_TRUE(val >= 0 && val <= 10)
            << "bad value: " << val;
    }

    int countMarked = 0;
    for (int val : arr) {
        if (val != 0) countMarked++;
    }
    EXPECT_TRUE(countMarked > 0);
}

// ======== 义耱 2.2 ========
TEST(MarkerSystemTest, SequentialTerminationAndCleanup) {
    MarkerSystem sys(30, 5, true);
    sys.start();
    sys.waitForAllBlocked();

    for (int i = 1; i <= 5; ++i) {
        sys.terminateMarker(i);

        auto arr = sys.getArray();

        for (int val : arr) {
            EXPECT_NE(val, i);
        }

        sys.continueAllActive();
        if (sys.hasActive())
            sys.waitForAllBlocked();
    }

    auto arr = sys.getArray();
    for (int val : arr) {
        EXPECT_EQ(val, 0);
    }
}
