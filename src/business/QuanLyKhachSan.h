// QuanLyKhachSan.h - Lop dieu phoi trung tam
// Quan ly ChiNhanhResort, KhachHang, PhieuDatPhong, HoaDon, DichVu va «create» PhieuDatPhong, HoaDon.
// Moi nghiep vu (dat phong, check-in, check-out...) deu di qua lop nay de trang thai PHONG
// (State Pattern) va trang thai PHIEU luon dong bo voi nhau. Du lieu duoc doc/ghi file (LuuTruFile.cpp).
#pragma once
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "ChiNhanhResort.h"
#include "DichVu.h"
#include "HoaDon.h"
#include "Nguoi.h"
#include "PhieuDatPhong.h"
#include "Phong.h"

struct ThongKeDichVu
{
    std::string ma, ten, loai;
    int soLuong;
    double doanhThu;
};

struct ThongKeNhanVien
{
    std::shared_ptr<NhanVien> nhanVien;
    int soPhieuLap;
    int soHoaDonLap;
    double doanhThu;
};

struct SuKienLuuTru
{
    std::time_t thoiGian;
    std::string loai; // DAT PHONG | CHECK-IN | CHECK-OUT | HUY
    std::shared_ptr<PhieuDatPhong> phieu;
};

struct ThongKeKhachHang
{
    std::shared_ptr<KhachHang> khach;
    int soHoaDon;
    double tongChi;
};

class QuanLyKhachSan
{
private:
    std::vector<std::shared_ptr<ChiNhanhResort>> danhSachChiNhanh;
    std::vector<std::shared_ptr<KhachHang>> danhSachKhachHang;
    std::vector<std::shared_ptr<PhieuDatPhong>> danhSachPhieu;
    std::vector<std::shared_ptr<HoaDon>> danhSachHoaDon;
    std::vector<std::shared_ptr<DichVu>> danhSachDichVu;
    // Phong da xoa nhung con lich su phieu: an khoi danh sach, van giu de doc lai phieu/hoa don cu
    std::vector<std::pair<std::string, std::shared_ptr<Phong>>> danhSachPhongDaXoa; // (maChiNhanh, phong)
    int demChiNhanh, demPhong, demKhachHang, demNhanVien, demPhieu, demHoaDon, demDichVu;

    static std::string taoMa(const std::string &tienTo, int so); // P01, KH01, DP01...
    std::shared_ptr<PhieuDatPhong> layPhieuBatBuoc(const std::string &ma) const;
    std::shared_ptr<HoaDon> lapHoaDon(const std::shared_ptr<PhieuDatPhong> &phieu,
                                      const std::shared_ptr<NhanVien> &nv, const std::string &ma) const;
    std::shared_ptr<PhieuDatPhong> taoPhieu(const std::string &maKH, const std::string &maPhong,
                                            const std::shared_ptr<NhanVien> &nv, const Ngay &ngayNhan,
                                            int soDem, int soNguoi, MuaDuLich mua, const std::string &ghiChu);
    void taoLichSuMau(const std::string &maKH, const std::string &maPhong, const std::string &maNV,
                      int lechNgay, int soDem, int soNguoi,
                      const std::vector<std::pair<std::string, int>> &dichVu,
                      const std::string &phuongThuc, bool donXong = true, bool daThanhToan = true);
    void capNhatBoDem(); // tinh lai bo dem ma sau khi doc file

public:
    QuanLyKhachSan();

