#include "intrusive_list.h"
#include <gtest/gtest.h>
#include <string>

using namespace turbokit;

class IntrusiveListTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

struct TestNode {
  int value;
  std::string name;
  IntrusiveListLink<TestNode> link;

  TestNode(int v = 0, const std::string &n = "") : value(v), name(n) {}

  bool operator==(const TestNode &other) const {
    return value == other.value && name == other.name;
  }
};

using TestList = IntrusiveList<TestNode, &TestNode::link>;

TEST_F(IntrusiveListTest, DefaultConstruction) {
  TestList list;
  EXPECT_TRUE(list.empty());
  EXPECT_EQ(list.size(), 0);
  EXPECT_EQ(list.begin(), list.end());
}

TEST_F(IntrusiveListTest, InsertAndIteration) {
  TestList list;

  TestNode node1(1, "one");
  TestNode node2(2, "two");
  TestNode node3(3, "three");

  list.insert(list.end(), node1);
  list.insert(list.end(), node2);
  list.insert(list.end(), node3);

  EXPECT_EQ(list.size(), 3);
  EXPECT_FALSE(list.empty());

  auto it = list.begin();
  EXPECT_EQ(it->value, 1);
  EXPECT_EQ(it->name, "one");
  ++it;
  EXPECT_EQ(it->value, 2);
  EXPECT_EQ(it->name, "two");
  ++it;
  EXPECT_EQ(it->value, 3);
  EXPECT_EQ(it->name, "three");
  ++it;
  EXPECT_EQ(it, list.end());
}

TEST_F(IntrusiveListTest, InsertAtPosition) {
  TestList list;

  TestNode node1(1, "one");
  TestNode node2(2, "two");
  TestNode node3(3, "three");

  list.insert(list.end(), node1);
  list.insert(list.end(), node3);

  // Insert in the middle
  auto it = list.begin();
  ++it; // Point to node3
  list.insert(it, node2);

  EXPECT_EQ(list.size(), 3);

  auto iter = list.begin();
  EXPECT_EQ(iter->value, 1);
  ++iter;
  EXPECT_EQ(iter->value, 2);
  ++iter;
  EXPECT_EQ(iter->value, 3);
}

TEST_F(IntrusiveListTest, Erase) {
  TestList list;

  TestNode node1(1, "one");
  TestNode node2(2, "two");
  TestNode node3(3, "three");

  list.insert(list.end(), node1);
  list.insert(list.end(), node2);
  list.insert(list.end(), node3);

  EXPECT_EQ(list.size(), 3);

  // Erase middle element
  auto it = list.begin();
  ++it;
  auto next_it = list.erase(it);

  EXPECT_EQ(list.size(), 2);
  EXPECT_EQ(next_it->value, 3);

  // Verify remaining elements
  it = list.begin();
  EXPECT_EQ(it->value, 1);
  ++it;
  EXPECT_EQ(it->value, 3);
  ++it;
  EXPECT_EQ(it, list.end());
}

TEST_F(IntrusiveListTest, EraseFirst) {
  TestList list;

  TestNode node1(1, "one");
  TestNode node2(2, "two");
  TestNode node3(3, "three");

  list.insert(list.end(), node1);
  list.insert(list.end(), node2);
  list.insert(list.end(), node3);

  auto it = list.erase(list.begin());
  EXPECT_EQ(list.size(), 2);
  EXPECT_EQ(it->value, 2);

  it = list.begin();
  EXPECT_EQ(it->value, 2);
  ++it;
  EXPECT_EQ(it->value, 3);
}

TEST_F(IntrusiveListTest, EraseLast) {
  TestList list;

  TestNode node1(1, "one");
  TestNode node2(2, "two");
  TestNode node3(3, "three");

  list.insert(list.end(), node1);
  list.insert(list.end(), node2);
  list.insert(list.end(), node3);

  auto it = list.begin();
  ++it;
  ++it; // Point to last element
  auto next_it = list.erase(it);
  EXPECT_EQ(list.size(), 2);
  EXPECT_EQ(next_it, list.end());

  it = list.begin();
  EXPECT_EQ(it->value, 1);
  ++it;
  EXPECT_EQ(it->value, 2);
  ++it;
  EXPECT_EQ(it, list.end());
}

