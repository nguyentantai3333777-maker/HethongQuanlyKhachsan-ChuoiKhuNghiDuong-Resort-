// PhieuDatPhong.h - Phieu dat phong: lien ket KhachHang, Phong, NhanVien (nhieu - 1)
#pragma once
#include <ctime>
#include <memory>
#include <string>
#include <vector>
#include "MuaDuLich.h"
#include "TienIch.h"

class KhachHang;
class NhanVien;
class Phong;
class DichVu;

enum class TrangThaiPhieu
{
    DA_DAT,        // da dat, phong dang giu cho
    DA_NHAN_PHONG, // da check-in, khach dang o
    DA_TRA_PHONG,  // da check-out, da lap hoa don
    DA_HUY
};

inline std::string trangThaiPhieuToString(TrangThaiPhieu t)
{
    switch (t)
    {
    case TrangThaiPhieu::DA_DAT:
        return "Cho nhan phong";
    case TrangThaiPhieu::DA_NHAN_PHONG:
        return "Dang o";
    case TrangThaiPhieu::DA_TRA_PHONG:
        return "Da tra phong";
    case TrangThaiPhieu::DA_HUY:
        return "Da huy";
    }
    return "Unknown";
}

// Mot lan su dung dich vu trong thoi gian luu tru (dich vu da duoc clone -> chot gia)
struct SuDungDichVu
{
    std::shared_ptr<DichVu> dichVu;
    int soLuong;
    std::time_t thoiGian;
    double thanhTien() const;
};

class PhieuDatPhong
{
private:
    std::string maPhieu;
    std::shared_ptr<KhachHang> khachHang;
    std::shared_ptr<Phong> phong;
    std::shared_ptr<NhanVien> nhanVien; // nhan vien lap phieu

    std::time_t thoiGianDat;
    std::time_t thoiGianCheckIn;
    std::time_t thoiGianCheckOut;
    std::time_t thoiGianHuy;

    Ngay ngayNhan;
    int soDem;
    int soNguoi;
    MuaDuLich mua;
    TrangThaiPhieu trangThai;
    std::string ghiChu;
    std::string lyDoHuy;
    std::string maHoaDon;
    std::vector<SuDungDichVu> dichVuSuDung;
    double tienCoc;
    std::string phuongThucCoc;
    bool daHoanCoc;

public:
    // kiemTraSucChua = false khi doc lai lich su tu file (suc chua phong co the da bi sua sau do)
    PhieuDatPhong(const std::string &ma, std::shared_ptr<KhachHang> kh, std::shared_ptr<Phong> p,
                  std::shared_ptr<NhanVien> nv, const Ngay &ngayNhan, int soDem, int soNguoi,
                  MuaDuLich mua, const std::string &ghiChu = "", bool kiemTraSucChua = true);

