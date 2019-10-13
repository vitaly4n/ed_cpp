#pragma once

#include "booking.h"

#include <stdexcept>
#include <string>

using namespace std;

class FlightProvider
{
public:
  using BookingId = int;
  using Booking = RAII::Booking<FlightProvider>;
  friend Booking; // РЇРІРЅРѕ СЂР°Р·СЂРµС€Р°РµРј С„СѓРЅРєС†РёСЏРј РєР»Р°СЃСЃР° Booking РІС‹Р·С‹РІР°С‚СЊ
                  // private-С„СѓРЅРєС†РёРё РЅР°С€РµРіРѕ РєР»Р°СЃСЃР° FlightProvider

  struct BookingData
  {
    string city_from;
    string city_to;
    string date;
  };

  Booking Book(const BookingData& data)
  {
    if (counter >= capacity) {
      throw runtime_error("Flight overbooking");
    }
    ++counter;
    return { this, counter };
  }

private:
  // РЎРєСЂС‹РІР°РµРј СЌС‚Сѓ С„СѓРЅРєС†РёСЋ РІ private, С‡С‚РѕР±С‹ РµС‘ РјРѕРі РїРѕР·РІР°С‚СЊ С‚РѕР»СЊРєРѕ СЃРѕРѕС‚РІРµС‚СЃС‚РІСѓСЋС‰РёР№
  // friend-РєР»Р°СЃСЃ Booking
  void CancelOrComplete(const Booking& booking) { --counter; }

public:
  static int capacity;
  static int counter;
};

class HotelProvider
{
public:
  using BookingId = int;
  using Booking = RAII::Booking<HotelProvider>;
  friend Booking;

  struct BookingData
  {
    string city;
    string date_from;
    string date_to;
  };

  Booking Book(const BookingData& data)
  {
    if (counter >= capacity) {
      throw runtime_error("Hotel overbooking");
    }
    ++counter;
    return { this, counter };
  }

private:
  void CancelOrComplete(const Booking& booking) { --counter; }

public:
  static int capacity;
  static int counter;
};
