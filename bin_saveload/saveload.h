#include <iostream>
#include <map>
#include <string>
#include <vector>

// Serialization

template<typename T>
void
Serialize(T pod, std::ostream& out);

void
Serialize(const std::string& str, std::ostream& out);

template<typename T>
void
Serialize(const std::vector<T>& data, std::ostream& out);

template<typename T1, typename T2>
void
Serialize(const std::map<T1, T2>& data, std::ostream& out);

// Deserialization

template<typename T>
void
Deserialize(std::istream& in, T& pod);

void
Deserialize(std::istream& in, std::string& str);

template<typename T>
void
Deserialize(std::istream& in, std::vector<T>& data);

template<typename T1, typename T2>
void
Deserialize(std::istream& in, std::map<T1, T2>& data);
