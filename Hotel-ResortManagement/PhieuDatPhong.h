// PhieuDatPhong.h
#pragma once
#include <string>
#include <vector>
#include <memory>
#include "KhachHang.h"
#include "Phong.h"
#include "MuaDuLich.h"

enum class TrangThaiDatPhong
{
    DA_DAT,
    DA_NHAN_PHONG,
    DA_TRA_PHONG,
    DA_HUY
};

class PhieuDatPhong
{
private:
    std::string maPhieu;
    KhachHang khachHang;
    std::string maPhong;
    std::string ngayCheckIn;  // YYYY-MM-DD
    std::string ngayCheckOut; // YYYY-MM-DD
    int soDem;
    MuaDuLich muaDuLich;
    double tienCoc;
    TrangThaiDatPhong trangThai;

public:
    PhieuDatPhong(std::string maP, KhachHang kh, std::string maPhong, std::string in, std::string out, int dem, MuaDuLich mua, double coc = 0)
        : maPhieu(maP), khachHang(kh), maPhong(maPhong), ngayCheckIn(in), ngayCheckOut(out),
          soDem(dem), muaDuLich(mua), tienCoc(coc), trangThai(TrangThaiDatPhong::DA_DAT) {}

    std::string layMaPhieu() const { return maPhieu; }
    std::string layMaPhong() const { return maPhong; }
    KhachHang layKhachHang() const { return khachHang; }
    int laySoDem() const { return soDem; }
    MuaDuLich layMua() const { return muaDuLich; }
    double layTienCoc() const { return tienCoc; }
    TrangThaiDatPhong layTrangThai() const { return trangThai; }

    void datTrangThai(TrangThaiDatPhong tt) { trangThai = tt; }
};