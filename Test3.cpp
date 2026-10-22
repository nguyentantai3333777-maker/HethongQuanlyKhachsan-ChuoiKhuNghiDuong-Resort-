#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <regex>
#include <limits>
#include <algorithm>

// ==========================================
// ENUMS & ENUM HELPERS
// ==========================================
enum class MuaDuLich { THAP_DIEM = 1, BINH_THUONG, CAO_DIEM, LE_TET };
enum class HangThanhVien { THUONG = 1, BAC, VANG, KIM_CUONG };
enum class LoaiNhanVien { LE_TAN = 1, BUONG_PHONG, QUAN_LY, KE_TOAN };

// ==========================================
// UTILITY FUNCTIONS & INPUT VALIDATION
// ==========================================
void xoaManHinh() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pauseScreen() {
    std::cout << "\nNhan phim Enter de tiep tuc...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

int nhapSoNguyenHopLe(const std::string& thongBao, int minVal = std::numeric_limits<int>::min(), int maxVal = std::numeric_limits<int>::max()) {
    int val;
    while (true) {
        std::cout << thongBao;
        if (std::cin >> val && val >= minVal && val <= maxVal) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return val;
        }
        std::cout << ">>> LOI: Gia tri nhap vao phai la so nguyen (" << minVal << " -> " << maxVal << "). Vui long nhap lai!\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

double nhapSoThucHopLe(const std::string& thongBao, double minVal = 0.0) {
    double val;
    while (true) {
        std::cout << thongBao;
        if (std::cin >> val && val >= minVal) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return val;
        }
        std::cout << ">>> LOI: Gia tri nhap vao phai la so thuc >= " << minVal << ". Vui long nhap lai!\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

std::string nhapChuoiKhongRong(const std::string& thongBao) {
    std::string str;
    while (true) {
        std::cout << thongBao;
        std::getline(std::cin, str);
        str = trim(str);
        if (!str.empty()) return str;
        std::cout << ">>> LOI: Chuoi nhap vao khong duoc de trong! Vui long nhap lai.\n";
    }
}

std::string nhapCCCDHopLe(const std::string& thongBao) {
    std::regex pattern("^\\d{12}$");
    while (true) {
        std::string cccd = nhapChuoiKhongRong(thongBao);
        if (std::regex_match(cccd, pattern)) return cccd;
        std::cout << ">>> LOI: So CCCD phai bao gom dung 12 chu so! Vui long nhap lai.\n";
    }
}

std::string nhapSDTHopLe(const std::string& thongBao) {
    std::regex pattern("^0\\d{9}$");
    while (true) {
        std::string sdt = nhapChuoiKhongRong(thongBao);
        if (std::regex_match(sdt, pattern)) return sdt;
        std::cout << ">>> LOI: So dien thoai phai bat dau bang 0 va gom dung 10 chu so! Vui long nhap lai.\n";
    }
}

std::string nhapEmailHopLe(const std::string& thongBao) {
    std::regex pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    while (true) {
        std::string email = nhapChuoiKhongRong(thongBao);
        if (std::regex_match(email, pattern)) return email;
        std::cout << ">>> LOI: Dinh dang Email khong hop le (Vi du: ten@domain.com)! Vui long nhap lai.\n";
    }
}

std::string nhapNgayHopLe(const std::string& thongBao) {
    std::regex pattern("^\\d{4}-\\d{2}-\\d{2}$");
    while (true) {
        std::string ngay = nhapChuoiKhongRong(thongBao);
        if (std::regex_match(ngay, pattern)) return ngay;
        std::cout << ">>> LOI: Dinh dang ngay phai la YYYY-MM-DD (Vi du: 2026-10-15)! Vui long nhap lai.\n";
    }
}

// ==========================================
// 1. LỚP PHÒNG & DỊCH VỤ STUB
// ==========================================
class Phong {
protected:
    std::string maPhong;
    double giaCoBan;
public:
    Phong(std::string ma = "", double gia = 0.0) : maPhong(ma), giaCoBan(gia) {}
    virtual ~Phong() = default;
    virtual double tinhGiaPhong(int soDem, MuaDuLich mua) const {
        double heSo = 1.0;
        if (mua == MuaDuLich::CAO_DIEM) heSo = 1.2;
        else if (mua == MuaDuLich::LE_TET) heSo = 1.5;
        else if (mua == MuaDuLich::THAP_DIEM) heSo = 0.8;
        return giaCoBan * soDem * heSo;
    }
    std::string layMaPhong() const { return maPhong; }
    double layGiaCoBan() const { return giaCoBan; }
};

class DichVu {
protected:
    std::string maDichVu;
    std::string tenDichVu;
    double donGia;
public:
    DichVu(std::string ma = "", std::string ten = "", double gia = 0.0)
        : maDichVu(ma), tenDichVu(ten), donGia(gia) {}
    virtual ~DichVu() = default;
    virtual double tinhChiPhi(int soLuong) const { return donGia * soLuong; }
    std::string layTenDichVu() const { return tenDichVu; }
    std::string layMaDichVu() const { return maDichVu; }
    double layDonGia() const { return donGia; }
};

// ==========================================
// 2. LỚP CƠ SỞ NGUOI
// ==========================================
class Nguoi {
private:
    std::string maSo;
protected:
    std::string hoTen;
    std::string soCCCD;
    std::string soDienThoai;
    std::string email;

public:
    Nguoi() : maSo(""), hoTen(""), soCCCD(""), soDienThoai(""), email("") {}
    Nguoi(std::string ma, std::string ten, std::string cccd, std::string sdt, std::string em)
        : maSo(ma), hoTen(ten), soCCCD(cccd), soDienThoai(sdt), email(em) {}

    virtual ~Nguoi() = default;

    std::string layMaSo() const { return maSo; }
    std::string layHoTen() const { return hoTen; }
    std::string laySoCCCD() const { return soCCCD; }
    std::string laySoDienThoai() const { return soDienThoai; }
    std::string layEmail() const { return email; }

    virtual void hienThiThongTin() const {
        std::cout << std::left << std::setw(10) << maSo
                  << std::setw(22) << hoTen
                  << std::setw(15) << soCCCD
                  << std::setw(13) << soDienThoai
                  << std::setw(25) << email;
    }
};

// ==========================================
// 3. LỚP KHACHHANG
// ==========================================
class PhieuDatPhong;

class KhachHang : public Nguoi {
private:
    HangThanhVien hangThanhVien;
    int diemTichLuy;
    std::string quocTich;
    std::vector<std::shared_ptr<PhieuDatPhong>> lichSuDatPhong;

    void capNhatHang() {
        if (diemTichLuy >= 5000) hangThanhVien = HangThanhVien::KIM_CUONG;
        else if (diemTichLuy >= 2000) hangThanhVien = HangThanhVien::VANG;
        else if (diemTichLuy >= 500) hangThanhVien = HangThanhVien::BAC;
        else hangThanhVien = HangThanhVien::THUONG;
    }

public:
    KhachHang() : Nguoi(), hangThanhVien(HangThanhVien::THUONG), diemTichLuy(0), quocTich("Viet Nam") {}
    KhachHang(std::string ma, std::string ten, std::string cccd, std::string sdt, std::string em,
              std::string qTich = "Viet Nam", HangThanhVien hang = HangThanhVien::THUONG, int diem = 0)
        : Nguoi(ma, ten, cccd, sdt, em), hangThanhVien(hang), diemTichLuy(diem), quocTich(qTich) {
        capNhatHang();
    }

    double layTyLeChietKhau() const {
        switch (hangThanhVien) {
            case HangThanhVien::BAC: return 0.05;
            case HangThanhVien::VANG: return 0.10;
            case HangThanhVien::KIM_CUONG: return 0.15;
            default: return 0.0;
        }
    }

    void congDiemTichLuy(int diem) {
        if (diem > 0) {
            diemTichLuy += diem;
            capNhatHang();
        }
    }

    void themLichSu(std::shared_ptr<PhieuDatPhong> phieu) {
        if (phieu) lichSuDatPhong.push_back(phieu);
    }

    HangThanhVien layHangThanhVien() const { return hangThanhVien; }
    
    std::string layTenHangThanhVien() const {
        switch (hangThanhVien) {
            case HangThanhVien::BAC: return "Bac";
            case HangThanhVien::VANG: return "Vang";
            case HangThanhVien::KIM_CUONG: return "Kim Cuong";
            default: return "Thuong";
        }
    }

    void hienThiThongTin() const override {
        Nguoi::hienThiThongTin();
        std::cout << std::left << std::setw(15) << quocTich
                  << std::setw(15) << layTenHangThanhVien()
                  << std::setw(10) << diemTichLuy << "\n";
    }
};

// ==========================================
// 4. LỚP NHANVIEN (Có Phân quyền)
// ==========================================
class NhanVien : public Nguoi {
private:
    LoaiNhanVien loaiNhanVien;
    double luongCoBan;
    double heSoLuong;
protected:
    std::string chiNhanh;

public:
    NhanVien() : Nguoi(), loaiNhanVien(LoaiNhanVien::LE_TAN), luongCoBan(5000000), heSoLuong(1.0), chiNhanh("Main") {}
    NhanVien(std::string ma, std::string ten, std::string cccd, std::string sdt, std::string em,
             LoaiNhanVien loai, double luongCB, double heSo, std::string cn = "Main")
        : Nguoi(ma, ten, cccd, sdt, em), loaiNhanVien(loai), luongCoBan(luongCB), heSoLuong(heSo), chiNhanh(cn) {}

    double tinhLuong() const { return luongCoBan * heSoLuong; }
    LoaiNhanVien layLoaiNhanVien() const { return loaiNhanVien; }

    std::string layTenChucVu() const {
        switch (loaiNhanVien) {
            case LoaiNhanVien::LE_TAN: return "Le Tan";
            case LoaiNhanVien::BUONG_PHONG: return "Buong Phong";
            case LoaiNhanVien::QUAN_LY: return "Quan Ly";
            case LoaiNhanVien::KE_TOAN: return "Ke Toan";
            default: return "Nhan Vien";
        }
    }

    bool kiemTraQuyen(const std::string& hanhDong) const {
        if (loaiNhanVien == LoaiNhanVien::QUAN_LY) return true;
        if (hanhDong == "DAT_PHONG" || hanhDong == "CHECK_IN" || hanhDong == "CHECK_OUT") {
            return (loaiNhanVien == LoaiNhanVien::LE_TAN);
        }
        if (hanhDong == "LAP_HOA_DON" || hanhDong == "THONG_KE") {
            return (loaiNhanVien == LoaiNhanVien::KE_TOAN || loaiNhanVien == LoaiNhanVien::LE_TAN);
        }
        return false;
    }

    void hienThiThongTin() const override {
        Nguoi::hienThiThongTin();
        std::cout << std::left << std::setw(15) << layTenChucVu()
                  << std::setw(12) << chiNhanh
                  << std::right << std::setw(15) << std::fixed << std::setprecision(0) << tinhLuong() << " VND\n";
    }
};

// ==========================================
// 5. LỚP PHIEUDATPHONG
// ==========================================
class PhieuDatPhong {
private:
    std::string maPhieu;
    std::shared_ptr<KhachHang> khachHang;
    std::shared_ptr<Phong> phong;
    std::shared_ptr<NhanVien> nhanVienLap;
    std::string ngayCheckIn;
    std::string ngayCheckOut;
    int soNguoi;
    double tienCoc;

protected:
    MuaDuLich mua;
    bool daHuy;
    bool daCheckIn;
    bool daCheckOut;
    double phuThuNhanSomTraMuon;

public:
    PhieuDatPhong() : maPhieu(""), soNguoi(1), tienCoc(0.0), mua(MuaDuLich::BINH_THUONG),
                      daHuy(false), daCheckIn(false), daCheckOut(false), phuThuNhanSomTraMuon(0.0) {}

    PhieuDatPhong(std::string ma, std::shared_ptr<KhachHang> kh, std::shared_ptr<Phong> p,
                  std::shared_ptr<NhanVien> nv, std::string nIn, std::string nOut,
                  int sNguoi, double coc, MuaDuLich m = MuaDuLich::BINH_THUONG)
        : maPhieu(ma), khachHang(kh), phong(p), nhanVienLap(nv), ngayCheckIn(nIn),
          ngayCheckOut(nOut), soNguoi(sNguoi), tienCoc(coc), mua(m), daHuy(false),
          daCheckIn(false), daCheckOut(false), phuThuNhanSomTraMuon(0.0) {}

    int tinhSoDem() const {
        try {
            int dIn = std::stoi(ngayCheckIn.substr(8, 2));
            int dOut = std::stoi(ngayCheckOut.substr(8, 2));
            int diff = dOut - dIn;
            return diff > 0 ? diff : 1;
        } catch (...) {
            return 1;
        }
    }

    double tinhTienPhong() const {
        if (!phong) return 0.0;
        return phong->tinhGiaPhong(tinhSoDem(), mua) + phuThuNhanSomTraMuon;
    }

    void apDungPhuThu(double soTien) { phuThuNhanSomTraMuon += soTien; }

    bool checkIn() {
        if (daHuy || daCheckIn) return false;
        daCheckIn = true;
        return true;
    }

    bool checkOut() {
        if (!daCheckIn || daCheckOut) return false;
        daCheckOut = true;
        return true;
    }

    void huyPhieu() {
        if (daCheckIn) throw std::logic_error("Khong the huy phieu da CHECK-IN!");
        daHuy = true;
    }

    std::string layMaPhieu() const { return maPhieu; }
    std::shared_ptr<KhachHang> layKhachHang() const { return khachHang; }
    std::shared_ptr<Phong> layPhong() const { return phong; }
    double layTienCoc() const { return tienCoc; }
    double layPhuThu() const { return phuThuNhanSomTraMuon; }
    bool isDaCheckIn() const { return daCheckIn; }
    bool isDaCheckOut() const { return daCheckOut; }
    bool isDaHuy() const { return daHuy; }

    void hienThiThongTin() const {
        std::cout << std::left << std::setw(12) << maPhieu
                  << std::setw(20) << (khachHang ? khachHang->layHoTen() : "N/A")
                  << std::setw(10) << (phong ? phong->layMaPhong() : "N/A")
                  << std::setw(13) << ngayCheckIn
                  << std::setw(13) << ngayCheckOut
                  << std::setw(15) << (daHuy ? "DA HUY" : (daCheckOut ? "DA CHECK-OUT" : (daCheckIn ? "DANG O" : "CHO CHECK-IN")))
                  << std::right << std::setw(15) << std::fixed << std::setprecision(0) << tinhTienPhong() << " VND\n";
    }
};

// ==========================================
// 6. LỚP CHITIETHOADON & HOADON
// ==========================================
class ChiTietHoaDon {
private:
    std::string tenKhoanMuc;
    int soLuong;
    double donGia;
public:
    ChiTietHoaDon(std::string ten = "", int sl = 1, double gia = 0.0)
        : tenKhoanMuc(ten), soLuong(sl), donGia(gia) {}

    double tinhThanhTien() const { return soLuong * donGia; }
    std::string layTenKhoanMuc() const { return tenKhoanMuc; }
    int laySoLuong() const { return soLuong; }
    double layDonGia() const { return donGia; }
};

class HoaDon {
private:
    std::string maHoaDon;
    std::shared_ptr<PhieuDatPhong> phieuDatPhong;
    std::string ngayLap;
    std::vector<ChiTietHoaDon> danhSachChiTiet;
    std::vector<std::shared_ptr<DichVu>> danhSachDichVu;

protected:
    double tienPhong;
    double tyLeChietKhau;
    double thueVAT;
    bool daThanhToan;

    double tinhThue(double tongTruocThue) const { return tongTruocThue * thueVAT; }

public:
    HoaDon(std::string ma, std::shared_ptr<PhieuDatPhong> phieu, std::string nLap = "2026-10-09")
        : maHoaDon(ma), phieuDatPhong(phieu), ngayLap(nLap), thueVAT(0.1), daThanhToan(false) {
        if (!phieu) throw std::invalid_argument("Phieu dat phong khong hop le!");
        tienPhong = phieu->tinhTienPhong();
        tyLeChietKhau = phieu->layKhachHang() ? phieu->layKhachHang()->layTyLeChietKhau() : 0.0;

        danhSachChiTiet.push_back(ChiTietHoaDon("Tien Phong " + phieu->layPhong()->layMaPhong(), 1, tienPhong));
        if (phieu->layPhuThu() > 0) {
            danhSachChiTiet.push_back(ChiTietHoaDon("Phu Thu Nhan Som/Tra Muon", 1, phieu->layPhuThu()));
        }
    }

    void themDichVu(std::shared_ptr<DichVu> dv, int soLuong = 1) {
        if (dv) {
            danhSachDichVu.push_back(dv);
            danhSachChiTiet.push_back(ChiTietHoaDon("Dich Vu: " + dv->layTenDichVu(), soLuong, dv->tinhChiPhi(soLuong)));
        }
    }

    double tinhTongTienDichVu() const {
        double tongDV = 0.0;
        for (const auto& ct : danhSachChiTiet) {
            if (ct.layTenKhoanMuc().rfind("Dich Vu:", 0) == 0) tongDV += ct.tinhThanhTien();
        }
        return tongDV;
    }

    double tinhChietKhauThanhVien() const { return (tienPhong + tinhTongTienDichVu()) * tyLeChietKhau; }

    double tinhTongTien() const {
        double tongTruocThue = (tienPhong + tinhTongTienDichVu()) - tinhChietKhauThanhVien();
        double tienCoc = phieuDatPhong ? phieuDatPhong->layTienCoc() : 0.0;
        double tongThanhToan = tongTruocThue + tinhThue(tongTruocThue) - tienCoc;
        return tongThanhToan > 0 ? tongThanhToan : 0.0;
    }

    bool thanhToan() {
        if (daThanhToan) return false;
        daThanhToan = true;
        if (phieuDatPhong) {
            phieuDatPhong->checkOut();
            if (phieuDatPhong->layKhachHang()) {
                phieuDatPhong->layKhachHang()->congDiemTichLuy(static_cast<int>(tinhTongTien() / 100000));
            }
        }
        return true;
    }

    std::string layMaHoaDon() const { return maHoaDon; }
    std::shared_ptr<PhieuDatPhong> layPhieuDat() const { return phieuDatPhong; }

    void inHoaDon() const {
        std::cout << "\n========================================================================\n";
        std::cout << "                     HOA DON THANH TOAN RESORT                          \n";
        std::cout << "========================================================================\n";
        std::cout << "Ma Hoa Don : " << maHoaDon << "\t\t\tNgay Lap: " << ngayLap << "\n";
        if (phieuDatPhong && phieuDatPhong->layKhachHang()) {
            std::cout << "Khach Hang : " << phieuDatPhong->layKhachHang()->layHoTen()
                      << " (Hang: " << phieuDatPhong->layKhachHang()->layTenHangThanhVien() << ")\n";
        }
        std::cout << "------------------------------------------------------------------------\n";
        std::cout << std::left << std::setw(40) << "Khoan Muc"
                  << std::setw(10) << "So Luong"
                  << std::right << std::setw(20) << "Thanh Tien (VND)" << "\n";
        std::cout << "------------------------------------------------------------------------\n";

        for (const auto& ct : danhSachChiTiet) {
            std::cout << std::left << std::setw(40) << ct.layTenKhoanMuc()
                      << std::setw(10) << ct.laySoLuong()
                      << std::right << std::setw(20) << std::fixed << std::setprecision(0) << ct.tinhThanhTien() << "\n";
        }
        std::cout << "------------------------------------------------------------------------\n";
        std::cout << std::left << std::setw(50) << "Chiet khau Hang Thanh Vien:" 
                  << std::right << std::setw(20) << "- " + std::to_string((int)tinhChietKhauThanhVien()) << " VND\n";
        std::cout << std::left << std::setw(50) << "Thue VAT (10%):" 
                  << std::right << std::setw(20) << "+ " + std::to_string((int)tinhThue((tienPhong + tinhTongTienDichVu()) - tinhChietKhauThanhVien())) << " VND\n";
        if (phieuDatPhong && phieuDatPhong->layTienCoc() > 0) {
            std::cout << std::left << std::setw(50) << "Tien Da Dat Coc:" 
                      << std::right << std::setw(20) << "- " + std::to_string((int)phieuDatPhong->layTienCoc()) << " VND\n";
        }
        std::cout << "------------------------------------------------------------------------\n";
        std::cout << std::left << std::setw(50) << "TONG CONG THANH TOAN:" 
                  << std::right << std::setw(20) << std::fixed << std::setprecision(0) << tinhTongTien() << " VND\n";
        std::cout << "Trang Thai : " << (daThanhToan ? "DA THANH TOAN" : "CHUA THANH TOAN") << "\n";
        std::cout << "========================================================================\n\n";
    }
};

// ==========================================
// HỆ THỐNG QUẢN LÝ THỰC THỂ (DATABASE IN-MEMORY)
// ==========================================
class QuanLyThucThe {
public:
    std::vector<std::shared_ptr<KhachHang>> dsKhachHang;
    std::vector<std::shared_ptr<NhanVien>> dsNhanVien;
    std::vector<std::shared_ptr<Phong>> dsPhong;
    std::vector<std::shared_ptr<DichVu>> dsDichVu;
    std::vector<std::shared_ptr<PhieuDatPhong>> dsPhieuDat;
    std::vector<std::shared_ptr<HoaDon>> dsHoaDon;

    void khoiTaoDuLieuMau() {
        dsNhanVien.push_back(std::make_shared<NhanVien>("NV01", "Nguyen Tan Tai", "079204001111", "0901111111", "tai@resort.com", LoaiNhanVien::LE_TAN, 6000000, 1.2));
        dsNhanVien.push_back(std::make_shared<NhanVien>("NV02", "Phan Minh Tan", "079204002222", "0902222222", "tan@resort.com", LoaiNhanVien::QUAN_LY, 12000000, 2.0));
        
        dsKhachHang.push_back(std::make_shared<KhachHang>("KH01", "Tran Van A", "079204003333", "0983333333", "a@gmail.com", "Viet Nam", HangThanhVien::VANG, 2200));
        dsKhachHang.push_back(std::make_shared<KhachHang>("KH02", "John Smith", "079204004444", "0984444444", "john@gmail.com", "Musa", HangThanhVien::THUONG, 100));

        dsPhong.push_back(std::make_shared<Phong>("P101", 1000000));
        dsPhong.push_back(std::make_shared<Phong>("P102", 2000000));
        dsPhong.push_back(std::make_shared<Phong>("V201", 5000000));

        dsDichVu.push_back(std::make_shared<DichVu>("DV01", "Spa Thu Gian", 300000));
        dsDichVu.push_back(std::make_shared<DichVu>("DV02", "Buffet Sang", 150000));
        dsDichVu.push_back(std::make_shared<DichVu>("DV03", "Dua Don San Bay", 500000));
    }

    std::shared_ptr<KhachHang> timKhachHang(const std::string& ma) {
        for (auto& k : dsKhachHang) if (k->layMaSo() == ma) return k;
        return nullptr;
    }

    std::shared_ptr<NhanVien> timNhanVien(const std::string& ma) {
        for (auto& n : dsNhanVien) if (n->layMaSo() == ma) return n;
        return nullptr;
    }

    std::shared_ptr<Phong> timPhong(const std::string& ma) {
        for (auto& p : dsPhong) if (p->layMaPhong() == ma) return p;
        return nullptr;
    }

    std::shared_ptr<DichVu> timDichVu(const std::string& ma) {
        for (auto& d : dsDichVu) if (d->layMaDichVu() == ma) return d;
        return nullptr;
    }

    std::shared_ptr<PhieuDatPhong> timPhieuDat(const std::string& ma) {
        for (auto& p : dsPhieuDat) if (p->layMaPhieu() == ma) return p;
        return nullptr;
    }
};

// ==========================================
// CHƯƠNG TRÌNH CHÍNH & MENU TƯƠNG TÁC
// ==========================================
int main() {
    QuanLyThucThe ql;
    ql.khoiTaoDuLieuMau();

    std::shared_ptr<NhanVien> nhanVienHienTai = ql.dsNhanVien[0]; // Mặc định NV01 Lễ Tân

    while (true) {
        xoaManHinh();
        std::cout << "========================================================================\n";
        std::cout << "          HE THONG QUAN LY KHACH SAN & RESORT (ENTITIES & BILLING)      \n";
        std::cout << "========================================================================\n";
        std::cout << " [Nhan vien dang dang nhap: " << nhanVienHienTai->layHoTen() 
                  << " - Chuc vu: " << nhanVienHienTai->layTenChucVu() << "]\n";
        std::cout << "------------------------------------------------------------------------\n";
        std::cout << "1. Quan ly Khach Hang (Them / Hien thi danh sach)\n";
        std::cout << "2. Quan ly Nhan Vien (Them / Hien thi danh sach)\n";
        std::cout << "3. Lap Phieu Dat Phong (Dat phong theo mua & ngay)\n";
        std::cout << "4. Thuc hien Check-In / Check-Out & Phu thu\n";
        std::cout << "5. Lap Hoa Don & Thanh Toan (Tich diem & In hoa don)\n";
        std::cout << "6. Doi Tai Khoan Nhan Vien Thao Tac (Kiem tra Phan quyen)\n";
        std::cout << "0. Thoat chuong trinh\n";
        std::cout << "------------------------------------------------------------------------\n";

        int chon = nhapSoNguyenHopLe("Moi ban chon chuc nang (0-6): ", 0, 6);

        if (chon == 0) {
            std::cout << "\nCam on ban da su dung he thong! Tam biet.\n";
            break;
        }

        switch (chon) {
            case 1: { // QUẢN LÝ KHÁCH HÀNG
                xoaManHinh();
                std::cout << "=== QUAN LY KHACH HANG ===\n";
                std::cout << "1. Xem danh sach Khach Hang\n";
                std::cout << "2. Them Khach Hang moi\n";
                int subChon = nhapSoNguyenHopLe("Lua chon: ", 1, 2);

                if (subChon == 1) {
                    std::cout << "\nDANH SACH KHACH HANG:\n";
                    std::cout << std::left << std::setw(10) << "Ma KH" << std::setw(22) << "Ho Ten" << std::setw(15) << "CCCD" 
                              << std::setw(13) << "SDT" << std::setw(25) << "Email" << std::setw(15) << "Quoc Tich" 
                              << std::setw(15) << "Hang TV" << std::setw(10) << "Diem" << "\n";
                    std::cout << "-------------------------------------------------------------------------------------------------------------------------\n";
                    for (const auto& k : ql.dsKhachHang) k->hienThiThongTin();
                } else {
                    std::cout << "\nNHAP THONG TIN KHACH HANG MOI:\n";
                    std::string ma = nhapChuoiKhongRong("Ma Khach Hang: ");
                    if (ql.timKhachHang(ma)) {
                        std::cout << ">>> LOI: Ma Khach Hang da ton tai!\n";
                    } else {
                        std::string ten = nhapChuoiKhongRong("Ho va Ten: ");
                        std::string cccd = nhapCCCDHopLe("So CCCD (12 so): ");
                        std::string sdt = nhapSDTHopLe("So Dien Thoai (10 so): ");
                        std::string em = nhapEmailHopLe("Email: ");
                        std::string qt = nhapChuoiKhongRong("Quoc Tich: ");

                        ql.dsKhachHang.push_back(std::make_shared<KhachHang>(ma, ten, cccd, sdt, em, qt));
                        std::cout << ">>> THEM KHACH HANG THANH CONG!\n";
                    }
                }
                pauseScreen();
                break;
            }

            case 2: { // QUẢN LÝ NHÂN VIÊN
                xoaManHinh();
                std::cout << "=== QUAN LY NHAN VIEN ===\n";
                std::cout << "1. Xem danh sach Nhan Vien\n";
                std::cout << "2. Them Nhan Vien moi\n";
                int subChon = nhapSoNguyenHopLe("Lua chon: ", 1, 2);

                if (subChon == 1) {
                    std::cout << "\nDANH SACH NHAN VIEN:\n";
                    std::cout << std::left << std::setw(10) << "Ma NV" << std::setw(22) << "Ho Ten" << std::setw(15) << "CCCD" 
                              << std::setw(13) << "SDT" << std::setw(25) << "Email" << std::setw(15) << "Chuc Vu" 
                              << std::setw(12) << "Chi Nhanh" << std::right << std::setw(19) << "Luong Thuc Nhan" << "\n";
                    std::cout << "-----------------------------------------------------------------------------------------------------------------------------------\n";
                    for (const auto& n : ql.dsNhanVien) n->hienThiThongTin();
                } else {
                    if (!nhanVienHienTai->kiemTraQuyen("QUAN_LY")) {
                        std::cout << ">>> TU CHOI TRUY CAP: Chi co QUAN LY moi co quyen them nhan vien!\n";
                    } else {
                        std::string ma = nhapChuoiKhongRong("Ma Nhan Vien: ");
                        if (ql.timNhanVien(ma)) {
                            std::cout << ">>> LOI: Ma Nhan Vien da ton tai!\n";
                        } else {
                            std::string ten = nhapChuoiKhongRong("Ho va Ten: ");
                            std::string cccd = nhapCCCDHopLe("So CCCD (12 so): ");
                            std::string sdt = nhapSDTHopLe("So Dien Thoai (10 so): ");
                            std::string em = nhapEmailHopLe("Email: ");
                            std::cout << "Chon chuc vu: 1-Le Tan, 2-Buong Phong, 3-Quan Ly, 4-Ke Toan: ";
                            int cv = nhapSoNguyenHopLe("", 1, 4);
                            double luongCB = nhapSoThucHopLe("Luong co ban (VND): ", 1000000);
                            double heSo = nhapSoThucHopLe("He so luong: ", 1.0);

                            ql.dsNhanVien.push_back(std::make_shared<NhanVien>(ma, ten, cccd, sdt, em, static_cast<LoaiNhanVien>(cv), luongCB, heSo));
                            std::cout << ">>> THEM NHAN VIEN THANH CONG!\n";
                        }
                    }
                }
                pauseScreen();
                break;
            }

            case 3: { // LẬP PHIẾU ĐẶT PHÒNG
                xoaManHinh();
                std::cout << "=== LAP PHIEU DAT PHONG MOI ===\n";
                if (!nhanVienHienTai->kiemTraQuyen("DAT_PHONG")) {
                    std::cout << ">>> TU CHOI TRUY CAP: Chuc vu hien tai khong co quyen dat phong!\n";
                } else {
                    std::string maPhieu = nhapChuoiKhongRong("Ma Phieu Dat: ");
                    if (ql.timPhieuDat(maPhieu)) {
                        std::cout << ">>> LOI: Ma phieu dat phong da ton tai!\n";
                    } else {
                        std::string maKH = nhapChuoiKhongRong("Nhap Ma Khach Hang: ");
                        auto kh = ql.timKhachHang(maKH);
                        if (!kh) {
                            std::cout << ">>> LOI: Khong tim thay Khach Hang co ma " << maKH << "!\n";
                        } else {
                            std::string maPhong = nhapChuoiKhongRong("Nhap Ma Phong muon dat: ");
                            auto phong = ql.timPhong(maPhong);
                            if (!phong) {
                                std::cout << ">>> LOI: Phong khong ton tai!\n";
                            } else {
                                std::string nIn = nhapNgayHopLe("Ngay Check-In (YYYY-MM-DD): ");
                                std::string nOut = nhapNgayHopLe("Ngay Check-Out (YYYY-MM-DD): ");
                                
                                if (nOut <= nIn) {
                                    std::cout << ">>> LOI: Ngay Check-Out phai sau Ngay Check-In!\n";
                                } else {
                                    int sNguoi = nhapSoNguyenHopLe("So nguoi o: ", 1, 10);
                                    double tienCoc = nhapSoThucHopLe("Tien dat coc (VND): ", 0.0);
                                    std::cout << "Chon Mua du lich: 1-Thap diem, 2-Binh thuong, 3-Cao diem, 4-Le Tet: ";
                                    int mInt = nhapSoNguyenHopLe("", 1, 4);

                                    auto pdp = std::make_shared<PhieuDatPhong>(maPhieu, kh, phong, nhanVienHienTai, nIn, nOut, sNguoi, tienCoc, static_cast<MuaDuLich>(mInt));
                                    ql.dsPhieuDat.push_back(pdp);
                                    kh->themLichSu(pdp);

                                    std::cout << ">>> LAP PHIEU DAT PHONG THANH CONG!\n";
                                }
                            }
                        }
                    }
                }
                pauseScreen();
                break;
            }

            case 4: { // CHECK-IN / CHECK-OUT / PHỤ THU
                xoaManHinh();
                std::cout << "=== QUAN LY CHECK-IN / CHECK-OUT & PHU THU ===\n";
                std::cout << "1. Xem danh sach Phieu Dat Phong\n";
                std::cout << "2. Thuc hien CHECK-IN\n";
                std::cout << "3. Ap dung PHU THU (Nhan phong som / Tra muon)\n";
                std::cout << "4. HUY Phieu Dat Phong\n";
                int subChon = nhapSoNguyenHopLe("Lua chon: ", 1, 4);

                if (subChon == 1) {
                    std::cout << "\nDANH SACH PHIEU DAT PHONG:\n";
                    std::cout << std::left << std::setw(12) << "Ma Phieu" << std::setw(20) << "Khach Hang" << std::setw(10) << "Phong" 
                              << std::setw(13) << "Check-In" << std::setw(13) << "Check-Out" << std::setw(15) << "Trang Thai" 
                              << std::right << std::setw(19) << "Tong Tien Phong" << "\n";
                    std::cout << "---------------------------------------------------------------------------------------------------------------------\n";
                    for (const auto& p : ql.dsPhieuDat) p->hienThiThongTin();
                } else if (subChon == 2) {
                    std::string maP = nhapChuoiKhongRong("Nhap Ma Phieu Dat can Check-In: ");
                    auto pdp = ql.timPhieuDat(maP);
                    if (!pdp) {
                        std::cout << ">>> LOI: Phieu dat phong khong ton tai!\n";
                    } else {
                        if (pdp->checkIn()) std::cout << ">>> CHECK-IN THANH CONG!\n";
                        else std::cout << ">>> LOI: Phieu da CHECK-IN hoac da bi HUY tu truoc!\n";
                    }
                } else if (subChon == 3) {
                    std::string maP = nhapChuoiKhongRong("Nhap Ma Phieu Dat can ap dung phu thu: ");
                    auto pdp = ql.timPhieuDat(maP);
                    if (!pdp) {
                        std::cout << ">>> LOI: Phieu dat phong khong ton tai!\n";
                    } else {
                        double pt = nhapSoThucHopLe("Nhap so tien phu thu nhan som/tra muon (VND): ", 0.0);
                        pdp->apDungPhuThu(pt);
                        std::cout << ">>> DA AP DUNG PHU THU THANH CONG!\n";
                    }
                } else if (subChon == 4) {
                    std::string maP = nhapChuoiKhongRong("Nhap Ma Phieu Dat can HUY: ");
                    auto pdp = ql.timPhieuDat(maP);
                    if (!pdp) {
                        std::cout << ">>> LOI: Phieu dat phong khong ton tai!\n";
                    } else {
                        try {
                            pdp->huyPhieu();
                            std::cout << ">>> HUY PHIEU DAT PHONG THANH CONG!\n";
                        } catch (const std::exception& e) {
                            std::cout << ">>> LOI: " << e.what() << "\n";
                        }
                    }
                }
                pauseScreen();
                break;
            }

            case 5: { // LẬP HÓA ĐƠN & THANH TOÁN
                xoaManHinh();
                std::cout << "=== LAP HOA DON & THANH TOAN ===\n";
                std::cout << "1. Xem danh sach Hoa Don\n";
                std::cout << "2. Tao Hoa Don & Them Dich Vu\n";
                std::cout << "3. Thanh toan Hoa Don & Tich diem\n";
                int subChon = nhapSoNguyenHopLe("Lua chon: ", 1, 3);

                if (subChon == 1) {
                    for (const auto& hd : ql.dsHoaDon) hd->inHoaDon();
                } else if (subChon == 2) {
                    if (!nhanVienHienTai->kiemTraQuyen("LAP_HOA_DON")) {
                        std::cout << ">>> TU CHOI TRUY CAP: Ban khong co quyen lap hoa don!\n";
                    } else {
                        std::string maHD = nhapChuoiKhongRong("Nhap Ma Hoa Don moi: ");
                        std::string maPDP = nhapChuoiKhongRong("Nhap Ma Phieu Dat Phong: ");
                        auto pdp = ql.timPhieuDat(maPDP);

                        if (!pdp) {
                            std::cout << ">>> LOI: Phieu dat phong khong ton tai!\n";
                        } else if (!pdp->isDaCheckIn()) {
                            std::cout << ">>> LOI: Phieu dat phong chua thuc hien CHECK-IN!\n";
                        } else {
                            auto hd = std::make_shared<HoaDon>(maHD, pdp);
                            
                            std::string tiepTuc = "y";
                            while (tiepTuc == "y" || tiepTuc == "Y") {
                                std::cout << "\nDanh sach Dich vu hien co: DV01 (Spa - 300k), DV02 (Buffet - 150k), DV03 (Dua don - 500k)\n";
                                std::string maDV = nhapChuoiKhongRong("Nhap Ma Dich Vu (hoac nhap '0' de dung): ");
                                if (maDV == "0") break;

                                auto dv = ql.timDichVu(maDV);
                                if (!dv) {
                                    std::cout << ">>> LOI: Dich vu khong ton tai!\n";
                                } else {
                                    int sl = nhapSoNguyenHopLe("So luong: ", 1, 100);
                                    hd->themDichVu(dv, sl);
                                    std::cout << ">>> Da them dich vu thanh cong!\n";
                                }
                                std::cout << "Ban co muon them dich vu khac? (y/n): ";
                                std::getline(std::cin, tiepTuc);
                                tiepTuc = trim(tiepTuc);
                            }

                            ql.dsHoaDon.push_back(hd);
                            std::cout << ">>> TAO HOA DON THANH CONG!\n";
                            hd->inHoaDon();
                        }
                    }
                } else if (subChon == 3) {
                    std::string maHD = nhapChuoiKhongRong("Nhap Ma Hoa Don can THANH TOAN: ");
                    std::shared_ptr<HoaDon> hdTarget = nullptr;
                    for (auto& h : ql.dsHoaDon) if (h->layMaHoaDon() == maHD) hdTarget = h;

                    if (!hdTarget) {
                        std::cout << ">>> LOI: Hoa don khong ton tai!\n";
                    } else {
                        if (hdTarget->thanhToan()) {
                            std::cout << ">>> THANH TOAN HOA DON THANH CONG! KHACH HANG DA DUOC CONG DIEM TICH LUY.\n";
                            hdTarget->inHoaDon();
                        } else {
                            std::cout << ">>> LOI: Hoa don nay DA DUOC THANH TOAN tu truoc!\n";
                        }
                    }
                }
                pauseScreen();
                break;
            }

            case 6: { // ĐỔI TÀI KHOẢN NHÂN VIÊN
                xoaManHinh();
                std::cout << "=== DOI NHAN VIEN THAO TAC (TEST PHAN QUYEN) ===\n";
                std::cout << "Danh sach Nhan vien:\n";
                for (const auto& n : ql.dsNhanVien) {
                    std::cout << "- " << n->layMaSo() << " | " << n->layHoTen() << " | Chuc vu: " << n->layTenChucVu() << "\n";
                }
                std::string maNV = nhapChuoiKhongRong("\nNhap Ma Nhan Vien muon chuyen sang: ");
                auto nv = ql.timNhanVien(maNV);
                if (nv) {
                    nhanVienHienTai = nv;
                    std::cout << ">>> CHUYEN TAI KHOAN THANH CONG! Nhan vien hien tai: " << nhanVienHienTai->layHoTen() << "\n";
                } else {
                    std::cout << ">>> LOI: Khong tim thay Nhan Vien!\n";
                }
                pauseScreen();
                break;
            }
        }
    }

    return 0;
}