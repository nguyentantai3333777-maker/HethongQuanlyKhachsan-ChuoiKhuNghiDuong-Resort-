// PhongCuThe.h - Concrete Components
#ifndef PHONGCUTHE_H
#define PHONGCUTHE_H

#include "Phong.h"
#include <string>
#include <vector>

class StandardRoom : public Phong
{
protected:
    std::string huongNhin;

public:
    StandardRoom(const std::string &ma, const std::string &so, int tang, double gia,
                 int sucChua, double dienTich, const std::string &huongNhin = "Bien");

    void datHuongNhin(const std::string &hn);
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
    double tinhGiaPhong(int soDem, MuaDuLich mua) const override;
    std::string moTa() const override;
    std::vector<std::string> layDichVuDiKem() const override;
    std::vector<std::pair<std::string, std::string>> layThongTinRieng() const override;
};

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
    double tinhGiaPhong(int soDem, MuaDuLich mua) const override;
    std::string moTa() const override;
    std::vector<std::string> layDichVuDiKem() const override;
    std::vector<std::pair<std::string, std::string>> layThongTinRieng() const override;
};

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
    double tinhGiaPhong(int soDem, MuaDuLich mua) const override;
    std::string moTa() const override;
    std::vector<std::string> layDichVuDiKem() const override;
    std::vector<std::pair<std::string, std::string>> layThongTinRieng() const override;
};

#endif