//#include "old_trip_manager.h"  // СЃРѕ СЃС‚Р°СЂС‹РјРё РєР»Р°СЃСЃР°РјРё РІСЃРµ С‚РµСЃС‚С‹ РїСЂРѕС…РѕРґСЏС‚
#include "new_trip_manager.h"

#include "test_runner.h"

#include <stdexcept>

using namespace std;

// Р­С‚Рё РѕРїСЂРµРґРµР»РµРЅРёСЏ СЃС‚Р°С‚РёС‡РµСЃРєРёС… РїРµСЂРµРјРµРЅРЅС‹С… РїСЂР°РІРёР»СЊРЅРµРµ Р±С‹Р»Рѕ Р±С‹ РїРѕРјРµСЃС‚РёС‚СЊ РІ
// СЃРѕРѕС‚РІРµС‚СЃС‚РІСѓСЋС‰РёР№ cpp-С„Р°Р№Р», РЅРѕ РјС‹ РґР»СЏ РїСЂРѕСЃС‚РѕС‚С‹ СЂР°Р·РјРµСЃС‚РёРј РёС… Р·РґРµСЃСЊ

int FlightProvider::capacity = 0;
int FlightProvider::counter = 0;

int HotelProvider::capacity = 0;
int HotelProvider::counter = 0;

void
TestNoOverbooking()
{
  FlightProvider::capacity = 100;
  HotelProvider::capacity = 100;
  FlightProvider::counter = 0;
  HotelProvider::counter = 0;
  {
    TripManager tm;
    auto trip = tm.Book({});
  }
  ASSERT_EQUAL(FlightProvider::counter, 0);
  ASSERT_EQUAL(HotelProvider::counter, 0);
}

void
TestFlightOverbooking()
{
  FlightProvider::capacity = 1;
  HotelProvider::capacity = 100;
  FlightProvider::counter = 0;
  HotelProvider::counter = 0;
  try {
    TripManager tm;
    auto trip = tm.Book({});
  } catch (const runtime_error&) {
    ASSERT_EQUAL(FlightProvider::counter, 0);
    ASSERT_EQUAL(HotelProvider::counter, 0);
    return;
  }
  Assert(false, "Flight overbooking was expected");
}

void
TestHotelOverbooking()
{
  FlightProvider::capacity = 100;
  HotelProvider::capacity = 0;
  FlightProvider::counter = 0;
  HotelProvider::counter = 0;
  try {
    TripManager tm;
    auto trip = tm.Book({});
  } catch (const runtime_error& ex) {
    ASSERT_EQUAL(FlightProvider::counter, 0);
    ASSERT_EQUAL(HotelProvider::counter, 0);
    return;
  }
  Assert(false, "Hotel overbooking was expected");
}

int
main()
{
  TestRunner tr;
  RUN_TEST(tr, TestNoOverbooking);
  RUN_TEST(tr, TestFlightOverbooking);
  RUN_TEST(tr, TestHotelOverbooking);

  int i;
  cin >> i;

  return 0;
}
