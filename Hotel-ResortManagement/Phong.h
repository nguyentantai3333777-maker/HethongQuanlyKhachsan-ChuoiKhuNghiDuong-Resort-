// Phong.h - Context
#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "LoaiPhong.h"
#include "MuaDuLich.h"
#include "TrangThaiPhong.h"

std::string dinhDangTien(double soTien);

class Phong
{
private:
    std::string maPhong;
    std::shared_ptr<TrangThaiPhong> trangThai;

protected:
    std::string soPhong;
    int tang;
    LoaiPhong loaiPhong;
    double giaCoBan;
    int sucChua;
    double dienTich;
    std::string chiNhanh;

    double tinhHeSoMua(MuaDuLich mua) const;

public:
    Phong();
    Phong(std::string maPhong, std::string soPhong, int tang, LoaiPhong loai,
          double giaCoBan, int sucChua, double dienTich);
    virtual ~Phong() = default;

    virtual double tinhGiaPhong(int soDem, MuaDuLich mua) const;
    virtual std::string moTa() const;
    virtual std::vector<std::string> layDichVuDiKem() const;
    virtual std::vector<std::pair<std::string, std::string>> layThongTinRieng() const;
    static std::string tieuDeBang();

    void giuCho();
    void nhanPhong();
    void traPhong();
    void donPhong();
    void baoTri();
    void hoanTat();
    void datTrangThai(std::shared_ptr<TrangThaiPhong> tt);
    std::string layTenTrangThai() const;

    double layHeSoMua(MuaDuLich mua) const { return tinhHeSoMua(mua); }

    std::string layMaPhong() const { return maPhong; }
    std::string laySoPhong() const { return soPhong; }
    int layTang() const { return tang; }
    LoaiPhong layLoaiPhong() const { return loaiPhong; }
    double layGiaPhong() const { return giaCoBan; }
    int laySucChua() const { return sucChua; }
    double layDienTich() const { return dienTich; }
    std::string layChiNhanh() const { return chiNhanh; }

    void datMaPhong(const std::string &ma);
    void datSoPhong(const std::string &so);
    void datTang(int t);
    void datLoaiPhong(LoaiPhong loai);
    void datGiaCoBan(double gia);
    void datSucChua(int sc);
    void datDienTich(double dt);
    void datChiNhanh(const std::string &cn);

    friend std::ostream &operator<<(std::ostream &os, const Phong &p);
    friend std::istream &operator>>(std::istream &is, Phong &p);
    friend bool operator==(const Phong &p1, const Phong &p2);
};