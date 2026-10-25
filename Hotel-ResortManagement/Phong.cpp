// Phong.cpp
#include "Phong.h"
#include "TrangThaiCuThe.h"
#include "InputHelper.h"
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>

using namespace std;

string dinhDangTien(double soTien)
{
    string s = to_string((long long)llround(soTien));
    for (int i = (int)s.size() - 3; i > 0; i -= 3)
        s.insert((size_t)i, ",");
    return s;
}

Phong::Phong()
    : maPhong(""), trangThai(make_shared<PhongTrong>()), soPhong(""), tang(1),
      loaiPhong(LoaiPhong::STANDARD), giaCoBan(1), sucChua(1), dienTich(1),
      chiNhanh("Chi nhanh chinh") {}

Phong::Phong(string ma, string so, int t, LoaiPhong loai, double gia, int sc, double dt)
    : trangThai(make_shared<PhongTrong>()), chiNhanh("Chi nhanh chinh")
{
    datMaPhong(ma);
    datSoPhong(so);
    datTang(t);
    datLoaiPhong(loai);
    datGiaCoBan(gia);
    datSucChua(sc);
    datDienTich(dt);
}

void Phong::datMaPhong(const string &ma)
{
    string m = InputHelper::trim(ma);
    if (!InputHelper::laSoDuong(m))
        throw invalid_argument("Ma phong phai la so duong.");
    maPhong = InputHelper::chuanHoaSoDuong(m);
}
void Phong::datSoPhong(const string &so)
{
    string s = InputHelper::trim(so);
    if (!InputHelper::laSoDuong(s))
        throw invalid_argument("So phong phai la so duong.");
    soPhong = InputHelper::chuanHoaSoDuong(s);
}
void Phong::datTang(int t)
{
    if (t < 0 || t > 100)
        throw invalid_argument("Tang phai tu 0 den 100.");
    tang = t;
}
void Phong::datLoaiPhong(LoaiPhong loai) { loaiPhong = loai; }
void Phong::datGiaCoBan(double gia)
{
    if (!(gia > 0))
        throw invalid_argument("Gia phong phai lon hon 0.");
    giaCoBan = gia;
}
void Phong::datSucChua(int sc)
{
    if (sc <= 0)
        throw invalid_argument("Suc chua phai lon hon 0.");
    sucChua = sc;
}
void Phong::datDienTich(double dt)
{
    if (!(dt > 0))
        throw invalid_argument("Dien tich phai lon hon 0.");
    dienTich = dt;
}
void Phong::datChiNhanh(const string &cn)
{
    string c = InputHelper::trim(cn);
    if (!InputHelper::laChuThuan(c))
        throw invalid_argument("Chi nhanh chi duoc chua chu cai.");
    chiNhanh = c;
}

double Phong::tinhHeSoMua(MuaDuLich mua) const
{
    switch (mua)
    {
    case MuaDuLich::THAP_DIEM:
        return 0.8;
    case MuaDuLich::BINH_THUONG:
        return 1.0;
    case MuaDuLich::CAO_DIEM:
        return 1.3;
    case MuaDuLich::LE_TET:
        return 1.6;
    }
    throw invalid_argument("Mua du lich khong hop le.");
}

double Phong::tinhGiaPhong(int soDem, MuaDuLich mua) const
{
    if (soDem <= 0)
        throw invalid_argument("So dem phai lon hon 0.");
    double tong = giaCoBan * soDem * tinhHeSoMua(mua);
    if (soDem >= 7)
        tong *= 0.9;
    return tong;
}

string Phong::moTa() const
{
    ostringstream os;
    os << left << setw(8) << maPhong << setw(8) << soPhong << setw(6) << tang
       << setw(10) << loaiPhongToString(loaiPhong) << setw(14) << dinhDangTien(giaCoBan)
       << setw(6) << sucChua << setw(8) << fixed << setprecision(1) << dienTich
       << trangThai->layTenTrangThai();
    return os.str();
}

vector<string> Phong::layDichVuDiKem() const
{
    switch (loaiPhong)
    {
    case LoaiPhong::STANDARD:
        return {"WiFi", "Nuoc uong"};
    case LoaiPhong::DELUXE:
        return {"WiFi", "An sang"};
    case LoaiPhong::SUITE:
        return {"WiFi", "An sang", "Minibar"};
    case LoaiPhong::VILLA:
        return {"WiFi", "An sang", "Ho boi rieng", "Xe dua don"};
    }
    return {};
}

vector<pair<string, string>> Phong::layThongTinRieng() const { return {}; }

void Phong::giuCho()
{
    auto tt = trangThai;
    tt->giuCho(this);
}
void Phong::nhanPhong()
{
    auto tt = trangThai;
    tt->nhanPhong(this);
}
void Phong::traPhong()
{
    auto tt = trangThai;
    tt->traPhong(this);
}
void Phong::donPhong()
{
    auto tt = trangThai;
    tt->donPhong(this);
}
void Phong::baoTri()
{
    auto tt = trangThai;
    tt->baoTri(this);
}
void Phong::hoanTat()
{
    auto tt = trangThai;
    tt->hoanTat(this);
}

void Phong::datTrangThai(shared_ptr<TrangThaiPhong> tt)
{
    if (!tt)
        throw invalid_argument("Trang thai null.");
    trangThai = tt;
}
string Phong::layTenTrangThai() const { return trangThai->layTenTrangThai(); }

ostream &operator<<(ostream &os, const Phong &p)
{
    os << "Phong " << p.maPhong << " - " << loaiPhongToString(p.loaiPhong) << " - Gia: " << dinhDangTien(p.giaCoBan) << " VND\n";
    return os;
}

istream &operator>>(istream &is, Phong &p)
{
    p.datMaPhong(InputHelper::nhapSoDuongChuoi("Ma phong: "));
    p.datSoPhong(InputHelper::nhapSoDuongChuoi("So phong: "));
    return is;
}

bool operator==(const Phong &p1, const Phong &p2) { return p1.maPhong == p2.maPhong; }