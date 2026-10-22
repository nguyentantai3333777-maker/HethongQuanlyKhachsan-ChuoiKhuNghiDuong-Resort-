// TaiKhoan.h - TaiKhoan va QuanLyTaiKhoan
#pragma once
#include <string>
#include <vector>

enum class Role { ADMIN, USER };

inline std::string roleToString(Role r) { return r == Role::ADMIN ? "ADMIN" : "USER"; }

class TaiKhoan
{
private:
    std::string username;
    std::string password;
    Role role;
public:
    TaiKhoan(const std::string& user, const std::string& pass, Role r);
    std::string layUsername() const { return username; }
    Role layRole() const { return role; }
    bool kiemTraMatKhau(const std::string& pass) const { return password == pass; }
    void datMatKhau(const std::string& passMoi) { password = passMoi; }
};

class QuanLyTaiKhoan
{
private:
    std::vector<TaiKhoan> danhSach;
    static void kiemTraDinhDang(const std::string& user, const std::string& pass);
public:
    void themTaiKhoan(const std::string& user, const std::string& pass, Role role);
    void xoaTaiKhoan(const std::string& user);
    void doiMatKhau(const std::string& user, const std::string& passMoi);
    const TaiKhoan& dangNhap(const std::string& user, const std::string& pass) const;
    void hienThi() const;
    int demAdmin() const;
};
