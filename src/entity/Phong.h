// Phong.h - Lop co so Phong (Context cua State Pattern, Component cua Decorator Pattern)
#pragma once
#include <ctime>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "LoaiPhong.h"
#include "MuaDuLich.h"
#include "TrangThaiPhong.h"

class Phong
{
private:
    std::string maPhong;
    std::shared_ptr<TrangThaiPhong> trangThai; // State Pattern: Phong HAS-A TrangThaiPhong (1)

protected:
    std::string soPhong;
    int tang;
    LoaiPhong loaiPhong;
    double giaCoBan; // gia moi dem (VND)
    int sucChua;
    double dienTich;
    std::string chiNhanh;

public:
    static const int SO_DEM_UU_DAI = 7;      // o tu 7 dem tro len
    static constexpr double TY_LE_UU_DAI = 0.10; // giam 10% tien phong

    Phong();
    Phong(const std::string &maPhong, const std::string &soPhong, int tang, LoaiPhong loai,
          double giaCoBan, int sucChua, double dienTich);
    virtual ~Phong() = default;

    // ---- Da hinh ----
    virtual double tinhGiaPhong(int soDem, MuaDuLich mua) const;
    virtual std::string moTa() const;
    virtual std::vector<std::string> layDichVuDiKem() const;
    virtual std::vector<std::pair<std::string, std::string>> layThongTinRieng() const;

    // ---- State Pattern: uy quyen cho doi tuong trang thai ----
    void giuCho(std::time_t hanGiuCho);
    void huyGiuCho();
    void nhanPhong();
    void traPhong();
    void donPhong();
    void baoTri(const std::string &lyDo);
    void hoanTat();
    void datTrangThai(std::shared_ptr<TrangThaiPhong> tt);
    std::shared_ptr<TrangThaiPhong> layTrangThai() const { return trangThai; }
    std::string layTenTrangThai() const { return trangThai->layTenTrangThai(); }
    MaTrangThai layMaTrangThai() const { return trangThai->layMa(); }
    bool coTheDat() const { return trangThai->coTheDat(); }
    std::string ghiChuTrangThai() const { return trangThai->ghiChu(); }

    // ---- Getter ----
    std::string layMaPhong() const { return maPhong; }
    std::string laySoPhong() const { return soPhong; }
    int layTang() const { return tang; }
    LoaiPhong layLoaiPhong() const { return loaiPhong; }
    double layGiaPhong() const { return giaCoBan; }
    int laySucChua() const { return sucChua; }
    double layDienTich() const { return dienTich; }
    std::string layChiNhanh() const { return chiNhanh; }

    // ---- Setter (co kiem tra) ----
    void datMaPhong(const std::string &ma);
    void datSoPhong(const std::string &so);
    void datTang(int t);
    void datLoaiPhong(LoaiPhong loai);
    void datGiaCoBan(double gia);
    void datSucChua(int sc);
    void datDienTich(double dt);
    void datChiNhanh(const std::string &cn);

    friend std::ostream &operator<<(std::ostream &os, const Phong &p);
    friend bool operator==(const Phong &p1, const Phong &p2);
};
