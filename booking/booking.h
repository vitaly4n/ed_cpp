#include <utility>

// to guarantee compilation on the server
using namespace std;

namespace RAII {

template<typename T>
class Booking
{
public:
  Booking(T* obj, int counter)
    : obj_{ obj }
    , counter_{ counter }
  {}

  Booking(const Booking&) = delete;
  Booking& operator=(const Booking&) = delete;

  Booking(Booking&& booking) { 
    swap(obj_, booking.obj_);
    swap(counter_, booking.counter_);
  }

  Booking& operator=(Booking&& booking)
  {
    if (obj_) {
      obj_->CancelOrComplete(*this);
    }

    obj_ = booking.obj_;
    counter_ = booking.counter_;
    
    booking.obj_ = nullptr;
    booking.counter_ = 0;
  }

  ~Booking()
  {
    if (obj_) {
      obj_->CancelOrComplete(*this);
    }
  }

private:
  T* obj_ = nullptr;
  int counter_ = 0;
};

}
