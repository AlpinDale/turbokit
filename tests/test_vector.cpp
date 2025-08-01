#include "vector.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace turbokit;

class VectorTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(VectorTest, DefaultConstruction) {
  Vector<int> vec;
  EXPECT_EQ(vec.size(), 0);
  EXPECT_TRUE(vec.is_empty());
  EXPECT_EQ(vec.get_capacity(), 0);
}

TEST_F(VectorTest, SizeConstruction) {
  Vector<int> vec(10);
  EXPECT_EQ(vec.size(), 10);
  EXPECT_FALSE(vec.is_empty());
  EXPECT_GE(vec.get_capacity(), 10);
}

TEST_F(VectorTest, CopyConstruction) {
  Vector<int> original;
  original.append(1);
  original.append(2);
  original.append(3);

  Vector<int> copy(original);
  EXPECT_EQ(copy.size(), original.size());
  EXPECT_EQ(copy[0], 1);
  EXPECT_EQ(copy[1], 2);
  EXPECT_EQ(copy[2], 3);
}

TEST_F(VectorTest, MoveConstruction) {
  Vector<int> original;
  original.append(1);
  original.append(2);

  Vector<int> moved(std::move(original));
  EXPECT_EQ(moved.size(), 2);
  EXPECT_EQ(moved[0], 1);
  EXPECT_EQ(moved[1], 2);
  EXPECT_EQ(original.size(), 0); // Moved from
}

TEST_F(VectorTest, CopyAssignment) {
  Vector<int> vec1;
  vec1.append(1);
  vec1.append(2);

  Vector<int> vec2;
  vec2 = vec1;
  EXPECT_EQ(vec2.size(), 2);
  EXPECT_EQ(vec2[0], 1);
  EXPECT_EQ(vec2[1], 2);
}

TEST_F(VectorTest, MoveAssignment) {
  Vector<int> vec1;
  vec1.append(1);
  vec1.append(2);

  Vector<int> vec2;
  vec2 = std::move(vec1);
  EXPECT_EQ(vec2.size(), 2);
  EXPECT_EQ(vec2[0], 1);
  EXPECT_EQ(vec2[1], 2);
  EXPECT_EQ(vec1.size(), 0);
}

TEST_F(VectorTest, PushBack) {
  Vector<int> vec;
  vec.append(1);
  EXPECT_EQ(vec.size(), 1);
  EXPECT_EQ(vec[0], 1);

  vec.append(2);
  EXPECT_EQ(vec.size(), 2);
  EXPECT_EQ(vec[1], 2);
}

TEST_F(VectorTest, PushBackMove) {
  Vector<std::string> vec;
  std::string str = "test";
  vec.append(std::move(str));
  EXPECT_EQ(vec.size(), 1);
  EXPECT_EQ(vec[0], "test");
  EXPECT_TRUE(str.empty()); // Moved from
}

TEST_F(VectorTest, EmplaceBack) {
  Vector<std::string> vec;
  vec.emplace_back("test", 4);
  EXPECT_EQ(vec.size(), 1);
  EXPECT_EQ(vec[0], "test");
}

TEST_F(VectorTest, PopBack) {
  Vector<int> vec;
  vec.append(1);
  vec.append(2);
  vec.append(3);

  vec.remove_last();
  EXPECT_EQ(vec.size(), 2);
  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 2);
}

TEST_F(VectorTest, PopFront) {
  Vector<int> vec;
  vec.append(1);
  vec.append(2);
  vec.append(3);

  vec.remove_first();
  EXPECT_EQ(vec.size(), 2);
  EXPECT_EQ(vec[0], 2);
  EXPECT_EQ(vec[1], 3);
}

TEST_F(VectorTest, Insert) {
  Vector<int> vec;
  vec.append(1);
  vec.append(3);

  auto it = vec.insert_at(vec.get_begin() + 1, 2);
  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 2);
  EXPECT_EQ(vec[2], 3);
  EXPECT_EQ(*it, 2);
}

TEST_F(VectorTest, Clear) {
  Vector<int> vec;
  vec.append(1);
  vec.append(2);
  vec.append(3);

  EXPECT_EQ(vec.size(), 3);
  vec.clear();
  EXPECT_EQ(vec.size(), 0);
  EXPECT_TRUE(vec.is_empty());
}

