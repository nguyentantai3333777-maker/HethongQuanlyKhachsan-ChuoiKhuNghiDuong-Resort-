// LoaiPhong.h - enum loai phong
#pragma once
#include <string>

enum class LoaiPhong { STANDARD, DELUXE, SUITE, VILLA };

// Chuyen enum sang chuoi de hien thi
inline std::string loaiPhongToString(LoaiPhong loai)
{
    switch (loai)
    {
    case LoaiPhong::STANDARD: return "Standard";
    case LoaiPhong::DELUXE:   return "Deluxe";
    case LoaiPhong::SUITE:    return "Suite";
    case LoaiPhong::VILLA:    return "Villa";
    }
    return "Unknown";
}
