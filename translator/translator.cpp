#include <map>
#include <set>
#include <string>
#include <string_view>

using namespace std;

class Translator {
 public:
  void Add(string_view source, string_view target) {
    auto source_emplace_res = words_.emplace(source);
    auto target_emplace_res = words_.emplace(target);

    string_view source_view{*source_emplace_res.first};
    string_view target_view{*target_emplace_res.first};

    forward_dict_[source_view] = target_view;
    backward_dict_[target_view] = source_view;
  }
  
  string_view TranslateForward(string_view source) const {
    auto forward_find_it = forward_dict_.find(source);
    return forward_find_it == end(forward_dict_) ? string_view{}
                                                 : forward_find_it->second;
  }

  string_view TranslateBackward(string_view target) const {
    auto backward_find_it = backward_dict_.find(target);
    return backward_find_it == end(backward_dict_) ? string_view{}
                                                   : backward_find_it->second;
  }

 private:
  set<string> words_;
  map<string_view, string_view> forward_dict_;
  map<string_view, string_view> backward_dict_;
};
