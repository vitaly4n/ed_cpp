#include <functional>
#include <iostream>
#include <memory>
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
  void SetNext(unique_ptr<Worker> next) { next_ = move(next); }

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
    // TODO
  }

  void Process(unique_ptr<Email> email) override
  {
    // TODO
  }

private:
  istream& is_;
};

class Filter : public Worker
{
public:
  using Function = function<bool(const Email&)>;
  Filter(Function f)
    : f_{ f }
  {}

  void Process(unique_ptr<Email> email) override
  {
    // TODO
  }

public:
  Function f_;
};

class Copier : public Worker
{
public:
  Copier(string duplicate_to)
    : duplicate_to_{ duplicate_to }
  {}

  void Process(unique_ptr<Email> email) override
  {
    // TODO
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
    // TODO
  }

private:
  ostream& os_;
};

class PipelineBuilder
{
public:
  explicit PipelineBuilder(istream& in)
  {
    // TODO
  }

  PipelineBuilder& FilterBy(Filter::Function filter)
  {
    // TODO
    return *this;
  }

  PipelineBuilder& CopyTo(string recipient)
  {
    // TODO
    return *this;
  }

  PipelineBuilder& Send(ostream& out)
  {
    // TODO
    return *this;
  }

  unique_ptr<Worker> Build()
  {
    // TODO
    return nullptr;
  }
};
