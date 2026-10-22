// ChuongTrinh.h - dieu khien menu console (de main() chi con 2 dong)
#pragma once
#include <memory>
#include <string>
#include "QuanLyPhong.h"
#include "TaiKhoan.h"

class ChuongTrinh
{
private:
    QuanLyPhong qlPhong;
    QuanLyTaiKhoan qlTaiKhoan;
    std::string userHienTai;
    Role roleHienTai;

    void khoiTaoDuLieuMau();
    bool dangNhap();
    void yeuCauAdmin() const; // nem runtime_error neu khong phai ADMIN

    void menuAdmin();
    void menuUser();

    // --- chuc nang ADMIN ---
    void xuLyThemPhong();
    void themMotPhong();
    void xuLyXoaPhong();
    void xuLySuaPhong();
    void suaMotPhong(const std::shared_ptr<Phong>& p);
    void xuLyXemTheoTrangThai();
    void xuLyChuyenTrangThai();
    void xuLyQuanLyTaiKhoan();
    // --- chuc nang dung chung ---
    void xuLyXemTatCa();
    void xuLyTimKiem();
    void xuLyXemPhongTrong();
    void xuLyXemChiTiet();
    // --- chuc nang USER ---
    void xuLyDatPhong();
    void datMotPhong();
    void xuLyTraPhong();

public:
    ChuongTrinh();
    void chay();
};
