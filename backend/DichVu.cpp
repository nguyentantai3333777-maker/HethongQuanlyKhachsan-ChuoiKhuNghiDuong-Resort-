// DichVu.cpp
#include "DichVu.h"
#include "InputHelper.h"

using namespace std;

const int TourTuyen::SO_NGUOI_GIAM_GIA;

DichVu::DichVu(const string &ma, const string &t, double gia, const string &donVi)
    : maDichVu(InputHelper::trim(ma)), ten(""), donGia(0), donViTinh(donVi)
{
    if (maDichVu.empty())
        throw invalid_argument("Ma dich vu khong duoc rong.");
    datTen(t);
    datDonGia(gia);
}

void DichVu::datTen(const string &t)
{
    string s = InputHelper::trim(t);
    if (s.empty())
        throw invalid_argument("Ten dich vu khong duoc rong.");
    if (!InputHelper::laTenRieng(s, 50))
        throw invalid_argument("Ten dich vu 2-50 ky tu, gom chu, so, khoang trang va - ' . (phai co chu cai).");
    ten = s;
}

void DichVu::datDonGia(double gia)
{
    if (!(gia > 0) || gia > 100000000.0)
        throw invalid_argument("Don gia dich vu phai lon hon 0 va khong qua 100.000.000 VND.");
    donGia = gia;
}

double DichVu::tinhChiPhi(int soLuong) const
{
    if (soLuong <= 0)
        throw invalid_argument("So luong phai lon hon 0.");
    return donGia * soLuong;
}

AmThuc::AmThuc(const string &ma, const string &t, double gia, const string &loai)
    : DichVu(ma, t, gia, "suat"), loaiBuaAn(loai.empty() ? "Set menu" : InputHelper::trim(loai))
{
    if (!InputHelper::laChuThuan(loaiBuaAn))
        throw invalid_argument("Loai bua an chi duoc chua chu cai.");
}

Spa::Spa(const string &ma, const string &t, double gia, int phut)
    : DichVu(ma, t, gia, "lan"), thoiLuongPhut(phut)
{
    if (phut <= 0 || phut > 600)
        throw invalid_argument("Thoi luong spa phai tu 1 den 600 phut.");
}

TourTuyen::TourTuyen(const string &ma, const string &t, double gia, const string &dd, int ngay)
    : DichVu(ma, t, gia, "nguoi"), diemDen(dd.empty() ? "Noi thanh" : InputHelper::trim(dd)), soNgay(ngay)
{
    if (!InputHelper::laTenRieng(diemDen, 50))
        throw invalid_argument("Diem den chi gom chu, so, khoang trang va - ' . (phai co chu cai).");
    if (ngay <= 0 || ngay > 30)
        throw invalid_argument("So ngay tour phai tu 1 den 30.");
}

double TourTuyen::tinhChiPhi(int soNguoi) const
{
    double tong = DichVu::tinhChiPhi(soNguoi);
    if (soNguoi >= SO_NGUOI_GIAM_GIA)
        tong *= 0.9;
    return tong;
}
