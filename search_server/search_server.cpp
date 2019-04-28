#include "search_server.h"

#include <algorithm>
#include <future>
#include <iostream>
#include <iterator>
#include <sstream>

#include "iterator_range.h"

vector<string_view> SplitIntoWords(string_view line) {
  vector<string_view> res;

  while (!line.empty()) {
    auto const word_start = line.find_first_not_of(' ');
    if (word_start == line.npos) {
      break;
    }
    auto const word_end = line.find(' ', word_start + 1);
    res.push_back(line.substr(word_start, word_end - word_start));
    line.remove_prefix(min({word_end, line.size()}));
  }
  return res;
}

SearchServer::SearchServer(istream& document_input) {
  UpdateDocumentBase(document_input);
}

void SearchServer::UpdateDocumentBase(istream& document_input) {
  InvertedIndex new_index;

  vector<string> input_strings;
  for (string line; getline(document_input, line);) {
    input_strings.push_back(move(line));
  }

  {
    vector<future<void>> futures;
    auto const threads = THREAD_COUNT;
    auto const block_size = input_strings.size() / threads + 1;

    for (auto it_start = begin(input_strings);
         it_start != end(input_strings);) {
      auto const offset =
          min({int(distance(it_start, end(input_strings))), int(block_size)});
      auto const docid_start = distance(begin(input_strings), it_start);
      auto const it_end = it_start + offset;

      futures.push_back(async([it_start, it_end, &new_index, docid_start] {
        size_t docid = docid_start;
        for (auto it = it_start; it != it_end; ++it) {
          new_index.Add(move(*it), docid);
          ++docid;
        }
      }));
      it_start = it_end;
    }
  }

  index = move(new_index);
}

void SearchServer::AddQueriesStream(istream& query_input,
                                    ostream& search_results_output) {
  vector<string> query_strings;
  for (string line; getline(query_input, line);) {
    query_strings.push_back(move(line));
  }

  using SearchResult = vector<pair<size_t, size_t>>;
  using SearchResults = vector<SearchResult>;
  SearchResults results{query_strings.size()};
  {
    vector<future<void>> futures;
    auto const threads = THREAD_COUNT;
    auto const block_size = query_strings.size() / threads + 1;

    for (auto it_start = begin(query_strings); it_start != end(query_strings);) {
      auto const offset = min({int(distance(it_start, end(query_strings))), int(block_size)});
      auto const queryid_start = distance(begin(query_strings), it_start);
      auto const it_end = it_start + offset;

      futures.push_back(async([it_start, it_end, queryid_start, &results, this] {
        for (auto it = it_start; it != it_end; ++it) {
          auto const queryid = queryid_start + distance(it_start, it);
          auto const words = SplitIntoWords(*it);
          map<size_t, size_t> docid_count;
          for (auto const& word : words) {
            for (size_t const docid : index.Lookup(word)) {
              docid_count[docid]++;
            }
          }
          SearchResult search_result(docid_count.begin(), docid_count.end());
          sort(begin(search_result), end(search_result),
               [](const auto& lhs, const auto& rhs) {
                 int64_t lhs_docid = lhs.first;
                 auto lhs_hit_count = lhs.second;
                 int64_t rhs_docid = rhs.first;
                 auto rhs_hit_count = rhs.second;
                 return make_pair(lhs_hit_count, -lhs_docid) >
                        make_pair(rhs_hit_count, -rhs_docid);
               });
          results[queryid] = move(search_result);
        }
      }));
      it_start = it_end;
    }
  }

  for (int i = 0; i < query_strings.size(); ++i) {
    search_results_output << query_strings[i] << ":";
    auto& search_result = results[i];
    for (auto [docid, hitcount] : Head(search_result, 5)) {
      search_results_output << " {"
                            << "docid: " << docid << ", "
                            << "hitcount: " << hitcount << '}';
    }
    search_results_output << '\n';
  }
}

void InvertedIndex::Add(string&& document, size_t docid) {
  docs_mutex_.lock();
  docs_.push_back(move(document));
  const string_view last_word_added{*docs_.rbegin()};
  docs_mutex_.unlock();

  for (const auto& word : SplitIntoWords(last_word_added)) {
    index_[word].ref_to_value.push_back(docid);
  }
}

list<size_t> InvertedIndex::Lookup(string_view word) {
  auto val = index_.find(word);
  if (val.has_value()) {
    return val->ref_to_value;
  } else {
    return {};
  }
}