    // ================= Chi nhanh =================
    std::shared_ptr<ChiNhanhResort> themChiNhanh(const std::string &ten, const std::string &diaChi,
                                                 const std::string &hotline);
    void damBaoNhanSuMacDinh();
    std::shared_ptr<ChiNhanhResort> timChiNhanh(const std::string &ma) const;
    const std::vector<std::shared_ptr<ChiNhanhResort>> &layDanhSachChiNhanh() const { return danhSachChiNhanh; }
    std::shared_ptr<ChiNhanhResort> chiNhanhCuaPhong(const std::string &maPhong) const;
    // Ma chi nhanh cua mot phong (ke ca phong da xoa con lich su) - dung de loc phieu / hoa don theo chi nhanh
    std::string maChiNhanhCuaPhong(const std::shared_ptr<Phong> &p) const;
    // Kiem tra quan he du lieu truoc khi xoa: tra ve danh sach ly do (rong = xoa duoc)
    std::vector<std::string> lyDoKhongXoaDuocChiNhanh(const std::string &ma) const;
    void xoaChiNhanh(const std::string &ma); // tu choi (kem ly do) neu chi nhanh con du lieu lien quan
    std::vector<std::shared_ptr<PhieuDatPhong>> phieuCuaChiNhanh(const std::string &maCN) const; // maCN rong = tat ca
    std::vector<std::shared_ptr<HoaDon>> hoaDonCuaChiNhanh(const std::string &maCN) const;      // maCN rong = tat ca
    bool hoaDonThuocChiNhanh(const HoaDon &hd, const std::string &maCN) const;                  // maCN rong = true

    // ================= Phong =================
    std::string taoMaPhong() const;
    void themPhong(const std::string &maChiNhanh, const std::shared_ptr<Phong> &p);
    std::shared_ptr<Phong> timPhong(const std::string &maPhong) const;
    std::vector<std::shared_ptr<Phong>> tatCaPhong() const;
    std::vector<std::shared_ptr<Phong>> locPhong(const std::function<bool(const Phong &)> &dieuKien) const;
    void doiSoPhong(const std::string &maPhong, const std::string &soMoi);
    void kiemTraCoTheDoiGia(const std::string &maPhong) const; // khong cho doi gia/suc chua khi con phieu
    void xoaPhong(const std::string &maPhong);                  // chi xoa khi phong khong con phieu hoat dong
    std::shared_ptr<Phong> timPhongKeCaDaXoa(const std::string &maPhong) const;

    // ================= Lich dat phong (chong trung ngay) =================
    std::vector<std::shared_ptr<PhieuDatPhong>> lichDatCuaPhong(const std::string &maPhong) const;
    std::shared_ptr<PhieuDatPhong> timPhieuTrungLich(const std::string &maPhong, const Ngay &ngayNhan,
                                                     int soDem) const;
    std::shared_ptr<PhieuDatPhong> phieuHienTaiCuaPhong(const std::string &maPhong) const; // dang o / nhan hom nay
    std::shared_ptr<PhieuDatPhong> phieuSapToiCuaPhong(const std::string &maPhong) const;
    // Ly do phong khong dat duoc (rong = dat duoc). Phong co khach DANG O bi khoa vo thoi han (ke ca qua han)
    std::string lyDoKhongDatDuoc(const Phong &p, const Ngay &ngayNhan, int soDem, int soNguoi) const;
    // Chuyen BAO TRI co kiem tra: tu choi (kem danh sach phieu) neu phong con phieu dat / dang o
    void chuyenBaoTri(const std::string &maPhong, const std::string &lyDo);
    // Tran tien coc = tong tien phong phai tra (sau giam gia thanh vien, gom VAT)
    double tranCocToiDa(const Phong &p, int soDem, MuaDuLich mua, double tyLeGiam) const;
    std::vector<std::shared_ptr<PhieuDatPhong>> phieuQuaHan() const; // khach chua check-out du da qua ngay tra
    // Phieu khach DANG O (da check-in, chua check-out) cua phong; nullptr neu phong khong co khach
    std::shared_ptr<PhieuDatPhong> phieuDangOCuaPhong(const std::string &maPhong) const;
    // Thong bao day du khi phong dang bi khach chua check-out chiem (dung cho nhan vien, co ten khach)
    std::string thongBaoPhongDangCoKhach(const Phong &p, const PhieuDatPhong &dangO) const;
    // Phieu "Cho nhan phong" da den ngay nhan nhung phong van bi khach truoc (qua han) chiem -> can xu ly
    std::vector<std::shared_ptr<PhieuDatPhong>> phieuBiChanBoiKhachDangO() const;

