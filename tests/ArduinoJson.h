#ifndef ARDUINOJSON_H
#define ARDUINOJSON_H
#include <string>
#include <map>
#include <iostream>
#include <string.h>

class JsonVariant {
    std::string _val;
    bool _isNull;
public:
    JsonVariant(std::string v = "", bool isNull = false) : _val(v), _isNull(isNull) {}
    operator const char*() const {
        if (_isNull) return nullptr;
        return _val.c_str();
    }
};

template<size_t size>
class StaticJsonDocument {
public:
    std::map<std::string, std::string> data;
    JsonVariant operator[](const char* key) {
        std::string k(key);
        if (data.find(k) == data.end()) return JsonVariant("", true);
        return JsonVariant(data[k], false);
    }
};

struct DeserializationError {
    bool error;
    DeserializationError(bool e = false) : error(e) {}
    const char* f_str() { return "error"; }
    operator bool() { return error; }
};

template<typename T>
DeserializationError deserializeJson(T& doc, String json) {
    std::string s = json.c_str();
    if (s.find("\"version\":\"") != std::string::npos) {
        size_t start = s.find("\"version\":\"") + 11;
        size_t end = s.find("\"", start);
        doc.data["version"] = s.substr(start, end - start);
    }
    if (s.find("\"url\":\"") != std::string::npos) {
        size_t start = s.find("\"url\":\"") + 7;
        size_t end = s.find("\"", start);
        doc.data["url"] = s.substr(start, end - start);
    }
    return DeserializationError(false);
}
#endif
