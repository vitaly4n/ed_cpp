#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std;

const char* captcha_ = "/captcha";
const char* captcha_question_ = "What's the answer for The Ultimate Question "
                                "of Life, the Universe, and Everything?";
const char* captcha_answer_ = "42";

const char* location_ = "Location";
const char* content_length_header = "Content-Length";

enum class HttpCode
{
  Ok = 200,
  NotFound = 404,
  Found = 302,
};

class HttpResponse
{
public:
  explicit HttpResponse(HttpCode code);

  HttpResponse& AddHeader(string name, string value);
  HttpResponse& SetContent(string a_content);
  HttpResponse& SetCode(HttpCode a_code);

  friend ostream& operator<<(ostream& output, const HttpResponse& resp);

private:
  HttpCode code_;
  string content_;
  unordered_multimap<string, string> headers_;
};

struct HttpRequest
{
  string method, path, body;
  map<string, string> get_params;
};

class CommentServer
{
public:
  HttpResponse ServeRequest(const HttpRequest& req);

private:
  HttpResponse& SetResponseContent(HttpResponse& resp, const string& str);

  HttpResponse PostAddUser();
  HttpResponse PostAddComment(size_t user_id, const string& comment);
  HttpResponse PostCheckCaptcha(size_t user_id, const string& user_answer);
  HttpResponse GetUserComments(size_t user_id);
  HttpResponse GetCaptcha();

private:
  struct LastCommentInfo
  {
    size_t user_id, consecutive_count;
  };

  vector<vector<string>> comments_;
  std::optional<LastCommentInfo> last_comment;
  unordered_set<size_t> banned_users;
};

////////////////////////////////////////////////////////////////////////////
/// HttpResponse impl

HttpResponse::HttpResponse(HttpCode code)
  : code_{ code }
{}

HttpResponse&
HttpResponse::AddHeader(string name, string value)
{
  headers_.emplace(move(name), move(value));
  return *this;
}

HttpResponse&
HttpResponse::SetContent(string content)
{
  content_ = move(content);
  return *this;
}

HttpResponse&
HttpResponse::SetCode(HttpCode code)
{
  code_ = code;
  return *this;
}

namespace {

string
get_code_string(HttpCode code)
{
  if (code == HttpCode::Ok)
    return "200 OK";
  else if (code == HttpCode::NotFound)
    return "404 Not found";
  else
    return "302 Found";
}

} // namespace

ostream&
operator<<(ostream& output, const HttpResponse& resp)
{
  output << "HTTP/1.1 " << get_code_string(resp.code_) << "\n";
  for (auto& header : resp.headers_) {
    output << header.first << ": " << header.second << "\n";
  }
  output << "\n" << resp.content_;
  return output;
}

////////////////////////////////////////////////////////////////////////////
/// CommentServer impl

namespace {

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

} // namespace

HttpResponse
CommentServer::ServeRequest(const HttpRequest& req)
{
  HttpResponse ret(HttpCode::NotFound);
  if (req.method == "POST") {
    if (req.path == "/add_user") {
      return PostAddUser();

    } else if (req.path == "/add_comment") {
      auto [user_id, comment] = ParseIdAndContent(req.body);
      return PostAddComment(user_id, comment);

    } else if (req.path == "/checkcaptcha") {
      auto [user_id, user_answer] = ParseIdAndContent(req.body);
      return PostCheckCaptcha(user_id, user_answer);
    }
  } else if (req.method == "GET") {
    if (req.path == "/user_comments") {
      auto user_id = FromString<size_t>(req.get_params.at("user_id"));
      return GetUserComments(user_id);

    } else if (req.path == "/captcha") {
      return GetCaptcha();
    }
  }
  return HttpResponse(HttpCode::NotFound);
}

HttpResponse&
CommentServer::SetResponseContent(HttpResponse& resp, const string& str)
{
  return resp.AddHeader(content_length_header, to_string(str.size()))
    .SetContent(str);
}

HttpResponse
CommentServer::PostAddUser()
{
  comments_.emplace_back();
  HttpResponse ret(HttpCode::Ok);
  return SetResponseContent(ret, to_string(comments_.size() - 1));
}

HttpResponse
CommentServer::PostAddComment(size_t user_id, const string& comment)
{
  if (!last_comment || last_comment->user_id != user_id) {
    last_comment = LastCommentInfo{ user_id, 1 };
  } else if (++last_comment->consecutive_count > 3) {
    banned_users.insert(user_id);
  }

  if (banned_users.count(user_id) == 0) {
    comments_[user_id].push_back(string(comment));
    return HttpResponse(HttpCode::Ok);
  } else {
    return HttpResponse(HttpCode::Found).AddHeader(location_, captcha_);
  }
}

HttpResponse
CommentServer::PostCheckCaptcha(size_t user_id, const string& user_answer)
{
  if (user_answer == captcha_answer_) {
    banned_users.erase(user_id);
    if (last_comment && last_comment->user_id == user_id) {
      last_comment.reset();
    }
    return HttpResponse(HttpCode::Ok);
  } else {
    return HttpResponse(HttpCode::Found).AddHeader(location_, captcha_);
  }
}

HttpResponse
CommentServer::GetUserComments(size_t user_id)
{
  string response;
  for (const string& c : comments_[user_id]) {
    response += c + '\n';
  }
  HttpResponse ret(HttpCode::Ok);
  return SetResponseContent(ret, response);
}

HttpResponse
CommentServer::GetCaptcha()
{
  HttpResponse ret(HttpCode::Ok);
  return SetResponseContent(ret, captcha_question_);
}
