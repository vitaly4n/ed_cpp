## Pipeline

Here you should implement a pipeline of email processors.
```
struct Email {
  string from;
  string to;
  string body;
};
```

All processors should be derived from an abstract class *Worker*:
```
class Worker {
public:
  virtual void Process(unique_ptr<Email> email) = 0;
  virtual void Run() {
    // only the first worker neets to implement this method
    throw logic_error("Unimplemented");
  }

protected:
  // implementations should call PassOn to pass the object further down a chain
  void PassOn(unique_ptr<Email> email) const;

public:
  void SetNext(unique_ptr<Worker> worker);
};
```
Requirements for *Worker*:
* *SetNext* should claim ownership of the next processor from a chain
* *PassOn* should call *Process* method of the next processor, if there is one

A list of required processors:
Reader:
* Constructor accepts *istream&* object to read emails
* *Run* should read emails from the stream and pass each of them further down a chain. Each email consists of three strings, each of them are on a separate line
Filter:
* Constructor accepts a predicate of type function<bool(const Email&)>
* *Proces* passes further only emails verified by the filter
Copier:
* Constructor accepts a string which represents an address of a reciever
* *Process* passes 

```
class PipelineBuilder {
public:
  // adds Reader as the first processor
  explicit PipelineBuilder(istream& in);

  // add new Filter processor
  PipelineBuilder& FilterBy(Filter::Function filter);

  // add new Copier processor
  PipelineBuilder& CopyTo(string recipient);

  // add new Sender processor
  PipelineBuilder& Send(ostream& out);

  // build a chain by means of Worker::SetNext method and returns the first processor in a built chain
  unique_ptr<Worker> Build();
};
```