#include "vector.h"

#include "test_runner.h"

using namespace std;

class C
{
public:
  inline static int created = 0;
  inline static int assigned = 0;
  inline static int deleted = 0;
  static void Reset() { created = assigned = deleted = 0; }

  C() { ++created; }
  C(const C& other) { ++created; }
  C& operator=(const C& other)
  {
    ++assigned;
    return *this;
  }
  ~C() { ++deleted; }
};

void
TestInit()
{
  {
    C::Reset();
    Vector<C> v(3);
    ASSERT(C::created == 3 && C::assigned == 0 && C::deleted == 0);
  }
  ASSERT(C::deleted == 3);
};

void
TestAssign()
{
  {
    C::Reset();
    Vector<C> v1(2), v2(3);
    ASSERT(C::created == 5 && C::assigned == 0 && C::deleted == 0);
    v1 = v2;
    ASSERT(C::created == 8 && C::assigned == 0 && C::deleted == 2);
    ASSERT(v1.Size() == 3 && v2.Size() == 3);
  }
  ASSERT(C::deleted == 8);

  {
    C::Reset();
    Vector<C> v1(3), v2(2);
    ASSERT(C::created == 5 && C::assigned == 0 && C::deleted == 0);
    v1 = v2;
    ASSERT(C::created == 5 && C::assigned == 2 && C::deleted == 1);
    ASSERT(v1.Size() == 2 && v2.Size() == 2);
  }
  ASSERT(C::deleted == 5);
}

void
TestPushBack()
{
  {
    C::Reset();
    Vector<C> v;
    C c;
    v.PushBack(c);
    ASSERT(C::created == 2 && C::assigned == 0 && C::deleted == 0);

    v.PushBack(c); // reallocation
    ASSERT(C::created == 4 && C::assigned == 0 && C::deleted == 1);
  }
  ASSERT(C::deleted == 4);
}

void
TestPopBack()
{
  {
    C::Reset();
    Vector<C> v(1);
    v.PopBack();
    ASSERT(C::created == 1 && C::assigned == 0 && C::deleted == 1 && v.Size() == 0);
  }
}

void
TestResize()
{
  C::Reset();
  Vector<C> v(2);
  v.Resize(0);
  ASSERT(C::created == 2 && C::assigned == 0 && C::deleted == 2 && v.Size() == 0);
  v.Resize(4);
  ASSERT(C::created == 6 && C::assigned == 0 && C::deleted == 2 && v.Size() == 4);
}

void
TestMoveAssignment()
{
  C::Reset();
  Vector<C> v1(2);
  Vector<C> v2(3);
  v2 = std::move(v1);
}

int
main()
{
  TestRunner tr;
  RUN_TEST(tr, TestInit);
  RUN_TEST(tr, TestAssign);
  RUN_TEST(tr, TestPushBack);
  RUN_TEST(tr, TestPopBack);
  RUN_TEST(tr, TestResize);
  RUN_TEST(tr, TestMoveAssignment);

  return 0;
}
