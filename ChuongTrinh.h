// ChuongTrinh.h - Module Giao dien (UI & Menu Driver)
// Lop nay chi hien thi + nhap lieu; moi nghiep vu goi xuong QuanLyKhachSan.
// Du lieu duoc doc tu thu muc "data/" khi khoi dong va ghi lai sau MOI thao tac.
//   ChuongTrinh.cpp   : khoi dong, dang nhap, menu chinh, khung menu, tai khoan, ham hien thi dung chung
//   MenuPhong.cpp     : menuQuanLyPhong          (mau vang nhat)
//   MenuDichVu.cpp    : menuQuanLyDichVu         (mau hong nhat)
//   MenuDatPhong.cpp  : menuDatPhongCheckIn      (mau xanh la nhat)
//   MenuThanhToan.cpp : menuThanhToanHoaDon      (mau cam dao)
//   MenuKhachHang.cpp : menuQuanLyKhachHang (xanh ngoc), menuNhanSuChiNhanh (tim nhat)
//   MenuThongKe.cpp   : menuThongKeBaoCao        (mau nau)
#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "ConsoleUI.h"
#include "QuanLyKhachSan.h"
#include "TaiKhoan.h"

struct MucMenu
{
    std::string ten;
    bool chiAdmin;
    std::function<void()> hanhDong;
    bool laMenuCon = false; // mo mot menu con: khong ve man hinh hanh dong, khong "Nhan Enter de tiep tuc"
};

class ChuongTrinh
{
private:
    QuanLyKhachSan ks;
    QuanLyTaiKhoan qlTaiKhoan;
    std::string userHienTai;
    Role roleHienTai;
    std::shared_ptr<NhanVien> nvHienTai;
    std::string thuMucDuLieu;
    bool khoaGhi = false;   // true khi doc du lieu that bai: TUYET DOI khong ghi xuong CSV
    bool daSaoLuu = false;  // da sao luu CSV truoc lan ghi dau tien cua phien chay
    std::string baoCaoLoiKhoiDong; // noi dung bao loi khi khong nap duoc du lieu
    // Pham vi dang xem: chi nhanh duoc chon o man hinh "CHON CHI NHANH" (nullptr = tat ca chi nhanh)
    std::shared_ptr<ChiNhanhResort> cnDangChon; // Quan ly phong / hoa don theo chi nhanh
    std::shared_ptr<ChiNhanhResort> cnThongKe;  // Pham vi thong ke (nullptr = toan he thong)

    // ================= Chon che do Console / Web =================
    void chayConsole();
    void chayWeb(); // WebServer.cpp: HTTP server dung chung ks + qlTaiKhoan

    // ================= Khung chuong trinh =================
    bool docDuLieu(std::string &baoCao); // doc toan bo CSV (chi doc, khong ghi); true neu khong co loi nao
    bool napDuLieu();                    // false: khong nap duoc -> khong tao du lieu mau, khong ghi de
    void saoLuuTruocKhiGhi();            // chep *.csv sang <data>/backup/<thoi gian>/ (1 lan / phien)
    void luuDuLieu();
    // Nap lai toan bo du lieu tu CSV (Single Source of Truth chung Console + Web).
    // Goi khi chuyen che do de dam bao bo nho khop file tren dia.
    bool napLaiDuLieuTuFile(std::string *thongBaoLoi = nullptr);
    bool dangNhap();
    void damBaoTaiKhoanNhanSu();
    bool laAdmin() const { return roleHienTai == Role::ADMIN; }
    void thucHien(const std::function<void()> &hanhDong); // bat loi + luu file + pauseScreen
    void inThanhTrangThai() const;
    void manHinh(const std::string &tieuDe, const std::string &phu = "") const;
    // noiDung (tuy chon): ve them du lieu (vd bang phong cua chi nhanh) giua tieu de va menu
    void chayMenu(UI::ChuDe chuDe, const std::string &tieuDe, const std::string &phu,
                  const std::vector<MucMenu> &muc, const std::function<void()> &noiDung = nullptr,
                  const std::string &phuHanhDong = "");
    // Man hinh "CHON CHI NHANH" (danh sach lay tu du lieu that). false = nguoi dung chon 0 (quay lai).
    // coTatCa: them muc "Tat ca chi nhanh" (kq = nullptr)
    bool chonChiNhanhManHinh(const std::string &tieuDe, std::shared_ptr<ChiNhanhResort> &kq, bool coTatCa,
                             const std::string &phu = "");

    // ================= Menu =================
    void hienThiMenuChinh();
    void menuQuanLyPhong();
    void menuQuanLyDichVu();
    void menuDatPhongCheckIn();
    void menuThanhToanHoaDon();
    void menuQuanLyKhachHang();
    void menuNhanSuChiNhanh();
    void menuThongKeBaoCao();
    void menuTaiKhoan();
    void menuPhongCuaChiNhanh();   // PHONG - <chi nhanh>: bang phong + thao tac trong chi nhanh
    void menuHoaDonCuaChiNhanh();  // HOA DON - <chi nhanh>
    void menuThongKeTheoPhamVi();  // THONG KE - <chi nhanh | tat ca>

