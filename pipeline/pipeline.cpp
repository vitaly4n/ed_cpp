#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace std;

struct Email
{
  string from;
  string to;
  string body;
};

class Worker
{
public:
  virtual ~Worker() = default;
  virtual void Process(unique_ptr<Email> email) = 0;
  virtual void Run() { throw logic_error("Unimplemented"); }

protected:
  void PassOn(unique_ptr<Email> email) const
  {
    if (next_.get()) {
      next_->Process(move(email));
    }
  }

public:
  void SetNext(unique_ptr<Worker> next)
  {
    if (!next_.get()) {
      next_ = move(next);
    } else {
      next_->SetNext(move(next));
    }
  }

private:
  unique_ptr<Worker> next_;
};

class Reader : public Worker
{
public:
  Reader(istream& is)
    : is_{ is }
  {}

  void Run() override
  {
    while (is_) {
      auto email = make_unique<Email>();
      getline(is_, email->from);
      getline(is_, email->to);
      getline(is_, email->body);

      Process(move(email));
    }
  }

  void Process(unique_ptr<Email> email) override { PassOn(move(email)); }

private:
  istream& is_;
};

class Filter : public Worker
{
public:
  using Function = function<bool(const Email&)>;
  Filter(Function f)
    : f_{ move(f) }
  {}

  void Process(unique_ptr<Email> email) override
  {
    if (f_(*email)) {
      PassOn(move(email));
    }
  }

public:
  Function f_;
};

class Copier : public Worker
{
public:
  Copier(string duplicate_to)
    : duplicate_to_{ move(duplicate_to) }
  {}

  void Process(unique_ptr<Email> email) override
  {
    unique_ptr<Email> email_copy;
    if (duplicate_to_ != email->to) {
      email_copy = make_unique<Email>(*email);
      email_copy->to = duplicate_to_;
    }
    PassOn(move(email));
    if (email_copy.get()) {
      PassOn(move(email_copy));
    }
  }

private:
  string duplicate_to_;
};

class Sender : public Worker
{
public:
  Sender(ostream& os)
    : os_{ os }
  {}

  void Process(unique_ptr<Email> email) override
  {
    os_ << email->from << "\n" << email->to << "\n" << email->body << "\n";
    PassOn(move(email));
  }

private:
  ostream& os_;
};

class PipelineBuilder
{
public:
  explicit PipelineBuilder(istream& in) { chain_ = make_unique<Reader>(in); }

  PipelineBuilder& FilterBy(Filter::Function filter)
  {
    chain_->SetNext(make_unique<Filter>(move(filter)));
    return *this;
  }

  PipelineBuilder& CopyTo(string recipient)
  {
    chain_->SetNext(make_unique<Copier>(move(recipient)));
    return *this;
  }

  PipelineBuilder& Send(ostream& out)
  {
    chain_->SetNext(make_unique<Sender>(out));
    return *this;
  }

  unique_ptr<Worker> Build() { return move(chain_); }

private:
  unique_ptr<Worker> chain_;
};
