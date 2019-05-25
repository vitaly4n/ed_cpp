#include "Common.h"
#include "test_runner.h"

#include <atomic>
#include <future>
#include <numeric>
#include <random>
#include <sstream>

using namespace std;

// Р”Р°РЅРЅР°СЏ СЂРµР°Р»РёР·Р°С†РёСЏ РёРЅС‚РµСЂС„РµР№СЃР° IBook РїРѕР·РІРѕР»СЏРµС‚ РѕС‚СЃР»РµРґРёС‚СЊ РѕР±СЉС‘Рј РїР°РјСЏС‚Рё, РІ РґР°РЅРЅС‹Р№
// РјРѕРјРµРЅС‚ Р·Р°РЅРёРјР°РµРјС‹Р№ РІСЃРµРјРё Р·Р°РіСЂСѓР¶РµРЅРЅС‹РјРё РєРЅРёРіР°РјРё. Р”Р»СЏ С‚РµСЃС‚РёСЂРѕРІР°РЅРёСЏ СЃРІРѕРµР№
// РїСЂРѕРіСЂР°РјРјС‹ РІС‹ РјРѕР¶РµС‚Рµ РЅР°РїРёСЃР°С‚СЊ РґСЂСѓРіСѓСЋ СЂРµР°Р»РёР·Р°С†РёСЋ, РєРѕС‚РѕСЂР°СЏ РїРѕР·РІРѕР»РёС‚ С‚Р°РєР¶Рµ
// СѓР±РµРґРёС‚СЊСЃСЏ, С‡С‚Рѕ РёР· РєСЌС€Р° РІС‹РіСЂСѓР¶Р°СЋС‚СЃСЏ РІ РїРµСЂРІСѓСЋ РѕС‡РµСЂРµРґСЊ РЅР°РёРјРµРЅРµРµ РёСЃРїРѕР»СЊР·СѓРµРјС‹Рµ
// СЌР»РµРјРµРЅС‚С‹. РЎРѕР±СЃС‚РІРµРЅРЅРѕ, С‚РµСЃС‚РёСЂСѓСЋС‰Р°СЏ СЃРёСЃС‚РµРјР° РєСѓСЂСЃРµСЂС‹ РёРјРµРµС‚ РєР°Рє СЂР°Р· Р±РѕР»РµРµ
// РїСЂРѕРґРІРёРЅСѓС‚СѓСЋ СЂРµР°Р»РёР·Р°С†РёСЋ.
class Book : public IBook
{
public:
  Book(string name, string content, atomic<size_t>& memory_used_by_books)
    : name_(move(name))
    , content_(move(content))
    , memory_used_by_books_(memory_used_by_books)
  {
    memory_used_by_books_ += content_.size();
  }

  ~Book() { memory_used_by_books_ -= content_.size(); }

  const string& GetName() const override { return name_; }

  const string& GetContent() const override { return content_; }

private:
  string name_;
  string content_;
  atomic<size_t>& memory_used_by_books_;
};

// Р”Р°РЅРЅР°СЏ СЂРµР°Р»РёР·Р°С†РёСЏ РёРЅС‚РµСЂС„РµР№СЃР° IBooksUnpacker РїРѕР·РІРѕР»СЏРµС‚ РѕС‚СЃР»РµРґРёС‚СЊ РѕР±СЉС‘Рј РїР°РјСЏС‚Рё,
// РІ РґР°РЅРЅС‹Р№ РјРѕРјРµРЅС‚ Р·Р°РЅРёРјР°РµРјС‹Р№ РІСЃРµРјРё Р·Р°РіСЂСѓР¶РµРЅРЅС‹РјРё РєРЅРёРіР°РјРё Рё Р·Р°РїСЂРѕСЃРёС‚СЊ РєРѕР»РёС‡РµСЃС‚РІРѕ
// РѕР±СЂР°С‰РµРЅРёР№ Рє РјРµС‚РѕРґСѓ UnpackBook(). Р”Р»СЏ С‚РµСЃС‚РёСЂРѕРІР°РЅРёСЏ СЃРІРѕРµР№ РїСЂРѕРіСЂР°РјРјС‹ РІС‹ РјРѕР¶РµС‚Рµ
// РЅР°РїРёСЃР°С‚СЊ РґСЂСѓРіСѓСЋ СЂРµР°Р»РёР·Р°С†РёСЋ. РЎРѕР±СЃС‚РІРµРЅРЅРѕ, С‚РµСЃС‚РёСЂСѓСЋС‰Р°СЏ СЃРёСЃС‚РµРјР° РєСѓСЂСЃРµСЂС‹ РёРјРµРµС‚ РєР°Рє
// СЂР°Р· Р±РѕР»РµРµ РїСЂРѕРґРІРёРЅСѓС‚СѓСЋ СЂРµР°Р»РёР·Р°С†РёСЋ.
class BooksUnpacker : public IBooksUnpacker
{
public:
  unique_ptr<IBook> UnpackBook(const string& book_name) override
  {
    ++unpacked_books_count_;
    return make_unique<Book>(book_name,
                             "Dummy content of the book " + book_name,
                             memory_used_by_books_);
  }

