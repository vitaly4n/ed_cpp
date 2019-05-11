#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

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
  using TimestampQuery = multimap<int, const Record*>;
  using KarmaQuery = multimap<int, const Record*>;
  using NameQuery = unordered_multimap<string_view, const Record*>;

  struct RecordData
  {
    RecordData(const Record& record)
      : record_{ record } {}

    Record record_;
    TimestampQuery::iterator timestamp_id_;
    KarmaQuery::iterator karma_id_;
    NameQuery::iterator name_id_;
  };

  using PrimaryIdQuery = unordered_map<string, RecordData>;

  PrimaryIdQuery primary_query_;
  TimestampQuery timestamp_query_;
  KarmaQuery karma_query_;
  NameQuery name_query_;
};

bool
Database::Put(const Record& record)
{
  auto emplace_res = primary_query_.emplace(record.id, record);
  if (emplace_res.second) {
    auto& record_data = emplace_res.first->second;
    auto& emplaced_record = record_data.record_;
    record_data.timestamp_id_ =
      timestamp_query_.emplace(emplaced_record.timestamp, &emplaced_record);
    record_data.karma_id_ =
      karma_query_.emplace(emplaced_record.karma, &emplaced_record);
    record_data.name_id_ =
      name_query_.emplace(emplaced_record.user, &emplaced_record); 
    return true;
  }
  return false;
}

const Record*
Database::GetById(const string& id) const
{
  auto it = primary_query_.find(id);
  return it != end(primary_query_) ? &it->second.record_ : nullptr;
}

bool
Database::Erase(const string& id)
{
  auto it = primary_query_.find(id);
  if (it != end(primary_query_)) {
    auto& record_data = it->second;
    timestamp_query_.erase(record_data.timestamp_id_);
    karma_query_.erase(record_data.karma_id_);
    name_query_.erase(record_data.name_id_);
    primary_query_.erase(it);
    return true;
  }
  return false;
}

template<typename Callback>
void
Database::RangeByTimestamp(int low, int high, Callback callback) const
{
  auto first = timestamp_query_.lower_bound(low);
  auto last = timestamp_query_.upper_bound(high);
  for (auto it = first; it != last; ++it) {
    if (!callback(*it->second))
      return;
  }
}

template<typename Callback>
void
Database::RangeByKarma(int low, int high, Callback callback) const
{
  auto first = karma_query_.lower_bound(low);
  auto last = karma_query_.upper_bound(high);
  for (auto it = first; it != last; ++it) {
    if (!callback(*it->second))
      return;
  }
}

template<typename Callback>
void
Database::AllByUser(const string& user, Callback callback) const
{
  auto eqrange = name_query_.equal_range(user);
  for (auto it = eqrange.first; it != eqrange.second; ++it)
  {
    if (!callback(*it->second))
      return;
  }
}
