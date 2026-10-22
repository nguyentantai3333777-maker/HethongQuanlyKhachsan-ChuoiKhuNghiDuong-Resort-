// MuaDuLich.h - enum mua du lich
#pragma once
#include <string>

enum class MuaDuLich { THAP_DIEM, BINH_THUONG, CAO_DIEM, LE_TET };

inline std::string muaToString(MuaDuLich mua)
{
    switch (mua)
    {
    case MuaDuLich::THAP_DIEM:   return "Thap diem";
    case MuaDuLich::BINH_THUONG: return "Binh thuong";
    case MuaDuLich::CAO_DIEM:    return "Cao diem";
    case MuaDuLich::LE_TET:      return "Le/Tet";
    }
    return "Unknown";
}
