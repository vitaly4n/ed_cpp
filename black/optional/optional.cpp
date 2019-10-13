#include "optional.h"

#include "test_runner.h"

using namespace std;

class C
{
public:
  inline static int created = 0;
  inline static int assigned = 0;
  inline static int deleted = 0;
  inline static int created_moving = 0;
  inline static int assigned_moving = 0;

  static void Reset() { created = assigned = deleted = created_moving = assigned_moving = 0; }

  C() { ++created; }
  C(const C&) { ++created; }
  C(C&&) { ++created_moving; }
  C& operator=(const C&)
  {
    ++assigned;
    return *this;
  }
  C& operator=(C&&)
  {
    ++assigned_moving;
    return *this;
  }

  ~C() { ++deleted; }
};

void
TestInit()
{
  {
    C::Reset();
    C c;
    Optional<C> o(c);
    ASSERT(C::created == 2 && C::assigned == 0 && C::deleted == 0);
  }
  ASSERT(C::deleted == 2);
};

void
TestAssign()
{
  Optional<C> o1, o2;

  { // Assign a Value to empty
    C::Reset();
    C c;
    o1 = c;
    ASSERT(C::created == 2 && C::assigned == 0 && C::deleted == 0);
  }
  { // Assign a non-empty to empty
    C::Reset();
    o2 = o1;
    ASSERT(C::created == 1 && C::assigned == 0 && C::deleted == 0);
  }
  { // Assign non-empty to non-empty
    C::Reset();
    o2 = o1;
    ASSERT(C::created == 0 && C::assigned == 1 && C::deleted == 0);
  }
}

void
TestReset()
{
  C::Reset();
  Optional<C> o = C();
  o.Reset();
  ASSERT(C::created == 1 && C::assigned == 0 && C::deleted == 2 && C::created_moving == 1);
}

void
TestMoveConstructor()
{
  C::Reset();
  Optional<C> o1 = C();
  Optional<C> o2(std::move(o1));
  ASSERT(C::created_moving == 2 && C::deleted == 1);
}

void
TestHasValue()
{
  Optional<int> o;
  ASSERT(!o.HasValue());

  o = 42;
  ASSERT(o.HasValue());

  o.Reset();
  ASSERT(!o.HasValue());
}

int
main()
{
  TestRunner tr;
  RUN_TEST(tr, TestInit);
  RUN_TEST(tr, TestAssign);
  RUN_TEST(tr, TestReset);
  RUN_TEST(tr, TestHasValue);
  RUN_TEST(tr, TestMoveConstructor);
  return 0;
}
