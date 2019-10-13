## Concurrent map

Concurrent map is a collection of maps keys of which do not intersect. Such collection would allow non-concurrent access to different areas of a 'unified' collection.

Interface:
```
template <typename K, typename V, typename Hash = std::hash<K>>
class ConcurrentMap {
public:
  using MapType = unordered_map<K, V, Hash>;

  struct WriteAccess {
    V& ref_to_value;
  };

  struct ReadAccess {
    const V& ref_to_value;
  };

  explicit ConcurrentMap(size_t bucket_count);

  WriteAccess operator[](const K& key);
  ReadAccess At(const K& key) const;

  bool Has(const K& key) const;

  MapType BuildOrdinaryMap() const;

private:
  Hash hasher;
};
```

* Constructor: accepts a number of parts by which the whole key space should be devided
* operator[]: the behavior should be the same as for an ordinary *unordered_map* - if a key is present in a dictionary it should return *WriteAccess* object encapsulating a reference to corresponding value; if a key is missing, a new value is added and *WriteAccess* encapsulating new value is returned
* At(): the behavior should be the same as for an ordinary *unordered_map* - if a key is present in a dictionary it should return *ReadAccess* encapsulating corresponding value; throws std::out_of_range exception otherwise
* BuildOrdinaryMap(): merges all parts of a collection to a single standard map and returns this map
* Hash: a template parameter used for keys hashing