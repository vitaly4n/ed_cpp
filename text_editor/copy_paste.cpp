#include <algorithm>
#include <cstdlib>
#include <list>
#include <string>

using namespace std;

class Editor {
 public:
  Editor() : cur_it_{begin(text_)} {}

  using Text = list<char>;

  void Left() {
    if (cur_it_ != begin(text_)) {
      cur_it_ = prev(cur_it_);
      --cur_pos_;
    }
  }
  void Right() {
    if (cur_it_ != end(text_)) {
      cur_it_ = next(cur_it_);
      ++cur_pos_;
    }
  }
  void Insert(char token) {
    text_.insert(cur_it_, token);
    ++cur_pos_;
  }

  void Cut(size_t tokens = 1) {
    auto chars_cut = min({tokens, text_.size() - cur_pos_});
    auto cut_to_it = cur_it_;

    advance(cut_to_it, chars_cut);
    buffer_ = string{cur_it_, cut_to_it};

    cur_it_ = text_.erase(cur_it_, cut_to_it);
  }
  void Copy(size_t tokens = 1) {
    auto chars_copy = min({tokens, text_.size() - cur_pos_});
    auto copy_to_it = cur_it_;

    advance(copy_to_it, chars_copy);
    buffer_ = string{cur_it_, copy_to_it};
  }

  void Paste() {
    text_.insert(cur_it_, begin(buffer_), end(buffer_));
    cur_pos_ += buffer_.size();
  }

  string GetText() const { return {begin(text_), end(text_)}; }

 private:
  Text text_;
  Text::iterator cur_it_;
  size_t cur_pos_ = 0;

  string buffer_;
};
