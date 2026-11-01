// test_nghiep_vu.cpp - Kiem thu nhanh nhom lop thuc the + nghiep vu (khong can UI / web)
// Chay: ./build.sh && ./test_nghiep_vu [thu_muc_data]
#include "QuanLyKhachSan.h"
#include "TaiKhoan.h"
#include "PhongCuThe.h"
#include "PhongDecorator.h"
#include <iostream>

static int dem = 0, hong = 0;
#define KT(dk, ten) do { ++dem; if (dk) std::cout << "  [OK]   " << ten << "\n"; \
                         else { ++hong; std::cout << "  [FAIL] " << ten << "\n"; } } while (0)

int main(int argc, char **argv)
{
    std::string thuMuc = argc > 1 ? argv[1] : "data";
    QuanLyKhachSan ks;
    int loi = 0;

    std::cout << "== Doc du lieu CSV: " << thuMuc << " ==\n";
    bool ok = ks.docFile(thuMuc, loi);
    for (auto &s : ks.nhatKyDoc) std::cout << "  " << s << "\n";
    KT(ok && loi == 0, "docFile() doc du 9 file CSV, khong loi");
    KT(!ks.layDanhSachChiNhanh().empty(), "Co chi nhanh");
    KT(!ks.layDanhSachKhachHang().empty(), "Co khach hang");
    KT(!ks.layDanhSachPhieu().empty(), "Co phieu dat phong");

    std::cout << "== Tinh tien (cong thuc duy nhat) ==\n";
    auto kq = HoaDon::tinhTien(1000000, 0.10); // giam 10%, VAT 10% tren phan con lai
    KT(kq.tienGiam == 100000 && kq.tienThue == 90000 && kq.tong == 990000, "tinhTien(1.000.000, 10%) = 990.000");

    std::cout << "== Hang thanh vien ==\n";
    KT(tyLeGiamCuaHang(HangThanhVien::KIM_CUONG) == 0.15, "Kim cuong giam 15%");
    KT(diemToiThieuCuaHang(HangThanhVien::VANG) == 400, "Hang Vang can 400 diem");

    std::cout << "== State Pattern (Phong) ==\n";
    StandardRoom p("P98", "999", 1, 500000, 2, 20);
    KT(p.coTheDat(), "Phong moi o trang thai Trong");
    p.nhanPhong();
    KT(p.layMaTrangThai() == MaTrangThai::DANG_O, "Trong -> Dang o (khach vang lai)");
    bool nem = false;
    try { p.giuCho(0); } catch (const std::exception &) { nem = true; }
    KT(nem, "Dang o khong the giu cho (nem ngoai le)");
    p.traPhong();
    p.donPhong();
    KT(p.coTheDat(), "Dang o -> Dang don -> Trong");

    std::cout << "== Decorator Pattern ==\n";
    auto goc = std::make_shared<StandardRoom>("P99", "998", 1, 500000, 2, 20);
    auto spa = std::make_shared<Spa>("DV90", "Spa", 400000, 60);
    auto boc = QuanLyKhachSan::bocDichVu(goc, spa, 2);
    KT(boc->tinhGiaPhong(1, MuaDuLich::BINH_THUONG) == goc->tinhGiaPhong(1, MuaDuLich::BINH_THUONG) + 800000,
       "Boc Spa x2 cong them 800.000");

    std::cout << "== Tai khoan ==\n";
    QuanLyTaiKhoan tk;
    KT(tk.docFile(thuMuc + "/taikhoan.csv"), "Doc taikhoan.csv");
    nem = false;
    try { tk.dangNhap("khong_ton_tai", "x"); } catch (const std::exception &) { nem = true; }
    KT(nem, "Dang nhap sai bi tu choi");

    std::cout << "\nKet qua: " << (dem - hong) << "/" << dem << " dat\n";
    return hong == 0 ? 0 : 1;
}
