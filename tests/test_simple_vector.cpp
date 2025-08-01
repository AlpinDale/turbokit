#include "simple_vector.h"
#include <gtest/gtest.h>
#include <string>

using namespace turbokit;

class SimpleVectorTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(SimpleVectorTest, DefaultConstruction) {
  SimpleVector<int> vec;
  EXPECT_EQ(vec.size(), 0);
  EXPECT_TRUE(vec.empty());
}

TEST_F(SimpleVectorTest, SizeConstruction) {
  SimpleVector<int> vec(10);
  EXPECT_EQ(vec.size(), 10);
  EXPECT_FALSE(vec.empty());
}

TEST_F(SimpleVectorTest, InitializerListConstruction) {
  SimpleVector<int> vec{1, 2, 3, 4, 5};
  EXPECT_EQ(vec.size(), 5);
  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 2);
  EXPECT_EQ(vec[2], 3);
  EXPECT_EQ(vec[3], 4);
  EXPECT_EQ(vec[4], 5);
}

TEST_F(SimpleVectorTest, CopyConstruction) {
  SimpleVector<int> original{1, 2, 3};
  SimpleVector<int> copy(original);

  EXPECT_EQ(copy.size(), original.size());
  EXPECT_EQ(copy[0], 1);
  EXPECT_EQ(copy[1], 2);
  EXPECT_EQ(copy[2], 3);
}

TEST_F(SimpleVectorTest, MoveConstruction) {
  SimpleVector<int> original{1, 2, 3};
  SimpleVector<int> moved(std::move(original));

  EXPECT_EQ(moved.size(), 3);
  EXPECT_EQ(moved[0], 1);
  EXPECT_EQ(moved[1], 2);
  EXPECT_EQ(moved[2], 3);
  EXPECT_EQ(original.size(), 0);
}

TEST_F(SimpleVectorTest, CopyAssignment) {
  SimpleVector<int> vec1{1, 2, 3};
  SimpleVector<int> vec2;
  vec2 = vec1;

  EXPECT_EQ(vec2.size(), 3);
  EXPECT_EQ(vec2[0], 1);
  EXPECT_EQ(vec2[1], 2);
  EXPECT_EQ(vec2[2], 3);
}

TEST_F(SimpleVectorTest, MoveAssignment) {
  SimpleVector<int> vec1{1, 2, 3};
  SimpleVector<int> vec2;
  vec2 = std::move(vec1);

  EXPECT_EQ(vec2.size(), 3);
  EXPECT_EQ(vec2[0], 1);
  EXPECT_EQ(vec2[1], 2);
  EXPECT_EQ(vec2[2], 3);
  EXPECT_EQ(vec1.size(), 0);
}

TEST_F(SimpleVectorTest, ElementAccess) {
  SimpleVector<int> vec{1, 2, 3};

  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 2);
  EXPECT_EQ(vec[2], 3);

  EXPECT_EQ(vec.at(0), 1);
  EXPECT_EQ(vec.at(1), 2);
  EXPECT_EQ(vec.at(2), 3);
}

TEST_F(SimpleVectorTest, ConstElementAccess) {
  const SimpleVector<int> vec{1, 2, 3};

  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 2);
  EXPECT_EQ(vec[2], 3);

  EXPECT_EQ(vec.at(0), 1);
  EXPECT_EQ(vec.at(1), 2);
  EXPECT_EQ(vec.at(2), 3);
}

TEST_F(SimpleVectorTest, OutOfRangeAccess) {
  SimpleVector<int> vec{1, 2, 3};

  EXPECT_THROW(vec.at(3), std::out_of_range);
  EXPECT_THROW(vec.at(-1), std::out_of_range);
}

TEST_F(SimpleVectorTest, Iteration) {
  SimpleVector<int> vec{1, 2, 3, 4, 5};

  int sum = 0;
  for (auto &item : vec) {
    sum += item;
  }
  EXPECT_EQ(sum, 15);
}

TEST_F(SimpleVectorTest, ConstIteration) {
  const SimpleVector<int> vec{1, 2, 3, 4, 5};

  int sum = 0;
  for (const auto &item : vec) {
    sum += item;
  }
  EXPECT_EQ(sum, 15);
}

TEST_F(SimpleVectorTest, DataAccess) {
  SimpleVector<int> vec{1, 2, 3};

  int *data = vec.data();
  EXPECT_EQ(data[0], 1);
  EXPECT_EQ(data[1], 2);
  EXPECT_EQ(data[2], 3);
}

TEST_F(SimpleVectorTest, Clear) {
  SimpleVector<int> vec{1, 2, 3, 4, 5};

  EXPECT_EQ(vec.size(), 5);
  vec.clear();
  EXPECT_EQ(vec.size(), 0);
  EXPECT_TRUE(vec.empty());
}

TEST_F(SimpleVectorTest, Resize) {
  SimpleVector<int> vec;

  vec.resize(5);
  EXPECT_EQ(vec.size(), 5);

  vec.resize(3);
  EXPECT_EQ(vec.size(), 3);
}

TEST_F(SimpleVectorTest, StringVector) {
  SimpleVector<std::string> vec{"hello", "world", "test"};

  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], "hello");
  EXPECT_EQ(vec[1], "world");
  EXPECT_EQ(vec[2], "test");
}

TEST_F(SimpleVectorTest, LargeVector) {
  SimpleVector<int> vec;
  const size_t count = 10000;

  for (size_t i = 0; i < count; ++i) {
    vec.resize(i + 1);
    vec[i] = static_cast<int>(i);
  }

  EXPECT_EQ(vec.size(), count);
  for (size_t i = 0; i < count; ++i) {
    EXPECT_EQ(vec[i], static_cast<int>(i));
  }
}

TEST_F(SimpleVectorTest, SelfAssignment) {
  SimpleVector<int> vec{1, 2, 3};

  vec = vec; // Self assignment should be safe
  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 2);
  EXPECT_EQ(vec[2], 3);
}

TEST_F(SimpleVectorTest, MoveSelfAssignment) {
  SimpleVector<int> vec{1, 2, 3};

  vec = std::move(vec); // Self move assignment
  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 2);
  EXPECT_EQ(vec[2], 3);
}

TEST_F(SimpleVectorTest, EmptyVector) {
  SimpleVector<int> vec;

  EXPECT_EQ(vec.size(), 0);
  EXPECT_TRUE(vec.empty());
  EXPECT_EQ(vec.begin(), vec.end());
}

TEST_F(SimpleVectorTest, ZeroSizeConstruction) {
  SimpleVector<int> vec(0);
  EXPECT_EQ(vec.size(), 0);
  EXPECT_TRUE(vec.empty());
}