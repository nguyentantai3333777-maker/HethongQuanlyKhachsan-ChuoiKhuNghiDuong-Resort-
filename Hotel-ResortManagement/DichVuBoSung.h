// DichVuBoSung.h - Class DichVu va cac dich vu cu the
#ifndef DICHVUBOSUNG_H
#define DICHVUBOSUNG_H

#include <iostream>
#include <string>
#include <iomanip>
#include <stdexcept>

// 1. LỚP CƠ SỞ TRỪU TƯỢNG: DichVu
class DichVu
{
private:
    std::string maDichVu;

protected:
    std::string tenDichVu;
    double donGia;
    std::string moTaDichVu;

public:
    DichVu() : maDichVu(""), tenDichVu(""), donGia(0.0), moTaDichVu("") {}

    DichVu(std::string ma, std::string ten, double gia, std::string moTa = "")
        : maDichVu(ma), tenDichVu(ten), donGia(gia), moTaDichVu(moTa)
    {
        if (!(gia >= 0))
            throw std::invalid_argument("Don gia khong hop le.");
    }

    virtual ~DichVu() {}

    // Phương thức ảo tính chi phí
    virtual double tinhChiPhi(int soLuong) const
    {
        if (soLuong <= 0)
            throw std::invalid_argument("So luong phai lon hon 0.");
        return donGia * soLuong;
    }

    // Phương thức thuần ảo lấy loại dịch vụ
    virtual std::string layLoaiDichVu() const = 0;

    // Getters & Setters
    std::string layMaDichVu() const { return maDichVu; }
    std::string layTenDichVu() const { return tenDichVu; }
    std::string layTen() const { return tenDichVu; }
    double layDonGia() const { return donGia; }
    void datDonGia(double gia) { donGia = gia; }

    friend std::ostream &operator<<(std::ostream &os, const DichVu &dv)
    {
        os << "[" << dv.maDichVu << "] " << dv.tenDichVu
           << " | Don gia: " << std::fixed << std::setprecision(0) << dv.donGia << " VND"
           << " | Mo ta: " << dv.moTaDichVu;
        return os;
    }
};

// 2. LỚP DỊCH VỤ CỤ THỂ: AmThuc
class AmThuc : public DichVu
{
protected:
    std::string loaiBuaAn;
    int soSuatAn;
    double phiPhucVu;

public:
    AmThuc(std::string ma = "DV_AT", std::string ten = "Am Thuc", double gia = 250000,
           std::string moTa = "", std::string loaiBua = "Thuong", int soSuat = 1, double phiDichVu = 0.0)
        : DichVu(ma, ten, gia, moTa), loaiBuaAn(loaiBua), soSuatAn(soSuat), phiPhucVu(phiDichVu) {}

    double tinhChiPhi(int soLuong) const override
    {
        return (donGia * soLuong) + phiPhucVu;
    }

    std::string layLoaiDichVu() const override
    {
        return "Am thuc (" + loaiBuaAn + ")";
    }
};

// 3. LỚP DỊCH VỤ CỤ THỂ: Spa
class Spa : public DichVu
{
protected:
    int thoiLuongPhut;
    std::string loaiLieuTrinh;
    std::string tenKyThuatVien;

public:
    Spa(std::string ma = "DV_SPA", std::string ten = "Spa Thu Gian", double gia = 400000,
        std::string moTa = "", int thoiLuong = 60, std::string lieuTrinh = "Toan than", std::string ktv = "N/A")
        : DichVu(ma, ten, gia, moTa), thoiLuongPhut(thoiLuong), loaiLieuTrinh(lieuTrinh), tenKyThuatVien(ktv) {}

    double tinhChiPhi(int soLuong) const override
    {
        return donGia * soLuong;
    }

    std::string layLoaiDichVu() const override
    {
        return "Spa & Tri lieu";
    }
};

// 4. LỚP DỊCH VỤ CỤ THỂ: TourTuyen
class TourTuyen : public DichVu
{
protected:
    std::string diaDiem;
    int soNguoiThamGia;
    std::string ngayKhoiHanh;
    double phiHuongDanVien;

public:
    TourTuyen(std::string ma = "DV_TOUR", std::string ten = "Tour Tron Goi", double gia = 650000,
              std::string moTa = "", std::string dd = "Nhanh resort", int soNguoi = 1, std::string ngay = "N/A", double phiHDV = 0.0)
        : DichVu(ma, ten, gia, moTa), diaDiem(dd), soNguoiThamGia(soNguoi), ngayKhoiHanh(ngay), phiHuongDanVien(phiHDV) {}

    double tinhChiPhi(int soLuong) const override
    {
        return (donGia * soLuong) + phiHuongDanVien;
    }

    std::string layLoaiDichVu() const override
    {
        return "Tour du lich";
    }
};

#endif