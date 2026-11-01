// TrangThaiPhong.h - State Pattern: interface trang thai phong
#pragma once
#include <ctime>
#include <stdexcept>
#include <string>

class Phong;

// Ma trang thai dung de so sanh/loc (khong so sanh bang chuoi)
enum class MaTrangThai
{
    TRONG,
    GIU_CHO,
    DANG_O,
    DANG_DON,
    BAO_TRI
};

inline std::string tenTrangThai(MaTrangThai ma)
{
    switch (ma)
    {
    case MaTrangThai::TRONG:
        return "Trong";
    case MaTrangThai::GIU_CHO:
        return "Dang giu cho";
    case MaTrangThai::DANG_O:
        return "Dang o";
    case MaTrangThai::DANG_DON:
        return "Dang don dep";
    case MaTrangThai::BAO_TRI:
        return "Dang bao tri";
    }
    return "Unknown";
}

// Moi trang thai chi cho phep mot so chuyen doi hop le.
// Mac dinh moi hanh dong deu bi tu choi (nem ngoai le) -> lop con override hanh dong hop le.
class TrangThaiPhong
{
protected:
    std::runtime_error khongHopLe(const std::string &hanhDong) const
    {
        return std::runtime_error("Khong the " + hanhDong + " khi phong o trang thai '" +
                                  layTenTrangThai() + "'.");
    }

public:
    virtual ~TrangThaiPhong() = default;

    virtual void giuCho(Phong *, std::time_t /*hanGiuCho*/) { throw khongHopLe("giu cho"); }
    virtual void huyGiuCho(Phong *) { throw khongHopLe("huy giu cho"); }
    virtual void nhanPhong(Phong *) { throw khongHopLe("nhan phong (check-in)"); }
    virtual void traPhong(Phong *) { throw khongHopLe("tra phong (check-out)"); }
    virtual void donPhong(Phong *) { throw khongHopLe("hoan tat don dep"); }
    virtual void baoTri(Phong *, const std::string & /*lyDo*/) { throw khongHopLe("chuyen sang bao tri"); }
    virtual void hoanTat(Phong *) { throw khongHopLe("hoan tat bao tri"); }

    virtual bool coTheDat() const { return false; }
    virtual std::string ghiChu() const { return ""; }
    virtual MaTrangThai layMa() const = 0;
    std::string layTenTrangThai() const { return tenTrangThai(layMa()); }
};
