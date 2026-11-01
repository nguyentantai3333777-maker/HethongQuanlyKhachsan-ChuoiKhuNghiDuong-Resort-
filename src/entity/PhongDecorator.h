// PhongDecorator.h - Decorator Pattern
// PhongDecorator IS-A Phong va HAS-A Phong (phongDuocBoc, 1).
// Moi decorator cong them chi phi mot dich vu vao tinhGiaPhong() cua phong duoc boc,
// nen co the boc long nhieu lop: Tour(Spa(AmThuc(Suite))).
#pragma once
#include "Phong.h"
#include "DichVu.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

class PhongDecorator : public Phong
{
protected:
    std::shared_ptr<Phong> phongDuocBoc;

    static const Phong &kiemTra(const std::shared_ptr<Phong> &p)
    {
        if (!p)
            throw std::invalid_argument("Phong duoc boc null.");
        return *p;
    }

public:
    // Sao chep thong tin co ban cua phong duoc boc (ma, so, loai, gia, trang thai...)
    explicit PhongDecorator(std::shared_ptr<Phong> p) : Phong(kiemTra(p)), phongDuocBoc(p) {}
    virtual ~PhongDecorator() = default;

    std::shared_ptr<Phong> layPhongDuocBoc() const { return phongDuocBoc; }

    double tinhGiaPhong(int soDem, MuaDuLich mua) const override
    {
        return phongDuocBoc->tinhGiaPhong(soDem, mua);
    }
    std::string moTa() const override { return phongDuocBoc->moTa(); }
    std::vector<std::string> layDichVuDiKem() const override { return phongDuocBoc->layDichVuDiKem(); }
    std::vector<std::pair<std::string, std::string>> layThongTinRieng() const override
    {
        return phongDuocBoc->layThongTinRieng();
    }
    virtual double chiPhiDichVu() const = 0;
};

class DecoratorAmThuc : public PhongDecorator
{
private:
    std::shared_ptr<AmThuc> dichVuAmThuc;
    int soSuat;

public:
    DecoratorAmThuc(std::shared_ptr<Phong> p, std::shared_ptr<AmThuc> dv, int suat)
        : PhongDecorator(p), dichVuAmThuc(dv), soSuat(suat)
    {
        if (!dv || suat <= 0)
            throw std::invalid_argument("Dich vu am thuc khong hop le.");
    }
    double chiPhiDichVu() const override { return dichVuAmThuc->tinhChiPhi(soSuat); }
    double tinhGiaPhong(int soDem, MuaDuLich mua) const override
    {
        return phongDuocBoc->tinhGiaPhong(soDem, mua) + chiPhiDichVu();
    }
    std::string moTa() const override
    {
        return phongDuocBoc->moTa() + " + " + dichVuAmThuc->layTen() + " x" + std::to_string(soSuat);
    }
    std::vector<std::string> layDichVuDiKem() const override
    {
        auto ds = phongDuocBoc->layDichVuDiKem();
        ds.push_back(dichVuAmThuc->layTen() + " x" + std::to_string(soSuat) + " suat");
        return ds;
    }
};

class DecoratorSpa : public PhongDecorator
{
private:
    std::shared_ptr<Spa> dichVuSpa;
    int soLanSuDung;

public:
    DecoratorSpa(std::shared_ptr<Phong> p, std::shared_ptr<Spa> dv, int soLan)
        : PhongDecorator(p), dichVuSpa(dv), soLanSuDung(soLan)
    {
        if (!dv || soLan <= 0)
            throw std::invalid_argument("Dich vu spa khong hop le.");
    }
    double chiPhiDichVu() const override { return dichVuSpa->tinhChiPhi(soLanSuDung); }
    double tinhGiaPhong(int soDem, MuaDuLich mua) const override
    {
        return phongDuocBoc->tinhGiaPhong(soDem, mua) + chiPhiDichVu();
    }
    std::string moTa() const override
    {
        return phongDuocBoc->moTa() + " + " + dichVuSpa->layTen() + " x" + std::to_string(soLanSuDung);
    }
    std::vector<std::string> layDichVuDiKem() const override
    {
        auto ds = phongDuocBoc->layDichVuDiKem();
        ds.push_back(dichVuSpa->layTen() + " x" + std::to_string(soLanSuDung) + " lan");
        return ds;
    }
};

class DecoratorTour : public PhongDecorator
{
private:
    std::shared_ptr<TourTuyen> dichVuTour;
    int soNguoi;

public:
    DecoratorTour(std::shared_ptr<Phong> p, std::shared_ptr<TourTuyen> dv, int nguoi)
        : PhongDecorator(p), dichVuTour(dv), soNguoi(nguoi)
    {
        if (!dv || nguoi <= 0)
            throw std::invalid_argument("Dich vu tour khong hop le.");
    }
    double chiPhiDichVu() const override { return dichVuTour->tinhChiPhi(soNguoi); }
    double tinhGiaPhong(int soDem, MuaDuLich mua) const override
    {
        return phongDuocBoc->tinhGiaPhong(soDem, mua) + chiPhiDichVu();
    }
    std::string moTa() const override
    {
        return phongDuocBoc->moTa() + " + " + dichVuTour->layTen() + " x" + std::to_string(soNguoi) + " nguoi";
    }
    std::vector<std::string> layDichVuDiKem() const override
    {
        auto ds = phongDuocBoc->layDichVuDiKem();
        ds.push_back(dichVuTour->layTen() + " x" + std::to_string(soNguoi) + " nguoi");
        return ds;
    }
};

// Dich vu tien ich (vd Don san bay) - loai "Tien ich" trong dichvu.csv
class DecoratorTienIch : public PhongDecorator
{
private:
    std::shared_ptr<DichVuTienIch> dichVuTienIch;
    int soLuot;

public:
    DecoratorTienIch(std::shared_ptr<Phong> p, std::shared_ptr<DichVuTienIch> dv, int luot)
        : PhongDecorator(p), dichVuTienIch(dv), soLuot(luot)
    {
        if (!dv || luot <= 0)
            throw std::invalid_argument("Dich vu tien ich khong hop le.");
    }
    double chiPhiDichVu() const override { return dichVuTienIch->tinhChiPhi(soLuot); }
    double tinhGiaPhong(int soDem, MuaDuLich mua) const override
    {
        return phongDuocBoc->tinhGiaPhong(soDem, mua) + chiPhiDichVu();
    }
    std::string moTa() const override
    {
        return phongDuocBoc->moTa() + " + " + dichVuTienIch->layTen() + " x" + std::to_string(soLuot);
    }
    std::vector<std::string> layDichVuDiKem() const override
    {
        auto ds = phongDuocBoc->layDichVuDiKem();
        ds.push_back(dichVuTienIch->layTen() + " x" + std::to_string(soLuot));
        return ds;
    }
};
