// Nguoi.h - Lop truu tuong Nguoi va cac lop con KhachHang, NhanVien
#pragma once
#include <memory>
#include <string>
#include <utility>
#include <vector>

class PhieuDatPhong;

class Nguoi
{
protected:
    std::string ma;
    std::string hoTen;
    std::string soDienThoai;
    std::string email;
    std::string gioiTinh;

public:
    Nguoi(const std::string &ma, const std::string &hoTen, const std::string &soDienThoai,
            const std::string &email = "", const std::string &gioiTinh = "");
    virtual ~Nguoi() = default;

    const std::string &layMa() const { return ma; }
    const std::string &layHoTen() const { return hoTen; }
    const std::string &laySoDienThoai() const { return soDienThoai; }
    const std::string &layEmail() const { return email; }
    const std::string &layGioiTinh() const { return gioiTinh; }

    void datHoTen(const std::string &ten);
    void datSoDienThoai(const std::string &sdt);
    void datEmail(const std::string &e);
    void datGioiTinh(const std::string &gt);

    // ---- Da hinh ----
    virtual std::string layVaiTro() const = 0;
    virtual std::vector<std::pair<std::string, std::string>> layThongTin() const;
};

// ===================== Hang thanh vien =====================
enum class HangThanhVien
{
    THANH_VIEN,
    BAC,
    VANG,
    KIM_CUONG
};

std::string hangToString(HangThanhVien h);
double tyLeGiamCuaHang(HangThanhVien h); // 0%, 5%, 10%, 15%
int diemToiThieuCuaHang(HangThanhVien h);  // 0, 100, 400, 1000

// Khach hang: giu lich su dat phong (1 - nhieu), tich diem va hang thanh vien
class KhachHang : public Nguoi
{
private:
    std::string cccd;
    std::string quocTich;
    std::string diaChiThuongTru;
    std::string maChiNhanh; // CN01 = Nha Trang, CN02 = Quy Nhon (phan chia theo chi nhanh)
    int diemTichLuy;
    HangThanhVien hang;
    std::vector<std::weak_ptr<PhieuDatPhong>> lichSuDatPhong; // weak_ptr tranh vong tham chieu

public:
    static const int TIEN_MOI_DIEM = 100000; // 100.000 VND = 1 diem

    KhachHang(const std::string &ma, const std::string &hoTen, const std::string &sdt,
              const std::string &cccd, const std::string &email = "",
              const std::string &quocTich = "Viet Nam", const std::string &diaChi = "",
              const std::string &gioiTinh = "", const std::string &maCN = "");

    const std::string &layCCCD() const { return cccd; }
    const std::string &layQuocTich() const { return quocTich; }
    const std::string &layDiaChiThuongTru() const { return diaChiThuongTru; }
    const std::string &layMaChiNhanh() const { return maChiNhanh; }
    int layDiemTichLuy() const { return diemTichLuy; }
    HangThanhVien layHang() const { return hang; }

    void datCCCD(const std::string &so);
    void datQuocTich(const std::string &qt);
    void datDiaChiThuongTru(const std::string &diaChi);
    void datMaChiNhanh(const std::string &cn) { maChiNhanh = cn; }
    void datDiem(int diem);              // dung khi doc file
    void datHang(HangThanhVien h) { hang = h; } // cap nhat thu cong
    void congDiem(int diem);              // tu dong nang hang khi du diem (khong tu ha hang)

    std::string hangThanhVien() const { return hangToString(hang); }
    double tyLeGiamGia() const { return tyLeGiamCuaHang(hang); }

    void themLichSu(const std::shared_ptr<PhieuDatPhong> &phieu);
    std::vector<std::shared_ptr<PhieuDatPhong>> layLichSuDatPhong() const;
    int soLanLuuTru() const; // so lan da tra phong

    std::string layVaiTro() const override { return "Khach hang"; }
    std::vector<std::pair<std::string, std::string>> layThongTin() const override;
};

class NhanVien : public Nguoi
{
private:
    std::string chucVu;
    double luongCoBan;
    std::string maChiNhanh;
    std::string cccd;
    std::string ngaySinh;
    bool dangLam; // xoa nhan vien = danh dau nghi viec (giu lich su phieu/hoa don)
    bool tamNghi;
    std::string lyDoNghi; // ly do cho nghi viec (luu khi nghi/xoa)

public:
    static constexpr double LUONG_CO_BAN_TOI_THIEU = 5500000.0; // muc toi thieu theo quy dinh du an

    NhanVien(const std::string &ma, const std::string &hoTen, const std::string &sdt,
             const std::string &chucVu, double luong, const std::string &maChiNhanh,
             const std::string &email = "", const std::string &cccd = "",
             const std::string &gioiTinh = "", const std::string &ngaySinh = "");

    const std::string &layChucVu() const { return chucVu; }
    double layLuongCoBan() const { return luongCoBan; }
    double heSoLuong() const;
    double luongThucNhan() const { return luongCoBan * heSoLuong(); }
    const std::string &layMaChiNhanh() const { return maChiNhanh; }
    const std::string &layCCCD() const { return cccd; }
    const std::string &layNgaySinh() const { return ngaySinh; }
    bool layDangLam() const { return dangLam; }
    bool layTamNghi() const { return tamNghi; }
    const std::string &layLyDoNghi() const { return lyDoNghi; }

    void datChucVu(const std::string &cv);
    void datLuongCoBan(double l);
    void datCCCD(const std::string &so);
    void datNgaySinh(const std::string &ngay);
    void datMaChiNhanh(const std::string &cn) { maChiNhanh = cn; }
    void datDangLam(bool v) { dangLam = v; if (!v) tamNghi = false; }
    void datTamNghi(bool v) { tamNghi = dangLam && v; }
    void datLyDoNghi(const std::string &lyDo) { lyDoNghi = lyDo; }

    std::string layVaiTro() const override { return "Nhan vien"; }
    std::vector<std::pair<std::string, std::string>> layThongTin() const override;
};
