// QuanLyPhong.h - quan ly danh sach phong
#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Phong.h"

enum class HanhDong { GIU_CHO, NHAN_PHONG, TRA_PHONG, DON_PHONG, BAO_TRI, HOAN_TAT };

class QuanLyPhong
{
private:
    std::vector<std::shared_ptr<Phong>> danhSach;

public:
    typedef std::vector<std::shared_ptr<Phong>> DanhSachPhong;

    void themPhong(std::shared_ptr<Phong> phong);
    void xoaPhong(const std::string& maPhong);

    std::shared_ptr<Phong> timTheoMa(const std::string& maPhong) const;      // khong thay -> runtime_error
    std::shared_ptr<Phong> timTheoSoPhong(const std::string& soPhong) const; // khong thay -> runtime_error
    bool daCoMaPhong(const std::string& maPhong) const;
    bool daCoSoPhong(const std::string& soPhong, const std::string& boQuaMa = "") const;

    DanhSachPhong timPhongTrong() const;
    DanhSachPhong timTheoLoai(LoaiPhong loai) const;
    DanhSachPhong timTheoKhoangGia(double giaTu, double giaDen) const;
    DanhSachPhong timTheoTrangThai(const std::string& tenTrangThai) const;
    const DanhSachPhong& layTatCa() const { return danhSach; }

    static void hienThiDanhSach(const DanhSachPhong& ds);
    void hienThiTatCa() const { hienThiDanhSach(danhSach); }

    void chuyenTrangThai(const std::string& maPhong, HanhDong hanhDong);
    double tinhGia(const std::string& maPhong, int soDem, MuaDuLich mua,
                   bool themAnSang, bool themMinibar,
                   int soSuatAmThuc = 0, int soLanSpa = 0, int soNguoiTour = 0) const;
};
