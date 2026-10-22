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
    for (int i = (int)s.size() - 3; i > 0; i -= 3) s.insert((size_t)i, ",");
    return s;
}

// ----- Constructor -----
Phong::Phong()
    : maPhong(""), trangThai(make_shared<PhongTrong>()), soPhong(""), tang(1),
      loaiPhong(LoaiPhong::STANDARD), giaCoBan(1), sucChua(1), dienTich(1),
      chiNhanh("Chi nhanh chinh")
{
}

Phong::Phong(string ma, string so, int t, LoaiPhong loai, double gia, int sc, double dt)
    : trangThai(make_shared<PhongTrong>()), chiNhanh("Chi nhanh chinh")
{
    // Dung setter de du lieu luon duoc kiem tra
    datMaPhong(ma);
    datSoPhong(so);
    datTang(t);
    datLoaiPhong(loai);
    datGiaCoBan(gia);
    datSucChua(sc);
    datDienTich(dt);
}

// ----- Setter co kiem tra -----
void Phong::datMaPhong(const string& ma)
{
    string m = InputHelper::trim(ma);
    if (!InputHelper::laSoDuong(m))
        throw invalid_argument("Ma phong phai la so duong (chi gom chu so, lon hon 0).");
    maPhong = InputHelper::chuanHoaSoDuong(m);
}
void Phong::datSoPhong(const string& so)
{
    string s = InputHelper::trim(so);
    if (!InputHelper::laSoDuong(s))
        throw invalid_argument("So phong phai la so duong (chi gom chu so, lon hon 0).");
    soPhong = InputHelper::chuanHoaSoDuong(s);
}
void Phong::datTang(int t)
{
    if (t < 0 || t > 100) throw invalid_argument("Tang phai tu 0 den 100.");
    tang = t;
}
void Phong::datLoaiPhong(LoaiPhong loai) { loaiPhong = loai; }
void Phong::datGiaCoBan(double gia)
{
    if (!(gia > 0)) throw invalid_argument("Gia phong phai lon hon 0.");
    giaCoBan = gia;
}
void Phong::datSucChua(int sc)
{
    if (sc <= 0) throw invalid_argument("Suc chua phai lon hon 0.");
    sucChua = sc;
}
void Phong::datDienTich(double dt)
{
    if (!(dt > 0)) throw invalid_argument("Dien tich phai lon hon 0.");
    dienTich = dt;
}
void Phong::datChiNhanh(const string& cn)
{
    string c = InputHelper::trim(cn);
    if (!InputHelper::laChuThuan(c))
        throw invalid_argument("Chi nhanh chi duoc chua chu cai (khong nhap so).");
    chiNhanh = c;
}

// ----- He so mua -----
// THAP_DIEM 0.8 (giam 20% de kich cau), BINH_THUONG 1.0 (gia goc),
// CAO_DIEM 1.3 (tang 30% mua du lich), LE_TET 1.6 (tang 60% - nhu cau cao nhat)
double Phong::tinhHeSoMua(MuaDuLich mua) const
{
    switch (mua)
    {
    case MuaDuLich::THAP_DIEM:   return 0.8;
    case MuaDuLich::BINH_THUONG: return 1.0;
    case MuaDuLich::CAO_DIEM:    return 1.3;
    case MuaDuLich::LE_TET:      return 1.6;
    }
    throw invalid_argument("Mua du lich khong hop le.");
}

// Tong tien = giaCoBan * soDem * heSoMua ; o tu 7 dem tro len giam 10%
double Phong::tinhGiaPhong(int soDem, MuaDuLich mua) const
{
    if (soDem <= 0) throw invalid_argument("So dem phai lon hon 0.");
    double tong = giaCoBan * soDem * tinhHeSoMua(mua);
    if (soDem >= 7) tong *= 0.9;
    return tong;
}

string Phong::tieuDeBang()
{
    ostringstream os;
    os << left << setw(8) << "Ma" << setw(8) << "So" << setw(6) << "Tang" << setw(10) << "Loai"
       << setw(14) << "Gia/dem" << setw(6) << "Chua" << setw(8) << "M2" << "Trang thai";
    return os.str();
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
    case LoaiPhong::STANDARD: return {"WiFi", "Nuoc uong"};
    case LoaiPhong::DELUXE:   return {"WiFi", "An sang"};
    case LoaiPhong::SUITE:    return {"WiFi", "An sang", "Minibar"};
    case LoaiPhong::VILLA:    return {"WiFi", "An sang", "Ho boi rieng", "Xe dua don"};
    }
    return {};
}

vector<pair<string, string>> Phong::layThongTinRieng() const { return {}; }

