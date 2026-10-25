// QuanLyPhong.cpp
#include "QuanLyPhong.h"
#include "PhongDecorator.h"
#include "InputHelper.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;

static string chuanHoaMa(const string &s)
{
    string t = InputHelper::trim(s);
    return InputHelper::laSoDuong(t) ? InputHelper::chuanHoaSoDuong(t) : InputHelper::chuHoa(t);
}

void QuanLyPhong::themPhong(shared_ptr<Phong> phong)
{
    if (!phong)
        throw invalid_argument("Phong null.");
    if (phong->layMaPhong() == phong->laySoPhong())
        throw invalid_argument("Ma phong va so phong phai khac nhau.");
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (*danhSach[i] == *phong)
            throw invalid_argument("Ma phong da ton tai.");
    danhSach.push_back(phong);
}

void QuanLyPhong::xoaPhong(const string &maPhong)
{
    shared_ptr<Phong> p = timTheoMa(maPhong);
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (danhSach[i] == p)
        {
            danhSach.erase(danhSach.begin() + (long)i);
            return;
        }
}

shared_ptr<Phong> QuanLyPhong::timTheoMa(const string &maPhong) const
{
    string ma = chuanHoaMa(maPhong);
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (danhSach[i]->layMaPhong() == ma)
            return danhSach[i];
    throw runtime_error("Khong tim thay ma phong.");
}

shared_ptr<Phong> QuanLyPhong::timTheoSoPhong(const string &soPhong) const
{
    string so = chuanHoaMa(soPhong);
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (danhSach[i]->laySoPhong() == so)
            return danhSach[i];
    throw runtime_error("Khong tim thay so phong.");
}

bool QuanLyPhong::daCoMaPhong(const string &maPhong) const
{
    string ma = chuanHoaMa(maPhong);
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (danhSach[i]->layMaPhong() == ma)
            return true;
    return false;
}

bool QuanLyPhong::daCoSoPhong(const string &soPhong, const string &boQuaMa) const
{
    string so = chuanHoaMa(soPhong);
    string bo = chuanHoaMa(boQuaMa);
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (danhSach[i]->laySoPhong() == so && danhSach[i]->layMaPhong() != bo)
            return true;
    return false;
}

QuanLyPhong::DanhSachPhong QuanLyPhong::timPhongTrong() const { return timTheoTrangThai("Trong"); }

QuanLyPhong::DanhSachPhong QuanLyPhong::timTheoLoai(LoaiPhong loai) const
{
    DanhSachPhong kq;
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (danhSach[i]->layLoaiPhong() == loai)
            kq.push_back(danhSach[i]);
    return kq;
}

QuanLyPhong::DanhSachPhong QuanLyPhong::timTheoKhoangGia(double giaTu, double giaDen) const
{
    DanhSachPhong kq;
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (danhSach[i]->layGiaPhong() >= giaTu && danhSach[i]->layGiaPhong() <= giaDen)
            kq.push_back(danhSach[i]);
    return kq;
}

QuanLyPhong::DanhSachPhong QuanLyPhong::timTheoTrangThai(const string &ten) const
{
    DanhSachPhong kq;
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (danhSach[i]->layTenTrangThai() == ten)
            kq.push_back(danhSach[i]);
    return kq;
}

void QuanLyPhong::hienThiDanhSach(const DanhSachPhong &ds)
{
    for (size_t i = 0; i < ds.size(); ++i)
        cout << ds[i]->moTa() << "\n";
}

void QuanLyPhong::chuyenTrangThai(const string &maPhong, HanhDong hanhDong)
{
    shared_ptr<Phong> p = timTheoMa(maPhong);
    switch (hanhDong)
    {
    case HanhDong::GIU_CHO:
        p->giuCho();
        break;
    case HanhDong::NHAN_PHONG:
        p->nhanPhong();
        break;
    case HanhDong::TRA_PHONG:
        p->traPhong();
        break;
    case HanhDong::DON_PHONG:
        p->donPhong();
        break;
    case HanhDong::BAO_TRI:
        p->baoTri();
        break;
    case HanhDong::HOAN_TAT:
        p->hoanTat();
        break;
    }
}

double QuanLyPhong::tinhGia(const string &maPhong, int soDem, MuaDuLich mua,
                            bool themAnSang, bool themMinibar,
                            int soSuatAmThuc, int soLanSpa, int soNguoiTour) const
{
    shared_ptr<Phong> p = timTheoMa(maPhong);
    if (themAnSang)
        p = make_shared<AnSangDecorator>(p);
    if (themMinibar)
        p = make_shared<MinibarDecorator>(p);
    if (soSuatAmThuc > 0)
        p = make_shared<DecoratorAmThuc>(p, make_shared<AmThuc>(), soSuatAmThuc);
    if (soLanSpa > 0)
        p = make_shared<DecoratorSpa>(p, make_shared<Spa>(), soLanSpa);
    if (soNguoiTour > 0)
        p = make_shared<DecoratorTour>(p, make_shared<TourTuyen>(), soNguoiTour);
    return p->tinhGiaPhong(soDem, mua);
}