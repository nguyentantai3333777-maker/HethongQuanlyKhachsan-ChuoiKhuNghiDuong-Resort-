// HoaDon.cpp
#include "HoaDon.h"
#include <cmath>
#include <stdexcept>

using namespace std;

constexpr double HoaDon::THUE_VAT;

ChiTietHoaDon::ChiTietHoaDon(const string &l, const string &m, const string &nd, int sl,
                             double dg, double tt)
    : loai(l), maThamChieu(m), noiDung(nd), soLuong(sl), donGia(dg), thanhTien(tt)
{
    if (sl <= 0 || dg < 0 || tt < 0)
        throw invalid_argument("Chi tiet hoa don khong hop le.");
}

HoaDon::HoaDon(const string &ma, shared_ptr<PhieuDatPhong> p, shared_ptr<NhanVien> nv,
               double tongDecorator, double giam, double coc)
    : maHoaDon(ma), phieu(p), nhanVienLap(nv), thoiGianLap(time(nullptr)), thoiGianThanhToan(0),
      tongTheoDecorator(tongDecorator), tyLeGiamGia(giam), tienCoc(coc < 0 ? 0 : coc), phuongThucThanhToan(""),
      daThanhToan(false)
{
    if (!p)
        throw invalid_argument("Hoa don phai gan voi mot phieu dat phong.");
    if (giam < 0 || giam > 0.5)
        throw invalid_argument("Ty le giam gia khong hop le.");
}

double HoaDon::tienPhong() const
{
    double t = 0;
    for (auto &ct : chiTiet)
        if (ct.layLoai() == "Phong")
            t += ct.layThanhTien();
    return t;
}

double HoaDon::tienDichVu() const
{
    double t = 0;
    for (auto &ct : chiTiet)
        if (ct.layLoai() != "Phong")
            t += ct.layThanhTien();
    return t;
}

KetQuaTinhTien HoaDon::tinhTien(double tamTinh, double tyLeGiam)
{
    if (tamTinh < 0)
        tamTinh = 0;
    if (tyLeGiam < 0)
        tyLeGiam = 0;
    KetQuaTinhTien kq;
    kq.tamTinh = tamTinh;
    kq.tyLeGiam = tyLeGiam;
    kq.tienGiam = tamTinh * tyLeGiam;
    kq.tienThue = (tamTinh - kq.tienGiam) * THUE_VAT;
    kq.tong = tamTinh - kq.tienGiam + kq.tienThue;
    return kq;
}

double HoaDon::tamTinh() const { return tienPhong() + tienDichVu(); }
double HoaDon::tienGiam() const { return tinhTien(tamTinh(), tyLeGiamGia).tienGiam; }
double HoaDon::tienThue() const { return tinhTien(tamTinh(), tyLeGiamGia).tienThue; }
double HoaDon::tongThanhToan() const { return tinhTien(tamTinh(), tyLeGiamGia).tong; }
double HoaDon::conLaiPhaiTra() const
{
    double c = tongThanhToan() - tienCoc;
    return c > 0.5 ? c : 0; // lech < 0.5 dong coi nhu da du
}
double HoaDon::tienCocDu() const
{
    double d = tienCoc - tongThanhToan();
    return d > 0.5 ? d : 0;
}
double HoaDon::tienCocDaTru() const { return tienCoc < tongThanhToan() ? tienCoc : tongThanhToan(); }

void HoaDon::khoiPhuc(time_t tgLap, time_t tgTT, bool daTT, const string &pt)
{
    thoiGianLap = tgLap;
    thoiGianThanhToan = tgTT;
    daThanhToan = daTT;
    phuongThucThanhToan = pt;
}

bool HoaDon::khopVoiDecorator() const { return fabs(tamTinh() - tongTheoDecorator) < 0.5; }

void HoaDon::thanhToan(const string &pt, time_t t)
{
    if (daThanhToan)
        throw runtime_error("Hoa don " + maHoaDon + " da duoc thanh toan truoc do.");
    if (pt.empty())
        throw invalid_argument("Phuong thuc thanh toan khong duoc rong.");
    phuongThucThanhToan = pt;
    thoiGianThanhToan = t;
    daThanhToan = true;
}
