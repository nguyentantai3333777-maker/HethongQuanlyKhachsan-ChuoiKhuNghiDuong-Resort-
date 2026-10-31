// PhongCuThe.cpp
#include "PhongCuThe.h"
#include "InputHelper.h"
#include "TienIch.h"
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
        if (!(v >= 0) || v > 100000000.0)
            throw invalid_argument(tenTruong + " phai tu 0 den 100.000.000.");
        return v;
    }
    int kiemTraSoPhongNgu(int n)
    {
        if (n <= 0 || n > 20)
            throw invalid_argument("So phong ngu phai tu 1 den 20.");
        return n;
    }
    string coKhong(bool b) { return b ? "Co" : "Khong"; }
}

// ======================= StandardRoom =======================
StandardRoom::StandardRoom(const string &ma, const string &so, int t, double gia,
                           int sc, double dt, const string &hn)
    : Phong(ma, so, t, LoaiPhong::STANDARD, gia, sc, dt),
      huongNhin(kiemTraChu(hn, "Huong nhin")) {}

void StandardRoom::datHuongNhin(const string &hn) { huongNhin = kiemTraChu(hn, "Huong nhin"); }
double StandardRoom::tinhGiaPhong(int soDem, MuaDuLich mua) const { return Phong::tinhGiaPhong(soDem, mua); }
string StandardRoom::moTa() const { return Phong::moTa() + ", huong " + huongNhin; }
vector<string> StandardRoom::layDichVuDiKem() const { return Phong::layDichVuDiKem(); }
vector<pair<string, string>> StandardRoom::layThongTinRieng() const { return {{"Huong nhin", huongNhin}}; }

// ======================= DeluxeRoom =======================
DeluxeRoom::DeluxeRoom(const string &ma, const string &so, int t, double gia,
                       int sc, double dt, bool bc, const string &hn)
    : Phong(ma, so, t, LoaiPhong::DELUXE, gia, sc, dt),
      coBanCong(bc), huongNhin(kiemTraChu(hn, "Huong nhin")) {}

void DeluxeRoom::datHuongNhin(const string &hn) { huongNhin = kiemTraChu(hn, "Huong nhin"); }
double DeluxeRoom::tinhGiaPhong(int soDem, MuaDuLich mua) const { return Phong::tinhGiaPhong(soDem, mua); }
string DeluxeRoom::moTa() const
{
    return Phong::moTa() + ", huong " + huongNhin + (coBanCong ? ", co ban cong" : "");
}
vector<string> DeluxeRoom::layDichVuDiKem() const
{
    auto dv = Phong::layDichVuDiKem();
    if (coBanCong)
        dv.push_back("Ban cong");
    return dv;
}
vector<pair<string, string>> DeluxeRoom::layThongTinRieng() const
{
    return {{"Huong nhin", huongNhin}, {"Ban cong", coKhong(coBanCong)}};
}

// ======================= Suite =======================
Suite::Suite(const string &ma, const string &so, int t, double gia,
             int sc, double dt, int spn, bool pk, double buffet)
    : Phong(ma, so, t, LoaiPhong::SUITE, gia, sc, dt),
      soPhongNgu(kiemTraSoPhongNgu(spn)), coPhongKhach(pk),
      phiBuffetSang(kiemTraKhongAm(buffet, "Phi buffet sang")) {}

void Suite::datSoPhongNgu(int n) { soPhongNgu = kiemTraSoPhongNgu(n); }
void Suite::datPhiBuffetSang(double phi) { phiBuffetSang = kiemTraKhongAm(phi, "Phi buffet sang"); }
double Suite::tinhGiaPhong(int soDem, MuaDuLich mua) const
{
    return Phong::tinhGiaPhong(soDem, mua) + phiBuffetSang * soDem;
}
string Suite::moTa() const
{
    return Phong::moTa() + ", " + to_string(soPhongNgu) + " phong ngu" + (coPhongKhach ? ", co phong khach" : "");
}
vector<string> Suite::layDichVuDiKem() const
{
    auto dv = Phong::layDichVuDiKem();
    if (coPhongKhach)
        dv.push_back("Phong khach rieng");
    return dv;
}
vector<pair<string, string>> Suite::layThongTinRieng() const
{
    return {{"So phong ngu", to_string(soPhongNgu)},
            {"Phong khach", coKhong(coPhongKhach)},
            {"Phi buffet sang/dem", dinhDangTien(phiBuffetSang) + " VND"}};
}

// ======================= Villa =======================
Villa::Villa(const string &ma, const string &so, int t, double gia,
             int sc, double dt, bool hb, double sanVuon, int spn, double quanGia)
    : Phong(ma, so, t, LoaiPhong::VILLA, gia, sc, dt),
      coHoBoi(hb), dienTichSanVuon(kiemTraKhongAm(sanVuon, "Dien tich san vuon")),
      soPhongNgu(kiemTraSoPhongNgu(spn)), phiQuanGia(kiemTraKhongAm(quanGia, "Phi quan gia")) {}

void Villa::datDienTichSanVuon(double dt) { dienTichSanVuon = kiemTraKhongAm(dt, "Dien tich san vuon"); }
void Villa::datSoPhongNgu(int n) { soPhongNgu = kiemTraSoPhongNgu(n); }
void Villa::datPhiQuanGia(double phi) { phiQuanGia = kiemTraKhongAm(phi, "Phi quan gia"); }
double Villa::tinhGiaPhong(int soDem, MuaDuLich mua) const
{
    return Phong::tinhGiaPhong(soDem, mua) + phiQuanGia * soDem;
}
string Villa::moTa() const
{
    return Phong::moTa() + ", " + to_string(soPhongNgu) + " phong ngu" + (coHoBoi ? ", ho boi rieng" : "");
}
vector<string> Villa::layDichVuDiKem() const
{
    auto dv = Phong::layDichVuDiKem();
    if (!coHoBoi)
    {
        for (size_t i = 0; i < dv.size(); ++i)
            if (dv[i] == "Ho boi rieng")
            {
                dv.erase(dv.begin() + (long)i);
                break;
            }
    }
    dv.push_back("Quan gia rieng");
    return dv;
}
vector<pair<string, string>> Villa::layThongTinRieng() const
{
    return {{"Ho boi rieng", coKhong(coHoBoi)},
            {"San vuon", dinhDangSoThuc(dienTichSanVuon) + " m2"},
            {"So phong ngu", to_string(soPhongNgu)},
            {"Phi quan gia/dem", dinhDangTien(phiQuanGia) + " VND"}};
}
