#include "serialization.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace turbokit;

class SerializationTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

struct TestStruct {
  int x;
  std::string y;
  double z;

  template <typename X> void serialize(X &x) { x(this->x, y, z); }
};

struct ComplexStruct {
  std::vector<std::pair<int, std::string>> data;
  std::optional<std::string> name;

  template <typename X> void serialize(X &x) { x(data, name); }

  bool operator==(const ComplexStruct &other) const {
    return data == other.data && name == other.name;
  }
};

TEST_F(SerializationTest, BasicSerialization) {
  TestStruct data{42, "test", 3.14};

  auto buffer = serializeToBuffer(data);
  EXPECT_TRUE(buffer);
  EXPECT_GT(buffer->get_size(), 0);
}

TEST_F(SerializationTest, BasicDeserialization) {
  TestStruct original{42, "test", 3.14};

  auto buffer = serializeToBuffer(original);
  TestStruct result;
  deserializeBuffer(buffer, result);

  EXPECT_EQ(result.x, original.x);
  EXPECT_EQ(result.y, original.y);
  EXPECT_EQ(result.z, original.z);
}

TEST_F(SerializationTest, VectorSerialization) {
  std::vector<int> data{1, 2, 3, 4, 5};

  auto buffer = serializeToBuffer(data);
  EXPECT_TRUE(buffer);

  std::vector<int> result;
  deserializeBuffer(buffer, result);

  EXPECT_EQ(result, data);
}

TEST_F(SerializationTest, StringSerialization) {
  std::string data = "Hello, World!";

  auto buffer = serializeToBuffer(data);
  EXPECT_TRUE(buffer);

  std::string result;
  deserializeBuffer(buffer, result);

  EXPECT_EQ(result, data);
}

TEST_F(SerializationTest, MultipleValues) {
  int a = 42;
  std::string b = "test";
  double c = 3.14;

  auto buffer = serializeToBuffer(a, b, c);
  EXPECT_TRUE(buffer);

  int ra, rc;
  std::string rb;
  deserializeBuffer(buffer, ra, rb, rc);

  EXPECT_EQ(ra, a);
  EXPECT_EQ(rb, b);
  EXPECT_EQ(rc, c);
}

TEST_F(SerializationTest, OptionalSerialization) {
  std::optional<int> data1 = 42;
  std::optional<int> data2 = std::nullopt;

  auto buffer1 = serializeToBuffer(data1);
  auto buffer2 = serializeToBuffer(data2);

  std::optional<int> result1, result2;
  deserializeBuffer(buffer1, result1);
  deserializeBuffer(buffer2, result2);

  EXPECT_EQ(result1, data1);
  EXPECT_EQ(result2, data2);
}

TEST_F(SerializationTest, PairSerialization) {
  std::pair<int, std::string> data{42, "test"};

  auto buffer = serializeToBuffer(data);
  EXPECT_TRUE(buffer);

  std::pair<int, std::string> result;
  deserializeBuffer(buffer, result);

  EXPECT_EQ(result, data);
}

TEST_F(SerializationTest, VariantSerialization) {
  std::variant<int, std::string, double> data1 = 42;
  std::variant<int, std::string, double> data2 = "test";
  std::variant<int, std::string, double> data3 = 3.14;

  auto buffer1 = serializeToBuffer(data1);
  auto buffer2 = serializeToBuffer(data2);
  auto buffer3 = serializeToBuffer(data3);

  std::variant<int, std::string, double> result1, result2, result3;
  deserializeBuffer(buffer1, result1);
  deserializeBuffer(buffer2, result2);
  deserializeBuffer(buffer3, result3);

  EXPECT_EQ(result1, data1);
  EXPECT_EQ(result2, data2);
  EXPECT_EQ(result3, data3);
}

TEST_F(SerializationTest, TupleSerialization) {
  std::tuple<int, std::string, double> data{42, "test", 3.14};

  auto buffer = serializeToBuffer(data);
  EXPECT_TRUE(buffer);

  std::tuple<int, std::string, double> result;
  deserializeBuffer(buffer, result);

  EXPECT_EQ(result, data);
}

TEST_F(SerializationTest, ComplexNestedStructure) {
  ComplexStruct original;
  original.data = {{1, "one"}, {2, "two"}, {3, "three"}};
  original.name = "test";

  auto buffer = serializeToBuffer(original);
  EXPECT_TRUE(buffer);

  ComplexStruct result;
  deserializeBuffer(buffer, result);

  EXPECT_EQ(result, original);
}

TEST_F(SerializationTest, SerializationError) {
  // Test that SerializationError is defined
  EXPECT_THROW(throw SerializationError("test error"), SerializationError);
}

TEST_F(SerializationTest, BufferHandleValidity) {
  TestStruct data{42, "test", 3.14};

  auto buffer = serializeToBuffer(data);
  EXPECT_TRUE(buffer);
  EXPECT_GT(buffer->get_size(), 0);
}

TEST_F(SerializationTest, EmptyVector) {
  std::vector<int> data;

  auto buffer = serializeToBuffer(data);
  EXPECT_TRUE(buffer);

  std::vector<int> result;
  deserializeBuffer(buffer, result);

  EXPECT_EQ(result, data);
  EXPECT_TRUE(result.empty());
}

TEST_F(SerializationTest, LargeData) {
  std::vector<int> data(10000);
  for (size_t i = 0; i < data.size(); ++i) {
    data[i] = static_cast<int>(i);
  }

  auto buffer = serializeToBuffer(data);
  EXPECT_TRUE(buffer);
  EXPECT_GT(buffer->get_size(), 0);

  std::vector<int> result;
  deserializeBuffer(buffer, result);

  EXPECT_EQ(result, data);
}