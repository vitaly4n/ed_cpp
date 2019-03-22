#include "../test_runner.h"
#include "airline_ticket.h"

using namespace std;

#include "update_field.cpp"

void TestUpdate() {
  AirlineTicket t;
  t.price = 0;

  const map<string, string> updates1 = {
      {"departure_date", "2018-2-28"},
      {"departure_time", "17:40"},
  };
  UPDATE_FIELD(t, departure_date, updates1);
  UPDATE_FIELD(t, departure_time, updates1);
  UPDATE_FIELD(t, price, updates1);

  ASSERT_EQUAL(t.departure_date, (Date{2018, 2, 28}));
  ASSERT_EQUAL(t.departure_time, (Time{17, 40}));
  ASSERT_EQUAL(t.price, 0);

  const map<string, string> updates2 = {
      {"price", "12550"},
      {"arrival_time", "20:33"},
  };
  UPDATE_FIELD(t, departure_date, updates2);
  UPDATE_FIELD(t, departure_time, updates2);
  UPDATE_FIELD(t, arrival_time, updates2);
  UPDATE_FIELD(t, price, updates2);

  // updates2 РЅРµ СЃРѕРґРµСЂР¶РёС‚ РєР»СЋС‡РµР№ "departure_date" Рё "departure_time", РїРѕСЌС‚РѕРјСѓ
  // Р·РЅР°С‡РµРЅРёСЏ СЌС‚РёС… РїРѕР»РµР№ РЅРµ РґРѕР»Р¶РЅС‹ РёР·РјРµРЅРёС‚СЊСЃСЏ
  ASSERT_EQUAL(t.departure_date, (Date{2018, 2, 28}));
  ASSERT_EQUAL(t.departure_time, (Time{17, 40}));
  ASSERT_EQUAL(t.price, 12550);
  ASSERT_EQUAL(t.arrival_time, (Time{20, 33}));
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestUpdate);

  int a;
  cin >> a;
}
