#include "hash_map.h"
#include <gtest/gtest.h>
#include <string>
#include <unordered_map>

using namespace turbokit;

class HashMapTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(HashMapTest, DefaultConstruction) {
  HashMap<int, std::string> map;
  EXPECT_EQ(map.size(), 0);
  EXPECT_TRUE(map.empty());
}

TEST_F(HashMapTest, InsertAndFind) {
  HashMap<int, std::string> map;
  map.insert(1, "one");
  map.insert(2, "two");
  map.insert(3, "three");

  EXPECT_EQ(map.size(), 3);

  auto it1 = map.find(1);
  EXPECT_NE(it1, map.end());
  EXPECT_EQ(it1->second, "one");

  auto it2 = map.find(2);
  EXPECT_NE(it2, map.end());
  EXPECT_EQ(it2->second, "two");

  auto it3 = map.find(3);
  EXPECT_NE(it3, map.end());
  EXPECT_EQ(it3->second, "three");
}

TEST_F(HashMapTest, InsertDuplicate) {
  HashMap<int, std::string> map;
  map.insert(1, "one");
  map.insert(1, "one_duplicate");

  EXPECT_EQ(map.size(), 1);
  auto it = map.find(1);
  EXPECT_NE(it, map.end());
  EXPECT_EQ(it->second, "one_duplicate"); // Should be overwritten
}

TEST_F(HashMapTest, FindNonExistent) {
  HashMap<int, std::string> map;
  map.insert(1, "one");

  auto it = map.find(999);
  EXPECT_EQ(it, map.end());
}

TEST_F(HashMapTest, Erase) {
  HashMap<int, std::string> map;
  map.insert(1, "one");
  map.insert(2, "two");
  map.insert(3, "three");

  EXPECT_EQ(map.size(), 3);

  map.remove(2);
  EXPECT_EQ(map.size(), 2);

  auto it = map.find(2);
  EXPECT_EQ(it, map.end());

  // Other elements should still be there
  EXPECT_NE(map.find(1), map.end());
  EXPECT_NE(map.find(3), map.end());
}

TEST_F(HashMapTest, EraseNonExistent) {
  HashMap<int, std::string> map;
  map.insert(1, "one");

  map.remove(999); // Should not crash
  EXPECT_EQ(map.size(), 1);
}

TEST_F(HashMapTest, Clear) {
  HashMap<int, std::string> map;
  map.insert(1, "one");
  map.insert(2, "two");
  map.insert(3, "three");

  EXPECT_EQ(map.size(), 3);
  map.clear();
  EXPECT_EQ(map.size(), 0);
  EXPECT_TRUE(map.empty());
}

TEST_F(HashMapTest, Iteration) {
  HashMap<int, std::string> map;
  map.insert(1, "one");
  map.insert(2, "two");
  map.insert(3, "three");

  std::unordered_map<int, std::string> expected;
  expected[1] = "one";
  expected[2] = "two";
  expected[3] = "three";

  std::unordered_map<int, std::string> found;
  for (const auto &pair : map) {
    found[pair.first] = pair.second;
  }

  EXPECT_EQ(found, expected);
}

TEST_F(HashMapTest, StringKeys) {
  HashMap<std::string, int> map;
  map.insert("one", 1);
  map.insert("two", 2);
  map.insert("three", 3);

  EXPECT_EQ(map.size(), 3);

  auto it = map.find("two");
  EXPECT_NE(it, map.end());
  EXPECT_EQ(it->second, 2);
}

TEST_F(HashMapTest, LargeMap) {
  HashMap<int, int> map;
  const size_t count = 1000;

  for (size_t i = 0; i < count; ++i) {
    map.insert(static_cast<int>(i), static_cast<int>(i * 2));
  }

  EXPECT_EQ(map.size(), count);

  for (size_t i = 0; i < count; ++i) {
    auto it = map.find(static_cast<int>(i));
    EXPECT_NE(it, map.end());
    EXPECT_EQ(it->second, static_cast<int>(i * 2));
  }
}

TEST_F(HashMapTest, CollisionHandling) {
  HashMap<int, std::string> map;

  // Force some collisions by using a small hash table
  // This tests the separate chaining implementation

  for (int i = 0; i < 100; ++i) {
    map.insert(i, "value" + std::to_string(i));
  }

  EXPECT_EQ(map.size(), 100);

  for (int i = 0; i < 100; ++i) {
    auto it = map.find(i);
    EXPECT_NE(it, map.end());
    EXPECT_EQ(it->second, "value" + std::to_string(i));
  }
}

