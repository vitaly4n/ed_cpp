#include <algorithm>
#include <map>
#include <set>
#include <string>

using namespace std;

struct Record
{
  string id;
  string title;
  string user;
  int timestamp;
  int karma;

  bool operator==(const Record& other) const { return id == other.id; }
};

struct RecordIdLess
{
  using is_transparent = true_type;

  bool operator()(const Record& lhs, const Record& rhs) const
  {
    return lhs.id < rhs.id;
  }

  bool operator()(const string& lhs, const Record& rhs) const
  {
    return lhs < rhs.id;
  }

  bool operator()(const Record& lhs, const string& rhs) const
  {
    return lhs.id < rhs;
  }

};

class Database
{
public:
  bool Put(const Record& record);
  const Record* GetById(const string& id) const;
  bool Erase(const string& id);

  template<typename Callback>
  void RangeByTimestamp(int low, int high, Callback callback) const;

  template<typename Callback>
  void RangeByKarma(int low, int high, Callback callback) const;

  template<typename Callback>
  void AllByUser(const string& user, Callback callback) const;

private:
  set<Record, RecordIdLess> primary_;
  multimap<int, const Record*> secondary_timestamp_;
  multimap<int, const Record*> secondary_karma_;
  multimap<string_view, const Record*> secondary_user_;
};

bool
Database::Put(const Record& record)
{
  auto emplace_res = primary_.emplace(record);
  if (emplace_res.second) {
    const auto& inserted_record = *emplace_res.first;
    secondary_timestamp_.emplace(inserted_record.timestamp, &inserted_record);
    secondary_karma_.emplace(inserted_record.karma, &inserted_record);
    secondary_user_.emplace(inserted_record.user, &inserted_record);
    return true;
  }
  return false;
}

const Record*
Database::GetById(const string& id) const
{
  auto it = primary_.find(id);
  return it != end(primary_) ? it.operator->() : nullptr;
}

bool
Database::Erase(const string& id)
{
  auto it = primary_.find(id);
  if (it != end(primary_))
  {
    auto erase_secondary = [&id](const auto& key, auto& container) {
      const auto eqrange = container.equal_range(key);
      const auto it =
        find_if(eqrange.first, eqrange.second, [&id](const auto& d) {
          return d.second->id == id;
        });
      container.erase(it);
    };

    erase_secondary(it->timestamp, secondary_timestamp_);
    erase_secondary(it->karma, secondary_karma_);
    erase_secondary(string_view{ it->user }, secondary_user_);
    primary_.erase(it);
    return true;
  }
  return false;
}

template<typename Callback>
void
Database::RangeByTimestamp(int low, int high, Callback callback) const
{
  const auto first = secondary_timestamp_.lower_bound(low);
  const auto last = secondary_timestamp_.upper_bound(high);
  for (auto it = first; it != last; ++it) {
    if (!callback(*(it->second)))
      break;
  }
}

template<typename Callback>
void
Database::RangeByKarma(int low, int high, Callback callback) const
{
  const auto first = secondary_karma_.lower_bound(low);
  const auto last = secondary_karma_.upper_bound(high);
  for (auto it = first; it != last; ++it) {
    if (!callback(*(it->second)))
      break;
  }
}

template<typename Callback>
void
Database::AllByUser(const string& user, Callback callback) const
{
  auto eqrange = secondary_user_.equal_range(user);
  for (auto it = eqrange.first; it != eqrange.second; ++it) {
    if (!callback(*(it->second)))
      break;
  }
}

