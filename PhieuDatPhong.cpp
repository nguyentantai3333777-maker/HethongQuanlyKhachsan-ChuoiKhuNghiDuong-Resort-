// PhieuDatPhong.cpp
#include "PhieuDatPhong.h"
#include "Nguoi.h"
#include "Phong.h"
#include "DichVu.h"
#include <stdexcept>

using namespace std;

double SuDungDichVu::thanhTien() const { return dichVu->tinhChiPhi(soLuong); }

PhieuDatPhong::PhieuDatPhong(const string &ma, shared_ptr<KhachHang> kh, shared_ptr<Phong> p,
                             shared_ptr<NhanVien> nv, const Ngay &nn, int dem, int nguoi,
                             MuaDuLich m, const string &gc, bool kiemTraSucChua)
    : maPhieu(ma), khachHang(kh), phong(p), nhanVien(nv), thoiGianDat(time(nullptr)),
      thoiGianCheckIn(0), thoiGianCheckOut(0), thoiGianHuy(0), ngayNhan(nn), soDem(dem),
      soNguoi(nguoi), mua(m), trangThai(TrangThaiPhieu::DA_DAT), ghiChu(gc), tienCoc(0),
      phuongThucCoc(""), daHoanCoc(false)
{
    if (!kh)
        throw invalid_argument("Phieu dat phong phai co khach hang.");
    if (!p)
        throw invalid_argument("Phieu dat phong phai co phong.");
    if (!nv)
        throw invalid_argument("Phieu dat phong phai co nhan vien lap phieu.");
    if (dem <= 0 || dem > 60)
        throw invalid_argument("So dem phai tu 1 den 60.");
    if (nguoi <= 0 || (kiemTraSucChua && nguoi > p->laySucChua()))
        throw invalid_argument("So nguoi phai tu 1 den " + to_string(p->laySucChua()) +
                               " (suc chua cua phong).");
}

double PhieuDatPhong::tinhTienPhong(int soDemThem) const
{
    // soDemThem > 0: dem qua han; < 0: tra som. Tong dem tinh phi >= 1.
    int tong = soDem + soDemThem;
    if (tong < 1)
        tong = 1;
    return phong->tinhGiaPhong(tong, mua);
}

double PhieuDatPhong::tinhTienDichVu() const
{
    double tong = 0;
    for (auto &sd : dichVuSuDung)
        tong += sd.thanhTien();
    return tong;
}

void PhieuDatPhong::xacNhanCheckIn(time_t t)
{
    if (trangThai != TrangThaiPhieu::DA_DAT)
        throw runtime_error("Chi check-in duoc phieu dang o trang thai 'Cho nhan phong'.");
    trangThai = TrangThaiPhieu::DA_NHAN_PHONG;
    thoiGianCheckIn = t;
}

void PhieuDatPhong::xacNhanCheckOut(time_t t, const string &maHD)
{
    if (trangThai != TrangThaiPhieu::DA_NHAN_PHONG)
        throw runtime_error("Chi check-out duoc phieu dang o trang thai 'Dang o'.");
    trangThai = TrangThaiPhieu::DA_TRA_PHONG;
    thoiGianCheckOut = t;
    maHoaDon = maHD;
}

void PhieuDatPhong::huy(time_t t, const string &lyDo, bool hoanCoc)
{
    if (trangThai != TrangThaiPhieu::DA_DAT)
        throw runtime_error("Chi huy duoc phieu chua nhan phong.");
    trangThai = TrangThaiPhieu::DA_HUY;
    thoiGianHuy = t;
    lyDoHuy = lyDo.empty() ? "Khach yeu cau huy" : lyDo;
    daHoanCoc = hoanCoc && tienCoc > 0;
}

void PhieuDatPhong::datCoc(double soTien, const string &pt)
{
    if (trangThai != TrangThaiPhieu::DA_DAT)
        throw runtime_error("Chi nhan coc khi phieu chua nhan phong.");
    if (soTien < 0)
        throw invalid_argument("Tien coc khong duoc am.");
    if (soTien > 0 && pt.empty())
        throw invalid_argument("Can chon phuong thuc dat coc.");
    tienCoc = soTien;
    phuongThucCoc = soTien > 0 ? pt : "";
}

Ngay PhieuDatPhong::ngayKetThucChiemPhong() const
{
    Ngay tra = layNgayTra();
    // Chi de hien thi: khach qua han thi \"den het hom nay\" (thuc te phong van bi khoa vo thoi han)
    if (trangThai == TrangThaiPhieu::DA_NHAN_PHONG)
    {
        Ngay mai = Ngay::homNay().congNgay(1);
        if (tra < mai)
            return mai;
    }
    return tra;
}

bool PhieuDatPhong::dangOverstay() const
{
    // Qua han = dang o va hom nay da SAU ngay tra du kien (ngay tra van con o trong han).
    // Vi du: nhan 28, dat 2 dem -> tra du kien 30; ngay 30 van trong han, tu 31 moi tinh qua han.
    return trangThai == TrangThaiPhieu::DA_NHAN_PHONG && layNgayTra() < Ngay::homNay();
}

