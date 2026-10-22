// TrangThaiCuThe.h - 5 CONCRETE STATE (gom vao 1 file cho gon)
#pragma once
#include <ctime>
#include <string>
#include "TrangThaiPhong.h"

// ---- Phong trong: giu cho / nhan phong / bao tri ----
class PhongTrong : public TrangThaiPhong
{
public:
    void giuCho(Phong* phong) override;
    void nhanPhong(Phong* phong) override;
    void baoTri(Phong* phong) override;
    std::string layTenTrangThai() const override;
};

// ---- Phong dang giu cho: nhan phong / huy giu cho (hoanTat) ----
class PhongGiuCho : public TrangThaiPhong
{
private:
    std::time_t hanGiuCho;           // thoi diem het han giu cho
    bool kiemTraHetHan() const;
public:
    explicit PhongGiuCho(int soGio = 2);
    void nhanPhong(Phong* phong) override;
    void hoanTat(Phong* phong) override;   // huy giu cho -> Trong
    std::string layTenTrangThai() const override;
};

// ---- Phong dang o: chi duoc tra phong ----
class PhongDangO : public TrangThaiPhong
{
public:
    void traPhong(Phong* phong) override;
    std::string layTenTrangThai() const override;
};

// ---- Phong dang don: don xong -> hoanTat -> Trong ----
class PhongDangDon : public TrangThaiPhong
{
public:
    void donPhong(Phong* phong) override;   // chi thong bao, khong doi trang thai
    void hoanTat(Phong* phong) override;
    std::string layTenTrangThai() const override;
};

// ---- Phong bao tri: chi duoc hoanTat ----
class PhongBaoTri : public TrangThaiPhong
{
private:
    std::string lyDoBaoTri;
public:
    explicit PhongBaoTri(const std::string& lyDo = "Bao tri dinh ky");
    void hoanTat(Phong* phong) override;
    std::string layTenTrangThai() const override;
};
