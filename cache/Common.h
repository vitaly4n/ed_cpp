#pragma once

#include <memory>
#include <string>

// РРЅС‚РµСЂС„РµР№СЃ, РїСЂРµРґСЃС‚Р°РІР»СЏСЋС‰РёР№ РєРЅРёРіСѓ
class IBook
{
public:
  virtual ~IBook() = default;

  // Р’РѕР·РІСЂР°С‰Р°РµС‚ РЅР°Р·РІР°РЅРёРµ РєРЅРёРіРё
  virtual const std::string& GetName() const = 0;

  // Р’РѕР·РІСЂР°С‰Р°РµС‚ С‚РµРєСЃС‚ РєРЅРёРіРё РєР°Рє СЃС‚СЂРѕРєСѓ.
  // Р Р°Р·РјРµСЂРѕРј РєРЅРёРіРё СЃС‡РёС‚Р°РµС‚СЃСЏ СЂР°Р·РјРµСЂ РµС‘
  // С‚РµРєСЃС‚Р° РІ Р±Р°Р№С‚Р°С….
  virtual const std::string& GetContent() const = 0;
};

// РРЅС‚РµСЂС„РµР№СЃ, РїРѕР·РІРѕР»СЏСЋС‰РёР№ СЂР°СЃРїР°РєРѕРІС‹РІР°С‚СЊ
// РєРЅРёРіРё
class IBooksUnpacker
{
public:
  virtual ~IBooksUnpacker() = default;

  // Р Р°СЃРїР°РєРѕРІС‹РІР°РµС‚ РєРЅРёРіСѓ СЃ СѓРєР°Р·Р°РЅРЅС‹Рј
  // РЅР°Р·РІР°РЅРёРµРј РёР· С…СЂР°РЅРёР»РёС‰Р°
  virtual std::unique_ptr<IBook> UnpackBook(const std::string& book_name) = 0;
};

// РРЅС‚РµСЂС„РµР№СЃ, РїСЂРµРґСЃС‚Р°РІР»СЏСЋС‰РёР№ РєСЌС€
class ICache
{
public:
  // РќР°СЃС‚СЂРѕР№РєРё РєСЌС€Р°
  struct Settings
  {
    // РњР°РєСЃРёРјР°Р»СЊРЅС‹Р№ РґРѕРїСѓСЃС‚РёРјС‹Р№ РѕР±СЉС‘Рј РїР°РјСЏС‚Рё,
    // РїРѕС‚СЂРµР±Р»СЏРµРјС‹Р№ Р·Р°РєСЌС€РёСЂРѕРІР°РЅРЅС‹РјРё
    // РѕР±СЉРµРєС‚Р°РјРё, РІ Р±Р°Р№С‚Р°С…
    size_t max_memory = 0;
  };

  using BookPtr = std::shared_ptr<const IBook>;

public:
  virtual ~ICache() = default;

  // Р’РѕР·РІСЂР°С‰Р°РµС‚ РєРЅРёРіСѓ СЃ Р·Р°РґР°РЅРЅС‹Рј РЅР°Р·РІР°РЅРёРµРј.
  // Р•СЃР»Рё РµС‘ РІ РґР°РЅРЅС‹Р№ РјРѕРјРµРЅС‚ РЅРµС‚ РІ РєСЌС€Рµ, С‚Рѕ
  // РїСЂРµРґРІР°СЂРёС‚РµР»СЊРЅРѕ СЃС‡РёС‚С‹РІР°РµС‚ РµС‘ Рё РґРѕР±Р°РІР»СЏРµС‚
  // РІ РєСЌС€. РЎР»РµРґРёС‚ Р·Р° С‚РµРј, С‡С‚РѕР±С‹ РѕР±С‰РёР№ РѕР±СЉС‘Рј
  // СЃС‡РёС‚Р°РЅРЅС‹С… РєРЅРёРі РЅРµ РїСЂРµРІРѕСЃС…РѕРґРёР»
  // СѓРєР°Р·Р°РЅРЅРѕРіРѕ РІ РїР°СЂР°РјРµС‚СЂРµ max_memory. РџСЂРё
  // РЅРµРѕР±С…РѕРґРёРјРѕСЃС‚Рё СѓРґР°Р»СЏРµС‚ РёР· РєСЌС€Р° РєРЅРёРіРё, Рє
  // РєРѕС‚РѕСЂС‹Рј РґРѕР»СЊС€Рµ РІСЃРµРіРѕ РЅРµ РѕР±СЂР°С‰Р°Р»РёСЃСЊ. Р•СЃР»Рё
  // СЂР°Р·РјРµСЂ СЃР°РјРѕР№ РєРЅРёРіРё СѓР¶Рµ Р±РѕР»СЊС€Рµ max_memory, С‚Рѕ
  // РѕСЃС‚Р°РІР»СЏРµС‚ РєСЌС€ РїСѓСЃС‚С‹Рј.
  virtual BookPtr GetBook(const std::string& book_name) = 0;
};

// РЎРѕР·РґР°С‘С‚ РѕР±СЉРµРєС‚ РєСЌС€Р° РґР»СЏ Р·Р°РґР°РЅРЅРѕРіРѕ
// СЂР°СЃРїР°РєРѕРІС‰РёРєР° Рё Р·Р°РґР°РЅРЅС‹С… РЅР°СЃС‚СЂРѕРµРє
std::unique_ptr<ICache>
MakeCache(std::shared_ptr<IBooksUnpacker> books_unpacker,
          const ICache::Settings& settings);
