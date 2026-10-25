// InputHelper.h - cac ham nhap du lieu an toan
#pragma once
#include <string>
#include <stdexcept>
#include "LoaiPhong.h"
#include "MuaDuLich.h"

class HetDuLieuNhap : public std::runtime_error
{
public:
    HetDuLieuNhap() : std::runtime_error("Het du lieu dau vao. Chuong trinh se thoat.") {}
};

namespace InputHelper
{
    std::string trim(const std::string &s);
    std::string chuHoa(std::string s);

    std::string nhapDong(const std::string &nhan);
    std::string nhapChuoi(const std::string &nhan);
    int nhapSoNguyen(const std::string &nhan, int nhoNhat, int lonNhat);
    double nhapSoThucDuong(const std::string &nhan, double lonNhat);
    int nhapLuaChon(int nhoNhat, int lonNhat);
    LoaiPhong nhapLoaiPhong();
    MuaDuLich nhapMuaDuLich();
    bool nhapCoKhong(const std::string &nhan);

    bool laSoDuong(const std::string &s);
    std::string chuanHoaSoDuong(const std::string &s);
    bool laChuThuan(const std::string &s);
    std::string nhapSoDuongChuoi(const std::string &nhan);
    std::string nhapChuoiChu(const std::string &nhan);
}