TEST_F(HashMapTest, ConstIteration) {
  HashMap<int, std::string> map;
  map.insert(1, "one");
  map.insert(2, "two");
  map.insert(3, "three");

  const auto &const_map = map;
  std::unordered_map<int, std::string> found;
  for (const auto &pair : const_map) {
    found[pair.first] = pair.second;
  }

  EXPECT_EQ(found.size(), 3);
  EXPECT_EQ(found[1], "one");
  EXPECT_EQ(found[2], "two");
  EXPECT_EQ(found[3], "three");
}

TEST_F(HashMapTest, CopyConstruction) {
  HashMap<int, std::string> original;
  original.insert(1, "one");
  original.insert(2, "two");

  HashMap<int, std::string> copy(original);
  EXPECT_EQ(copy.size(), original.size());
  EXPECT_EQ(copy.find(1)->second, "one");
  EXPECT_EQ(copy.find(2)->second, "two");
}

TEST_F(HashMapTest, MoveConstruction) {
  HashMap<int, std::string> original;
  original.insert(1, "one");
  original.insert(2, "two");

  HashMap<int, std::string> moved(std::move(original));
  EXPECT_EQ(moved.size(), 2);
  EXPECT_EQ(original.size(), 0); // Moved from
}

TEST_F(HashMapTest, CopyAssignment) {
  HashMap<int, std::string> map1;
  map1.insert(1, "one");
  map1.insert(2, "two");

  HashMap<int, std::string> map2;
  map2.insert(3, "three");

  map2 = map1;
  EXPECT_EQ(map2.size(), 2);
  EXPECT_EQ(map2.find(1)->second, "one");
  EXPECT_EQ(map2.find(2)->second, "two");
  EXPECT_EQ(map2.find(3), map2.end()); // Should not exist
}

TEST_F(HashMapTest, MoveAssignment) {
  HashMap<int, std::string> map1;
  map1.insert(1, "one");
  map1.insert(2, "two");

  HashMap<int, std::string> map2;
  map2.insert(3, "three");

  map2 = std::move(map1);
  EXPECT_EQ(map2.size(), 2);
  EXPECT_EQ(map2.find(1)->second, "one");
  EXPECT_EQ(map2.find(2)->second, "two");
  EXPECT_EQ(map1.size(), 0); // Moved from
}

TEST_F(HashMapTest, SelfAssignment) {
  HashMap<int, std::string> map;
  map.insert(1, "one");
  map.insert(2, "two");

  map = map; // Self assignment
  EXPECT_EQ(map.size(), 2);
  EXPECT_EQ(map.find(1)->second, "one");
  EXPECT_EQ(map.find(2)->second, "two");
}

TEST_F(HashMapTest, ComplexValueType) {
  struct ComplexValue {
    int x, y;
    std::string name;

    bool operator==(const ComplexValue &other) const {
      return x == other.x && y == other.y && name == other.name;
    }
  };

  HashMap<int, ComplexValue> map;

  ComplexValue val1{10, 20, "test1"};
  ComplexValue val2{30, 40, "test2"};

  map.insert(1, val1);
  map.insert(2, val2);

  EXPECT_EQ(map.size(), 2);

  auto it1 = map.find(1);
  EXPECT_NE(it1, map.end());
  EXPECT_EQ(it1->second, val1);

  auto it2 = map.find(2);
  EXPECT_NE(it2, map.end());
  EXPECT_EQ(it2->second, val2);
}

TEST_F(HashMapTest, IteratorValidity) {
  HashMap<int, std::string> map;
  map.insert(1, "one");
  map.insert(2, "two");
  map.insert(3, "three");

  auto it = map.find(2);
  EXPECT_NE(it, map.end());
  EXPECT_EQ(it->first, 2);
  EXPECT_EQ(it->second, "two");

  // Test iterator increment
  ++it;
  EXPECT_NE(it, map.end());
}

TEST_F(HashMapTest, EmptyMapIteration) {
  HashMap<int, std::string> map;
  EXPECT_TRUE(map.empty());

  for (const auto &pair : map) {
    FAIL() << "Should not iterate over empty map";
  }
}