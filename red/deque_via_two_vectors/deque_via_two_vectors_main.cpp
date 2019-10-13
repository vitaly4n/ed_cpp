#include "../test_runner.h"

#include "deque_via_two_vectors.cpp"

using namespace std;

void TestDequeViaTwoVectors() {
  Deque<int> deq;

  // check Deque::Empty() and Deque::Size()
  [](const auto& const_deq) {
    ASSERT_EQUAL(const_deq.Size(), 0);
    ASSERT_EQUAL(const_deq.Empty(), true);
  }(deq);

  // check Deque::PushFront() and Deque::PushBack()
  {
    deq.PushFront(20);

    ASSERT_EQUAL(deq.Front(), deq.Back());
    ASSERT_EQUAL(deq.Front(), 20);
    ASSERT_EQUAL(deq.Size(), 1);
    ASSERT_EQUAL(deq.Empty(), false);

    deq.PushBack(30);

    ASSERT_EQUAL(deq.Size(), 2);

    deq.PushFront(10);
    
    ASSERT_EQUAL(deq.Front(), deq.At(0));
    ASSERT_EQUAL(deq.At(0), deq[0]);
    ASSERT_EQUAL(deq.Front(), 10);

    ASSERT_EQUAL(deq.Back(), deq.At(2));
    ASSERT_EQUAL(deq.At(2), deq[2]);
    ASSERT_EQUAL(deq.Back(), 30);

    ASSERT_EQUAL(deq[1], 20);
  }
  // check non-const version of Deque::Front() and Deque::Back()
  {
    deq.Front() += 1;
    deq.Back() += 1;

    ASSERT_EQUAL(deq.Front(), 11);
    ASSERT_EQUAL(deq.Back(), 31);
  }
  // check non-const version of Deque::operator[]
  {
    deq[0] += 1;
    deq[2] += 1;

    ASSERT_EQUAL(deq.Front(), 12);
    ASSERT_EQUAL(deq.Back(), 32);
  }
  // check non-const version of Deque::At
  {
    deq.At(0) += 1;
    deq.At(2) += 1;

    ASSERT_EQUAL(deq.Front(), 13);
    ASSERT_EQUAL(deq.Back(), 33);
  }
  // check presence const version of methods
  [](const auto& const_deq) {
    ASSERT_EQUAL(const_deq.Front(), const_deq[0]);
    ASSERT_EQUAL(const_deq.Back(), const_deq.At(2));
  }(deq);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestDequeViaTwoVectors);
  return 0;
}