  size_t GetMemoryUsedByBooks() const { return memory_used_by_books_; }

  int GetUnpackedBooksCount() const { return unpacked_books_count_; }

private:
  // РЁР°Р±Р»РѕРЅРЅС‹Р№ РєР»Р°СЃСЃ atomic РїРѕР·РІРѕР»СЏРµС‚ Р±РµР·РѕРїР°СЃРЅРѕ РёСЃРїРѕР»СЊР·РѕРІР°С‚СЊ СЃРєР°Р»СЏСЂРЅС‹Р№ С‚РёРї РёР·
  // РЅРµСЃРєРѕР»СЊРєРёС… РїРѕС‚РѕРєРѕРІ. Р’ РїСЂРѕС‚РёРІРЅРѕРј СЃР»СѓС‡Р°Рµ Сѓ РЅР°СЃ Р±С‹Р»Рѕ Р±С‹ СЃРѕСЃС‚РѕСЏРЅРёРµ РіРѕРЅРєРё.
  atomic<size_t> memory_used_by_books_ = 0;
  atomic<int> unpacked_books_count_ = 0;
};

struct Library
{
  vector<string> book_names;
  unordered_map<string, unique_ptr<IBook>> content;
  size_t size_in_bytes = 0;

  explicit Library(vector<string> a_book_names, IBooksUnpacker& unpacker)
    : book_names(std::move(a_book_names))
  {
    for (const auto& book_name : book_names) {
      auto& book_content = content[book_name];
      book_content = unpacker.UnpackBook(book_name);
      size_in_bytes += book_content->GetContent().size();
    }
  }
};

void
TestUnpacker(const Library& lib)
{
  BooksUnpacker unpacker;
  for (const auto& book_name : lib.book_names) {
    auto book = unpacker.UnpackBook(book_name);
    ASSERT_EQUAL(book->GetName(), book_name);
  }
}

void
TestMaxMemory(const Library& lib)
{
  auto unpacker = make_shared<BooksUnpacker>();
  ICache::Settings settings;
  settings.max_memory = lib.size_in_bytes / 2;
  auto cache = MakeCache(unpacker, settings);

  for (const auto& [name, book] : lib.content) {
    cache->GetBook(name);
    ASSERT(unpacker->GetMemoryUsedByBooks() <= settings.max_memory);
  }
}

void
TestCaching(const Library& lib)
{
  auto unpacker = make_shared<BooksUnpacker>();
  ICache::Settings settings;
  settings.max_memory = lib.size_in_bytes;
  auto cache = MakeCache(unpacker, settings);

  // Р•СЃР»Рё Р·Р°РїСЂР°С€РёРІР°С‚СЊ РѕРґРЅСѓ Рё С‚Сѓ Р¶Рµ РєРЅРёРіСѓ РїРѕРґСЂСЏРґ, С‚Рѕ РѕРЅР° РѕРїСЂРµРґРµР»С‘РЅРЅРѕ РґРѕР»Р¶РЅР°
  // РІРѕР·РІСЂР°С‰Р°С‚СЊСЃСЏ РёР· РєСЌС€Р°. Р—Р°РјРµС‚СЊС‚Рµ, С‡С‚Рѕ СЌС‚РѕРіРѕ РїСЂРѕСЃС‚РѕРіРѕ С‚РµСЃС‚Р° РІРѕРІСЃРµ
  // РЅРµРґРѕСЃС‚Р°С‚РѕС‡РЅРѕ, С‡С‚РѕР±С‹ РїРѕР»РЅРѕСЃС‚СЊСЋ РїСЂРѕРІРµСЂРёС‚СЊ РїСЂР°РІРёР»СЊРЅРѕСЃС‚СЊ СЂРµР°Р»РёР·Р°С†РёРё СЃС‚СЂР°С‚РµРіРёРё
  // Р·Р°РјРµС‰РµРЅРёСЏ СЌР»РµРјРµРЅС‚РѕРІ РІ РєСЌС€Рµ. Р”Р»СЏ СЌС‚РёС… С†РµР»РµР№ РјРѕР¶РµС‚Рµ РЅР°РїРёСЃР°С‚СЊ С‚РµСЃС‚
  // СЃР°РјРѕСЃС‚РѕСЏС‚РµР»СЊРЅРѕ.
  cache->GetBook(lib.book_names[0]);
  cache->GetBook(lib.book_names[0]);
  cache->GetBook(lib.book_names[0]);
  ASSERT_EQUAL(unpacker->GetUnpackedBooksCount(), 1);
}