    // ================= Chuyen phieu sang phong khac =================
    std::string kiemTraChuyenPhong(const std::string &maPhieu) const; // "" = phieu duoc phep chuyen
    // Phong co the nhan phieu: cung chi nhanh, du suc chua, khong bao tri, khong co khach chiem,
    // khong trung phieu khac trong TOAN BO khoang ngay cua phieu. cungLoai = chi lay phong cung loai.
    std::vector<std::shared_ptr<Phong>> phongCoTheChuyen(const std::string &maPhieu, bool cungLoai = true) const;
    // Cap nhat phong cua CHINH phieu hien tai (khong tao phieu moi); tra ve phong cu
    std::shared_ptr<Phong> chuyenPhongPhieu(const std::string &maPhieu, const std::string &maPhongMoi);

    // ================= Nhan vien =================
    std::shared_ptr<NhanVien> themNhanVien(const std::string &maChiNhanh, const std::string &hoTen,
                                           const std::string &sdt, const std::string &chucVu,
                                           double luong, const std::string &email = "",
                                           const std::string &cccd = "", const std::string &gioiTinh = "",
                                           const std::string &ngaySinh = "");
    std::shared_ptr<NhanVien> timNhanVien(const std::string &ma) const;
    std::vector<std::shared_ptr<NhanVien>> tatCaNhanVien(bool caNguoiDaNghi = false) const;
    void xoaNhanVien(const std::string &ma); // danh dau nghi viec

    // ================= Khach hang =================
    std::shared_ptr<KhachHang> themKhachHang(const std::string &hoTen, const std::string &sdt,
                                             const std::string &cccd, const std::string &email = "",
                                             const std::string &quocTich = "Viet Nam",
                                             const std::string &diaChi = "", const std::string &gioiTinh = "");
    void kiemTraTrungKhach(const std::string &sdt, const std::string &cccd,
                           const std::string &boQuaMa = "") const;
    std::shared_ptr<KhachHang> timKhachHang(const std::string &ma) const;
    std::shared_ptr<KhachHang> timKhachTheoSDT(const std::string &sdt) const;
    std::shared_ptr<KhachHang> timKhachTheoCCCD(const std::string &cccd) const;
    std::vector<std::shared_ptr<KhachHang>> timKhachTheoTuKhoa(const std::string &tuKhoa) const;
    const std::vector<std::shared_ptr<KhachHang>> &layDanhSachKhachHang() const { return danhSachKhachHang; }

    // ================= Dich vu =================
    std::string taoMaDichVu() const;
    void themDichVu(const std::shared_ptr<DichVu> &dv);
    std::shared_ptr<DichVu> timDichVu(const std::string &ma) const;
    void xoaDichVu(const std::string &ma);
    const std::vector<std::shared_ptr<DichVu>> &layDanhSachDichVu() const { return danhSachDichVu; }

    // ================= Nghiep vu luu tru =================
    std::shared_ptr<PhieuDatPhong> datPhong(const std::string &maKH, const std::string &maPhong,
                                            const std::shared_ptr<NhanVien> &nv, const Ngay &ngayNhan,
                                            int soDem, int soNguoi, MuaDuLich mua,
                                            const std::string &ghiChu = "", double tienCoc = 0,
                                            const std::string &phuongThucCoc = "");
    void checkIn(const std::string &maPhieu);
    void huyDatPhong(const std::string &maPhieu, const std::string &lyDo, bool hoanCoc);
    // Gia han them soDemThem dem cho khach DANG O: bao xung dot neu phong da co phieu khac trong khoang gia han
    std::shared_ptr<PhieuDatPhong> giaHan(const std::string &maPhieu, int soDemThem);
    // Xem truoc gia han (khong ghi): tra ve "" neu hop le, nguoc lai la ly do tu choi
    std::string kiemTraGiaHan(const std::string &maPhieu, int soDemThem) const;
    void ghiDichVu(const std::string &maPhieu, const std::string &maDV, int soLuong);
    std::shared_ptr<Phong> taoPhongCoDichVu(const PhieuDatPhong &phieu) const; // chuoi Decorator
    static std::shared_ptr<Phong> bocDichVu(const std::shared_ptr<Phong> &p, const std::shared_ptr<DichVu> &dv,
                                            int soLuong); // boc them 1 lop Decorator
    std::shared_ptr<HoaDon> xemTamTinh(const std::string &maPhieu, const std::shared_ptr<NhanVien> &nv) const;
    std::shared_ptr<HoaDon> checkOut(const std::string &maPhieu, const std::shared_ptr<NhanVien> &nv);
    void thanhToanHoaDon(const std::string &maHoaDon, const std::string &phuongThuc);
    // Dong bo trang thai theo ngay: huy phieu qua han, giu cho phong cho khach den hom nay
    int dongBoTheoNgay();

