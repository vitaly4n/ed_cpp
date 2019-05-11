## Ini file library

Implement library with the following interface:
```
using Section = unordered_map<string, string>;

class Document {
public:
  Section& AddSection(string name);
  const Section& GetSection(const string& name) const;
  size_t SectionCount() const;

private:
  unordered_map<string, Section> sections;
};

Document Load(istream& input);
```

Requirements:
- all functions and classes of the library should reside in Ini namespace
- the required interface should be declared in ini.h file
- it is guaranteed that all ini input files are correct
- a line in ini file can be either empty or without leading whitespaces