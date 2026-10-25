// ChuongTrinh.h
#pragma once
#include <memory>
#include <string>
#include <vector>
#include "QuanLyPhong.h"
#include "TaiKhoan.h"
#include "KhachHang.h"
#include "PhieuDatPhong.h"
#include "HoaDon.h"
#include "QuanLyLuuTruFile.h"
#include "QuanLyBaoCao.h"

class ChuongTrinh
{
private:
    QuanLyPhong qlPhong;
    QuanLyTaiKhoan qlTaiKhoan;
    QuanLyBaoCao qlBaoCao;
    std::vector<PhieuDatPhong> danhSachPhieuDat;
    std::string userHienTai;
    Role roleHienTai;

    void khoiTaoDuLieuMau();
    bool dangNhap();
    void yeuCauAdmin() const;

    void menuAdmin();
    void menuUser();

    void xuLyThemPhong();
    void xuLyXoaPhong();
    void xuLyXemTheoTrangThai();
    void xuLyChuyenTrangThai();

    // Các chức năng mới bổ sung
    void xuLyDatPhong();
    void xuLyTraPhongVaThanhToan();
    void xuLyXemBaoCaoDoanhThu();
    void xuLyLuuLichSuFile();

public:
    ChuongTrinh();
    void chay();
};