// MenuThanhToan.cpp - menuThanhToanHoaDon (mau cam dao)
#include "ChuongTrinh.h"
#include "InputHelper.h"
#include "TienIch.h"
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;
using namespace InputHelper;
using namespace UI;

void ChuongTrinh::menuThanhToanHoaDon()
{
    chayMenu(ChuDe::THANH_TOAN, "CHECK-OUT & THANH TOAN",
             "Hoa don = tien phong + dich vu - giam gia + VAT - coc. ESC / Quay lai de thoat.",
             {{"Check-out & thanh toan", false, [this]
               { checkOutLapHoaDon(); }},
              {"Xem tam tinh khach dang o", false, [this]
               { xemTamTinh(); }},
              {"Thanh toan hoa don con no", false, [this]
               { thanhToanHoaDonCho(); }},
              {"Quan ly hoa don theo chi nhanh", false, [this]
               { menuHoaDonCuaChiNhanh(); }, true},
              {"Tim kiem hoa don toan he thong (quan tri)", true, [this]
               { timHoaDonToanHeThong(); }}});
}

// QUAN LY HOA DON: chon chi nhanh -> "HOA DON - <chi nhanh>" chi hien hoa don cua chi nhanh do
void ChuongTrinh::menuHoaDonCuaChiNhanh()
{
    while (true)
    {
        datChuDe(ChuDe::THANH_TOAN);
        shared_ptr<ChiNhanhResort> cn;
        if (!chonChiNhanhManHinh("CHON CHI NHANH", cn, false, "Quan ly hoa don - chon chi nhanh"))
            break;
        cnDangChon = cn;
        string ten = tenChiNhanh(*cn);
        chayMenu(ChuDe::THANH_TOAN, "HOA DON - " + chuHoa(ten), "Chi hien thi hoa don cua chi nhanh nay",
                 {{"Xem chi tiet hoa don", false, [this]
                   { xemChiTietHoaDon(); }},
                  {"Tim kiem hoa don (trong chi nhanh)", false, [this]
                   { timHoaDonTrongChiNhanh(); }},
                  {"Thanh toan hoa don con no", false, [this]
                   { thanhToanHoaDonCho(); }}},
                 [this] { xemDanhSachHoaDon(); }, "Chi nhanh: " + ten);
        cnDangChon = nullptr;
    }
    cnDangChon = nullptr;
}

void ChuongTrinh::checkOutLapHoaDon()
{
    auto ds = ks.locPhieu(TrangThaiPhieu::DA_NHAN_PHONG);
    if (ds.empty())
    {
        canhBao("Hien khong co khach dang o.");
        return;
    }
    auto ph = chonPhieu(ds, "Ma phieu check-out");
    if (!ph)
        return;
    manHinh("CHECK-OUT " + ph->layMaPhieu());
    inHoaDon(*ks.xemTamTinh(ph->layMaPhieu(), nvHienTai));
    cout << "\n";
    if (ph->traSom())
        thongTin("Khach tra som (du kien " + ph->layNgayTra().toString() + "): chi tinh " +
                 to_string(ph->soDemThucTe()) + "/" + to_string(ph->laySoDem()) + " dem thuc te da o, khong tinh them phi.");
    if (ph->dangOverstay())
        canhBao("Khach o QUA HAN " + to_string(ph->soNgayOverstay()) + " ngay (du kien tra " + ph->layNgayTra().toString() +
                "): hoa don da tinh them " + to_string(ph->soNgayOverstay()) + " dem.");
    if (!xacNhanEnterEsc("XAC NHAN CHECK-OUT?"))
        return;
    auto hd = ks.checkOut(ph->layMaPhieu(), nvHienTai);
    thanhCong("Da check-out, lap hoa don " + hd->layMa() + ". Phong " + tenPhong(*ph->layPhong()) + " -> " +
              ph->layPhong()->layTenTrangThai() + ".");
    thucHienThanhToan(hd);
    luuDuLieu();
    // Hoa don da duoc hien thi truoc khi xac nhan; sau thanh toan quay ve menu,
    // khong dung lai mot man hinh hoa don thu hai.
}

