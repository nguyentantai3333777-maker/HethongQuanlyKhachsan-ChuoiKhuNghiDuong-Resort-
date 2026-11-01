// MuaDuLich.h - enum mua du lich va he so gia
#pragma once
#include <stdexcept>
#include <string>

enum class MuaDuLich
{
    THAP_DIEM = 1,
    BINH_THUONG,
    CAO_DIEM,
    LE_TET
};

inline std::string muaToString(MuaDuLich mua)
{
    switch (mua)
    {
    case MuaDuLich::THAP_DIEM:
        return "Thap diem";
    case MuaDuLich::BINH_THUONG:
        return "Binh thuong";
    case MuaDuLich::CAO_DIEM:
        return "Cao diem";
    case MuaDuLich::LE_TET:
        return "Le/Tet";
    }
    return "Unknown";
}

// He so nhan voi tien phong theo mua
inline double heSoMua(MuaDuLich mua)
{
    switch (mua)
    {
    case MuaDuLich::THAP_DIEM:
        return 0.8;
    case MuaDuLich::BINH_THUONG:
        return 1.0;
    case MuaDuLich::CAO_DIEM:
        return 1.3;
    case MuaDuLich::LE_TET:
        return 1.6;
    }
    throw std::invalid_argument("Mua du lich khong hop le.");
}
