// TrangThaiPhong.h - State Pattern Interface
#pragma once
#include <string>

class Phong;

class TrangThaiPhong
{
public:
    virtual ~TrangThaiPhong() = default;
    virtual void giuCho(Phong *) {}
    virtual void nhanPhong(Phong *) {}
    virtual void traPhong(Phong *) {}
    virtual void donPhong(Phong *) {}
    virtual void baoTri(Phong *) {}
    virtual void hoanTat(Phong *) {}
    virtual std::string layTenTrangThai() const = 0;
};