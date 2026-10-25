// PhongCuThe.cpp
#include "PhongCuThe.h"
#include "InputHelper.h"
#include <stdexcept>

using namespace std;

namespace
{
    string kiemTraChu(const string &s, const string &tenTruong)
    {
        string t = InputHelper::trim(s);
        if (!InputHelper::laChuThuan(t))
            throw invalid_argument(tenTruong + " chi duoc chua chu cai.");
        return t;
    }
    double kiemTraKhongAm(double v, const string &tenTruong)
    {
        if (!(v >= 0))
            throw invalid_argument(tenTruong + " khong duoc am.");
        return v;
    }
    int kiemTraSoPhongNgu(int n)
    {
        if (n <= 0)
            throw invalid_argument("So phong ngu phai lon hon 0.");
        return n;
    }
    string coKhong(bool b) { return b ? "Co" : "Khong"; }
}

// StandardRoom
StandardRoom::StandardRoom(const string &ma, const string &so, int tang, double gia,
                           int sucChua, double dienTich, const string &hn)
    : Phong(ma, so, tang, LoaiPhong::STANDARD, gia, sucChua, dienTich),
      huongNhin(kiemTraChu(hn, "Huong nhin")) {}

void StandardRoom::datHuongNhin(const string &hn) { huongNhin = kiemTraChu(hn, "Huong nhin"); }
double StandardRoom::tinhGiaPhong(int soDem, MuaDuLich mua) const { return Phong::tinhGiaPhong(soDem, mua); }
string StandardRoom::moTa() const { return Phong::moTa() + " | Huong nhin: " + huongNhin; }
vector<string> StandardRoom::layDichVuDiKem() const { return Phong::layDichVuDiKem(); }
vector<pair<string, string>> StandardRoom::layThongTinRieng() const { return {{"Huong nhin", huongNhin}}; }

// DeluxeRoom
DeluxeRoom::DeluxeRoom(const string &ma, const string &so, int tang, double gia,
                       int sucChua, double dienTich, bool bc, const string &hn)
    : Phong(ma, so, tang, LoaiPhong::DELUXE, gia, sucChua, dienTich),
      coBanCong(bc), huongNhin(kiemTraChu(hn, "Huong nhin")) {}

void DeluxeRoom::datHuongNhin(const string &hn) { huongNhin = kiemTraChu(hn, "Huong nhin"); }
double DeluxeRoom::tinhGiaPhong(int soDem, MuaDuLich mua) const { return Phong::tinhGiaPhong(soDem, mua); }
string DeluxeRoom::moTa() const { return Phong::moTa() + " | Huong nhin: " + huongNhin + " | Ban cong: " + coKhong(coBanCong); }
vector<string> DeluxeRoom::layDichVuDiKem() const
{
    auto dv = Phong::layDichVuDiKem();
    if (coBanCong)
        dv.push_back("Ban cong");
    return dv;
}
vector<pair<string, string>> DeluxeRoom::layThongTinRieng() const { return {{"Huong nhin", huongNhin}, {"Ban cong", coKhong(coBanCong)}}; }

// Suite
Suite::Suite(const string &ma, const string &so, int tang, double gia,
             int sucChua, double dienTich, int spn, bool pk, double buffet)
    : Phong(ma, so, tang, LoaiPhong::SUITE, gia, sucChua, dienTich),
      soPhongNgu(kiemTraSoPhongNgu(spn)), coPhongKhach(pk),
      phiBuffetSang(kiemTraKhongAm(buffet, "Phi buffet sang")) {}

void Suite::datSoPhongNgu(int n) { soPhongNgu = kiemTraSoPhongNgu(n); }
void Suite::datPhiBuffetSang(double phi) { phiBuffetSang = kiemTraKhongAm(phi, "Phi buffet sang"); }
double Suite::tinhGiaPhong(int soDem, MuaDuLich mua) const { return Phong::tinhGiaPhong(soDem, mua); }
string Suite::moTa() const { return Phong::moTa() + " | " + to_string(soPhongNgu) + " phong ngu"; }
vector<string> Suite::layDichVuDiKem() const { return Phong::layDichVuDiKem(); }
vector<pair<string, string>> Suite::layThongTinRieng() const { return {{"So phong ngu", to_string(soPhongNgu)}}; }

// Villa
Villa::Villa(const string &ma, const string &so, int tang, double gia,
             int sucChua, double dienTich, bool hb, double sanVuon, int spn, double quanGia)
    : Phong(ma, so, tang, LoaiPhong::VILLA, gia, sucChua, dienTich),
      coHoBoi(hb), dienTichSanVuon(kiemTraKhongAm(sanVuon, "Dien tich san vuon")),
      soPhongNgu(kiemTraSoPhongNgu(spn)), phiQuanGia(kiemTraKhongAm(quanGia, "Phi quan gia")) {}

void Villa::datDienTichSanVuon(double dt) { dienTichSanVuon = kiemTraKhongAm(dt, "Dien tich san vuon"); }
void Villa::datSoPhongNgu(int n) { soPhongNgu = kiemTraSoPhongNgu(n); }
void Villa::datPhiQuanGia(double phi) { phiQuanGia = kiemTraKhongAm(phi, "Phi quan gia"); }
double Villa::tinhGiaPhong(int soDem, MuaDuLich mua) const { return Phong::tinhGiaPhong(soDem, mua); }
string Villa::moTa() const { return Phong::moTa() + " | Villa " + to_string(soPhongNgu) + " phong ngu"; }
vector<string> Villa::layDichVuDiKem() const { return Phong::layDichVuDiKem(); }
vector<pair<string, string>> Villa::layThongTinRieng() const { return {{"San vuon", to_string((long long)dienTichSanVuon) + " m2"}}; }