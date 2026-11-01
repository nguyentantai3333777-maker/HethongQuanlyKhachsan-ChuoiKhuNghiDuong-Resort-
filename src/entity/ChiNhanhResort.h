// ChiNhanhResort.h - Chi nhanh resort: quan ly Phong va NhanVien
#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Phong.h"
#include "Nguoi.h"

class ChiNhanhResort
{
private:
    std::string maChiNhanh;
    std::string tenChiNhanh;
    std::string diaChi;
    std::string hotline;
    std::vector<std::shared_ptr<Phong>> danhSachPhong;
    std::vector<std::shared_ptr<NhanVien>> danhSachNhanVien;

public:
    ChiNhanhResort(const std::string &ma, const std::string &ten, const std::string &diaChi,
                   const std::string &hotline);

    const std::string &layMa() const { return maChiNhanh; }
    const std::string &layTen() const { return tenChiNhanh; }
    const std::string &layDiaChi() const { return diaChi; }
    const std::string &layHotline() const { return hotline; }

    // ---- Phong ----
    void themPhong(const std::shared_ptr<Phong> &p);
    void xoaPhong(const std::string &maPhong);
    std::shared_ptr<Phong> timPhong(const std::string &maPhong) const;
    bool coSoPhong(const std::string &soPhong, const std::string &boQuaMa = "") const;
    const std::vector<std::shared_ptr<Phong>> &layDanhSachPhong() const { return danhSachPhong; }
    int demPhong(MaTrangThai tt) const;

    // ---- Nhan vien ----
    void themNhanVien(const std::shared_ptr<NhanVien> &nv);
    void xoaNhanVien(const std::string &maNV);
    std::shared_ptr<NhanVien> timNhanVien(const std::string &maNV) const;
    const std::vector<std::shared_ptr<NhanVien>> &layDanhSachNhanVien() const { return danhSachNhanVien; }
};
