// InputHelper.h - Cac ham nhap du lieu an toan (Input Validation) va kiem tra dinh dang
#pragma once
#include <stdexcept>
#include <string>
#include "LoaiPhong.h"
#include "MuaDuLich.h"

class Ngay;

class HetDuLieuNhap : public std::runtime_error
{
public:
    HetDuLieuNhap() : std::runtime_error("Het du lieu dau vao. Chuong trinh se thoat.") {}
};

namespace InputHelper
{
    class HuyNhap : public std::runtime_error
    {
    public:
        HuyNhap() : std::runtime_error("Nhap lieu da duoc huy.") {}
    };

    class PhamViHuyBangEsc
    {
    public:
        PhamViHuyBangEsc();
        ~PhamViHuyBangEsc();
        PhamViHuyBangEsc(const PhamViHuyBangEsc &) = delete;
        PhamViHuyBangEsc &operator=(const PhamViHuyBangEsc &) = delete;

    private:
        bool trangThaiTruoc;
    };

    // ---- Xu ly chuoi ----
    std::string trim(const std::string &s);
    std::string chuHoa(std::string s);
    std::string chuanHoaHoTen(const std::string &s); // "  nguyen   van an " -> "Nguyen Van An"
    std::string chuanHoaMa(const std::string &s);    // "p1" -> "P01", "kh003" -> "KH03"
    bool laMa(const std::string &s, const std::string &tienTo); // vd laMa("P01", "P")

    // ---- Kiem tra dinh dang (khong nhap) ----
    bool laSoDuong(const std::string &s);
    std::string chuanHoaSoDuong(const std::string &s);
    bool laChuThuan(const std::string &s);
    bool laSoDienThoai(const std::string &s); // 10 so, bat dau bang 0
    bool laCCCD(const std::string &s);        // 12 so, hoac ho chieu 8-9 ky tu (chu cai dau)
    bool laEmail(const std::string &s);
    // ---- Quy tac cho tung loai truong (backend la lop kiem tra cuoi cung, khong tin frontend) ----
    bool coKyTuDieuKhien(const std::string &s);                 // \0..\x1f, '|' (ky tu phan cach file), '<' '>'
    bool laSoThuan(const std::string &s, size_t toiThieu = 1, size_t toiDa = 18); // chi gom chu so 0-9
    bool laTenRieng(const std::string &s, size_t toiDa = 60);   // chu + so + khoang trang + - ' . (phai co chu cai)
    bool laDiaChi(const std::string &s);                        // chu, so, khoang trang va , . / - ( ) # ' (5-120 ky tu)
    bool laUsername(const std::string &s);                      // 3-20 ky tu: chu, so, _ .

    bool docSoThuc(std::string s, double &kq); // cho phep "1,500,000"

    // ---- Nhap lieu co kiem tra, lap lai den khi hop le ----
    std::string nhapDong(const std::string &nhan);
    // Tra ve "\x1b" neu nguoi dung nhan ESC; Enter trong van la chuoi rong.
    std::string nhapDongCoEsc(const std::string &nhan, bool hienHuongDan = true);
    std::string nhapChuoi(const std::string &nhan);
    // Nhap mat khau bat buoc (khong duoc de trong). Ky tu duoc an bang '*'.
    // choPhepEsc=true: ESC tra ve "\x1b". choPhepEsc=false: khong cho huy bang ESC.
    std::string nhapMatKhau(const std::string &nhan, bool choPhepEsc = false, bool hienHuongDan = true);
    // Nhap username/tai khoan bat buoc, khong de trong, chi chap nhan ky tu hop le.
    std::string nhapTaiKhoan(const std::string &nhan, bool choPhepEsc = false, bool hienHuongDan = true);
    int nhapSoNguyenHopLe(const std::string &nhan, int nhoNhat, int lonNhat);
    double nhapSoThucHopLe(const std::string &nhan, double nhoNhat, double lonNhat);
    // Gia phong khi tim kiem: chap nhan "10000000" hoac "10,000,000" (nhom 3 so); khongNhoHon > 0: gia phai >= gia tri do
    double nhapGiaTimKiem(const std::string &nhan, double nhoNhat, double lonNhat, double khongNhoHon = 0);
    int nhapLuaChon(int nhoNhat, int lonNhat);
    bool nhapCoKhong(const std::string &nhan); // chi chap nhan Y/y hoac N/n
    // Xac nhan bang ENTER (dong y) / ESC (huy). In nhan + huong dan, cho phim.
    // Tra ve true neu ENTER, false neu ESC.
    bool xacNhanEnterEsc(const std::string &nhan);
    std::string nhapGioiTinh();
    // Buoc xac nhan bat buoc (them / xoa chi nhanh):
    // "Bạn có chắc chắn muốn xác nhận thực hiện hay không? (Y/N): " -> true chi khi nhap Y hoac y
    extern const char *const CAU_XAC_NHAN;
    bool nhapXacNhanYN();
    std::string nhapHoTen(const std::string &nhan);
    std::string nhapChuoiChu(const std::string &nhan);
    // Ten chi nhanh: chi chu cai + khoang trang (khong so, khong ky tu la); sai -> bao loi ro va NHAP LAI ngay
    std::string nhapTenChiNhanh(const std::string &nhan);
    std::string nhapDiaChi(const std::string &nhan);
    std::string nhapSoDienThoai(const std::string &nhan);
    std::string nhapCCCD(const std::string &nhan);
    std::string nhapEmail(const std::string &nhan); // cho phep de trong
    std::string nhapSoDuongChuoi(const std::string &nhan);
    // Nhap ma (vd CN01, DP03): kiem tra dung dinh dang tien to + so; ESC = huy (neu choPhepHuy). Tra ve ma chuan hoa.
    // hienHuongDanEsc=false: khong in dong "ESC: Quay lai" (vd nhap ma dich vu sau bang).
    std::string nhapMa(const std::string &nhan, const std::string &tienTo, bool choPhepHuy = true,
                       bool hienHuongDanEsc = true);
    // Kiem tra chuoi vua nhap co dung dinh dang ma khong (bao loi ro rang neu sai)
    bool kiemTraMaNhap(const std::string &s, const std::string &tienTo);
    // Chuoi tuy chon (ghi chu, ly do): khong chua | < > (lam hong file du lieu), toi da toiDa ky tu
    std::string nhapGhiChu(const std::string &nhan, size_t toiDa = 200);
    Ngay nhapNgay(const std::string &nhan, bool enterLaHomNay = true);
    LoaiPhong nhapLoaiPhong();
    MuaDuLich nhapMuaDuLich();
}
