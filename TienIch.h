// TienIch.h - Tien ich dung chung: ngay thang, thoi gian, dinh dang tien, xac dinh mua
#pragma once
#include <ctime>
#include <string>
#include "MuaDuLich.h"

// Lop Ngay: luu ngay/thang/nam, ho tro cong ngay, so sanh, doc chuoi dd/mm/yyyy
class Ngay
{
private:
    int ngay, thang, nam;

public:
    Ngay(int d = 1, int m = 1, int y = 2000);

    static bool hopLe(int d, int m, int y);
    static int soNgayTrongThang(int m, int y);
    static Ngay homNay();
    static Ngay tuTimeT(std::time_t t);
    static bool docChuoi(const std::string &s, Ngay &kq); // dd/mm/yyyy

    int layNgay() const { return ngay; }
    int layThang() const { return thang; }
    int layNam() const { return nam; }

    long long soNgayTuyetDoi() const;           // so ngay tinh tu 1/1/1970
    Ngay congNgay(int n) const;                 // cong (tru) n ngay
    int khoangCachDen(const Ngay &khac) const;  // khac - *this (ngay)
    std::time_t toTimeT(int gio = 0, int phut = 0, int giay = 0) const;
    std::string toString() const;               // dd/mm/yyyy

    bool operator==(const Ngay &o) const { return soNgayTuyetDoi() == o.soNgayTuyetDoi(); }
    bool operator!=(const Ngay &o) const { return !(*this == o); }
    bool operator<(const Ngay &o) const { return soNgayTuyetDoi() < o.soNgayTuyetDoi(); }
    bool operator<=(const Ngay &o) const { return soNgayTuyetDoi() <= o.soNgayTuyetDoi(); }
    bool operator>(const Ngay &o) const { return o < *this; }
    bool operator>=(const Ngay &o) const { return o <= *this; }
};

std::tm layTm(std::time_t t);
std::string dinhDangThoiGian(std::time_t t);     // dd/mm/yyyy hh:mm
std::string dinhDangThoiGianNgan(std::time_t t); // dd/mm/yy hh:mm
std::string dinhDangTien(double soTien);         // 1,250,000
std::string dinhDangTienVN(double soTien);       // 1,250,000d (dung trong bang: luon kem don vi "d")
std::string dinhDangSoThuc(double v, int chuSoThapPhan = 1);

// Goi y mua du lich theo ngay nhan phong (thi truong Viet Nam)
MuaDuLich xacDinhMua(const Ngay &n);
// Neu ky nghi [nhan, nhan+soDem) di qua nhieu mua thi tra ve cau canh bao (rong neu cung mot mua).
// Thiet ke hien tai: MOT he so mua (cua NGAY NHAN) ap cho toan bo ky -> khach can biet dieu nay truoc khi dat.
std::string canhBaoNhieuMua(const Ngay &nhan, int soDem);
