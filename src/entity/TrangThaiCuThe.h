// TrangThaiCuThe.h - Concrete States
//
//   Trong --giuCho--> GiuCho --nhanPhong--> DangO --traPhong--> DangDon --donPhong--> Trong
//     |                  \--huyGiuCho/het han--> Trong             |
//     |--nhanPhong (khach vang lai) --> DangO                      |--baoTri--> BaoTri
//     \--baoTri--> BaoTri --hoanTat--> Trong
#pragma once
#include <ctime>
#include <string>
#include "TrangThaiPhong.h"

class PhongTrong : public TrangThaiPhong
{
public:
    void giuCho(Phong *phong, std::time_t hanGiuCho) override;
    void nhanPhong(Phong *phong) override;
    void baoTri(Phong *phong, const std::string &lyDo) override;
    bool coTheDat() const override { return true; }
    MaTrangThai layMa() const override { return MaTrangThai::TRONG; }
};

class PhongGiuCho : public TrangThaiPhong
{
private:
    std::time_t hanGiuCho;

public:
    explicit PhongGiuCho(std::time_t han);
    bool daHetHan() const;
    std::time_t layHanGiuCho() const { return hanGiuCho; }
    void nhanPhong(Phong *phong) override;
    void huyGiuCho(Phong *phong) override;
    std::string ghiChu() const override;
    MaTrangThai layMa() const override { return MaTrangThai::GIU_CHO; }
};

class PhongDangO : public TrangThaiPhong
{
public:
    void traPhong(Phong *phong) override;
    MaTrangThai layMa() const override { return MaTrangThai::DANG_O; }
};

class PhongDangDon : public TrangThaiPhong
{
public:
    void donPhong(Phong *phong) override;
    void baoTri(Phong *phong, const std::string &lyDo) override;
    MaTrangThai layMa() const override { return MaTrangThai::DANG_DON; }
};

class PhongBaoTri : public TrangThaiPhong
{
private:
    std::string lyDoBaoTri;

public:
    explicit PhongBaoTri(const std::string &lyDo = "Bao tri dinh ky");
    void hoanTat(Phong *phong) override;
    std::string ghiChu() const override { return lyDoBaoTri; }
    MaTrangThai layMa() const override { return MaTrangThai::BAO_TRI; }
};
