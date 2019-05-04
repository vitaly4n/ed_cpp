#pragma once

#include <algorithm>
#include <istream>
#include <list>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "synchronized_wrapper.h"

using namespace std;

struct SearchResultEntry {
  SearchResultEntry() = default;
  SearchResultEntry(size_t docid, size_t hitcount)
      : docid_{docid}, hitcount_{hitcount} {}

  size_t docid_ = 100'001;
  size_t hitcount_ = 0;

  bool operator<(SearchResultEntry const& other) const {
    if (hitcount_ > other.hitcount_) {
      return true;
    }
    if (hitcount_ < other.hitcount_) {
      return false;
    }
    if (docid_ < other.docid_) {
      return true;
    }
    return false;
  }
};

using SearchResultEntries = vector<SearchResultEntry>;

template <size_t page_size = 5>
class SearchResultWindow {
 public:
  using Window = array<SearchResultEntry, page_size>;
  using iterator = typename Window::iterator;
  using const_iterator = typename Window::const_iterator;

  iterator begin() { return window_.begin(); }
  const_iterator begin() const { return window_.begin(); }
  iterator end() { return window_.end(); }
  const_iterator end() const { return window_.end(); }

  size_t size() const { return page_size; }

  void add(SearchResultEntry const& entry) {
    if (entry < window_.back()) {
      auto it = find_if(begin(), end(), [&entry](auto const& window_entry) {
        return window_entry.docid_ == entry.docid_;
      });
      if (it == end()) {
        window_.back() = entry;
      } else {
        it->hitcount_ = entry.hitcount_;
      }

      sort(begin(), end());
    }
  }

 private:
  Window window_;
};

class InvertedIndex {
 public:
  void Add(string&& document);
  SearchResultEntries const& Lookup(string_view word) const;

  size_t docs_count() const { return docs_.size(); }

 private:
  map<string_view, SearchResultEntries> index_;
  list<string> docs_;
};

class SearchServer {
 public:
  SearchServer() = default;
  explicit SearchServer(istream& document_input);
  void UpdateDocumentBase(istream& document_input);
  void AddQueriesStream(istream& query_input,
                        ostream& search_results_output) const;

 private:
  Synchronized<InvertedIndex> index_sync_;
};
