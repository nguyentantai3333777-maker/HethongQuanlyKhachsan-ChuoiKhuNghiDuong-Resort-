// HoaDon.h - Hoa don thanh toan cho PhieuDatPhong
// HoaDon gom ChiTietHoaDon (Composition, 1 - nhieu) va dung DichVu (nhieu - nhieu)
#pragma once
#include <ctime>
#include <memory>
#include <string>
#include <vector>

class PhieuDatPhong;
class NhanVien;
class DichVu;

class ChiTietHoaDon
{
private:
    std::string loai;        // "Phong" | "Dich vu"
    std::string maThamChieu; // ma phong / ma dich vu
    std::string noiDung;
    int soLuong;
    double donGia;
    double thanhTien;

public:
    ChiTietHoaDon(const std::string &loai, const std::string &maThamChieu, const std::string &noiDung,
                  int soLuong, double donGia, double thanhTien);
    const std::string &layLoai() const { return loai; }
    const std::string &layMaThamChieu() const { return maThamChieu; }
    const std::string &layNoiDung() const { return noiDung; }
    int laySoLuong() const { return soLuong; }
    double layDonGia() const { return donGia; }
    double layThanhTien() const { return thanhTien; }
};

// Ket qua tinh tien: Tam tinh -> giam gia khach hang -> VAT -> tong thanh toan
struct KetQuaTinhTien
{
    double tamTinh;
    double tyLeGiam;
    double tienGiam;
    double tienThue;
    double tong;
};

class HoaDon
{
private:
    std::string maHoaDon;
    std::shared_ptr<PhieuDatPhong> phieu;
    std::shared_ptr<NhanVien> nhanVienLap;
    std::time_t thoiGianLap;
    std::time_t thoiGianThanhToan;
    std::vector<ChiTietHoaDon> chiTiet;                // Composition
    std::vector<std::shared_ptr<DichVu>> dichVuSuDung; // nhieu - nhieu
    double tongTheoDecorator; // tong tinh qua chuoi Decorator (doi chieu voi tong chi tiet)
    double tyLeGiamGia;
    double tienCoc; // khach da dat coc truoc
    std::string phuongThucThanhToan;
    bool daThanhToan;

public:
    static constexpr double THUE_VAT = 0.10;
    // MOT cong thuc duy nhat cho hoa don that, bao gia staff, bao gia khach va tran coc:
    //   giam = tamTinh * tyLeGiam;  VAT = (tamTinh - giam) * 10%;  tong = tamTinh - giam + VAT
    static KetQuaTinhTien tinhTien(double tamTinh, double tyLeGiam);

    HoaDon(const std::string &ma, std::shared_ptr<PhieuDatPhong> phieu,
           std::shared_ptr<NhanVien> nv, double tongTheoDecorator, double tyLeGiamGia,
           double tienCoc = 0);

    void themChiTiet(const ChiTietHoaDon &ct) { chiTiet.push_back(ct); }
    void themDichVu(const std::shared_ptr<DichVu> &dv) { dichVuSuDung.push_back(dv); }

    const std::string &layMa() const { return maHoaDon; }
    std::shared_ptr<PhieuDatPhong> layPhieu() const { return phieu; }
    std::shared_ptr<NhanVien> layNhanVienLap() const { return nhanVienLap; }
    std::time_t layThoiGianLap() const { return thoiGianLap; }
    std::time_t layThoiGianThanhToan() const { return thoiGianThanhToan; }
    const std::vector<ChiTietHoaDon> &layChiTiet() const { return chiTiet; }
    const std::vector<std::shared_ptr<DichVu>> &layDichVuSuDung() const { return dichVuSuDung; }
    double layTongTheoDecorator() const { return tongTheoDecorator; }
    double layTyLeGiamGia() const { return tyLeGiamGia; }
    const std::string &layPhuongThuc() const { return phuongThucThanhToan; }
    bool layDaThanhToan() const { return daThanhToan; }

    double tienPhong() const;
    double tienDichVu() const;
    double tamTinh() const;  // tong cac dong chi tiet
    double tienGiam() const; // giam theo hang thanh vien
    double tienThue() const; // VAT tinh tren (tam tinh - giam)
    double tongThanhToan() const; // tong gia tri hoa don (sau giam, gom VAT)
    double layTienCoc() const { return tienCoc; }
    double conLaiPhaiTra() const;  // khach con phai tra them = max(0, tong - coc)
    double tienCocDu() const;      // coc lon hon tong -> phan du phai HOAN lai cho khach = max(0, coc - tong)
    double tienCocDaTru() const;   // phan coc duoc tru vao hoa don = min(coc, tong)
    bool khopVoiDecorator() const; // kiem tra tinh nhat quan

    void thanhToan(const std::string &phuongThuc, std::time_t t);
    void datThoiGianLap(std::time_t t) { thoiGianLap = t; } // du lieu mau
    void khoiPhuc(std::time_t tgLap, std::time_t tgThanhToan, bool daTT, const std::string &phuongThuc);
};
