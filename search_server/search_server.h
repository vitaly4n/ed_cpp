#pragma once

#include <istream>
#include <list>
#include <map>
#include <ostream>
#include <set>
#include <string>
#include <vector>

#include "concurrent_map.h"

using namespace std;

class InvertedIndex {
 public:
  InvertedIndex() : index_{THREAD_COUNT} {}
  InvertedIndex& operator=(InvertedIndex&& other) { 
    lock_guard<mutex> g{docs_mutex_};
    lock_guard<mutex> g_other{other.docs_mutex_};

    index_ = move(other.index_);
    docs_ = move(other.docs_);
    return *this;
  }

  void Add(string&& document, size_t docid);
  list<size_t> Lookup(string_view word);

 private:
  ConcurrentMap<string_view, list<size_t>> index_;
  list<string> docs_;

  mutex docs_mutex_;
};

class SearchServer {
 public:
  SearchServer() = default;
  explicit SearchServer(istream& document_input);
  void UpdateDocumentBase(istream& document_input);
  void AddQueriesStream(istream& query_input, ostream& search_results_output);

 private:
  InvertedIndex index;
};
