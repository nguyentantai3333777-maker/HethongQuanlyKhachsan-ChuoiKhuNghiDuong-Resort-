// TaiKhoan.cpp
#include "TaiKhoan.h"
#include <iostream>
#include <stdexcept>

using namespace std;

TaiKhoan::TaiKhoan(const string &user, const string &pass, Role r)
    : username(user), password(pass), role(r) {}

void QuanLyTaiKhoan::kiemTraDinhDang(const string &user, const string &pass)
{
    if (user.empty() || pass.empty())
        throw invalid_argument("Tai khoan/Mat khau rong.");
}

void QuanLyTaiKhoan::themTaiKhoan(const string &user, const string &pass, Role role)
{
    kiemTraDinhDang(user, pass);
    danhSach.push_back(TaiKhoan(user, pass, role));
}

void QuanLyTaiKhoan::xoaTaiKhoan(const string &user)
{
    for (size_t i = 0; i < danhSach.size(); ++i)
    {
        if (danhSach[i].layUsername() == user)
        {
            danhSach.erase(danhSach.begin() + (long)i);
            return;
        }
    }
}

void QuanLyTaiKhoan::doiMatKhau(const string &user, const string &passMoi)
{
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (danhSach[i].layUsername() == user)
        {
            danhSach[i].datMatKhau(passMoi);
            return;
        }
}

const TaiKhoan &QuanLyTaiKhoan::dangNhap(const string &user, const string &pass) const
{
    for (size_t i = 0; i < danhSach.size(); ++i)
    {
        if (danhSach[i].layUsername() == user && danhSach[i].kiemTraMatKhau(pass))
            return danhSach[i];
    }
    throw runtime_error("Sai ten dang nhap hoac mat khau.");
}

void QuanLyTaiKhoan::hienThi() const
{
    for (size_t i = 0; i < danhSach.size(); ++i)
        cout << danhSach[i].layUsername() << " (" << roleToString(danhSach[i].layRole()) << ")\n";
}

int QuanLyTaiKhoan::demAdmin() const
{
    int dem = 0;
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (danhSach[i].layRole() == Role::ADMIN)
            ++dem;
    return dem;
}