TEST_F(IntrusiveListTest, Clear) {
  TestList list;

  TestNode node1(1, "one");
  TestNode node2(2, "two");
  TestNode node3(3, "three");

  list.insert(list.end(), node1);
  list.insert(list.end(), node2);
  list.insert(list.end(), node3);

  EXPECT_EQ(list.size(), 3);
  list.clear();
  EXPECT_EQ(list.size(), 0);
  EXPECT_TRUE(list.empty());
  EXPECT_EQ(list.begin(), list.end());
}

TEST_F(IntrusiveListTest, MoveConstruction) {
  TestList list1;

  TestNode node1(1, "one");
  TestNode node2(2, "two");

  list1.insert(list1.end(), node1);
  list1.insert(list1.end(), node2);

  TestList list2(std::move(list1));

  EXPECT_EQ(list2.size(), 2);
  EXPECT_EQ(list1.size(), 0);
  EXPECT_TRUE(list1.empty());

  auto it = list2.begin();
  EXPECT_EQ(it->value, 1);
  ++it;
  EXPECT_EQ(it->value, 2);
}

TEST_F(IntrusiveListTest, MoveAssignment) {
  TestList list1;
  TestList list2;

  TestNode node1(1, "one");
  TestNode node2(2, "two");

  list1.insert(list1.end(), node1);
  list1.insert(list1.end(), node2);

  list2 = std::move(list1);

  EXPECT_EQ(list2.size(), 2);
  EXPECT_EQ(list1.size(), 0);
  EXPECT_TRUE(list1.empty());

  auto it = list2.begin();
  EXPECT_EQ(it->value, 1);
  ++it;
  EXPECT_EQ(it->value, 2);
}

TEST_F(IntrusiveListTest, ReverseIteration) {
  TestList list;

  TestNode node1(1, "one");
  TestNode node2(2, "two");
  TestNode node3(3, "three");

  list.insert(list.end(), node1);
  list.insert(list.end(), node2);
  list.insert(list.end(), node3);

  auto it = list.end();
  --it;
  EXPECT_EQ(it->value, 3);
  --it;
  EXPECT_EQ(it->value, 2);
  --it;
  EXPECT_EQ(it->value, 1);
  EXPECT_EQ(it, list.begin());
}

TEST_F(IntrusiveListTest, ConstIteration) {
  TestList list;

  TestNode node1(1, "one");
  TestNode node2(2, "two");

  list.insert(list.end(), node1);
  list.insert(list.end(), node2);

  const TestList &const_list = list;

  auto it = const_list.begin();
  EXPECT_EQ(it->value, 1);
  ++it;
  EXPECT_EQ(it->value, 2);
  ++it;
  EXPECT_EQ(it, const_list.end());
}

TEST_F(IntrusiveListTest, EmptyListOperations) {
  TestList list;

  EXPECT_TRUE(list.empty());
  EXPECT_EQ(list.size(), 0);
  EXPECT_EQ(list.begin(), list.end());

  // Erase from empty list should be safe
  auto it = list.erase(list.begin());
  EXPECT_EQ(it, list.end());
}

TEST_F(IntrusiveListTest, SingleElementList) {
  TestList list;

  TestNode node(42, "test");
  list.insert(list.end(), node);

  EXPECT_EQ(list.size(), 1);
  EXPECT_FALSE(list.empty());

  auto it = list.begin();
  EXPECT_EQ(it->value, 42);
  EXPECT_EQ(it->name, "test");
  ++it;
  EXPECT_EQ(it, list.end());

  // Erase the only element
  it = list.begin();
  auto next_it = list.erase(it);
  EXPECT_EQ(list.size(), 0);
  EXPECT_TRUE(list.empty());
  EXPECT_EQ(next_it, list.end());
}

TEST_F(IntrusiveListTest, MultipleLinks) {
  struct MultiLinkNode {
    int value;
    IntrusiveListLink<MultiLinkNode> link1;
    IntrusiveListLink<MultiLinkNode> link2;

    MultiLinkNode(int v = 0) : value(v) {}
  };

  using MultiList1 = IntrusiveList<MultiLinkNode, &MultiLinkNode::link1>;
  using MultiList2 = IntrusiveList<MultiLinkNode, &MultiLinkNode::link2>;

  MultiLinkNode node1(1);
  MultiLinkNode node2(2);
  MultiLinkNode node3(3);

  MultiList1 list1;
  MultiList2 list2;

  // Insert into both lists
  list1.insert(list1.end(), node1);
  list1.insert(list1.end(), node2);
  list1.insert(list1.end(), node3);

  list2.insert(list2.end(), node3);
  list2.insert(list2.end(), node2);
  list2.insert(list2.end(), node1);

  EXPECT_EQ(list1.size(), 3);
  EXPECT_EQ(list2.size(), 3);

  // Verify list1 order
  auto it1 = list1.begin();
  EXPECT_EQ(it1->value, 1);
  ++it1;
  EXPECT_EQ(it1->value, 2);
  ++it1;
  EXPECT_EQ(it1->value, 3);

  // Verify list2 order
  auto it2 = list2.begin();
  EXPECT_EQ(it2->value, 3);
  ++it2;
  EXPECT_EQ(it2->value, 2);
  ++it2;
  EXPECT_EQ(it2->value, 1);
}

