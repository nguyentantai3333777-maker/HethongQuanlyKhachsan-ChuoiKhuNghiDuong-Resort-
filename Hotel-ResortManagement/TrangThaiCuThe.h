// TrangThaiCuThe.h - Concrete States
#pragma once
#include <ctime>
#include <string>
#include "TrangThaiPhong.h"

class PhongTrong : public TrangThaiPhong
{
public:
    void giuCho(Phong *phong) override;
    void nhanPhong(Phong *phong) override;
    void baoTri(Phong *phong) override;
    std::string layTenTrangThai() const override;
};

class PhongGiuCho : public TrangThaiPhong
{
private:
    std::time_t hanGiuCho;
    bool kiemTraHetHan() const;

public:
    explicit PhongGiuCho(int soGio = 2);
    void nhanPhong(Phong *phong) override;
    void hoanTat(Phong *phong) override;
    std::string layTenTrangThai() const override;
};

class PhongDangO : public TrangThaiPhong
{
public:
    void traPhong(Phong *phong) override;
    std::string layTenTrangThai() const override;
};

class PhongDangDon : public TrangThaiPhong
{
public:
    void donPhong(Phong *phong) override;
    void hoanTat(Phong *phong) override;
    std::string layTenTrangThai() const override;
};

class PhongBaoTri : public TrangThaiPhong
{
private:
    std::string lyDoBaoTri;

public:
    explicit PhongBaoTri(const std::string &lyDo = "Bao tri dinh ky");
    void hoanTat(Phong *phong) override;
    std::string layTenTrangThai() const override;
};