void ChuongTrinh::thucHienThanhToan(const shared_ptr<HoaDon> &hd)
{
    double conLai = round(hd->conLaiPhaiTra());
    auto kh = hd->layPhieu()->layKhachHang();
    string hangCu = kh->hangThanhVien();
    int diemCu = kh->layDiemTichLuy();
    if (conLai <= 0)
    {
        // Coc du: van cho ESC huy o buoc xac nhan
        if (!xacNhanEnterEsc("XAC NHAN THANH TOAN BANG TIEN COC?"))
        {
            canhBao("Da huy thanh toan. Hoa don " + hd->layMa() + " van o trang thai 'Chua thanh toan'.");
            return;
        }
        ks.thanhToanHoaDon(hd->layMa(), "Tien coc");
        thanhCong("Tien coc da du thanh toan hoa don.");
        if (hd->tienCocDu() > 0)
            thongTin("Hoan lai cho khach phan coc du: " + dinhDangTien(hd->tienCocDu()) + " VND.");
    }
    else
    {
        tieuDeNho("Thanh toan: con phai tra " + dinhDangTien(conLai) + " VND");
        string pt = chonPhuongThuc("Huy / thanh toan sau");
        if (pt.empty())
        {
            canhBao("Da huy / thanh toan sau. Hoa don " + hd->layMa() + " van o trang thai 'Chua thanh toan'.");
            return;
        }
        if (pt == "Tien mat")
        {
            double dua = 0;
            try
            {
                PhamViHuyBangEsc choPhepHuy;
                // Nhap gon, can le thong nhat voi khoi thong tin ben duoi
                dua = nhapSoThucHopLe("Khach dua (VND): ", conLai, 1e10);
            }
            catch (const HuyNhap &)
            {
                canhBao("Da huy thanh toan. Hoa don " + hd->layMa() + " van o trang thai 'Chua thanh toan'.");
                return;
            }
            // Khoi thong tin thanh toan: can giua, thu gon, thang hang nhan/gia tri
            // (khong doi cong thuc: tien thoi = dua - conLai)
            inKhoi("THANH TOAN TIEN MAT", {
                {"So tien phai tra", dinhDangTien(conLai) + " VND"},
                {"Khach dua", dinhDangTien(dua) + " VND"},
                {"Tien thoi lai", dinhDangTien(dua - conLai) + " VND"}
            });
        }
        // Xac nhan cuoi (hop thoai ENTER/ESC da can giua trong veKhungXacNhanEnterEsc)
        if (!xacNhanEnterEsc("XAC NHAN THANH TOAN?"))
        {
            canhBao("Da huy thanh toan. Hoa don " + hd->layMa() + " van o trang thai 'Chua thanh toan'.");
            return;
        }
        ks.thanhToanHoaDon(hd->layMa(), pt);
        thanhCong("Thanh toan thanh cong (" + pt + ").");
    }
    thongTin(kh->layHoTen() + " duoc cong " + to_string(kh->layDiemTichLuy() - diemCu) + " diem, tong " +
             to_string(kh->layDiemTichLuy()) + " diem.");
    if (kh->hangThanhVien() != hangCu)
        thanhCong("Khach duoc nang hang: " + hangCu + " -> " + kh->hangThanhVien() + ".");
}

void ChuongTrinh::xemTamTinh()
{
    auto ph = chonPhieu(ks.locPhieu(TrangThaiPhieu::DA_NHAN_PHONG), "Ma phieu");
    if (!ph)
        return;
    manHinh("TAM TINH " + ph->layMaPhieu());
    inHoaDon(*ks.xemTamTinh(ph->layMaPhieu(), nvHienTai));
}

void ChuongTrinh::thanhToanHoaDonCho()
{
    vector<shared_ptr<HoaDon>> ds;
    for (auto &hd : ks.hoaDonCuaChiNhanh(cnDangChon ? cnDangChon->layMa() : ""))
        if (!hd->layDaThanhToan())
            ds.push_back(hd);
    if (ds.empty())
    {
        thanhCong(cnDangChon ? "Chi nhanh " + tenChiNhanh(*cnDangChon) + " khong co hoa don nao con no."
                             : "Khong co hoa don nao con no.");
        return;
    }
    auto hd = chonHoaDon(ds);
    if (!hd)
        return;
    manHinh("THANH TOAN " + hd->layMa());
    inHoaDon(*hd);
    thucHienThanhToan(hd);
}

void ChuongTrinh::xemDanhSachHoaDon()
{
    string maCN = cnDangChon ? cnDangChon->layMa() : "";
    auto ds = ks.hoaDonCuaChiNhanh(maCN);
    sort(ds.begin(), ds.end(), [](const shared_ptr<HoaDon> &a, const shared_ptr<HoaDon> &b)
         { return a->layThoiGianLap() > b->layThoiGianLap(); });
    inBangHoaDon(ds, !cnDangChon);
    double chuaThu = 0;
    for (auto &hd : ds)
        if (!hd->layDaThanhToan())
            chuaThu += hd->conLaiPhaiTra();
    // Dua "Da thu" / "Con no" vao khung; dinhDangTienVN da co hau to "d"
    inKhoi("TONG HOP", {
        {"Da thu", dinhDangTienVN(ks.tongDoanhThu(maCN))},
        {"Con no", dinhDangTienVN(chuaThu)}
    }, 14, false);
}

