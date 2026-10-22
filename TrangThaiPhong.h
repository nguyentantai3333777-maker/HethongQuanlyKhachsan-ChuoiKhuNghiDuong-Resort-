// TrangThaiPhong.h - ABSTRACT STATE (State Pattern)
#pragma once
#include <string>

class Phong; // khai bao truoc de tranh include vong tron

class TrangThaiPhong
{
public:
    // Destructor ao: xoa doi tuong con qua con tro lop cha (shared_ptr<TrangThaiPhong>)
    // van goi dung destructor cua lop con
    virtual ~TrangThaiPhong() = default;

    // Mac dinh: hanh dong KHONG hop le -> nem logic_error.
    // Lop con chi override nhung hanh dong duoc phep.
    virtual void giuCho(Phong* phong);
    virtual void nhanPhong(Phong* phong);
    virtual void traPhong(Phong* phong);
    virtual void donPhong(Phong* phong);
    virtual void baoTri(Phong* phong);
    virtual void hoanTat(Phong* phong);

    // Ham thuan ao: lop con BAT BUOC cai dat -> TrangThaiPhong la lop abstract
    virtual std::string layTenTrangThai() const = 0;

protected:
    void tuChoi(const std::string& hanhDong) const; // nem std::logic_error
};
