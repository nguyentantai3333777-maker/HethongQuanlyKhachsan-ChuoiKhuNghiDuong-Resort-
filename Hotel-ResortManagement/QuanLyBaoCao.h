// QuanLyBaoCao.h - Thong ke va bao cao doanh thu
#pragma once
#include <vector>
#include <iostream>
#include <iomanip>
#include "HoaDon.h"

class QuanLyBaoCao
{
private:
    std::vector<HoaDon> danhSachHoaDon;

public:
    void themHoaDon(const HoaDon &hd)
    {
        danhSachHoaDon.push_back(hd);
    }

    double tinhTongDoanhThu() const
    {
        double tong = 0;
        for (const auto &hd : danhSachHoaDon)
        {
            tong += hd.layTongTien();
        }
        return tong;
    }

    void inBaoCaoDoanhThu() const
    {
        std::cout << "\n======================================================\n";
        std::cout << "                BAO CAO DOANH THU RESORT              \n";
        std::cout << "======================================================\n";
        std::cout << "Tong so hoa don da thanh toan: " << danhSachHoaDon.size() << "\n";
        for (const auto &hd : danhSachHoaDon)
        {
            std::cout << " -> HD [" << hd.layMaHD() << "] Ngay: " << hd.layNgayThanhToan()
                      << " | So tien: " << dinhDangTien(hd.layTongTien()) << " VND\n";
        }
        std::cout << "------------------------------------------------------\n";
        std::cout << "TONG DOANH THU TICH LUY: " << dinhDangTien(tinhTongDoanhThu()) << " VND\n";
        std::cout << "======================================================\n\n";
    }
};