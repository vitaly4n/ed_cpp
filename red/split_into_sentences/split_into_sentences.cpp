#include <algorithm>
#include <vector>

using namespace std;

template <typename Token>
using Sentence = vector<Token>;

template <typename Token>
vector<Sentence<Token>> SplitIntoSentences(vector<Token> tokens) {
  vector<Sentence<Token>> res;
  auto is_end = [](auto const& token) {
    return token.IsEndSentencePunctuation();
  };

  auto sentence_end = begin(tokens);
  do {
    auto const sentence_begin = sentence_end;
    sentence_end = find_if(sentence_begin, end(tokens), is_end);
    sentence_end = find_if_not(sentence_end, end(tokens), is_end);

    res.push_back(Sentence<Token>{make_move_iterator(sentence_begin),
                                  make_move_iterator(sentence_end)});
  } while (sentence_end != end(tokens));
  return res;
}
