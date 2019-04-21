#include <algorithm>
#include <future>
#include <iostream>
#include <map>
#include <mutex>
#include <numeric>
#include <set>
#include <string>
#include <vector>

using namespace std;

struct Stats {
  void operator+=(const Stats& other) {
    for (const auto& stat : other.word_frequences) {
      word_frequences[stat.first] += stat.second;
    }
  }

  void add_stat(string&& word) { ++word_frequences[move(word)]; }

 public:
  map<string, int> word_frequences;
};

struct SaveStats {
  Stats stats_;
  mutex m_;

  void operator+=(const Stats& other) {
    lock_guard<mutex> l{m_};
    stats_ += other;
  }

  void add_stat(string&& word) {
    lock_guard<mutex> l{m_};
    stats_.add_stat(move(word));
  }
};

Stats ExploreLine(const set<string>& key_words, string line) {
  Stats res;

  auto is_delimiter = [](auto const& ch) { return ch == ' '; };

  auto word_start = find_if_not(begin(line), end(line), is_delimiter);
  for (; word_start != end(line);
       word_start = find_if_not(word_start, end(line), is_delimiter)) {
    auto const word_end = find_if(word_start, end(line), is_delimiter);
    string word{make_move_iterator(word_start), make_move_iterator(word_end)};

    auto key_word_it = key_words.find(word);
    if (key_word_it != end(key_words)) {
      res.add_stat(move(word));
    }
    word_start = word_end;
  }
  return res;
}

template <typename It>
Stats ExploreKeyWordsSingleThread(const set<string>& key_words, It first,
                                  It last) {
  Stats res;
  for (; first != last; ++first) {
    res += ExploreLine(key_words, move(*first));
  }
  return res;
}

Stats ExploreKeyWords(const set<string>& key_words, istream& input) {
  vector<string> input_strings;
  for (string line; getline(input, line);) {
    input_strings.push_back(move(line));
  }
  SaveStats result;
  {
    vector<future<void>> futures;
    auto const threads = 8;
    auto const block_size = input_strings.size() / threads + 1;

    for (auto it_start = begin(input_strings);
         it_start != end(input_strings);) {
      auto const offset =
          min({int(distance(it_start, end(input_strings))), int(block_size)});

      auto const it_end = it_start + offset;
      futures.push_back(async([it_start, it_end, &key_words, &result] {
        result += ExploreKeyWordsSingleThread(key_words, it_start, it_end);
      }));
      it_start = it_end;
    }
  }
  return result.stats_;
}
