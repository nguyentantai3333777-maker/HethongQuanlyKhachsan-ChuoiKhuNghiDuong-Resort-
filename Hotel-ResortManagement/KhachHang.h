// KhachHang.h
#pragma once
#include <string>
#include <iostream>

class KhachHang
{
private:
    std::string maKhachHang;
    std::string hoTen;
    std::string cccd;
    std::string soDienThoai;
    std::string email;

public:
    KhachHang() = default;
    KhachHang(std::string ma, std::string ten, std::string cccd, std::string sdt, std::string email = "")
        : maKhachHang(ma), hoTen(ten), cccd(cccd), soDienThoai(sdt), email(email) {}

    std::string layMa() const { return maKhachHang; }
    std::string layTen() const { return hoTen; }
    std::string layCCCD() const { return cccd; }
    std::string laySDT() const { return soDienThoai; }
    std::string layEmail() const { return email; }

    void hienThi() const
    {
        std::cout << "KH [" << maKhachHang << "] " << hoTen
                  << " | CCCD: " << cccd << " | SDT: " << soDienThoai << "\n";
    }
};