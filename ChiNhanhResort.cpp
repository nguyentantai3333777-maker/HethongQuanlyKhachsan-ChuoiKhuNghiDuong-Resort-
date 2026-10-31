// ChiNhanhResort.cpp
#include "ChiNhanhResort.h"
#include "InputHelper.h"
#include <stdexcept>

using namespace std;

ChiNhanhResort::ChiNhanhResort(const string &ma, const string &ten, const string &dc, const string &hl)
    : maChiNhanh(ma), tenChiNhanh(InputHelper::trim(ten)), diaChi(InputHelper::trim(dc)),
      hotline(InputHelper::trim(hl))
{
    if (!InputHelper::laMa(maChiNhanh, "CN"))
        throw invalid_argument("Ma chi nhanh khong hop le.");
    if (tenChiNhanh.empty())
        throw invalid_argument("Ten chi nhanh khong duoc rong.");
    if (tenChiNhanh.size() < 2 || tenChiNhanh.size() > 40 || !InputHelper::laChuThuan(tenChiNhanh))
        throw invalid_argument("Ten chi nhanh chi duoc chua chu cai va khoang trang (2-40 ky tu), khong duoc co so.");
    if (diaChi.empty())
        throw invalid_argument("Dia chi chi nhanh khong duoc rong.");
    if (!InputHelper::laDiaChi(diaChi))
        throw invalid_argument("Dia chi chi nhanh 5-120 ky tu, gom chu, so va , . / - ( ) # (phai co chu cai).");
    if (!InputHelper::laSoDienThoai(hotline))
        throw invalid_argument("Hotline phai gom dung 10 chu so va bat dau bang 0.");
}

void ChiNhanhResort::themPhong(const shared_ptr<Phong> &p)
{
    if (!p)
        throw invalid_argument("Phong null.");
    if (timPhong(p->layMaPhong()))
        throw invalid_argument("Ma phong " + p->layMaPhong() + " da ton tai trong chi nhanh.");
    if (coSoPhong(p->laySoPhong()))
        throw invalid_argument("So phong " + p->laySoPhong() + " da ton tai trong chi nhanh.");
    p->datChiNhanh(tenChiNhanh);
    danhSachPhong.push_back(p);
}

void ChiNhanhResort::xoaPhong(const string &maPhong)
{
    for (size_t i = 0; i < danhSachPhong.size(); ++i)
        if (danhSachPhong[i]->layMaPhong() == maPhong)
        {
            danhSachPhong.erase(danhSachPhong.begin() + (long)i);
            return;
        }
    throw runtime_error("Khong tim thay phong " + maPhong + " trong chi nhanh.");
}

shared_ptr<Phong> ChiNhanhResort::timPhong(const string &maPhong) const
{
    for (auto &p : danhSachPhong)
        if (p->layMaPhong() == maPhong)
            return p;
    return nullptr;
}

bool ChiNhanhResort::coSoPhong(const string &soPhong, const string &boQuaMa) const
{
    for (auto &p : danhSachPhong)
        if (p->laySoPhong() == soPhong && p->layMaPhong() != boQuaMa)
            return true;
    return false;
}

int ChiNhanhResort::demPhong(MaTrangThai tt) const
{
    int d = 0;
    for (auto &p : danhSachPhong)
        if (p->layMaTrangThai() == tt)
            ++d;
    return d;
}

void ChiNhanhResort::themNhanVien(const shared_ptr<NhanVien> &nv)
{
    if (!nv)
        throw invalid_argument("Nhan vien null.");
    if (timNhanVien(nv->layMa()))
        throw invalid_argument("Nhan vien da thuoc chi nhanh.");
    nv->datMaChiNhanh(maChiNhanh);
    danhSachNhanVien.push_back(nv);
}

void ChiNhanhResort::xoaNhanVien(const string &maNV)
{
    for (size_t i = 0; i < danhSachNhanVien.size(); ++i)
        if (danhSachNhanVien[i]->layMa() == maNV)
        {
            danhSachNhanVien.erase(danhSachNhanVien.begin() + (long)i);
            return;
        }
    throw runtime_error("Khong tim thay nhan vien " + maNV + " trong chi nhanh.");
}

shared_ptr<NhanVien> ChiNhanhResort::timNhanVien(const string &maNV) const
{
    for (auto &nv : danhSachNhanVien)
        if (nv->layMa() == maNV)
            return nv;
    return nullptr;
}
