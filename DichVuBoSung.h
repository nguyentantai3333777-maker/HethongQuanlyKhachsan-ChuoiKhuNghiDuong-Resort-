// DichVuBoSung.h - cac dich vu duoc gan them vao phong qua Decorator
#ifndef DICHVUBOSUNG_H
#define DICHVUBOSUNG_H

#include <string>
#include <stdexcept>

class DichVu
{
protected:
    std::string ten;
    double donGia;

public:
    DichVu(const std::string& t, double gia) : ten(t), donGia(gia)
    {
        if (!(gia >= 0)) throw std::invalid_argument("Don gia dich vu khong hop le.");
    }
    virtual ~DichVu() {}
    const std::string& layTen() const { return ten; }
    double layDonGia() const { return donGia; }

    // soLuong: so suat / so lan / so nguoi
    virtual double tinhChiPhi(int soLuong) const
    {
        if (soLuong <= 0) throw std::invalid_argument("So luong dich vu phai lon hon 0.");
        return donGia * soLuong;
    }
};

class AmThuc : public DichVu
{
public:
    AmThuc(const std::string& t = "Am thuc", double giaMoiSuat = 250000) : DichVu(t, giaMoiSuat) {}
};

class Spa : public DichVu
{
public:
    Spa(const std::string& t = "Spa", double giaMoiLan = 400000) : DichVu(t, giaMoiLan) {}
};

class TourTuyen : public DichVu
{
public:
    // Tour tron goi: gia an uong cho 1 nguoi trong 1 ngay.
    // soLuong truyen vao tinhChiPhi = so nguoi * so dem.
    TourTuyen(const std::string& t = "Tour tron goi", double giaMoiNguoiMoiNgay = 650000)
        : DichVu(t, giaMoiNguoiMoiNgay) {}
};

#endif