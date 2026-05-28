#ifndef ARDUINOJSON_H
#define ARDUINOJSON_H
#include <string>
#include <map>

class JsonVariant {
    std::string _val;
public:
    JsonVariant(std::string v = "") : _val(v) {}
    operator const char*() const { return _val.c_str(); }
};

template<size_t size>
class StaticJsonDocument {
public:
    std::map<std::string, std::string> data;
    JsonVariant operator[](const char* key) { return JsonVariant(data[key]); }
};

struct DeserializationError {
    const char* f_str() { return ""; }
    operator bool() { return false; }
};

template<typename T>
DeserializationError deserializeJson(T& doc, std::string json) { return DeserializationError(); }
#endif
