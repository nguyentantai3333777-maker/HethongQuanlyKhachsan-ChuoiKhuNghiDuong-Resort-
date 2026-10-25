// QuanLyLuuTruFile.h - Doc va Ghi du lieu ra file text
#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "TaiKhoan.h"

class QuanLyLuuTruFile
{
public:
    // Ghi danh sach tai khoan ra file text
    static bool luuTaiKhoan(const std::string &filename, const std::vector<TaiKhoan> &danhSach)
    {
        std::ofstream file(filename);
        if (!file.is_open())
            return false;
        for (const auto &tk : danhSach)
        {
            file << tk.layUsername() << "," << (tk.layRole() == Role::ADMIN ? "ADMIN" : "USER") << "\n";
        }
        file.close();
        return true;
    }

    // Ghi nhat ky giao dịch/hóa đơn ra file log
    static bool ghiLogHoaDon(const std::string &filename, const std::string &logData)
    {
        std::ofstream file(filename, std::ios::app); // Append mode
        if (!file.is_open())
            return false;
        file << logData << "\n";
        file.close();
        return true;
    }
};