    // ================= Tra cuu =================
    std::shared_ptr<PhieuDatPhong> timPhieu(const std::string &ma) const;
    const std::vector<std::shared_ptr<PhieuDatPhong>> &layDanhSachPhieu() const { return danhSachPhieu; }
    std::vector<std::shared_ptr<PhieuDatPhong>> locPhieu(TrangThaiPhieu tt) const;
    std::shared_ptr<HoaDon> timHoaDon(const std::string &ma) const;
    std::vector<SuKienLuuTru> layLichSuSuKien() const; // moi nhat truoc
    const std::vector<std::shared_ptr<HoaDon>> &layDanhSachHoaDon() const { return danhSachHoaDon; }

    // ================= Thong ke =================
    // maCN (tuy chon): chi tinh du lieu cua MOT chi nhanh; rong = toan he thong
    double tongDoanhThu(const std::string &maCN = "") const; // chi tinh hoa don da thanh toan
    double doanhThuThangHienTai(const std::string &maCN = "") const;
    double chiPhiLuongThang(const std::string &maCN = "") const;
    double doanhThuPhong(const std::string &maCN = "") const;
    double doanhThuDichVu(const std::string &maCN = "") const;
    int soHoaDonChuaThanhToan(const std::string &maCN = "") const;
    double tongCocGiuLai() const;  // coc cua phieu huy khong hoan (ghi nhan theo chinh sach no-show/huy)
    double tongCocDangGiu() const; // coc cua phieu con hieu luc, se tru vao hoa don khi check-out
    double congSuatPhong(const std::string &maCN = "") const; // % phong dang co khach
    std::vector<std::pair<std::string, double>> doanhThuTheoChiNhanh() const;
    std::vector<std::pair<std::string, double>> doanhThuTheoLoaiPhong(const std::string &maCN = "") const;
    std::vector<std::pair<std::string, double>> doanhThuTheoThang(int soThang, const std::string &maCN = "") const;
    std::vector<ThongKeDichVu> thongKeDichVu(const std::string &maCN = "") const;
    std::vector<ThongKeKhachHang> topKhachHang(size_t soLuong, const std::string &maCN = "") const;
    std::vector<ThongKeNhanVien> thongKeNhanVien() const;
    // [tu, den), theo ngay thanh toan
    double doanhThuTrongKhoang(std::time_t tu, std::time_t den, const std::string &maCN = "") const;
    // ky = "ngay" | "tuan" | "thang" | "quy" | "nam"; tra ve soKy moc gan nhat (cu -> moi)
    std::vector<std::pair<std::string, double>> doanhThuTheoKy(const std::string &ky, int soKy,
                                                               const std::string &maCN = "") const;

    // ================= Du lieu =================
    void napDuLieuMau();
    void ghiFile(const std::string &thuMuc) const;      // LuuTruFile.cpp
    bool docFile(const std::string &thuMuc, int &soDongLoi); // false neu chua co du lieu
    // Nhat ky lan doc file gan nhat: moi file 1 dong "ten: doc N ban ghi, loi M" + chi tiet tung dong loi
    // (ten file, so dong, ly do). Dung de bao cao ro rang, khong bo qua dong loi trong im lang.
    std::vector<std::string> nhatKyDoc;
};
