// TaiKhoan.h - Tai khoan dang nhap (gan voi mot NhanVien) va phan quyen
#pragma once
#include <memory>
#include <string>
#include <vector>

class NhanVien;

enum class Role
{
    ADMIN, // quan ly: toan quyen
    USER   // le tan: nghiep vu dat phong, check-in/out, thanh toan
};

inline std::string roleToString(Role r) { return r == Role::ADMIN ? "ADMIN (Quan ly)" : "USER (Le tan)"; }
bool laChucVuQuanLy(const std::string &chucVu);
bool laChucVuLeTan(const std::string &chucVu);
bool chucVuKhopVaiTro(const std::string &chucVu, Role role);

class TaiKhoan
{
private:
    std::string username;
    std::string password;
    Role role;
    std::string maNhanVien;

public:
    TaiKhoan(const std::string &user, const std::string &pass, Role r, const std::string &maNV);
    const std::string &layUsername() const { return username; }
    Role layRole() const { return role; }
    const std::string &layMaNhanVien() const { return maNhanVien; }
    bool kiemTraMatKhau(const std::string &pass) const { return password == pass; }
    const std::string &matKhauLuuFile() const { return password; }
    void datMatKhau(const std::string &passMoi) { password = passMoi; }
};

class QuanLyTaiKhoan
{
private:
    std::vector<TaiKhoan> danhSach;

public:
    static void kiemTraUsername(const std::string &user);
    static void kiemTraMatKhau(const std::string &pass);

    void themTaiKhoan(const std::string &user, const std::string &pass, Role role, const std::string &maNV);
    void xoaTaiKhoan(const std::string &user);
    void doiMatKhau(const std::string &user, const std::string &passCu, const std::string &passMoi);
    const TaiKhoan &dangNhap(const std::string &user, const std::string &pass) const;
    const TaiKhoan *timTheoUsername(const std::string &user) const;
    const TaiKhoan *timTheoNhanVien(const std::string &maNV) const;
    const std::vector<TaiKhoan> &layDanhSach() const { return danhSach; }
    int demAdmin() const;
    void damBaoTaiKhoanNhanSu(const std::vector<std::shared_ptr<NhanVien>> &nhanVien);

    void ghiFile(const std::string &duongDan) const;
    bool docFile(const std::string &duongDan); // false neu chua co file
};