int PhieuDatPhong::soNgayOverstay() const
{
    return dangOverstay() ? layNgayTra().khoangCachDen(Ngay::homNay()) : 0;
}

bool PhieuDatPhong::traSom() const
{
    return trangThai == TrangThaiPhieu::DA_NHAN_PHONG && Ngay::homNay() < layNgayTra();
}

int PhieuDatPhong::soDemThucTe() const
{
    // So dem tinh phi theo thoi gian luu tru THUC TE (mot nguon duy nhat cho hoa don):
    // - Dang o: dem = khoang cach ngay nhan -> hom nay (toi thieu 1).
    //   + Tra dung han (hom nay == ngay tra): dem == soDem dat.
    //   + Tra som (hom nay < ngay tra): dem < soDem.
    //   + Qua han (hom nay > ngay tra): dem == soDem + soNgayOverstay, KHONG can gia han tren he thong.
    // - Da check-out / huy: soDem da duoc chot luc check-out (rut ngan / gia han qua han).
    if (trangThai != TrangThaiPhieu::DA_NHAN_PHONG)
        return soDem > 0 ? soDem : 1;
    int dem = ngayNhan.khoangCachDen(Ngay::homNay());
    if (dem < 1)
        dem = 1; // check-in va check-out trong cung ngay: tinh toi thieu 1 dem
    return dem;
}

void PhieuDatPhong::rutNganSoDem(int soDemMoi)
{
    if (trangThai != TrangThaiPhieu::DA_NHAN_PHONG)
        throw runtime_error("Chi chot so dem cho phieu dang o.");
    if (soDemMoi < 1 || soDemMoi > soDem)
        throw invalid_argument("So dem thuc te khong hop le.");
    soDem = soDemMoi;
}

bool PhieuDatPhong::trungLich(const Ngay &tu, int soDemKhac) const
{
    if (!dangHoatDong())
        return false;
    Ngay den = tu.congNgay(soDemKhac);
    // Khach DANG O: phong bi chiem vo thoi han tu ngay nhan -> bat ky khoang nao ket thuc sau ngay nhan deu trung
    if (chiemVoThoiHan())
        return ngayNhan < den;
    return tu < layNgayTra() && ngayNhan < den; // [a1,a2) giao [b1,b2)
}

void PhieuDatPhong::giaHan(int soDemThem, bool kiemTraTran)
{
    if (trangThai != TrangThaiPhieu::DA_NHAN_PHONG)
        throw runtime_error("Chi gia han duoc phieu dang o (da check-in, chua check-out).");
    if (soDemThem <= 0)
        throw invalid_argument("So dem gia han phai lon hon 0.");
    if (kiemTraTran && soDem + soDemThem > 60)
        throw invalid_argument("Tong so dem toi da 60 (hien o " + to_string(soDem) + " dem).");
    soDem += soDemThem;
}

void PhieuDatPhong::khoiPhuc(time_t tgDat, time_t in, time_t out, time_t tgHuy, TrangThaiPhieu tt,
                             const string &lyDo, const string &maHD, double coc, const string &ptCoc,
                             bool hoan)
{
    thoiGianDat = tgDat;
    thoiGianCheckIn = in;
    thoiGianCheckOut = out;
    thoiGianHuy = tgHuy;
    trangThai = tt;
    lyDoHuy = lyDo;
    maHoaDon = maHD;
    tienCoc = coc;
    phuongThucCoc = ptCoc;
    daHoanCoc = hoan;
}

void PhieuDatPhong::khoiPhucDichVu(const shared_ptr<DichVu> &dv, int soLuong, time_t t)
{
    if (dv && soLuong > 0)
        dichVuSuDung.push_back({dv, soLuong, t});
}

void PhieuDatPhong::doiPhong(const shared_ptr<Phong> &phongMoi)
{
    if (trangThai != TrangThaiPhieu::DA_DAT)
        throw runtime_error("Chi chuyen phong duoc phieu dang 'Cho nhan phong'.");
    if (!phongMoi)
        throw invalid_argument("Phong moi khong hop le.");
    if (phongMoi == phong)
        throw invalid_argument("Phong moi trung voi phong hien tai.");
    if (soNguoi > phongMoi->laySucChua())
        throw invalid_argument("Phong moi chi chua toi da " + to_string(phongMoi->laySucChua()) + " khach.");
    phong = phongMoi;
}

void PhieuDatPhong::themDichVu(const shared_ptr<DichVu> &dv, int soLuong, time_t t)
{
    if (trangThai != TrangThaiPhieu::DA_DAT && trangThai != TrangThaiPhieu::DA_NHAN_PHONG)
        throw runtime_error("Chi ghi dich vu cho phieu dang cho nhan phong (dat truoc) hoac dang o.");
    if (!dv)
        throw invalid_argument("Dich vu null.");
    if (soLuong <= 0 || soLuong > 100)
        throw invalid_argument("So luong phai tu 1 den 100.");
    dichVuSuDung.push_back({dv, soLuong, t});
}
