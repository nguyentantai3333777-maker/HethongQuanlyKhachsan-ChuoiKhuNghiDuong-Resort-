// PhongCuThe.cpp
#include "PhongCuThe.h"
#include "InputHelper.h"
#include <stdexcept>

using namespace std;

namespace
{
    string kiemTraChu(const string& s, const string& tenTruong)
    {
        string t = InputHelper::trim(s);
        if (!InputHelper::laChuThuan(t))
            throw invalid_argument(tenTruong + " chi duoc chua chu cai (khong nhap so).");
        return t;
    }
    double kiemTraKhongAm(double v, const string& tenTruong)
    {
        if (!(v >= 0)) throw invalid_argument(tenTruong + " khong duoc am.");
        return v;
    }
    int kiemTraSoPhongNgu(int n)
    {
        if (n <= 0) throw invalid_argument("So phong ngu phai lon hon 0.");
        return n;
    }
    string coKhong(bool b) { return b ? "Co" : "Khong"; }
    void thayDichVu(vector<string>& dv, const string& cu, const string& moi)
    {
        for (size_t i = 0; i < dv.size(); ++i)
            if (dv[i] == cu) { dv[i] = moi; return; }
        dv.push_back(moi);
    }
}

// ===== StandardRoom =====
StandardRoom::StandardRoom(const string& ma, const string& so, int tang, double gia,
                           int sucChua, double dienTich, const string& hn)
    : Phong(ma, so, tang, LoaiPhong::STANDARD, gia, sucChua, dienTich),
      huongNhin(kiemTraChu(hn, "Huong nhin")) {}

void StandardRoom::datHuongNhin(const string& hn) { huongNhin = kiemTraChu(hn, "Huong nhin"); }

double StandardRoom::tinhGiaPhong(int soDem, MuaDuLich mua) const
{
    return Phong::tinhGiaPhong(soDem, mua);   // gia chuan, khong phu thu
}

string StandardRoom::moTa() const { return Phong::moTa() + " | Huong nhin: " + huongNhin; }

vector<string> StandardRoom::layDichVuDiKem() const { return Phong::layDichVuDiKem(); }

vector<pair<string, string>> StandardRoom::layThongTinRieng() const
{
    return {make_pair("Huong nhin", huongNhin)};
}

// ===== DeluxeRoom =====
DeluxeRoom::DeluxeRoom(const string& ma, const string& so, int tang, double gia,
                       int sucChua, double dienTich, bool bc, const string& hn)
    : Phong(ma, so, tang, LoaiPhong::DELUXE, gia, sucChua, dienTich),
      coBanCong(bc), huongNhin(kiemTraChu(hn, "Huong nhin")) {}

void DeluxeRoom::datHuongNhin(const string& hn) { huongNhin = kiemTraChu(hn, "Huong nhin"); }

// Ban cong da mac dinh co san trong phong, khong tinh them tien
double DeluxeRoom::tinhGiaPhong(int soDem, MuaDuLich mua) const
{
    return Phong::tinhGiaPhong(soDem, mua);
}

string DeluxeRoom::moTa() const
{
    return Phong::moTa() + " | Huong nhin: " + huongNhin + " | Ban cong: " + coKhong(coBanCong);
}

vector<string> DeluxeRoom::layDichVuDiKem() const
{
    vector<string> dv = Phong::layDichVuDiKem();
    if (coBanCong) dv.push_back("Ban cong");
    return dv;
}

vector<pair<string, string>> DeluxeRoom::layThongTinRieng() const
{
    vector<pair<string, string>> kq;
    kq.push_back(make_pair("Huong nhin", huongNhin));
    kq.push_back(make_pair("Ban cong", coKhong(coBanCong)));
    return kq;
}

