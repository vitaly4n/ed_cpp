## Comment server

You are implementing comment server with the following HTTP protocol:
* POST /add_user - add new user and return answer *200 OK* containing new user's id in its body
* POST /add_comment - retrieves user id and comment from request's body. If a user is recognized as spammer, returns *203 Found* answer with header _Location: /captcha_, which will lead the user to a page with captcha verification. Otherwise, the comment is saved and *200 OK* is returned
* GET /user_comments?user_id=[id] - returns *200 OK* which contains all comments of the user having the required id
* GET /captcha - returns *200 OK* which contains captcha's verification page
* POST /checkcaptcha - retrieve captcha answer from request's body. Returns *200 OK* if the answer is correct, returns *302 Found* with _Location: /captcha_ header otherwise.
* returns 404 *Not found* if the request is not supported

Web-server is implemented by means of *CommentServer* class:
```
struct HttpRequest {
  string method, path, body;
  map<string, string> get_params;
};

class CommentServer {
public:
  void ServeRequest(const HttpRequest& req, ostream& response_output);

private:
  ...
};
```
Where *CommentServer::ServeRequest* accepts HTTP-request, processes it and writes an answer to the *response_output* stream. The following format is used for an answer:
```
HTTP/1.1 [response code] [comment]
[header 1]: [value of Header 1]
...
[header N]: [value of Header N]
<empty string>
[response body]
```
* answer code: 200, 302, 404 etc
* comment: "OK", "Found", "Not found" etc
* header X: name of the header, i.e. "Location"
* response body: content of captcha's page or new user's id. if body is not empty there must be a header _Countnt-Length_ a value of which should equal to response length in bytes

For instance, here id a response to /add_user where a new user was successfully added and was given id with value 12:
```
HTTP/1.1 200 OK
Countent-Length: 2

12
```

However, there is a problem with our server - sometimes it doesn't respond on requests, and sometimes responses are ill-formed. The source of the problem is errorprone implementation of *ServeRequest* - responses are manually constructed every time:
```
void ServeRequest(const HttpRequest& req, ostream& os) {
  if (req.method == "POST") {
    if (req.path == "/add_user") {
      comments_.emplace_back();
      auto response = to_string(comments_.size() - 1);
      os << "HTTP/1.1 200 OK\n" << "Content-Length: " << response.size() << "\n" << "\n"
        << response;
    } else if (req.path == "/checkcaptcha") {
       ...
        os << "HTTP/1.1  20 OK\n\n";
      }
    } else {
      os << "HTTP/1.1 404 Not found\n\n";
    }
  ...
}
```

You decided to get rid of this problem. The following refactoring should be perfomed:
* implement *HttpResponse* class which will represent HTTP-response; its *operator<<* should take care of writing the response to an output stream
* *ServeRequest* should have the following signature - *HttpResponse ServeRequest(const HttpRequest& req)*
* writing a response to an output stream should be only where *ServeRequest* is called

Here is an interface of *HttpResponse*:
```
enum class HttpCode {
  Ok = 200,
  NotFound = 404,
  Found = 302,
};

class HttpResponse {
public:
  explicit HttpResponse(HttpCode code);

  HttpResponse& AddHeader(string name, string value);
  HttpResponse& SetContent(string a_content);
  HttpResponse& SetCode(HttpCode a_code);

  friend ostream& operator << (ostream& output, const HttpResponse& resp);
};
```