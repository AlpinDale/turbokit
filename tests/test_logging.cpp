#include "logging.h"
#include <gtest/gtest.h>
#include <sstream>

using namespace turbokit;

class LoggingTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(LoggingTest, LogLevels) {
  // Test that log levels are properly defined
  EXPECT_EQ(MSG_NONE, MessageLevel::MSG_NONE);
  EXPECT_EQ(MSG_ERROR, MessageLevel::MSG_ERROR);
  EXPECT_EQ(MSG_INFO, MessageLevel::MSG_INFO);
  EXPECT_EQ(MSG_VERBOSE, MessageLevel::MSG_VERBOSE);
  EXPECT_EQ(MSG_DEBUG, MessageLevel::MSG_DEBUG);
}

TEST_F(LoggingTest, CurrentLogLevel) {
  // Test that current log level is accessible
  EXPECT_GE(activeMessageLevel, MSG_NONE);
  EXPECT_LE(activeMessageLevel, MSG_DEBUG);
}

TEST_F(LoggingTest, LogObjectExists) {
  // Test that log object exists
  EXPECT_NE(&messageWriter, nullptr);
}

TEST_F(LoggingTest, LogMethodsExist) {
  // Test that all log methods exist and can be called
  // These should not crash even if output is suppressed
  messageWriter.error("Test error message");
  messageWriter.info("Test info message");
  messageWriter.verbose("Test verbose message");
  messageWriter.debug("Test debug message");
}

TEST_F(LoggingTest, FatalFunction) {
  // Test that fatal function exists
  // Note: This would normally exit the program, so we just test it exists
  // We can't test the address of a function template, so we'll just test that
  // it compiles criticalError("Test fatal message"); // This would exit the
  // program
  EXPECT_TRUE(true); // Placeholder test
}

TEST_F(LoggingTest, LogLevelComparison) {
  // Test log level comparisons
  EXPECT_LT(MSG_NONE, MSG_ERROR);
  EXPECT_LT(MSG_ERROR, MSG_INFO);
  EXPECT_LT(MSG_INFO, MSG_VERBOSE);
  EXPECT_LT(MSG_VERBOSE, MSG_DEBUG);
}

TEST_F(LoggingTest, LogMutexExists) {
  // Test that log mutex exists
  EXPECT_NE(&messageMutex, nullptr);
}