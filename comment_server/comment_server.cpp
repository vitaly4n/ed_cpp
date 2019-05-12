#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std;

struct HttpRequest
{
  string method, path, body;
  map<string, string> get_params;
};

pair<string, string>
SplitBy(const string& what, const string& by)
{
  size_t pos = what.find(by);
  if (by.size() < what.size() && pos < what.size() - by.size()) {
    return { what.substr(0, pos), what.substr(pos + by.size()) };
  } else {
    return { what, {} };
  }
}

template<typename T>
T
FromString(const string& s)
{
  T x;
  istringstream is(s);
  is >> x;
  return x;
}

pair<size_t, string>
ParseIdAndContent(const string& body)
{
  auto [id_string, content] = SplitBy(body, " ");
  return { FromString<size_t>(id_string), content };
}

struct LastCommentInfo
{
  size_t user_id, consecutive_count;
};

class CommentServer
{
private:
  vector<vector<string>> comments_;
  std::optional<LastCommentInfo> last_comment;
  unordered_set<size_t> banned_users;

public:
  void ServeRequest(const HttpRequest& req, ostream& os)
  {
    if (req.method == "POST") {
      if (req.path == "/add_user") {
        comments_.emplace_back();
        auto response = to_string(comments_.size() - 1);
        os << "HTTP/1.1 200 OK\n"
           << "Content-Length: " << response.size() << "\n"
           << "\n"
           << response;
      } else if (req.path == "/add_comment") {
        auto [user_id, comment] = ParseIdAndContent(req.body);

        if (!last_comment || last_comment->user_id != user_id) {
          last_comment = LastCommentInfo{ user_id, 1 };
        } else if (++last_comment->consecutive_count > 3) {
          banned_users.insert(user_id);
        }

        if (banned_users.count(user_id) == 0) {
          comments_[user_id].push_back(string(comment));
          os << "HTTP/1.1 200 OK\n\n";
        } else {
          os << "HTTP/1.1 302 Found\n\n"
                "Location: /captcha\n"
                "\n";
        }
      } else if (req.path == "/checkcaptcha") {
        if (auto [id, response] = ParseIdAndContent(req.body);
            response == "42") {
          banned_users.erase(id);
          if (last_comment && last_comment->user_id == id) {
            last_comment.reset();
          }
          os << "HTTP/1.1 200 OK\n\n";
        }
      } else {
        os << "HTTP/1.1 404 Not found\n\n";
      }
    } else if (req.method == "GET") {
      if (req.path == "/user_comments") {
        auto user_id = FromString<size_t>(req.get_params.at("user_id"));
        string response;
        for (const string& c : comments_[user_id]) {
          response += c + '\n';
        }

        os << "HTTP/1.1 200 OK\n"
           << "Content-Length: " << response.size() << response;
      } else if (req.path == "/captcha") {
        os << "HTTP/1.1 200 OK\n"
           << "Content-Length: 80\n"
           << "\n"
           << "What's the answer for The Ultimate Question of Life, the "
              "Universe, and Everything?";
      } else {
        os << "HTTP/1.1 404 Not found\n\n";
      }
    }
  }
};