// ----- Uy quyen cho State. Giu 1 ban sao shared_ptr de trang thai cu
// khong bi huy trong luc dang chay ham cua no -----
void Phong::giuCho()    { auto tt = trangThai; tt->giuCho(this); }
void Phong::nhanPhong() { auto tt = trangThai; tt->nhanPhong(this); }
void Phong::traPhong()  { auto tt = trangThai; tt->traPhong(this); }
void Phong::donPhong()  { auto tt = trangThai; tt->donPhong(this); }
void Phong::baoTri()    { auto tt = trangThai; tt->baoTri(this); }
void Phong::hoanTat()   { auto tt = trangThai; tt->hoanTat(this); }

void Phong::datTrangThai(shared_ptr<TrangThaiPhong> tt)
{
    if (!tt) throw invalid_argument("Trang thai khong hop le (null).");
    trangThai = tt;
}
string Phong::layTenTrangThai() const { return trangThai->layTenTrangThai(); }

// ----- Toan tu -----
namespace
{
    const int RONG_KHUNG = 60;

    string vienKhung() { return "+" + string((size_t)RONG_KHUNG - 2, '-') + "+"; }

    string dongKhung(const string& noiDung)
    {
        string s = "| " + noiDung;
        int can = RONG_KHUNG - 1 - (int)s.size();
        if (can > 0) s.append((size_t)can, ' ');
        return s + "|";
    }

    string dongTT(const string& nhan, const string& giaTri)
    {
        ostringstream os;
        if (nhan.empty()) os << string(13, ' ') << "  " << giaTri;
        else os << left << setw(13) << nhan << ": " << giaTri;
        return dongKhung(os.str());
    }
}

ostream& operator<<(ostream& os, const Phong& p)
{
    string tieuDe = "THONG TIN PHONG " + p.maPhong + " - " + loaiPhongToString(p.loaiPhong);
    size_t trong = (size_t)RONG_KHUNG - 2;
    size_t le = trong > tieuDe.size() ? (trong - tieuDe.size()) / 2 : 0;
    size_t phai = trong > tieuDe.size() + le ? trong - tieuDe.size() - le : 0;
    ostringstream dt;
    dt << fixed << setprecision(1) << p.dienTich << " m2";

    os << vienKhung() << "\n"
       << "|" << string(le, ' ') << tieuDe << string(phai, ' ') << "|\n"
       << vienKhung() << "\n"
       << dongTT("Ma phong", p.maPhong) << "\n"
       << dongTT("So phong", p.soPhong) << "\n"
       << dongTT("Tang", to_string(p.tang)) << "\n"
       << dongTT("Loai", loaiPhongToString(p.loaiPhong)) << "\n"
       << dongTT("Gia co ban", dinhDangTien(p.giaCoBan) + " VND/dem") << "\n"
       << dongTT("Suc chua", to_string(p.sucChua) + " nguoi") << "\n"
       << dongTT("Dien tich", dt.str()) << "\n"
       << dongTT("Chi nhanh", p.chiNhanh) << "\n"
       << dongTT("Trang thai", p.layTenTrangThai()) << "\n";

    vector<pair<string, string>> rieng = p.layThongTinRieng();
    if (!rieng.empty())
    {
        os << vienKhung() << "\n";
        for (size_t i = 0; i < rieng.size(); ++i)
            os << dongTT(rieng[i].first, rieng[i].second) << "\n";
    }

    // Dich vu di kem: tu dong xuong dong cho khoi tran khung
    os << vienKhung() << "\n";
    vector<string> dv = p.layDichVuDiKem();
    string cur;
    bool dau = true;
    for (size_t i = 0; i <= dv.size(); ++i)
    {
        bool het = (i == dv.size());
        string item = het ? "" : dv[i] + (i + 1 < dv.size() ? "," : "");
        if (!cur.empty() && (het || cur.size() + 1 + item.size() > 40))
        {
            os << dongTT(dau ? "Dich vu" : "", cur) << "\n";
            dau = false;
            cur.clear();
        }
        if (!het) cur += (cur.empty() ? "" : " ") + item;
    }
    os << vienKhung() << "\n";
    return os;
}

// Nhap tung truong qua InputHelper (da kiem tra chat che, khong crash)
istream& operator>>(istream& is, Phong& p)
{
    p.datMaPhong(InputHelper::nhapSoDuongChuoi("Ma phong (so duong): "));
    p.datSoPhong(InputHelper::nhapSoDuongChuoi("So phong (so duong): "));
    p.datTang(InputHelper::nhapSoNguyen("Tang (0-100): ", 0, 100));
    p.datLoaiPhong(InputHelper::nhapLoaiPhong());
    p.datGiaCoBan(InputHelper::nhapSoThucDuong("Gia co ban VND/dem (>0): ", 1e10));
    p.datSucChua(InputHelper::nhapSoNguyen("Suc chua (1-50): ", 1, 50));
    p.datDienTich(InputHelper::nhapSoThucDuong("Dien tich m2 (>0): ", 10000));
    p.datChiNhanh(InputHelper::nhapChuoiChu("Chi nhanh: "));
    return is;
}

bool operator==(const Phong& p1, const Phong& p2) { return p1.maPhong == p2.maPhong; }