TEST_F(VectorTest, Reserve) {
  Vector<int> vec;
  vec.reserve(100);
  EXPECT_GE(vec.get_capacity(), 100);
}

TEST_F(VectorTest, Resize) {
  Vector<int> vec;
  vec.resize(5);
  EXPECT_EQ(vec.size(), 5);
  EXPECT_EQ(vec[0], 0); // Default constructed

  vec.resize(3);
  EXPECT_EQ(vec.size(), 3);
}

TEST_F(VectorTest, ElementAccess) {
  Vector<int> vec;
  vec.append(1);
  vec.append(2);
  vec.append(3);

  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 2);
  EXPECT_EQ(vec[2], 3);

  EXPECT_EQ(vec.get_at(0), 1);
  EXPECT_EQ(vec.get_at(1), 2);
  EXPECT_EQ(vec.get_at(2), 3);

  // Test out of bounds access
  EXPECT_THROW(vec.get_at(3), std::out_of_range);
  EXPECT_THROW(vec.get_at(-1), std::out_of_range);
}

TEST_F(VectorTest, ElementAccessConst) {
  Vector<int> temp;
  temp.append(1);
  temp.append(2);
  temp.append(3);
  const Vector<int> vec = temp;
  EXPECT_EQ(vec.get_at(0), 1);
  EXPECT_EQ(vec.get_at(1), 2);
  EXPECT_EQ(vec.get_at(2), 3);
}

TEST_F(VectorTest, OutOfRangeAccess) {
  Vector<int> vec;
  vec.append(1);

  EXPECT_THROW(vec.get_at(1), std::out_of_range);
  EXPECT_THROW(vec.get_at(-1), std::out_of_range);
}

TEST_F(VectorTest, Iteration) {
  Vector<int> vec;
  vec.append(1);
  vec.append(2);
  vec.append(3);

  int sum = 0;
  for (auto &item : vec) {
    sum += item;
  }
  EXPECT_EQ(sum, 6);

  // Test with lambda
  sum = 0;
  std::for_each(vec.get_begin(), vec.get_end(),
                [&sum](int &item) { sum += item; });
  EXPECT_EQ(sum, 6);
}

TEST_F(VectorTest, ConstIteration) {
  Vector<int> temp;
  temp.append(1);
  temp.append(2);
  temp.append(3);
  const Vector<int> vec = temp;

  int sum = 0;
  for (const auto &item : vec) {
    sum += item;
  }
  EXPECT_EQ(sum, 6);
}

TEST_F(VectorTest, DataAccess) {
  Vector<int> vec;
  vec.append(1);
  vec.append(2);
  vec.append(3);

  int *data = vec.get_data();
  EXPECT_EQ(data[0], 1);
  EXPECT_EQ(data[1], 2);
  EXPECT_EQ(data[2], 3);
}

TEST_F(VectorTest, LargeVector) {
  Vector<int> vec;
  const size_t count = 10000;

  for (size_t i = 0; i < count; ++i) {
    vec.append(static_cast<int>(i));
  }

  EXPECT_EQ(vec.size(), count);
  for (size_t i = 0; i < count; ++i) {
    EXPECT_EQ(vec[i], static_cast<int>(i));
  }
}

TEST_F(VectorTest, StringVector) {
  Vector<std::string> vec;
  vec.append("hello");
  vec.append("world");
  vec.append("test");

  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], "hello");
  EXPECT_EQ(vec[1], "world");
  EXPECT_EQ(vec[2], "test");
}

TEST_F(VectorTest, ExpandCapacity) {
  Vector<int> vec;
  size_t initial_capacity = vec.get_capacity();

  // Add elements to trigger capacity expansion
  for (int i = 0; i < 100; ++i) {
    vec.append(i);
  }

  EXPECT_EQ(vec.size(), 100);
  EXPECT_GT(vec.get_capacity(), initial_capacity);
}

TEST_F(VectorTest, SelfAssignment) {
  Vector<int> vec;
  vec.append(1);
  vec.append(2);

  vec = vec; // Self assignment
  EXPECT_EQ(vec.size(), 2);
  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 2);
}

TEST_F(VectorTest, MoveSelfAssignment) {
  Vector<int> vec;
  vec.append(1);
  vec.append(2);

  vec = std::move(vec); // Self move assignment
  EXPECT_EQ(vec.size(), 2);
  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 2);
}