#include "search_server.h"

#include <algorithm>
#include <array>
#include <future>
#include <iostream>
#include <iterator>
#include <sstream>
#include <unordered_map>

#include "iterator_range.h"

vector<string_view> SplitIntoWords(string_view line) {
  vector<string_view> res;
  res.reserve(line.size() / 2);

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
  for (string line; getline(document_input, line);) {
    new_index.Add(move(line));
  }

  auto index_access = index_sync_.get();
  auto& index = index_access.ref_;
  index = move(new_index);
}

void SearchServer::AddQueriesStream(istream& query_input,
                                    ostream& search_results_output) const {
  vector<string> query_strings;
  for (string line; getline(query_input, line);) {
    query_strings.push_back(move(line));
  }

  using SearchResults = map<string_view, SearchResultWindow<5>>;
  
  auto index_access = index_sync_.get();
  auto const& index = index_access.ref_;

  SearchResults results;
  SearchResultEntries complete_query_result(index.docs_count());
  auto it_start = begin(query_strings);
  auto it_end = end(query_strings);
  auto queryid_start = 0;
  {
    for (auto it = it_start; it != it_end; ++it) {
      auto query_result_p = results.emplace(*it, SearchResultWindow{});
      if (!query_result_p.second) {
        continue;
      }

      fill(begin(complete_query_result), end(complete_query_result),
           SearchResultEntry{});

      auto& res_window = query_result_p.first->second;
      auto const words = SplitIntoWords(*it);
      for (auto const& word : words) {
        for (const auto& doc_counter : index.Lookup(word)) {
          auto& res_entry = complete_query_result[doc_counter.docid_];
          res_entry.docid_ = doc_counter.docid_;
          res_entry.hitcount_ += doc_counter.hitcount_;
          res_window.add(res_entry);
        }
      }
    }
  }

  for (int i = 0; i < query_strings.size(); ++i) {
    search_results_output << query_strings[i] << ":";
    auto const& search_result = results[query_strings[i]];
    for (auto const& entry : search_result) {
      if (entry.hitcount_ <= 0) {
        break;
      }
      search_results_output << " {"
                            << "docid: " << entry.docid_ << ", "
                            << "hitcount: " << entry.hitcount_ << '}';
    }
    search_results_output << '\n';
  }
}

void InvertedIndex::Add(string&& document) {
  docs_.push_back(move(document));
  const string_view last_word_added{docs_.back()};
  const size_t docid = docs_.size() - 1;

  map<string_view, size_t> doc_index;
  for (auto word : SplitIntoWords(last_word_added)) {
    doc_index[move(word)]++;
  }

  for (auto const& doc_index_pair : doc_index) {
    auto const& word = doc_index_pair.first;
    auto const& count = doc_index_pair.second;
    index_[word].emplace_back(docid, count);
  }
}

SearchResultEntries const& InvertedIndex::Lookup(string_view word) const {
  auto it = index_.find(word);
  if (it != end(index_)) {
    return it->second;
  } else {
    static SearchResultEntries const s_entries;
    return s_entries;
  }
}