void
TestSmallCache(const Library& lib)
{
  auto unpacker = make_shared<BooksUnpacker>();
  ICache::Settings settings;
  settings.max_memory =
    unpacker->UnpackBook(lib.book_names[0])->GetContent().size() - 1;
  auto cache = MakeCache(unpacker, settings);

  cache->GetBook(lib.book_names[0]);
  ASSERT_EQUAL(unpacker->GetMemoryUsedByBooks(), size_t(0));
}

void
TestAsync(const Library& lib)
{
  static const int tasks_count = 10;
  static const int trials_count = 10000;

  auto unpacker = make_shared<BooksUnpacker>();
  ICache::Settings settings;
  settings.max_memory = lib.size_in_bytes - 1;
  auto cache = MakeCache(unpacker, settings);

  vector<future<void>> tasks;

  for (int task_num = 0; task_num < tasks_count; ++task_num) {
    tasks.push_back(async([&cache, &lib, task_num] {
      default_random_engine gen;
      uniform_int_distribution<size_t> dis(0, lib.book_names.size() - 1);
      for (int i = 0; i < trials_count; ++i) {
        const auto& book_name = lib.book_names[dis(gen)];
        ASSERT_EQUAL(cache->GetBook(book_name)->GetContent(),
                     lib.content.find(book_name)->second->GetContent());
      }
      stringstream ss;
      ss << "Task #" << task_num << " completed\n";
      cout << ss.str();
    }));
  }

  // РІС‹Р·РѕРІ РјРµС‚РѕРґР° get РїСЂРѕР±СЂР°СЃС‹РІР°РµС‚ РёСЃРєР»СЋС‡РµРЅРёСЏ РІ РѕСЃРЅРѕРІРЅРѕР№ РїРѕС‚РѕРє
  for (auto& task : tasks) {
    task.get();
  }
}

int
main()
{
  BooksUnpacker unpacker;
  const Library lib(
    // РќР°Р·РІР°РЅРёСЏ РєРЅРёРі РґР»СЏ Р»РѕРєР°Р»СЊРЅРѕРіРѕ С‚РµСЃС‚РёСЂРѕРІР°РЅРёСЏ. Р’ С‚РµСЃС‚РёСЂСѓСЋС‰РµР№ СЃРёСЃС‚РµРјРµ РєСѓСЂСЃРµСЂС‹
    // Р±СѓРґРµС‚ РґСЂСѓРіРѕР№ РЅР°Р±РѕСЂ, РЅР°РјРЅРѕРіРѕ Р±РѕР»СЊС€Рµ.
    { "Sherlock Holmes",
      "Don Quixote",
      "Harry Potter",
      "A Tale of Two Cities",
      "The Lord of the Rings",
      "Le Petit Prince",
      "Alice in Wonderland",
      "Dream of the Red Chamber",
      "And Then There Were None",
      "The Hobbit" },
    unpacker);

#define RUN_CACHE_TEST(tr, f) tr.RunTest([&lib] { f(lib); }, #f)

  TestRunner tr;
  RUN_CACHE_TEST(tr, TestUnpacker);
  RUN_CACHE_TEST(tr, TestMaxMemory);
  RUN_CACHE_TEST(tr, TestCaching);
  RUN_CACHE_TEST(tr, TestSmallCache);
  RUN_CACHE_TEST(tr, TestAsync);

#undef RUN_CACHE_TEST
  return 0;
}