// ===== Suite =====
Suite::Suite(const string& ma, const string& so, int tang, double gia,
             int sucChua, double dienTich, int spn, bool pk, double buffet)
    : Phong(ma, so, tang, LoaiPhong::SUITE, gia, sucChua, dienTich),
      soPhongNgu(kiemTraSoPhongNgu(spn)), coPhongKhach(pk),
      phiBuffetSang(kiemTraKhongAm(buffet, "Phi buffet sang")) {}

void Suite::datSoPhongNgu(int n) { soPhongNgu = kiemTraSoPhongNgu(n); }
void Suite::datPhiBuffetSang(double phi) { phiBuffetSang = kiemTraKhongAm(phi, "Phi buffet sang"); }

// Buffet sang mac dinh co san trong phong, khong tinh them tien
double Suite::tinhGiaPhong(int soDem, MuaDuLich mua) const
{
    return Phong::tinhGiaPhong(soDem, mua);
}

string Suite::moTa() const
{
    return Phong::moTa() + " | " + to_string(soPhongNgu) + " phong ngu | Phong khach: " + coKhong(coPhongKhach);
}

vector<string> Suite::layDichVuDiKem() const
{
    vector<string> dv = Phong::layDichVuDiKem();
    thayDichVu(dv, "An sang", "Buffet sang");     // Suite mac dinh co buffet sang
    if (coPhongKhach) dv.push_back("Phong khach");
    return dv;
}

vector<pair<string, string>> Suite::layThongTinRieng() const
{
    vector<pair<string, string>> kq;
    kq.push_back(make_pair("So phong ngu", to_string(soPhongNgu)));
    kq.push_back(make_pair("Phong khach", coKhong(coPhongKhach)));
    return kq;
}

// ===== Villa =====
Villa::Villa(const string& ma, const string& so, int tang, double gia,
             int sucChua, double dienTich, bool hb, double sanVuon, int spn, double quanGia)
    : Phong(ma, so, tang, LoaiPhong::VILLA, gia, sucChua, dienTich),
      coHoBoi(hb), dienTichSanVuon(kiemTraKhongAm(sanVuon, "Dien tich san vuon")),
      soPhongNgu(kiemTraSoPhongNgu(spn)), phiQuanGia(kiemTraKhongAm(quanGia, "Phi quan gia")) {}

void Villa::datDienTichSanVuon(double dt) { dienTichSanVuon = kiemTraKhongAm(dt, "Dien tich san vuon"); }
void Villa::datSoPhongNgu(int n) { soPhongNgu = kiemTraSoPhongNgu(n); }
void Villa::datPhiQuanGia(double phi) { phiQuanGia = kiemTraKhongAm(phi, "Phi quan gia"); }

// Buffet sang, quan gia mac dinh co san trong phong, khong tinh them tien
double Villa::tinhGiaPhong(int soDem, MuaDuLich mua) const
{
    return Phong::tinhGiaPhong(soDem, mua);
}

string Villa::moTa() const
{
    return Phong::moTa() + " | " + to_string(soPhongNgu) + " phong ngu | San vuon: "
           + to_string((long long)dienTichSanVuon) + " m2 | Ho boi: " + coKhong(coHoBoi);
}

vector<string> Villa::layDichVuDiKem() const
{
    vector<string> dv = Phong::layDichVuDiKem();   // mac dinh cua VILLA co "Ho boi rieng"
    thayDichVu(dv, "An sang", "Buffet sang");      // Villa mac dinh co buffet sang
    if (!coHoBoi)
        for (size_t i = 0; i < dv.size(); ++i)
            if (dv[i] == "Ho boi rieng") { dv.erase(dv.begin() + (long)i); break; }
    dv.push_back("Quan gia");
    return dv;
}

vector<pair<string, string>> Villa::layThongTinRieng() const
{
    vector<pair<string, string>> kq;
    kq.push_back(make_pair("So phong ngu", to_string(soPhongNgu)));
    kq.push_back(make_pair("Ho boi", coKhong(coHoBoi)));
    kq.push_back(make_pair("San vuon", to_string((long long)dienTichSanVuon) + " m2"));
    return kq;
}
