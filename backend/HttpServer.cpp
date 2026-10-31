// HttpServer.cpp - Cai dat HTTP server toi gian
#include "HttpServer.h"
#include <cctype>
#include <cstring>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET SocketT;
#define DONG_SOCKET closesocket
#define CO_GUI 0
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
typedef int SocketT;
#define DONG_SOCKET close
#define CO_GUI MSG_NOSIGNAL // tranh SIGPIPE khi trinh duyet dong ket noi som
#endif

using namespace std;

static SocketT S(long long v) { return (SocketT)v; }

HttpServer::HttpServer() : sock(-1), daKhoiTaoWinsock(false)
{
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) == 0)
        daKhoiTaoWinsock = true;
#endif
}

HttpServer::~HttpServer()
{
    dong();
#ifdef _WIN32
    if (daKhoiTaoWinsock)
        WSACleanup();
#endif
}

int HttpServer::batDau(int congDau)
{
    for (int cong = congDau; cong <= congDau + 10; ++cong)
    {
        SocketT s = socket(AF_INET, SOCK_STREAM, 0);
        if ((long long)s == -1 || (long long)s < 0)
            throw runtime_error("Khong tao duoc socket.");
        int bat = 1;
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&bat, sizeof(bat));
        sockaddr_in dc;
        memset(&dc, 0, sizeof(dc));
        dc.sin_family = AF_INET;
        dc.sin_port = htons((unsigned short)cong);
        dc.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1
        if (::bind(s, (sockaddr *)&dc, sizeof(dc)) == 0 && listen(s, 16) == 0)
        {
            sock = (long long)s;
            return cong;
        }
        DONG_SOCKET(s);
    }
    throw runtime_error("Khong mo duoc cong " + to_string(congDau) + "-" + to_string(congDau + 10) + ".");
}

void HttpServer::dong()
{
    if (sock != -1)
    {
        DONG_SOCKET(S(sock));
        sock = -1;
    }
}

void HttpServer::vongLap(const atomic<bool> &dung, const XuLy &xuLy)
{
    while (!dung && sock != -1)
    {
        fd_set doc;
        FD_ZERO(&doc);
        FD_SET(S(sock), &doc);
        timeval tg;
        tg.tv_sec = 0;
        tg.tv_usec = 300000;
        int n = select((int)sock + 1, &doc, nullptr, nullptr, &tg);
        if (n <= 0)
            continue;
        SocketT c = accept(S(sock), nullptr, nullptr);
        if ((long long)c == -1 || (long long)c < 0)
            continue;
        xuLyKetNoi((long long)c, xuLy);
        DONG_SOCKET(c);
    }
}

// ---------------------------------------------------------------------
//  Giai ma URL & tach tham so
// ---------------------------------------------------------------------
string HttpServer::giaiMaUrl(const string &s)
{
    string kq;
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '+')
            kq += ' ';
        else if (s[i] == '%' && i + 2 < s.size() && isxdigit((unsigned char)s[i + 1]) &&
                 isxdigit((unsigned char)s[i + 2]))
        {
            kq += (char)strtol(s.substr(i + 1, 2).c_str(), nullptr, 16);
            i += 2;
        }
        else
            kq += s[i];
    }
    return kq;
}

void HttpServer::tachThamSo(const string &s, map<string, string> &kq)
{
    size_t i = 0;
    while (i <= s.size())
    {
        size_t j = s.find('&', i);
        if (j == string::npos)
            j = s.size();
        string cap = s.substr(i, j - i);
        if (!cap.empty())
        {
            size_t e = cap.find('=');
            if (e == string::npos)
                kq[giaiMaUrl(cap)] = "";
            else
                kq[giaiMaUrl(cap.substr(0, e))] = giaiMaUrl(cap.substr(e + 1));
        }
        i = j + 1;
    }
}

// ---------------------------------------------------------------------
//  Doc 1 request, goi xuLy, gui response
// ---------------------------------------------------------------------
static const char *tenTrangThai(int st)
{
    switch (st)
    {
    case 200:
        return "OK";
    case 400:
        return "Bad Request";
    case 401:
        return "Unauthorized";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    default:
        return "Internal Server Error";
    }
}

void HttpServer::xuLyKetNoi(long long client, const XuLy &xuLy)
{
    const size_t TOI_DA = 1 << 20; // 1 MB
    string du;
    char buf[8192];
    size_t hetHeader = string::npos;
    while (hetHeader == string::npos && du.size() < TOI_DA)
    {
        int n = (int)recv(S(client), buf, sizeof(buf), 0);
        if (n <= 0)
            return;
        du.append(buf, (size_t)n);
        hetHeader = du.find("\r\n\r\n");
    }
    if (hetHeader == string::npos)
        return;

    HttpRequest req;
    // Dong dau: METHOD /duong-dan?query HTTP/1.1
    size_t eol = du.find("\r\n");
    string dongDau = du.substr(0, eol);
    size_t s1 = dongDau.find(' '), s2 = dongDau.find(' ', s1 + 1);
    if (s1 == string::npos || s2 == string::npos)
        return;
    req.method = dongDau.substr(0, s1);
    string url = dongDau.substr(s1 + 1, s2 - s1 - 1);
    size_t hoi = url.find('?');
    req.path = giaiMaUrl(url.substr(0, hoi));
    if (hoi != string::npos)
        tachThamSo(url.substr(hoi + 1), req.params);

    // Header
    size_t pos = eol + 2;
    while (pos < hetHeader)
    {
        size_t e = du.find("\r\n", pos);
        string h = du.substr(pos, e - pos);
        size_t hai = h.find(':');
        if (hai != string::npos)
        {
            string k = h.substr(0, hai), v = h.substr(hai + 1);
            for (char &c : k)
                c = (char)tolower((unsigned char)c);
            size_t d = v.find_first_not_of(' ');
            req.headers[k] = d == string::npos ? "" : v.substr(d);
        }
        pos = e + 2;
    }

    // Body (form urlencoded)
    size_t doDai = (size_t)strtoul(req.header("content-length").c_str(), nullptr, 10);
    if (doDai > TOI_DA)
        return;
    string body = du.substr(hetHeader + 4);
    while (body.size() < doDai)
    {
        int n = (int)recv(S(client), buf, sizeof(buf), 0);
        if (n <= 0)
            break;
        body.append(buf, (size_t)n);
    }
    if (!body.empty())
        tachThamSo(body.substr(0, doDai), req.params);

    HttpResponse res = xuLy(req);
    string goi = "HTTP/1.1 " + to_string(res.status) + " " + tenTrangThai(res.status) + "\r\n" +
                 "Content-Type: " + res.contentType + "\r\n" + "Content-Length: " + to_string(res.body.size()) +
                 "\r\n" + "Cache-Control: no-store\r\nConnection: close\r\n\r\n" + res.body;
    size_t daGui = 0;
    while (daGui < goi.size())
    {
        int n = (int)send(S(client), goi.data() + daGui, (int)(goi.size() - daGui), CO_GUI);
        if (n <= 0)
            break;
        daGui += (size_t)n;
    }
}