    // ================= Hien thi dung chung =================
    static std::string mauTrangThaiPhong(MaTrangThai tt);
    static std::string mauTrangThaiPhieu(TrangThaiPhieu tt);
    static std::string tenPhong(const Phong &p); // "P03 - 201"
    static std::string tenChiNhanh(const ChiNhanhResort &cn); // "Ocean Pearl Resort Nha Trang" (ten lay tu du lieu)
    std::string tenChiNhanhCuaPhong(const Phong &p) const;
    std::vector<std::shared_ptr<Phong>> phongDangXem() const; // phong cua chi nhanh dang chon (hoac tat ca)
    std::string lichGanNhat(const Phong &p) const;
    void inBangPhong(const std::vector<std::shared_ptr<Phong>> &ds, bool hienChiNhanh = true) const;
    void inBangPhieu(const std::vector<std::shared_ptr<PhieuDatPhong>> &ds,
                     const std::string &khiRong = "(Khong co phieu nao)") const;
    void inChiTietPhieu(const std::shared_ptr<PhieuDatPhong> &ph) const;
    void inLichSu(const std::vector<std::shared_ptr<PhieuDatPhong>> &ds) const;
    void inBangKhachHang(const std::vector<std::shared_ptr<KhachHang>> &ds) const;
    void inBangNhanVien(const std::vector<std::shared_ptr<NhanVien>> &ds) const;
    std::vector<std::pair<std::string, std::string>> thongTinChiTietKhachHang(const KhachHang &kh) const;
    void inBangDichVu(const std::vector<std::shared_ptr<DichVu>> &ds) const;
    void inBangHoaDon(const std::vector<std::shared_ptr<HoaDon>> &ds, bool hienChiNhanh = true) const;
    void inBangChiNhanh() const;
    void inHoaDon(const HoaDon &hd) const;

    // ================= Chon doi tuong (nhap 0 de huy -> nullptr) =================
    std::shared_ptr<Phong> chonPhong(const std::vector<std::shared_ptr<Phong>> &ds, const std::string &nhan);
    std::shared_ptr<PhieuDatPhong> chonPhieu(const std::vector<std::shared_ptr<PhieuDatPhong>> &ds,
                                             const std::string &nhan);
    std::shared_ptr<KhachHang> chonKhachHang();
    std::shared_ptr<KhachHang> nhapKhachHangMoi();
    std::shared_ptr<KhachHang> chonHoacTaoKhachHang();
    std::shared_ptr<DichVu> chonDichVu();
    std::shared_ptr<ChiNhanhResort> chonChiNhanh(bool choPhepTatCa = false);
    std::shared_ptr<HoaDon> chonHoaDon(const std::vector<std::shared_ptr<HoaDon>> &ds);
    static std::string chonPhuongThuc(const std::string &mucThoat);

    // ================= Phong =================
    void xemTatCaPhong();
    void timKiemPhong();
    void xemChiTietPhong();
    void themPhong();
    void suaPhong();
    void xoaPhong();
    void capNhatTrangThaiPhong();
    void baoGiaPhong();

    // ================= Dich vu =================
    void xemDanhMucDichVu();
    void ghiNhanDichVu();
    void xemDichVuCuaPhieu();
    void themDichVu();
    void suaDichVu();
    void xoaDichVu();

    // ================= Dat phong & Check-in =================
    void datPhongMoi();
    void checkIn();
    void huyDatPhong();
    void giaHanPhong();
    void xemDanhSachNguoiDat();
    void xemLichSuDatTra();
    void traCuuPhieu();
    void chuyenPhieuSangPhongKhac();

    // ================= Thanh toan & Hoa don =================
    void checkOutLapHoaDon();
    void xemTamTinh();
    void thanhToanHoaDonCho();
    void xemDanhSachHoaDon();
    void xemChiTietHoaDon();
    void timHoaDonTrongChiNhanh();
    void timHoaDonToanHeThong(); // chuc nang rieng cho ADMIN
    void thucHienThanhToan(const std::shared_ptr<HoaDon> &hd);

    // ================= Khach hang =================
    void xemDanhSachKhachHang();
    void timKiemKhachHang();
    void themKhachHang();
    void suaKhachHang();
    void xemLichSuKhachHang();

    // ================= Nhan su & Chi nhanh =================
    void xemChiNhanh();
    void themChiNhanh();
    void xoaChiNhanh();
    void xemNhanVien();
    void themNhanVien();
    void suaNhanVien(); // Chinh sua thong tin NV hien co (khong doi ma)
    void xoaNhanVien();

    // ================= Thong ke =================
    void thongKeTongQuan();
    void baoCaoDoanhThu();
    void thongKeCongSuat();
    void thongKeDichVu();
    void thongKeKhachHang();

    // ================= Tai khoan =================
    void xemTaiKhoanCuaToi();
    void doiMatKhau();
    void xemDanhSachTaiKhoan();
    void taoTaiKhoan();
    void xoaTaiKhoan();

public:
    explicit ChuongTrinh(const std::string &thuMucDuLieu = "data");
    void chay();
    int kiemTraDuLieu(); // che do --kiem-tra: chi doc va in so ban ghi tung file; 0 = khong loi
};
