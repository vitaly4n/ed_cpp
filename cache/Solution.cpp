#include "Common.h"

using namespace std;

class LruCache : public ICache
{
public:
  LruCache(shared_ptr<IBooksUnpacker> books_unpacker, const Settings& settings)
  {
    // СЂРµР°Р»РёР·СѓР№С‚Рµ РјРµС‚РѕРґ
  }

  BookPtr GetBook(const string& book_name) override
  {
    // СЂРµР°Р»РёР·СѓР№С‚Рµ РјРµС‚РѕРґ
  }
};

unique_ptr<ICache>
MakeCache(shared_ptr<IBooksUnpacker> books_unpacker,
          const ICache::Settings& settings)
{
  return nullptr;
}
