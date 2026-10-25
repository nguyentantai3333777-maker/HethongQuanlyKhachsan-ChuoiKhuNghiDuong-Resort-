// HoaDon.h
#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include "PhieuDatPhong.h"
#include "DichVuBoSung.h"

class HoaDon
{
private:
    std::string maHoaDon;
    PhieuDatPhong phieuDat;
    double tienPhong;
    double tienDichVu;
    double thueVAT; // VD: 0.1 (10%)
    double tongTien;
    std::string ngayThanhToan;

public:
    HoaDon(std::string maHD, PhieuDatPhong pd, double tPhong, double tDichVu, double vat = 0.10, std::string ngay = "2026-10-22")
        : maHoaDon(maHD), phieuDat(pd), tienPhong(tPhong), tienDichVu(tDichVu), thueVAT(vat), ngayThanhToan(ngay)
    {
        double tongChuaThue = (tienPhong + tienDichVu) - pd.layTienCoc();
        tongTien = tongChuaThue + (tongChuaThue * thueVAT);
    }

    std::string layMaHD() const { return maHoaDon; }
    double layTongTien() const { return tongTien; }
    std::string layNgayThanhToan() const { return ngayThanhToan; }

    void inHoaDon() const
    {
        std::cout << "\n======================================================\n";
        std::cout << "               HOA DON THANH TOAN RESORT              \n";
        std::cout << "======================================================\n";
        std::cout << "Ma hoa don: " << maHoaDon << " | Ngay: " << ngayThanhToan << "\n";
        std::cout << "Khach hang: " << phieuDat.layKhachHang().layTen()
                  << " - SĐT: " << phieuDat.layKhachHang().laySDT() << "\n";
        std::cout << "Phong: " << phieuDat.layMaPhong() << " | So dem: " << phieuDat.laySoDem() << "\n";
        std::cout << "------------------------------------------------------\n";
        std::cout << std::left << std::setw(30) << "Tien phong:" << std::right << std::setw(15) << dinhDangTien(tienPhong) << " VND\n";
        std::cout << std::left << std::setw(30) << "Tien dich vu bo sung:" << std::right << std::setw(15) << dinhDangTien(tienDichVu) << " VND\n";
        if (phieuDat.layTienCoc() > 0)
            std::cout << std::left << std::setw(30) << "Tien coc da khau tru:" << std::right << std::setw(15) << "-" + dinhDangTien(phieuDat.layTienCoc()) << " VND\n";
        std::cout << std::left << std::setw(30) << "Thue VAT (10%):" << std::right << std::setw(15) << dinhDangTien((tienPhong + tienDichVu - phieuDat.layTienCoc()) * thueVAT) << " VND\n";
        std::cout << "------------------------------------------------------\n";
        std::cout << std::left << std::setw(30) << "TONG THANH TOAN:" << std::right << std::setw(15) << dinhDangTien(tongTien) << " VND\n";
        std::cout << "======================================================\n\n";
    }
};