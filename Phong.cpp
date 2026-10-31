// Phong.cpp
#include "Phong.h"
#include "TrangThaiCuThe.h"
#include "InputHelper.h"
#include "TienIch.h"
#include <sstream>
#include <stdexcept>

using namespace std;

const int Phong::SO_DEM_UU_DAI;
constexpr double Phong::TY_LE_UU_DAI;

Phong::Phong()
    : maPhong(""), trangThai(make_shared<PhongTrong>()), soPhong(""), tang(1),
      loaiPhong(LoaiPhong::STANDARD), giaCoBan(1), sucChua(1), dienTich(1),
      chiNhanh("") {}

Phong::Phong(const string &ma, const string &so, int t, LoaiPhong loai, double gia, int sc, double dt)
    : trangThai(make_shared<PhongTrong>()), tang(1), loaiPhong(loai), giaCoBan(1),
      sucChua(1), dienTich(1), chiNhanh("")
{
    datMaPhong(ma);
    datSoPhong(so);
    datTang(t);
    datGiaCoBan(gia);
    datSucChua(sc);
    datDienTich(dt);
}

// ================== Setter ==================
void Phong::datMaPhong(const string &ma)
{
    string m = InputHelper::chuanHoaMa(ma);
    if (!InputHelper::laMa(m, "P"))
        throw invalid_argument("Ma phong khong hop le.");
    maPhong = m;
}
void Phong::datSoPhong(const string &so)
{
    string s = InputHelper::trim(so);
    if (!InputHelper::laSoDuong(s))
        throw invalid_argument("So phong phai la so duong (toi da 9 chu so).");
    soPhong = InputHelper::chuanHoaSoDuong(s);
}
void Phong::datTang(int t)
{
    if (t < 1 || t > 5)
        throw invalid_argument("Tang phai tu 1 den 5.");
    tang = t;
}
void Phong::datLoaiPhong(LoaiPhong loai) { loaiPhong = loai; }
void Phong::datGiaCoBan(double gia)
{
    if (!(gia > 0) || gia > 1000000000.0)
        throw invalid_argument("Gia phong phai lon hon 0 va khong qua 1.000.000.000 VND.");
    giaCoBan = gia;
}
void Phong::datSucChua(int sc)
{
    if (sc <= 0 || sc > 20)
        throw invalid_argument("Suc chua phai tu 1 den 20 nguoi.");
    sucChua = sc;
}
void Phong::datDienTich(double dt)
{
    if (!(dt > 0) || dt > 5000)
        throw invalid_argument("Dien tich phai lon hon 0 va khong qua 5000 m2.");
    dienTich = dt;
}
void Phong::datChiNhanh(const string &cn)
{
    string c = InputHelper::trim(cn);
    if (c.empty())
        throw invalid_argument("Ten chi nhanh khong duoc rong.");
    chiNhanh = c;
}

// ================== Tinh gia ==================
// Tien phong = gia/dem x so dem x he so mua; o tu 7 dem duoc giam 10%
double Phong::tinhGiaPhong(int soDem, MuaDuLich mua) const
{
    if (soDem <= 0)
        throw invalid_argument("So dem phai lon hon 0.");
    double tong = giaCoBan * soDem * heSoMua(mua);
    if (soDem >= SO_DEM_UU_DAI)
        tong *= (1.0 - TY_LE_UU_DAI);
    return tong;
}

string Phong::moTa() const
{
    ostringstream os;
    os << loaiPhongToString(loaiPhong) << " - phong " << soPhong << ", tang " << tang
       << ", " << sucChua << " khach, " << dinhDangSoThuc(dienTich) << " m2";
    return os.str();
}

vector<string> Phong::layDichVuDiKem() const
{
    switch (loaiPhong)
    {
    case LoaiPhong::STANDARD:
        return {"WiFi", "Nuoc uong"};
    case LoaiPhong::DELUXE:
        return {"WiFi", "Nuoc uong", "Bon tam"};
    case LoaiPhong::SUITE:
        return {"WiFi", "Buffet sang", "Minibar"};
    case LoaiPhong::VILLA:
        return {"WiFi", "Buffet sang", "Ho boi rieng", "Xe dua don"};
    }
    return {};
}

vector<pair<string, string>> Phong::layThongTinRieng() const { return {}; }

// ================== State Pattern ==================
// Giu ban sao shared_ptr de doi tuong trang thai khong bi huy khi no tu thay the chinh minh
void Phong::giuCho(time_t han)
{
    auto tt = trangThai;
    tt->giuCho(this, han);
}
void Phong::huyGiuCho()
{
    auto tt = trangThai;
    tt->huyGiuCho(this);
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
void Phong::baoTri(const string &lyDo)
{
    auto tt = trangThai;
    tt->baoTri(this, lyDo);
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

ostream &operator<<(ostream &os, const Phong &p)
{
    os << "Phong " << p.maPhong << " (so " << p.soPhong << ") - " << loaiPhongToString(p.loaiPhong)
       << " - " << dinhDangTien(p.giaCoBan) << " VND/dem - " << p.layTenTrangThai();
    return os;
}

bool operator==(const Phong &p1, const Phong &p2) { return p1.maPhong == p2.maPhong; }
