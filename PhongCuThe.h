// PhongCuThe.h - Concrete Components: StandardRoom, DeluxeRoom, Suite, Villa
#pragma once
#include "Phong.h"
#include <string>
#include <vector>

class StandardRoom : public Phong
{
protected:
    std::string huongNhin;

public:
    StandardRoom(const std::string &ma, const std::string &so, int tang, double gia,
                 int sucChua, double dienTich, const std::string &huongNhin = "Thanh pho");

    void datHuongNhin(const std::string &hn);
    const std::string &layHuongNhin() const { return huongNhin; }
    double tinhGiaPhong(int soDem, MuaDuLich mua) const override;
    std::string moTa() const override;
    std::vector<std::string> layDichVuDiKem() const override;
    std::vector<std::pair<std::string, std::string>> layThongTinRieng() const override;
};

class DeluxeRoom : public Phong
{
protected:
    bool coBanCong;
    std::string huongNhin;

public:
    DeluxeRoom(const std::string &ma, const std::string &so, int tang, double gia,
               int sucChua, double dienTich, bool coBanCong = true,
               const std::string &huongNhin = "Bien");

    void datCoBanCong(bool v) { coBanCong = v; }
    void datHuongNhin(const std::string &hn);
    bool layCoBanCong() const { return coBanCong; }
    const std::string &layHuongNhin() const { return huongNhin; }
    double tinhGiaPhong(int soDem, MuaDuLich mua) const override;
    std::string moTa() const override;
    std::vector<std::string> layDichVuDiKem() const override;
    std::vector<std::pair<std::string, std::string>> layThongTinRieng() const override;
};

// Suite: tien phong + phi buffet sang moi dem
class Suite : public Phong
{
protected:
    int soPhongNgu;
    bool coPhongKhach;
    double phiBuffetSang;

public:
    Suite(const std::string &ma, const std::string &so, int tang, double gia,
          int sucChua, double dienTich, int soPhongNgu = 1, bool coPhongKhach = true,
          double phiBuffetSang = 300000);

    void datSoPhongNgu(int n);
    void datCoPhongKhach(bool v) { coPhongKhach = v; }
    void datPhiBuffetSang(double phi);
    int laySoPhongNgu() const { return soPhongNgu; }
    bool layCoPhongKhach() const { return coPhongKhach; }
    double layPhiBuffetSang() const { return phiBuffetSang; }
    double tinhGiaPhong(int soDem, MuaDuLich mua) const override;
    std::string moTa() const override;
    std::vector<std::string> layDichVuDiKem() const override;
    std::vector<std::pair<std::string, std::string>> layThongTinRieng() const override;
};

// Villa: tien phong + phi quan gia (butler) moi dem
class Villa : public Phong
{
protected:
    bool coHoBoi;
    double dienTichSanVuon;
    int soPhongNgu;
    double phiQuanGia;

public:
    Villa(const std::string &ma, const std::string &so, int tang, double gia,
          int sucChua, double dienTich, bool coHoBoi = true, double dienTichSanVuon = 100,
          int soPhongNgu = 3, double phiQuanGia = 500000);

    void datCoHoBoi(bool v) { coHoBoi = v; }
    void datDienTichSanVuon(double dt);
    void datSoPhongNgu(int n);
    void datPhiQuanGia(double phi);
    bool layCoHoBoi() const { return coHoBoi; }
    double layDienTichSanVuon() const { return dienTichSanVuon; }
    int laySoPhongNgu() const { return soPhongNgu; }
    double layPhiQuanGia() const { return phiQuanGia; }
    double tinhGiaPhong(int soDem, MuaDuLich mua) const override;
    std::string moTa() const override;
    std::vector<std::string> layDichVuDiKem() const override;
    std::vector<std::pair<std::string, std::string>> layThongTinRieng() const override;
};
