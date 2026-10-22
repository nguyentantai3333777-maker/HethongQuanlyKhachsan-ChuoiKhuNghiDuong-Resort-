// TaiKhoan.cpp
#include "TaiKhoan.h"
#include <iostream>
#include <stdexcept>

using namespace std;

TaiKhoan::TaiKhoan(const string& user, const string& pass, Role r)
    : username(user), password(pass), role(r) {}

void QuanLyTaiKhoan::kiemTraDinhDang(const string& user, const string& pass)
{
    if (user.empty()) throw invalid_argument("Ten dang nhap khong duoc de trong.");
    if (pass.empty()) throw invalid_argument("Mat khau khong duoc de trong.");
    if (user.size() < 3 || user.find(' ') != string::npos)
        throw invalid_argument("Ten dang nhap phai tu 3 ky tu va khong chua khoang trang.");
    if (pass.size() < 4 || pass.find(' ') != string::npos)
        throw invalid_argument("Mat khau phai tu 4 ky tu va khong chua khoang trang.");
}

void QuanLyTaiKhoan::themTaiKhoan(const string& user, const string& pass, Role role)
{
    kiemTraDinhDang(user, pass);
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (danhSach[i].layUsername() == user)
            throw invalid_argument("Ten dang nhap '" + user + "' da ton tai.");
    danhSach.push_back(TaiKhoan(user, pass, role));
}

void QuanLyTaiKhoan::xoaTaiKhoan(const string& user)
{
    for (size_t i = 0; i < danhSach.size(); ++i)
    {
        if (danhSach[i].layUsername() == user)
        {
            if (danhSach[i].layRole() == Role::ADMIN && demAdmin() <= 1)
                throw logic_error("Khong the xoa ADMIN cuoi cung cua he thong.");
            danhSach.erase(danhSach.begin() + (long)i);
            return;
        }
    }
    throw runtime_error("Khong tim thay tai khoan '" + user + "'.");
}

void QuanLyTaiKhoan::doiMatKhau(const string& user, const string& passMoi)
{
    if (passMoi.size() < 4 || passMoi.find(' ') != string::npos)
        throw invalid_argument("Mat khau phai tu 4 ky tu va khong chua khoang trang.");
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (danhSach[i].layUsername() == user) { danhSach[i].datMatKhau(passMoi); return; }
    throw runtime_error("Khong tim thay tai khoan '" + user + "'.");
}

const TaiKhoan& QuanLyTaiKhoan::dangNhap(const string& user, const string& pass) const
{
    if (user.empty()) throw invalid_argument("Ten dang nhap dang de trong.");
    if (pass.empty()) throw invalid_argument("Mat khau dang de trong.");
    for (size_t i = 0; i < danhSach.size(); ++i)
    {
        if (danhSach[i].layUsername() == user)
        {
            if (!danhSach[i].kiemTraMatKhau(pass)) throw runtime_error("Sai mat khau.");
            return danhSach[i];
        }
    }
    throw runtime_error("Khong tim thay tai khoan.");
}

void QuanLyTaiKhoan::hienThi() const
{
    cout << "Danh sach tai khoan:\n";
    for (size_t i = 0; i < danhSach.size(); ++i)
        cout << "  " << i + 1 << ". " << danhSach[i].layUsername()
             << " (" << roleToString(danhSach[i].layRole()) << ")\n";
}

int QuanLyTaiKhoan::demAdmin() const
{
    int dem = 0;
    for (size_t i = 0; i < danhSach.size(); ++i)
        if (danhSach[i].layRole() == Role::ADMIN) ++dem;
    return dem;
}
