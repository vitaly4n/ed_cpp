#include <algorithm>
#include <forward_list>
#include <vector>

using namespace std;

template<typename Type, typename Hasher>
class HashSet
{
public:
  using BucketList = forward_list<Type>;
  using Buckets = vector<BucketList>;

public:
  explicit HashSet(size_t num_buckets, const Hasher& hasher = {})
    : buckets_{ num_buckets }
    , hasher_{ hasher }
  {}

  void Add(const Type& value)
  {
    auto& bucket = GetBucketImpl(value);
    if (find(begin(bucket), end(bucket), value) == end(bucket)) {
      bucket.push_front(value);
    }
  }

  bool Has(const Type& value) const
  {
    const auto& bucket = GetBucketImpl(value);
    return find(begin(bucket), end(bucket), value) != end(bucket);
  }

  void Erase(const Type& value)
  {
    auto& bucket = GetBucketImpl(value);
    bucket.remove(value);
  }

  const BucketList& GetBucket(const Type& value) const { return GetBucketImpl(value); }

private:
  BucketList& GetBucketImpl(const Type& value) { return GetBucket(hasher_(value)); }
  const BucketList& GetBucketImpl(const Type& value) const { return GetBucket(hasher_(value)); }

  BucketList& GetBucket(size_t hash) { return buckets_[hash % buckets_.size()]; }
  const BucketList& GetBucket(size_t hash) const { return buckets_[hash % buckets_.size()]; }

private:
  Buckets buckets_;
  Hasher hasher_;
};
