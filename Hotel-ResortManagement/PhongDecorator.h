// PhongDecorator.h - Decorator Pattern
#ifndef PHONGDECORATOR_H
#define PHONGDECORATOR_H

#include "Phong.h"
#include "DichVuBoSung.h"
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>

class PhongDecorator : public Phong
{
protected:
    std::shared_ptr<Phong> phongDuocBoc;

public:
    explicit PhongDecorator(std::shared_ptr<Phong> p) : Phong(), phongDuocBoc(p)
    {
        if (!p)
            throw std::invalid_argument("Phong duoc boc null.");
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

class DecoratorAmThuc : public PhongDecorator
{
    std::shared_ptr<AmThuc> dichVuAmThuc;
    int soSuat;

public:
    DecoratorAmThuc(std::shared_ptr<Phong> p, std::shared_ptr<AmThuc> dv, int suat)
        : PhongDecorator(p), dichVuAmThuc(dv), soSuat(suat) {}

    double tinhGiaPhong(int soDem, MuaDuLich mua) const override
    {
        return phongDuocBoc->tinhGiaPhong(soDem, mua) + dichVuAmThuc->tinhChiPhi(soSuat);
    }
};

class DecoratorSpa : public PhongDecorator
{
    std::shared_ptr<Spa> dichVuSpa;
    int soLanSuDung;

public:
    DecoratorSpa(std::shared_ptr<Phong> p, std::shared_ptr<Spa> dv, int soLan)
        : PhongDecorator(p), dichVuSpa(dv), soLanSuDung(soLan) {}

    double tinhGiaPhong(int soDem, MuaDuLich mua) const override
    {
        return phongDuocBoc->tinhGiaPhong(soDem, mua) + dichVuSpa->tinhChiPhi(soLanSuDung);
    }
};

class DecoratorTour : public PhongDecorator
{
    std::shared_ptr<TourTuyen> dichVuTour;
    int soNguoi;

public:
    DecoratorTour(std::shared_ptr<Phong> p, std::shared_ptr<TourTuyen> dv, int nguoi)
        : PhongDecorator(p), dichVuTour(dv), soNguoi(nguoi) {}

    double tinhGiaPhong(int soDem, MuaDuLich mua) const override
    {
        return phongDuocBoc->tinhGiaPhong(soDem, mua) + dichVuTour->tinhChiPhi(soNguoi * soDem);
    }
};

class AnSangDecorator : public PhongDecorator
{
public:
    explicit AnSangDecorator(std::shared_ptr<Phong> p) : PhongDecorator(p) {}
    double tinhGiaPhong(int soDem, MuaDuLich mua) const override
    {
        return phongDuocBoc->tinhGiaPhong(soDem, mua) + 150000.0 * soDem;
    }
};

class MinibarDecorator : public PhongDecorator
{
public:
    explicit MinibarDecorator(std::shared_ptr<Phong> p) : PhongDecorator(p) {}
    double tinhGiaPhong(int soDem, MuaDuLich mua) const override
    {
        return phongDuocBoc->tinhGiaPhong(soDem, mua) + 200000.0 * soDem;
    }
};

#endif