// WebServer.cpp - Che do WEB: mo HTTP server, phuc vu frontend/ va API, dung CHUNG du lieu voi console
#include "ChuongTrinh.h"
#include "HttpServer.h"
#include "InputHelper.h"
#include "WebApi.h"
#include <atomic>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;
using namespace UI;

// Tim thu muc frontend (chua index.html hoac admin.html) - chay tu backend/ hay tu thu muc goc deu duoc
static string timThuMucWeb(const string &thuMucDuLieu)
{
    namespace fs = std::filesystem;
    vector<string> ungVien;
    auto them = [&](const fs::path &p) {
        std::error_code ec;
        fs::path r = fs::weakly_canonical(p, ec);
        if (ec)
            r = fs::absolute(p, ec);
        if (!ec)
            ungVien.push_back(r.string());
        else
            ungVien.push_back(p.string());
    };

    // 1) Theo thu muc data: .../backend/data -> .../frontend
    if (!thuMucDuLieu.empty())
    {
        fs::path data(thuMucDuLieu);
        them(data / ".." / ".." / "frontend"); // hotel-resort-management/frontend
        them(data / ".." / "frontend");        // backend/frontend (neu co)
        them(data / "frontend");
    }
    // 2) Theo thu muc lam viec / cau truc thuong gap
    them(fs::path("frontend"));
    them(fs::path("..") / "frontend");
    them(fs::path("..") / ".." / "frontend");
    them(fs::path("hotel-resort-management") / "frontend");
    them(fs::path("..") / "hotel-resort-management" / "frontend");
    them(fs::path("./web"));

    // 3) Bien moi truong (tuy chon)
    if (const char *env = getenv("HOTEL_FRONTEND"))
        them(fs::path(env));

    for (const auto &d : ungVien)
    {
        ifstream f(d + "/index.html");
        if (f)
            return d;
        ifstream a(d + "/admin.html");
        if (a)
            return d;
    }
    return "";
}

static void moTrinhDuyet(const string &url)
{
    if (getenv("HOTEL_NO_BROWSER")) // dung khi kiem thu tu dong
        return;
#ifdef _WIN32
    string lenh = "start \"\" \"" + url + "\"";
#elif __APPLE__
    string lenh = "open \"" + url + "\" >/dev/null 2>&1 &";
#else
    string lenh = "xdg-open \"" + url + "\" >/dev/null 2>&1 &";
#endif
    if (system(lenh.c_str()) != 0)
        cout << "  (Khong tu mo duoc trinh duyet - hay mo link o tren bang tay)\n";
}

void ChuongTrinh::chayWeb()
{
    datChuDe(ChuDe::CHINH);
    xoaManHinh();
    tieuDe("CHE DO WEB", "Frontend HTML/CSS/JS  <->  HTTP API  <->  logic C++ (QuanLyKhachSan) — dung chung CSV voi Console");
    // Dong bo: ghi bo nho hien tai + nap lai tu dia truoc khi mo API (tranh lech so lieu)
    luuDuLieu();
    {
        string loiNap;
        if (!napLaiDuLieuTuFile(&loiNap) && !loiNap.empty())
            canhBao(loiNap);
    }
    string thuMucWeb = timThuMucWeb(thuMucDuLieu);
    if (thuMucWeb.empty())
    {
        loi("Khong tim thay thu muc 'frontend/' (chua index.html). Hay chay chuong trinh tu thu muc backend/.");
        pauseScreen();
        return;
    }
    HttpServer server;
    int cong = 0;
    try
    {
        cong = server.batDau(8080);
    }
    catch (const exception &e)
    {
        loi(e.what());
        pauseScreen();
        return;
    }
    string url = "http://localhost:" + to_string(cong);
    WebApi api(ks, qlTaiKhoan, [this]
               { luuDuLieu(); },
               thuMucWeb);

    thanhCong("Website cho khach : " + url + "/");
    thanhCong("Trang quan tri    : " + url + "/admin.html");
    thongTin("Thu muc giao dien: " + thuMucWeb);
    thongTin("Nguon du lieu CHUNG (Console + Web): " + thuMucDuLieu + "/");
    thongTin("Moi thay doi qua API deu ghi CSV ngay — Console se thay du lieu moi khi chuyen che do.");
    thongTin("Tai khoan: admin / admin123  (Quan ly)    user / user123  (Le tan)");
    moTrinhDuyet(url);

    // Server chay o luong rieng; luong chinh cho nguoi dung nhan Enter de dung.
    // Trong luc nay chi luong server truy cap du lieu -> khong can khoa.
    atomic<bool> dung(false);
    thread luongServer([&]
                       { server.vongLap(dung, [&](const HttpRequest &r)
                                        { return api.xuLy(r); }); });
    cout << "\n";
    try
    {
        InputHelper::nhapDong("Nhan Enter de DUNG web server va quay lai man hinh chinh... ");
    }
    catch (const HetDuLieuNhap &)
    {
        dung = true;
        luongServer.join();
        server.dong();
        luuDuLieu();
        throw;
    }
    dung = true;
    luongServer.join();
    server.dong();
    luuDuLieu();
}