    // ---- Getter ----
    const std::string &layMaPhieu() const { return maPhieu; }
    std::shared_ptr<KhachHang> layKhachHang() const { return khachHang; }
    std::shared_ptr<Phong> layPhong() const { return phong; }
    std::shared_ptr<NhanVien> layNhanVien() const { return nhanVien; }
    std::time_t layThoiGianDat() const { return thoiGianDat; }
    std::time_t layThoiGianCheckIn() const { return thoiGianCheckIn; }
    std::time_t layThoiGianCheckOut() const { return thoiGianCheckOut; }
    std::time_t layThoiGianHuy() const { return thoiGianHuy; }
    const Ngay &layNgayNhan() const { return ngayNhan; }
    Ngay layNgayTra() const { return ngayNhan.congNgay(soDem); }
    int laySoDem() const { return soDem; }
    int laySoNguoi() const { return soNguoi; }
    MuaDuLich layMua() const { return mua; }
    TrangThaiPhieu layTrangThai() const { return trangThai; }
    const std::string &layGhiChu() const { return ghiChu; }
    const std::string &layLyDoHuy() const { return lyDoHuy; }
    const std::string &layMaHoaDon() const { return maHoaDon; }
    const std::vector<SuDungDichVu> &layDichVuSuDung() const { return dichVuSuDung; }
    double layTienCoc() const { return tienCoc; }
    const std::string &layPhuongThucCoc() const { return phuongThucCoc; }
    bool layDaHoanCoc() const { return daHoanCoc; }
    // Phieu "chiem" phong de kiem tra trung lich:
    //  - DA_DAT:         chiem [ngayNhan, ngayTra)
    //  - DA_NHAN_PHONG:  chiem VO THOI HAN ke tu ngayNhan cho den khi khach that su check-out
    //                    (khong tin vao ngay tra du kien -> khach o qua han van khoa phong)
    Ngay ngayKetThucChiemPhong() const; // chi de HIEN THI: ngay tra du kien (hoac mai neu da qua han)
    bool chiemVoThoiHan() const { return trangThai == TrangThaiPhieu::DA_NHAN_PHONG; }
    bool dangOverstay() const;           // dang o ma da qua ngay tra du kien (chua check-out)
    int soNgayOverstay() const;          // so ngay qua han (0 neu chua qua han)
    // Check-out SOM: khach dang o, hom nay chua den ngay tra du kien. Ngay tra du kien chi la du kien, khong chan check-out.
    bool traSom() const;                 // dang o va hom nay < ngay tra du kien
    int soDemThucTe() const;             // so dem tinh phi: ngay nhan -> hom nay (gom qua han); toi thieu 1
    void rutNganSoDem(int soDemThucTe);  // chot so dem = so dem thuc te khi check-out som (giai phong phong tu ngay do)
    bool trungLich(const Ngay &tu, int soDemKhac) const;
    // Gia han them soDemThem dem cho khach dang o. Chi doi so dem; QuanLyKhachSan kiem tra xung dot lich truoc.
    void giaHan(int soDemThem, bool kiemTraTran = true);
    bool dangHoatDong() const
    {
        return trangThai == TrangThaiPhieu::DA_DAT || trangThai == TrangThaiPhieu::DA_NHAN_PHONG;
    }

    // ---- Tinh tien ----
    double tinhTienPhong(int soDemThem = 0) const; // soDemThem: dem qua han se tinh them luc check-out
    double tinhTienDichVu() const;
    double tamTinh() const { return tinhTienPhong() + tinhTienDichVu(); }

    // ---- Chuyen trang thai phieu (dong bo voi trang thai phong do QuanLyKhachSan dieu phoi) ----
    void xacNhanCheckIn(std::time_t t);
    void xacNhanCheckOut(std::time_t t, const std::string &maHoaDon);
    void huy(std::time_t t, const std::string &lyDo, bool hoanCoc = false);
    void datCoc(double soTien, const std::string &phuongThuc);
    void themDichVu(const std::shared_ptr<DichVu> &dvDaChotGia, int soLuong, std::time_t t);
    // Chuyen phieu sang phong khac: CHI doi phong; giu nguyen ma phieu, khach, ngay, coc, dich vu, trang thai.
    // Chi ap dung cho phieu "Cho nhan phong"; QuanLyKhachSan kiem tra lich trong cua phong moi truoc.
    void doiPhong(const std::shared_ptr<Phong> &phongMoi);

    // Dung cho du lieu mau va khi doc file
    void datThoiGianDat(std::time_t t) { thoiGianDat = t; }
    void khoiPhuc(std::time_t tgDat, std::time_t tgCheckIn, std::time_t tgCheckOut, std::time_t tgHuy,
                  TrangThaiPhieu tt, const std::string &lyDoHuy, const std::string &maHoaDon,
                  double tienCoc, const std::string &phuongThucCoc, bool daHoanCoc);
    void khoiPhucDichVu(const std::shared_ptr<DichVu> &dv, int soLuong, std::time_t t);
};
