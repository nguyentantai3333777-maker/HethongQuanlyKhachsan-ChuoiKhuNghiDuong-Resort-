// TrangThaiPhong.cpp
#include "TrangThaiPhong.h"
#include <stdexcept>

void TrangThaiPhong::tuChoi(const std::string& hanhDong) const
{
    throw std::logic_error("Khong the " + hanhDong + " khi phong dang o trang thai: " + layTenTrangThai() + ".");
}

void TrangThaiPhong::giuCho(Phong*)   { tuChoi("giu cho"); }
void TrangThaiPhong::nhanPhong(Phong*){ tuChoi("nhan phong"); }
void TrangThaiPhong::traPhong(Phong*) { tuChoi("tra phong"); }
void TrangThaiPhong::donPhong(Phong*) { tuChoi("don phong"); }
void TrangThaiPhong::baoTri(Phong*)   { tuChoi("bao tri"); }
void TrangThaiPhong::hoanTat(Phong*)  { tuChoi("hoan tat"); }