// Tim hoa don CHI trong chi nhanh dang chon (khong tim tren toan he thong)
void ChuongTrinh::timHoaDonTrongChiNhanh()
{
    if (!cnDangChon)
        return timHoaDonToanHeThong();
    dongThongTin("Chi nhanh", tenChiNhanh(*cnDangChon), DAM + CHU);
    string k;
    {
        PhamViHuyBangEsc choPhepHuy;
        try {
            k = nhapDong("Nhap ma hoa don / ma phieu / ten / SDT khach: ");
        } catch (const HuyNhap &) {
            return;
        }
    }
    if (k.empty())
    {
        canhBao("Da huy tim kiem.");
        return;
    }
    string ma = chuanHoaMa(k), K = chuHoa(k);
    vector<shared_ptr<HoaDon>> kq;
    for (auto &hd : ks.hoaDonCuaChiNhanh(cnDangChon->layMa()))
    {
        auto kh = hd->layPhieu()->layKhachHang();
        if (hd->layMa() == ma || hd->layPhieu()->layMaPhieu() == ma || chuHoa(kh->layHoTen()).find(K) != string::npos ||
            kh->laySoDienThoai().find(k) != string::npos)
            kq.push_back(hd);
    }
    if (kq.empty())
    {
        bool oNoiKhac = false;
        if (auto hd = ks.timHoaDon(ma))
            oNoiKhac = !ks.hoaDonThuocChiNhanh(*hd, cnDangChon->layMa());
        loi("Khong tim thay hoa don '" + k + "' trong chi nhanh " + tenChiNhanh(*cnDangChon) + "." +
            (oNoiKhac ? " (Hoa don nay thuoc chi nhanh khac.)" : ""));
        return;
    }
    tieuDeNho("Ket qua (" + to_string(kq.size()) + " hoa don)");
    inBangHoaDon(kq, false);
    if (kq.size() == 1)
    {
        cout << "\n";
        inHoaDon(*kq[0]);
    }
}

// Chuc nang RIENG cho quan tri: tim tren toan he thong (khong dung chung voi man hinh chi nhanh)
void ChuongTrinh::timHoaDonToanHeThong()
{
    shared_ptr<ChiNhanhResort> cn;
    if (!chonChiNhanhManHinh("CHON CHI NHANH TIM HOA DON", cn, false,
                             "Chon chi nhanh truoc khi tim kiem hoa don"))
        return;
    string k;
    {
        PhamViHuyBangEsc choPhepHuy;
        try {
            k = nhapDong("Nhap ma hoa don / ma phieu / ten / SDT khach: ");
        } catch (const HuyNhap &) {
            return;
        }
    }
    if (k.empty())
    {
        canhBao("Da huy tim kiem.");
        return;
    }
    string ma = chuanHoaMa(k), K = chuHoa(k);
    vector<shared_ptr<HoaDon>> kq;
    for (auto &hd : ks.layDanhSachHoaDon())
    {
        if (!ks.hoaDonThuocChiNhanh(*hd, cn->layMa()))
            continue;
        auto kh = hd->layPhieu()->layKhachHang();
        if (hd->layMa() == ma || hd->layPhieu()->layMaPhieu() == ma ||
            chuHoa(kh->layHoTen()).find(K) != string::npos || kh->laySoDienThoai().find(k) != string::npos)
            kq.push_back(hd);
    }
    sort(kq.begin(), kq.end(), [](const shared_ptr<HoaDon> &a, const shared_ptr<HoaDon> &b)
         { return a->layThoiGianLap() > b->layThoiGianLap(); });
    tieuDeNho("Ket qua hoa don - " + tenChiNhanh(*cn) + " (" + to_string(kq.size()) + " hoa don)");
    inBangHoaDon(kq, false);
    if (kq.size() == 1)
    {
        cout << "\n";
        inHoaDon(*kq[0]);
    }
}

void ChuongTrinh::xemChiTietHoaDon()
{
    auto ds = ks.hoaDonCuaChiNhanh(cnDangChon ? cnDangChon->layMa() : "");
    sort(ds.begin(), ds.end(), [](const shared_ptr<HoaDon> &a, const shared_ptr<HoaDon> &b)
         { return a->layThoiGianLap() > b->layThoiGianLap(); });
    auto hd = chonHoaDon(ds);
    if (!hd)
        return;
    manHinh("HOA DON " + hd->layMa());
    inHoaDon(*hd);
}
