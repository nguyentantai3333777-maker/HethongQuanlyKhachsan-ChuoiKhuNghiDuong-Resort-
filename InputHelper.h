// InputHelper.h - cac ham nhap du lieu an toan (khong crash khi nhap sai)
#pragma once
#include <string>
#include <stdexcept>
#include "LoaiPhong.h"
#include "MuaDuLich.h"

// Nem ra khi het du lieu nhap (Ctrl+Z / Ctrl+D) de chuong trinh thoat nhe nhang
class HetDuLieuNhap : public std::runtime_error
{
public:
    HetDuLieuNhap() : std::runtime_error("Het du lieu dau vao. Chuong trinh se thoat.") {}
};

namespace InputHelper
{
    std::string trim(const std::string& s);
    std::string chuHoa(std::string s);

    std::string nhapDong(const std::string& nhan);   // cho phep rong (dung cho dang nhap)
    std::string nhapChuoi(const std::string& nhan);  // khong duoc rong / toan khoang trang
    int    nhapSoNguyen(const std::string& nhan, int nhoNhat, int lonNhat);
    double nhapSoThucDuong(const std::string& nhan, double lonNhat); // > 0 va <= lonNhat
    int    nhapLuaChon(int nhoNhat, int lonNhat);    // dung cho menu
    LoaiPhong nhapLoaiPhong();
    MuaDuLich nhapMuaDuLich();
    bool   nhapCoKhong(const std::string& nhan);     // c/k

    // ---- Kiem tra / nhap ma phong, so phong, chi nhanh ----
    bool laSoDuong(const std::string& s);            // chi gom chu so, > 0, toi da 9 chu so
    std::string chuanHoaSoDuong(const std::string& s); // bo so 0 o dau: "007" -> "7"
    bool laChuThuan(const std::string& s);           // chi chu cai va khoang trang
    std::string nhapSoDuongChuoi(const std::string& nhan); // nhap lai den khi la so duong
    std::string nhapChuoiChu(const std::string& nhan);     // nhap lai den khi chi toan chu
}