TEST_F(IntrusiveListTest, IteratorValidity) {
  TestList list;

  TestNode node1(1, "one");
  TestNode node2(2, "two");
  TestNode node3(3, "three");

  list.insert(list.end(), node1);
  list.insert(list.end(), node2);
  list.insert(list.end(), node3);

  auto it = list.begin();
  ++it; // Point to node2

  // Insert before the iterator
  TestNode node4(4, "four");
  list.insert(it, node4);

  // Iterator should still be valid and point to node2
  EXPECT_EQ(it->value, 2);

  // Verify the new order
  auto check_it = list.begin();
  EXPECT_EQ(check_it->value, 1);
  ++check_it;
  EXPECT_EQ(check_it->value, 4);
  ++check_it;
  EXPECT_EQ(check_it->value, 2);
  ++check_it;
  EXPECT_EQ(check_it->value, 3);
}

TEST_F(IntrusiveListTest, SelfAssignment) {
  TestList list;

  TestNode node1(1, "one");
  TestNode node2(2, "two");

  list.insert(list.end(), node1);
  list.insert(list.end(), node2);

  list = list; // Self assignment should be safe

  EXPECT_EQ(list.size(), 2);
  auto it = list.begin();
  EXPECT_EQ(it->value, 1);
  ++it;
  EXPECT_EQ(it->value, 2);
}

TEST_F(IntrusiveListTest, MoveSelfAssignment) {
  TestList list;

  TestNode node1(1, "one");
  TestNode node2(2, "two");

  list.insert(list.end(), node1);
  list.insert(list.end(), node2);

  list = std::move(list); // Self move assignment

  EXPECT_EQ(list.size(), 2);
  auto it = list.begin();
  EXPECT_EQ(it->value, 1);
  ++it;
  EXPECT_EQ(it->value, 2);
}

TEST_F(IntrusiveListTest, LargeList) {
  TestList list;
  const size_t count = 1000;

  for (size_t i = 0; i < count; ++i) {
    TestNode node(static_cast<int>(i), "node" + std::to_string(i));
    list.insert(list.end(), node);
  }

  EXPECT_EQ(list.size(), count);

  size_t index = 0;
  for (const auto &node : list) {
    EXPECT_EQ(node.value, static_cast<int>(index));
    EXPECT_EQ(node.name, "node" + std::to_string(index));
    ++index;
  }

  EXPECT_EQ(index, count);
}

TEST_F(IntrusiveListTest, IteratorComparison) {
  TestList list;

  TestNode node1(1, "one");
  TestNode node2(2, "two");

  list.insert(list.end(), node1);
  list.insert(list.end(), node2);

  auto it1 = list.begin();
  auto it2 = list.begin();

  EXPECT_EQ(it1, it2);
  ++it1;
  EXPECT_NE(it1, it2);
  ++it2;
  EXPECT_EQ(it1, it2);
}

TEST_F(IntrusiveListTest, IteratorPostIncrement) {
  TestList list;

  TestNode node1(1, "one");
  TestNode node2(2, "two");

  list.insert(list.end(), node1);
  list.insert(list.end(), node2);

  auto it = list.begin();
  auto old_it = it++;

  EXPECT_EQ(old_it->value, 1);
  EXPECT_EQ(it->value, 2);
}

TEST_F(IntrusiveListTest, IteratorPostDecrement) {
  TestList list;

  TestNode node1(1, "one");
  TestNode node2(2, "two");

  list.insert(list.end(), node1);
  list.insert(list.end(), node2);

  auto it = list.end();
  --it;
  auto old_it = it--;

  EXPECT_EQ(old_it->value, 2);
  EXPECT_EQ(it->value, 1);
}