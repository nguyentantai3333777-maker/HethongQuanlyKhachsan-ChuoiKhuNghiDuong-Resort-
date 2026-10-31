// Json.h - Bo tao chuoi JSON toi gian (khong dung thu vien ngoai)
// Vi du:  JsonObject o; o.set("ma", "P01").set("gia", 650000.0).set("trong", true);  o.str()
#pragma once
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <string>

inline std::string jsonEscape(const std::string &s)
{
    std::string kq = "\"";
    for (unsigned char c : s)
    {
        switch (c)
        {
        case '"':
            kq += "\\\"";
            break;
        case '\\':
            kq += "\\\\";
            break;
        case '\n':
            kq += "\\n";
            break;
        case '\r':
            kq += "\\r";
            break;
        case '\t':
            kq += "\\t";
            break;
        default:
            if (c < 0x20)
            {
                char buf[8];
                snprintf(buf, sizeof(buf), "\\u%04x", c);
                kq += buf;
            }
            else
                kq += (char)c;
        }
    }
    return kq + "\"";
}

inline std::string jsonSo(double v)
{
    std::ostringstream os;
    os << std::setprecision(15) << v;
    return os.str();
}

class JsonObject
{
private:
    std::string noiDung;
    void them(const std::string &k, const std::string &giaTriDaMaHoa)
    {
        if (!noiDung.empty())
            noiDung += ",";
        noiDung += jsonEscape(k) + ":" + giaTriDaMaHoa;
    }

public:
    JsonObject &set(const std::string &k, const std::string &v) { them(k, jsonEscape(v)); return *this; }
    JsonObject &set(const std::string &k, const char *v) { them(k, jsonEscape(v)); return *this; }
    JsonObject &set(const std::string &k, double v) { them(k, jsonSo(v)); return *this; }
    JsonObject &set(const std::string &k, int v) { them(k, std::to_string(v)); return *this; }
    JsonObject &set(const std::string &k, long long v) { them(k, std::to_string(v)); return *this; }
    JsonObject &set(const std::string &k, bool v) { them(k, v ? "true" : "false"); return *this; }
    JsonObject &setRaw(const std::string &k, const std::string &json) { them(k, json); return *this; }
    std::string str() const { return "{" + noiDung + "}"; }
};

class JsonArray
{
private:
    std::string noiDung;

public:
    JsonArray &pushRaw(const std::string &json)
    {
        if (!noiDung.empty())
            noiDung += ",";
        noiDung += json;
        return *this;
    }
    JsonArray &push(const JsonObject &o) { return pushRaw(o.str()); }
    JsonArray &push(const std::string &s) { return pushRaw(jsonEscape(s)); }
    JsonArray &push(double v) { return pushRaw(jsonSo(v)); }
    std::string str() const { return "[" + noiDung + "]"; }
};
