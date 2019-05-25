#include "Common.h"

#include <list>
#include <mutex>
#include <unordered_map>

using namespace std;

class LruCache : public ICache
{
public:
  LruCache(shared_ptr<IBooksUnpacker> books_unpacker, const Settings& settings)
    : books_unpacker_{ books_unpacker }
    , max_memory_{ settings.max_memory }
  {}

  BookPtr GetBook(const string& book_name) override
  {
    lock_guard<mutex> lg{ mutex_ };

    auto cache_it = cache_.find(book_name);
    if (cache_it == end(cache_)) {
      auto book = books_unpacker_->UnpackBook(book_name);
      const auto book_size = book->GetContent().size();
      while ((max_memory_ < used_memory_ + book_size) && used_memory_ != 0) {
        auto oldest_storage_it = storage_.begin();
        auto oldest_cache_it = cache_.find((*oldest_storage_it)->GetName());

        cache_.erase(oldest_cache_it);
        used_memory_ -= (*oldest_storage_it)->GetContent().size();

        storage_.pop_front();
      }

      if (max_memory_ >= used_memory_ + book_size) {
        storage_.emplace_back(move(book));
        const auto storage_it = prev(end(storage_));
        cache_[book_name] = storage_it;
        used_memory_ += book_size;
        return *storage_it;
      } else {
        return book;
      }
    } else {
      auto storage_it = cache_it->second;
      storage_.splice(end(storage_), storage_, storage_it);
      return *storage_it;
    }
    return nullptr;
  }

private:
  shared_ptr<IBooksUnpacker> books_unpacker_;
  const size_t max_memory_ = 0;
  size_t used_memory_ = 0;

  using BooksStorage = list<BookPtr>;
  using StorageCache = unordered_map<string, BooksStorage::iterator>;

  BooksStorage storage_;
  StorageCache cache_;

  mutex mutex_;
};

unique_ptr<ICache>
MakeCache(shared_ptr<IBooksUnpacker> books_unpacker,
          const ICache::Settings& settings)
{
  return make_unique<LruCache>(move(books_unpacker), settings);
}
