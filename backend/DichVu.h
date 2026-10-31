// DichVu.h - Lop truu tuong DichVu va cac dich vu cu the: AmThuc, Spa, TourTuyen, DichVuTienIch
#pragma once
#include <memory>
#include <stdexcept>
#include <string>

class DichVu
{
protected:
    std::string maDichVu;
    std::string ten;
    double donGia;
    std::string donViTinh;

public:
    DichVu(const std::string &ma, const std::string &ten, double donGia, const std::string &donVi);
    virtual ~DichVu() = default;

    const std::string &layMa() const { return maDichVu; }
    const std::string &layTen() const { return ten; }
    double layDonGia() const { return donGia; }
    const std::string &layDonViTinh() const { return donViTinh; }

    void datTen(const std::string &t);
    void datDonGia(double gia);

    // ---- Da hinh ----
    virtual std::string layLoai() const = 0;
    virtual std::string moTaChiTiet() const = 0;
    virtual double tinhChiPhi(int soLuong) const;
    // Tao ban sao (chot gia tai thoi diem su dung -> doi gia sau nay khong anh huong hoa don cu)
    virtual std::shared_ptr<DichVu> clone() const = 0;
};

class AmThuc : public DichVu
{
private:
    std::string loaiBuaAn;

public:
    AmThuc(const std::string &ma, const std::string &ten = "Am thuc", double giaMoiSuat = 250000,
           const std::string &loaiBuaAn = "Set menu");
    const std::string &layLoaiBuaAn() const { return loaiBuaAn; }
    std::string layLoai() const override { return "Am thuc"; }
    std::string moTaChiTiet() const override { return loaiBuaAn; }
    std::shared_ptr<DichVu> clone() const override { return std::make_shared<AmThuc>(*this); }
};

class Spa : public DichVu
{
private:
    int thoiLuongPhut;

public:
    Spa(const std::string &ma, const std::string &ten = "Spa", double giaMoiLan = 400000,
        int thoiLuongPhut = 60);
    int layThoiLuong() const { return thoiLuongPhut; }
    std::string layLoai() const override { return "Spa"; }
    std::string moTaChiTiet() const override { return std::to_string(thoiLuongPhut) + " phut/lan"; }
    std::shared_ptr<DichVu> clone() const override { return std::make_shared<Spa>(*this); }
};

// Tour tuyen: gia tinh theo nguoi; doan tu 5 nguoi tro len giam 10%
class TourTuyen : public DichVu
{
private:
    std::string diemDen;
    int soNgay;

public:
    static const int SO_NGUOI_GIAM_GIA = 5;

    TourTuyen(const std::string &ma, const std::string &ten = "Tour tron goi",
              double giaMoiNguoi = 650000, const std::string &diemDen = "Noi thanh", int soNgay = 1);
    const std::string &layDiemDen() const { return diemDen; }
    int laySoNgay() const { return soNgay; }
    double tinhChiPhi(int soNguoi) const override;
    std::string layLoai() const override { return "Tour"; }
    std::string moTaChiTiet() const override
    {
        return diemDen + ", " + std::to_string(soNgay) + " ngay";
    }
    std::shared_ptr<DichVu> clone() const override { return std::make_shared<TourTuyen>(*this); }
};

// Dich vu tien ich chung (vd: Don san bay) - loai "Tien ich" trong dichvu.csv, khong co truong rieng
class DichVuTienIch : public DichVu
{
public:
    DichVuTienIch(const std::string &ma, const std::string &ten = "Tien ich", double giaMoiLuot = 100000)
        : DichVu(ma, ten, giaMoiLuot, "luot") {}
    std::string layLoai() const override { return "Tien ich"; }
    std::string moTaChiTiet() const override { return "Tien ich di kem"; }
    std::shared_ptr<DichVu> clone() const override { return std::make_shared<DichVuTienIch>(*this); }
};
