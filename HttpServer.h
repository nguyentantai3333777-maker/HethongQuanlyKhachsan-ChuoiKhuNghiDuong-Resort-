// HttpServer.h - HTTP server toi gian bang socket (Winsock tren Windows, POSIX tren Linux/macOS)
// Chi lang nghe 127.0.0.1 (may cua minh), xu ly tuan tu tung request -> de hieu, khong can khoa.
#pragma once
#include <atomic>
#include <functional>
#include <map>
#include <string>

struct HttpRequest
{
    std::string method; // GET / POST
    std::string path;   // /api/phong
    std::map<std::string, std::string> headers; // ten header viet thuong
    std::map<std::string, std::string> params;  // gop query string + body dang form (a=1&b=2)
    std::string param(const std::string &k, const std::string &macDinh = "") const
    {
        auto it = params.find(k);
        return it == params.end() ? macDinh : it->second;
    }
    std::string header(const std::string &k) const
    {
        auto it = headers.find(k);
        return it == headers.end() ? "" : it->second;
    }
};

struct HttpResponse
{
    int status = 200;
    std::string contentType = "application/json; charset=utf-8";
    std::string body;
};

class HttpServer
{
public:
    using XuLy = std::function<HttpResponse(const HttpRequest &)>;

    HttpServer();
    ~HttpServer();
    // Thu mo cong tu congDau den congDau+10, tra ve cong da mo (nem loi neu that bai)
    int batDau(int congDau);
    // Vong lap nhan request cho den khi dung = true (kiem tra moi 0.3 giay)
    void vongLap(const std::atomic<bool> &dung, const XuLy &xuLy);
    void dong();

    static std::string giaiMaUrl(const std::string &s);
    static void tachThamSo(const std::string &s, std::map<std::string, std::string> &kq);

private:
    long long sock; // SOCKET (Windows) hoac int (POSIX), luu chung kieu so
    bool daKhoiTaoWinsock;
    void xuLyKetNoi(long long client, const XuLy &xuLy);
};
