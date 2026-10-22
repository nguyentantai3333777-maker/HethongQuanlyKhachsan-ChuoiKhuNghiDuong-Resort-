// QuanLyPhong.cpp
#include "QuanLyPhong.h"
#include "PhongDecorator.h"
#include "InputHelper.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;

// Ma / so phong la so duong: "007" va "7" la mot
static string chuanHoaMa(const string& s)
{
    string t = InputHelper::trim(s);
    return InputHelper::laSoDuong(t) ? InputHelper::chuanHoaSoDuong(t) : InputHelper::chuHoa(t);
}

void QuanLyPhong::themPhong(shared_ptr<Phong> phong)
{
    if (!phong) throw invalid_argument("Phong khong hop le (null).");
    if (phong->layMaPhong() == phong->laySoPhong())
        throw invalid_argument("Ma phong va so phong phai khac nhau.");
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (*danhSach[i] == *phong)           // dung operator==
            throw invalid_argument("Ma phong '" + phong->layMaPhong() + "' da ton tai.");
    if (daCoSoPhong(phong->laySoPhong()))
        throw invalid_argument("So phong '" + phong->laySoPhong() + "' da ton tai.");
    danhSach.push_back(phong);
}

void QuanLyPhong::xoaPhong(const string& maPhong)
{
    shared_ptr<Phong> p = timTheoMa(maPhong);
    string tt = p->layTenTrangThai();
    if (tt == "Dang o" || tt == "Dang giu cho")
        throw logic_error("Khong the xoa phong dang co khach hoac dang duoc giu cho.");
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (danhSach[i] == p) { danhSach.erase(danhSach.begin() + (long)i); return; }
}

shared_ptr<Phong> QuanLyPhong::timTheoMa(const string& maPhong) const
{
    string ma = chuanHoaMa(maPhong);
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (danhSach[i]->layMaPhong() == ma) return danhSach[i];
    throw runtime_error("Khong tim thay phong co ma '" + ma + "'.");
}

shared_ptr<Phong> QuanLyPhong::timTheoSoPhong(const string& soPhong) const
{
    string so = chuanHoaMa(soPhong);
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (danhSach[i]->laySoPhong() == so) return danhSach[i];
    throw runtime_error("Khong tim thay phong co so '" + so + "'.");
}

bool QuanLyPhong::daCoMaPhong(const string& maPhong) const
{
    string ma = chuanHoaMa(maPhong);
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (danhSach[i]->layMaPhong() == ma) return true;
    return false;
}

bool QuanLyPhong::daCoSoPhong(const string& soPhong, const string& boQuaMa) const
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
        if (danhSach[i]->layLoaiPhong() == loai) kq.push_back(danhSach[i]);
    return kq;
}

QuanLyPhong::DanhSachPhong QuanLyPhong::timTheoKhoangGia(double giaTu, double giaDen) const
{
    if (giaTu > giaDen) throw invalid_argument("Gia 'tu' phai nho hon hoac bang gia 'den'.");
    DanhSachPhong kq;
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (danhSach[i]->layGiaPhong() >= giaTu && danhSach[i]->layGiaPhong() <= giaDen)
            kq.push_back(danhSach[i]);
    return kq;
}

QuanLyPhong::DanhSachPhong QuanLyPhong::timTheoTrangThai(const string& ten) const
{
    DanhSachPhong kq;
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (danhSach[i]->layTenTrangThai() == ten) kq.push_back(danhSach[i]);
    return kq;
}

// In danh sach phong dang bang co khung
void QuanLyPhong::hienThiDanhSach(const DanhSachPhong& ds)
{
    if (ds.empty())
    {
        cout << "Khong co phong nao phu hop.\n";
        return;
    }
    const int N = 8;
    const int w[N] = {6, 6, 4, 8, 13, 5, 7, 13};
    vector<string> tieuDe = {"Ma", "So", "Tang", "Loai", "Gia/dem (VND)", "Chua", "M2", "Trang thai"};

    auto vien = [&]()
    {
        cout << "+";
        for (int i = 0; i < N; ++i) cout << string((size_t)w[i] + 2, '-') << "+";
        cout << "\n";
    };
    auto dong = [&](const vector<string>& c)
    {
        cout << "|";
        for (int i = 0; i < N; ++i) cout << " " << left << setw(w[i]) << c[(size_t)i] << " |";
        cout << "\n";
    };

    vien();
    dong(tieuDe);
    vien();
    for (size_t i = 0; i < ds.size(); ++i)
    {
        ostringstream dt;
        dt << fixed << setprecision(1) << ds[i]->layDienTich();
        vector<string> c;
        c.push_back(ds[i]->layMaPhong());
        c.push_back(ds[i]->laySoPhong());
        c.push_back(to_string(ds[i]->layTang()));
        c.push_back(loaiPhongToString(ds[i]->layLoaiPhong()));
        c.push_back(dinhDangTien(ds[i]->layGiaPhong()));
        c.push_back(to_string(ds[i]->laySucChua()));
        c.push_back(dt.str());
        c.push_back(ds[i]->layTenTrangThai());
        dong(c);
    }
    vien();
    cout << "Tong: " << ds.size() << " phong.\n";
}

// Phong chi goi ham tuong ung, trang thai hien tai se quyet dinh hop le hay nem loi
void QuanLyPhong::chuyenTrangThai(const string& maPhong, HanhDong hanhDong)
{
    shared_ptr<Phong> p = timTheoMa(maPhong);
    switch (hanhDong)
    {
    case HanhDong::GIU_CHO:    p->giuCho();    break;
    case HanhDong::NHAN_PHONG: p->nhanPhong(); break;
    case HanhDong::TRA_PHONG:  p->traPhong();  break;
    case HanhDong::DON_PHONG:  p->donPhong();  break;
    case HanhDong::BAO_TRI:    p->baoTri();    break;
    case HanhDong::HOAN_TAT:   p->hoanTat();   break;
    }
}

// Ghep Decorator len phong goc roi goi tinhGiaPhong() (da hinh)
double QuanLyPhong::tinhGia(const string& maPhong, int soDem, MuaDuLich mua,
                            bool themAnSang, bool themMinibar,
                            int soSuatAmThuc, int soLanSpa, int soNguoiTour) const
{
    shared_ptr<Phong> p = timTheoMa(maPhong);
    if (themAnSang)  p = make_shared<AnSangDecorator>(p);
    if (themMinibar) p = make_shared<MinibarDecorator>(p);
    if (soSuatAmThuc > 0) p = make_shared<DecoratorAmThuc>(p, make_shared<AmThuc>(), soSuatAmThuc);
    if (soLanSpa > 0)     p = make_shared<DecoratorSpa>(p, make_shared<Spa>(), soLanSpa);
    if (soNguoiTour > 0)  p = make_shared<DecoratorTour>(p, make_shared<TourTuyen>(), soNguoiTour);
    return p->tinhGiaPhong(soDem, mua);
}
