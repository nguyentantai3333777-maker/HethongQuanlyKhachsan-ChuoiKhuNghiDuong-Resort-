// PhongDecorator.h - Decorator Pattern (header-only)
#ifndef PHONGDECORATOR_H
#define PHONGDECORATOR_H

#include "Phong.h"
#include "DichVuBoSung.h"
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>

// ===== Abstract Decorator =====
class PhongDecorator : public Phong
{
protected:
    std::shared_ptr<Phong> phongDuocBoc;

public:
    explicit PhongDecorator(std::shared_ptr<Phong> p) : Phong(), phongDuocBoc(p)
    {
        if (!p) throw std::invalid_argument("Phong duoc boc khong hop le (null).");
        // Sao chep thong tin nhan dang de hien thi / so sanh dung
        datMaPhong(p->layMaPhong());
        datSoPhong(p->laySoPhong());
        datLoaiPhong(p->layLoaiPhong());
    }
    virtual ~PhongDecorator() {}

    double tinhGiaPhong(int soDem, MuaDuLich mua) const override
    {
        return phongDuocBoc->tinhGiaPhong(soDem, mua);
    }
    std::string moTa() const override { return phongDuocBoc->moTa(); }
    std::vector<std::string> layDichVuDiKem() const override
    {
        return phongDuocBoc->layDichVuDiKem();
    }
};

// ===== Concrete Decorators =====
class DecoratorAmThuc : public PhongDecorator
{
    std::shared_ptr<AmThuc> dichVuAmThuc;
    int soSuat;

public:
    DecoratorAmThuc(std::shared_ptr<Phong> p, std::shared_ptr<AmThuc> dv, int suat)
        : PhongDecorator(p), dichVuAmThuc(dv), soSuat(suat)
    {
        if (!dv) throw std::invalid_argument("Dich vu am thuc khong hop le (null).");
        if (suat <= 0) throw std::invalid_argument("So suat phai lon hon 0.");
    }

    double tinhGiaPhong(int soDem, MuaDuLich mua) const override
    {
        return phongDuocBoc->tinhGiaPhong(soDem, mua) + dichVuAmThuc->tinhChiPhi(soSuat);
    }
    std::string moTa() const override
    {
        return phongDuocBoc->moTa() + " + " + dichVuAmThuc->layTen() + " x" + std::to_string(soSuat) + " suat";
    }
    std::vector<std::string> layDichVuDiKem() const override
    {
        std::vector<std::string> dv = phongDuocBoc->layDichVuDiKem();
        dv.push_back(dichVuAmThuc->layTen());
        return dv;
    }
};

class DecoratorSpa : public PhongDecorator
{
    std::shared_ptr<Spa> dichVuSpa;
    int soLanSuDung;

public:
    DecoratorSpa(std::shared_ptr<Phong> p, std::shared_ptr<Spa> dv, int soLan)
        : PhongDecorator(p), dichVuSpa(dv), soLanSuDung(soLan)
    {
        if (!dv) throw std::invalid_argument("Dich vu spa khong hop le (null).");
        if (soLan <= 0) throw std::invalid_argument("So lan su dung phai lon hon 0.");
    }

    double tinhGiaPhong(int soDem, MuaDuLich mua) const override
    {
        return phongDuocBoc->tinhGiaPhong(soDem, mua) + dichVuSpa->tinhChiPhi(soLanSuDung);
    }
    std::string moTa() const override
    {
        return phongDuocBoc->moTa() + " + " + dichVuSpa->layTen() + " x" + std::to_string(soLanSuDung) + " lan";
    }
    std::vector<std::string> layDichVuDiKem() const override
    {
        std::vector<std::string> dv = phongDuocBoc->layDichVuDiKem();
        dv.push_back(dichVuSpa->layTen());
        return dv;
    }
};

class DecoratorTour : public PhongDecorator
{
    std::shared_ptr<TourTuyen> dichVuTour;
    int soNguoi;

public:
    DecoratorTour(std::shared_ptr<Phong> p, std::shared_ptr<TourTuyen> dv, int nguoi)
        : PhongDecorator(p), dichVuTour(dv), soNguoi(nguoi)
    {
        if (!dv) throw std::invalid_argument("Dich vu tour khong hop le (null).");
        if (nguoi <= 0) throw std::invalid_argument("So nguoi phai lon hon 0.");
    }

    double tinhGiaPhong(int soDem, MuaDuLich mua) const override
    {
        return phongDuocBoc->tinhGiaPhong(soDem, mua) + dichVuTour->tinhChiPhi(soNguoi * soDem);
    }
    std::string moTa() const override
    {
        return phongDuocBoc->moTa() + " + " + dichVuTour->layTen() + " x" + std::to_string(soNguoi) + " nguoi";
    }
    std::vector<std::string> layDichVuDiKem() const override
    {
        std::vector<std::string> dv = phongDuocBoc->layDichVuDiKem();
        dv.push_back(dichVuTour->layTen());
        return dv;
    }
};

// ===== Giu lai 2 decorator cu de QuanLyPhong::tinhGia van chay =====
class AnSangDecorator : public PhongDecorator   // +150,000/dem
{
public:
    explicit AnSangDecorator(std::shared_ptr<Phong> p) : PhongDecorator(p) {}
    double tinhGiaPhong(int soDem, MuaDuLich mua) const override
    {
        return phongDuocBoc->tinhGiaPhong(soDem, mua) + 150000.0 * soDem;
    }
    std::string moTa() const override { return phongDuocBoc->moTa() + " + An sang"; }
    std::vector<std::string> layDichVuDiKem() const override
    {
        std::vector<std::string> dv = phongDuocBoc->layDichVuDiKem();
        dv.push_back("An sang them");
        return dv;
    }
};

class MinibarDecorator : public PhongDecorator  // +200,000/dem
{
public:
    explicit MinibarDecorator(std::shared_ptr<Phong> p) : PhongDecorator(p) {}
    double tinhGiaPhong(int soDem, MuaDuLich mua) const override
    {
        return phongDuocBoc->tinhGiaPhong(soDem, mua) + 200000.0 * soDem;
    }
    std::string moTa() const override { return phongDuocBoc->moTa() + " + Minibar"; }
    std::vector<std::string> layDichVuDiKem() const override
    {
        std::vector<std::string> dv = phongDuocBoc->layDichVuDiKem();
        dv.push_back("Minibar them");
        return dv;
    }
};

#endif