// MenuDatPhong.cpp - menuDatPhongCheckIn (mau xanh la nhat)
// Dat phong: chon chi nhanh -> ngay o -> phong (kiem tra trung lich) -> khach hang -> coc -> xac nhan
#include "ChuongTrinh.h"
#include "InputHelper.h"
#include "TienIch.h"
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;
using namespace InputHelper;
using namespace UI;

void ChuongTrinh::menuDatPhongCheckIn()
{
    chayMenu(ChuDe::DAT_PHONG, "DAT PHONG & CHECK-IN",
             "Mot phong nhan nhieu phieu (khong trung ngay). ESC / Quay lai de thoat.",
             {{"Dat phong moi", false, [this]
               { datPhongMoi(); }},
              {"Check-in (khach den hom nay)", false, [this]
               { checkIn(); }},
              {"Huy dat phong", false, [this]
               { huyDatPhong(); }},
              {"Gia han cho khach dang o", false, [this]
               { giaHanPhong(); }},
              {"Danh sach nguoi dat phong", false, [this]
               { xemDanhSachNguoiDat(); }},
              {"Lich su dat & tra phong", false, [this]
               { xemLichSuDatTra(); }},
              {"Tra cuu phieu", false, [this]
               { traCuuPhieu(); }},
              {"Chuyen phieu sang phong khac", false, [this]
               { chuyenPhieuSangPhongKhac(); }}});
}

void ChuongTrinh::datPhongMoi()
{
    // Cho phep nhan ESC tai bat ky buoc nhap nao de huy toan bo quy trinh dat phong.
    PhamViHuyBangEsc choPhepHuyBangEsc;
    // ---- B1. Chi nhanh: chi tim phong trong chi nhanh duoc chon ----
    tieuDeNho("Buoc 1/5  Chon chi nhanh dat phong");
    auto cn = chonChiNhanh();
    if (!cn)
        return;
    dongThongTin("Chi nhanh", tenChiNhanh(*cn), DAM + CHU);

    Ngay homNay = Ngay::homNay(), ngayNhan, ngayTra;
    int soDem = 0, soNguoi = 0;
    MuaDuLich muaGoiY = MuaDuLich::BINH_THUONG;
    shared_ptr<Phong> p;
    while (!p)
    {
        // ---- B2. Ngay o & so khach ----
        tieuDeNho("Buoc 2/5  Ngay nhan / tra & so khach");
        while (true)
        {
            // enterLaHomNay=false: khong chap nhan de trong (dong nhat voi ngay tra)
            string sNhan = nhapDong("Ngay nhan phong (dd/mm/yyyy): ");
            if (sNhan.empty())
            {
                UI::ghiTrucTiep(string("  ") + UI::DO +
                                "[!] Ngay nhan phong khong duoc de trong. Vui long nhap lai." + UI::RESET + "\n");
                continue;
            }
            if (!Ngay::docChuoi(sNhan, ngayNhan))
            {
                UI::ghiTrucTiep(string("  ") + UI::DO +
                                "[!] Ngay khong hop le. Hay nhap theo dinh dang dd/mm/yyyy." + UI::RESET + "\n");
                continue;
            }
            if (ngayNhan < homNay)
                UI::ghiTrucTiep(string("  ") + UI::DO + "[!] Ngay nhan phong khong duoc o qua khu." + UI::RESET + "\n");
            else if (homNay.khoangCachDen(ngayNhan) > 365)
                UI::ghiTrucTiep(string("  ") + UI::DO + "[!] Chi nhan dat truoc toi da 365 ngay." + UI::RESET + "\n");
            else
                break;
        }
        while (true)
        {
            ngayTra = nhapNgay("Ngay tra phong (dd/mm/yyyy): ", false);
            int d = ngayNhan.khoangCachDen(ngayTra);
            if (d <= 0)
                UI::ghiTrucTiep(string("  ") + UI::DO + "[!] Ngay tra phai sau ngay nhan (" + ngayNhan.toString() + ")." + UI::RESET + "\n");
            else if (d > 60)
                UI::ghiTrucTiep(string("  ") + UI::DO + "[!] Toi da 60 dem cho mot phieu." + UI::RESET + "\n");
            else
            {
                soDem = d;
                break;
            }
        }
        soNguoi = nhapSoNguyenHopLe("So luong khach (1-20): ", 1, 20);
        thongTin("Loai phong mong muon:  0. Tat ca   1. Standard   2. Deluxe   3. Suite   4. Villa");
        int loai = nhapSoNguyenHopLe("Chon loai phong [0-4]: ", 0, 4);

        // ---- B3. Chi hien thi phong THUC SU dat duoc (backend van kiem tra lai khi dat) ----
        muaGoiY = xacDinhMua(ngayNhan);
        vector<shared_ptr<Phong>> datDuoc;
        for (auto &x : cn->layDanhSachPhong())
            if ((loai == 0 || (int)x->layLoaiPhong() == loai - 1) &&
                ks.lyDoKhongDatDuoc(*x, ngayNhan, soDem, soNguoi).empty())
                datDuoc.push_back(x);
        tieuDeNho("Buoc 3/5  Chon phong - " + ngayNhan.toString() + " -> " + ngayTra.toString() + ", " +
                  to_string(soDem) + " dem, " + to_string(soNguoi) + " khach");
        if (datDuoc.empty())
        {
            canhBao("Khong co phong phu hop voi yeu cau cua khach.");
            thongTin("Vui long thay doi so nguoi, ngay dat hoac loai phong.");
            if (!nhapCoKhong("Thay doi thong tin tim phong?"))
                return;
            continue;
        }
        vector<string> muc;
        for (auto &x : datDuoc)
            muc.push_back("Phong " + x->laySoPhong() + " - " + loaiPhongToString(x->layLoaiPhong()) + " - " +
                          to_string(x->laySucChua()) + " nguoi - " + dinhDangTien(x->layGiaPhong()) + "d/dem");
        int c = chonMenuTuongTac("PHONG CO THE DAT", tenChiNhanh(*cn), muc, "Huy dat phong");
        if (c == 0)
            return;
        auto chon = datDuoc[(size_t)c - 1];
        // Kiem tra lai o backend (phong co the vua bi dat / doi trang thai)
        string lyDo = ks.lyDoKhongDatDuoc(*chon, ngayNhan, soDem, soNguoi);
        if (!lyDo.empty())
        {
            loi("Phong " + chon->laySoPhong() + " vua khong con dat duoc: " + lyDo + ". Vui long chon lai.");
            continue;
        }
        p = chon;
        thanhCong("Da chon phong " + p->laySoPhong() + " (" + p->layMaPhong() + ").");
    }

    // ---- B4. Khach hang ----
    tieuDeNho("Buoc 4/5  Thong tin khach hang");
    auto kh = chonHoacTaoKhachHang();
    if (!kh)
    {
        canhBao("Da huy dat phong.");
        return;
    }

    // ---- B5. Mua, coc, xac nhan ----
    tieuDeNho("Buoc 5/5  Mua, tien coc & xac nhan");
    MuaDuLich mua = muaGoiY;
    // Mua du lich duoc xac dinh tu dong theo ngay nhan phong; khong hoi khach chon mua.
    mua = xacDinhMua(ngayNhan);
    thongTin("Mua tu dong theo ngay nhan: " + muaToString(mua) + " (x" + dinhDangSoThuc(heSoMua(mua), 1) + ")");
    if (string cb = canhBaoNhieuMua(ngayNhan, soDem); !cb.empty())
        canhBao(cb);
    double tienPhong = p->tinhGiaPhong(soDem, mua);
    string ghiChu = nhapGhiChu("Ghi chu (Enter de bo qua): ");

    // Dich vu di kem dat truoc: ghi vao phieu, tinh vao hoa don (huy phieu thi khong tinh)
    // ENTER = them, ESC = ket thuc chon dich vu
    vector<pair<shared_ptr<DichVu>, int>> dvDatTruoc;
    double tienDV = 0;
    while (xacNhanEnterEsc(dvDatTruoc.empty() ? "DAT TRUOC DICH VU DI KEM?" : "THEM DICH VU KHAC?"))
    {
        auto dv = chonDichVu();
        if (!dv)
            break;
        string nhanSL = dv->layLoai() == "Tour" ? "So nguoi" : (dv->layLoai() == "Spa" ? "So lan" : "So suat");
        int sl = nhapSoNguyenHopLe(nhanSL + ": ", 1, 100);
        dvDatTruoc.push_back({dv, sl});
        tienDV += dv->tinhChiPhi(sl);
        thongTin("Da them " + dv->layTen() + " x" + to_string(sl) +
                 " | Don gia: " + dinhDangTien(dv->layDonGia()) +
                 " | Thanh tien: " + dinhDangTien(dv->tinhChiPhi(sl)) + " VND");
        thongTin("Tong tien dich vu: " + dinhDangTien(tienDV) + " VND");
    }

    double coc = 0;
    string ptCoc;
    if (xacNhanEnterEsc("KHACH CO MUON DAT COC?"))
    {
        // Tran coc = tong tien phong phai tra (da tru giam gia thanh vien, gom VAT) - cung quy tac voi web & hoa don
        double tranCoc = ks.tranCocToiDa(*p, soDem, mua, kh->tyLeGiamGia());
        double goiY = min(tranCoc, round(HoaDon::tinhTien(tienPhong + tienDV, kh->tyLeGiamGia()).tong * 0.3 / 1000) * 1000);
        while (true)
        {
            string s = nhapDong("So tien coc: ");
            if (s.empty())
                coc = goiY;
            else if (!docSoThuc(s, coc))
            {
                loi("Loi: Vui long chi nhap so!");
                continue;
            }
            if (coc <= 0 || coc > tranCoc + 0.5)
            {
                loi("Tien coc phai lon hon 0 va khong vuot tong tien phong phai tra (" + dinhDangTien(tranCoc) +
                    " VND, da tinh giam gia va VAT).");
                continue;
            }
            break;
        }
        ptCoc = chonPhuongThuc("Khong coc");
        if (ptCoc.empty())
            coc = 0;
    }

    // ===== PHIEU THONG TIN DAT PHONG (hien thi day du truoc khi xac nhan) =====
    vector<pair<string, string>> dongPhieu = {
        {"Chi nhanh", cn->layTen() + " - " + cn->layDiaChi()},
        {"Phong", tenPhong(*p) + " (" + loaiPhongToString(p->layLoaiPhong()) + ") - " +
                      dinhDangTien(p->layGiaPhong()) + " VND/dem"},
        {"Ngay nhan phong", ngayNhan.toString()},
        {"Ngay tra phong", ngayTra.toString()},
        {"So dem", to_string(soDem)},
        {"So luong khach", to_string(soNguoi)},
        {"Khach hang", kh->layHoTen() + " (" + kh->layMa() + ")"},
        {"SDT khach", kh->laySoDienThoai()},
        {"Mua", muaToString(mua) + " (x" + dinhDangSoThuc(heSoMua(mua), 1) + ")"},
        {"Tien phong du kien", dinhDangTien(tienPhong) + " VND"},
        {"Ghi chu", ghiChu.empty() ? "(Khong)" : ghiChu}};
    if (dvDatTruoc.empty())
        dongPhieu.push_back({"Dich vu dat truoc", "Khong"});
    else
    {
        for (size_t i = 0; i < dvDatTruoc.size(); ++i)
        {
            auto &x = dvDatTruoc[i];
            string nhan = "DV " + to_string(i + 1);
            string chiTiet = x.first->layTen() + " | SL: " + to_string(x.second) +
                             " | Don gia: " + dinhDangTien(x.first->layDonGia()) +
                             " | TT: " + dinhDangTien(x.first->tinhChiPhi(x.second)) + " VND";
            dongPhieu.push_back({nhan, chiTiet});
        }
        dongPhieu.push_back({"Tong tien dich vu", dinhDangTien(tienDV) + " VND"});
    }
    dongPhieu.push_back({"Giam gia thanh vien",
                         kh->tyLeGiamGia() > 0 ? to_string((int)(kh->tyLeGiamGia() * 100 + 0.5)) + "%" : "Khong"});
    dongPhieu.push_back({"Tong du kien (sau giam, VAT 10%)",
                         dinhDangTien(HoaDon::tinhTien(tienPhong + tienDV, kh->tyLeGiamGia()).tong) + " VND"});
    dongPhieu.push_back({"Tien coc", coc > 0 ? dinhDangTien(coc) + " VND - " + ptCoc : "Khong coc"});

    inKhoi("PHIEU THONG TIN DAT PHONG", dongPhieu);
    cout << "\n";
    // ENTER: xac nhan va tao phieu | ESC: huy, khong tao phieu
    if (!xacNhanEnterEsc("XAC NHAN DAT PHONG?"))
    {
        canhBao("Da huy dat phong. Khong tao phieu.");
        return;
    }
    if (!nvHienTai)
    {
        loi("Khong xac dinh nhan vien dang dang nhap. Khong the tao phieu dat phong.");
        return;
    }
    auto ph = ks.datPhong(kh->layMa(), p->layMaPhong(), nvHienTai, ngayNhan, soDem, soNguoi, mua, ghiChu, coc, ptCoc);
    for (auto &x : dvDatTruoc)
        ks.ghiDichVu(ph->layMaPhieu(), x.first->layMa(), x.second);
    cout << "\n";
    thanhCong("Dat phong thanh cong: " + ph->layMaPhieu() + "  |  Chi nhanh " + cn->layTen() + "  |  Phong " +
              tenPhong(*p) + "  |  Dat luc " + dinhDangThoiGian(ph->layThoiGianDat()));

    if (ngayNhan == homNay && nhapCoKhong("Khach nhan phong ngay bay gio?"))
    {
        ks.checkIn(ph->layMaPhieu());
        thanhCong("Check-in luc " + dinhDangThoiGian(ph->layThoiGianCheckIn()) + ". Phong " + tenPhong(*p) +
                  " -> " + p->layTenTrangThai() + ".");
    }
}

void ChuongTrinh::checkIn()
{
    ks.dongBoTheoNgay();
    vector<shared_ptr<PhieuDatPhong>> ds;
    for (auto &ph : ks.locPhieu(TrangThaiPhieu::DA_DAT))
        if (ph->layNgayNhan() == Ngay::homNay())
            ds.push_back(ph);
    if (ds.empty())
    {
        canhBao("Hom nay khong co khach nao can check-in.");
        return;
    }
    auto ph = chonPhieu(ds, "Ma phieu check-in");
    if (!ph)
        return;
    inChiTietPhieu(ph);
    cout << "\n";
    if (!nhapCoKhong("Xac nhan check-in?"))
        return;
    ks.checkIn(ph->layMaPhieu());
    thanhCong("Check-in luc " + dinhDangThoiGian(ph->layThoiGianCheckIn()) + ". Phong " +
              tenPhong(*ph->layPhong()) + " (" + ph->layPhong()->layChiNhanh() + ") -> " +
              ph->layPhong()->layTenTrangThai() + ".");
}

void ChuongTrinh::huyDatPhong()
{
    // Dong bo no-show truoc khi liet ke (chi huy phieu qua ngay nhan, khong dung phieu tuong lai)
    ks.dongBoTheoNgay();
    auto ds = ks.locPhieu(TrangThaiPhieu::DA_DAT);
    // Sap xep moi nhat len tren de de nhin cac phieu vua dat
    sort(ds.begin(), ds.end(), [](const shared_ptr<PhieuDatPhong> &a, const shared_ptr<PhieuDatPhong> &b)
         { return a->layThoiGianDat() > b->layThoiGianDat(); });
    if (ds.empty())
    {
        canhBao("Khong co phieu dat phong nao o trang thai Cho nhan phong de huy.");
        return;
    }
    auto ph = chonPhieu(ds, "Ma phieu can huy");
    if (!ph)
        return;
    string lyDo = nhapGhiChu("Ly do huy: ", 100);
    bool hoanCoc = false;
    if (ph->layTienCoc() > 0)
        hoanCoc = xacNhanEnterEsc("HOAN LAI TIEN COC " + dinhDangTien(ph->layTienCoc()) + " VND CHO KHACH?");
    if (!xacNhanEnterEsc("XAC NHAN HUY PHIEU " + ph->layMaPhieu() + "?"))
        return;
    ks.huyDatPhong(ph->layMaPhieu(), lyDo, hoanCoc);
    thanhCong("Da huy phieu " + ph->layMaPhieu() + "." +
              (ph->layTienCoc() > 0 ? (hoanCoc ? " Da hoan coc." : " Giu lai tien coc.") : ""));
}

void ChuongTrinh::giaHanPhong()
{
    auto ph = chonPhieu(ks.locPhieu(TrangThaiPhieu::DA_NHAN_PHONG), "Ma phieu dang o can gia han");
    if (!ph)
        return;
    if (ph->dangOverstay())
        canhBao("Khach dang o QUA HAN " + to_string(ph->soNgayOverstay()) + " ngay (du kien tra " +
                ph->layNgayTra().toString() + ").");
    int them = nhapSoNguyenHopLe("So dem gia han them (1-30): ", 1, 30);
    string ly = ks.kiemTraGiaHan(ph->layMaPhieu(), them);
    if (!ly.empty())
    {
        loi(ly);
        return;
    }
    double truoc = ph->tinhTienPhong();
    double sau = ph->tinhTienPhong(them);
    thongTin("Tien phong: " + dinhDangTien(truoc) + " -> " + dinhDangTien(sau) + " VND (+" + dinhDangTien(sau - truoc) +
             "). Ngay tra moi: " + ph->layNgayTra().congNgay(them).toString());
    if (!nhapCoKhong("Xac nhan gia han " + to_string(them) + " dem?"))
        return;
    ks.giaHan(ph->layMaPhieu(), them);
    thanhCong("Da gia han phieu " + ph->layMaPhieu() + ", ngay tra moi " + ph->layNgayTra().toString() + ".");
}

void ChuongTrinh::xemDanhSachNguoiDat()
{
    int c = chonMenuTuongTac("", "", {"Tat ca", "Cho nhan phong", "Dang o", "Da tra phong", "Da huy"});
    if (c == 0)
        return;
    auto cn = chonChiNhanh(true);
    vector<shared_ptr<PhieuDatPhong>> ds;
    for (auto &ph : ks.layDanhSachPhieu())
        if ((c == 1 || ph->layTrangThai() == static_cast<TrangThaiPhieu>(c - 2)) &&
            (!cn || ks.chiNhanhCuaPhong(ph->layPhong()->layMaPhong()) == cn))
            ds.push_back(ph);
    sort(ds.begin(), ds.end(), [](const shared_ptr<PhieuDatPhong> &a, const shared_ptr<PhieuDatPhong> &b)
         { return a->layThoiGianDat() > b->layThoiGianDat(); });
    static const char *ten[] = {"", "TAT CA", "CHO NHAN PHONG", "DANG O", "DA TRA PHONG", "DA HUY"};
    manHinh(string("NGUOI DAT PHONG - ") + ten[c] + (cn ? " - " + cn->layTen() : ""), "Moi nhat o tren");
    inBangPhieu(ds);
    if (ds.empty())
        return;

    // Xem chi tiet theo ma: chi hien thi khi tim thay dung phieu; khong lay phieu khac thay the
    while (true)
    {
        string ma = nhapDong("Ma phieu de xem chi tiet (Enter de bo qua): ");
        if (ma.empty())
            return;
        auto ph = ks.timPhieu(ma);
        // Chi chap nhan phieu nam trong danh sach dang loc (cung trang thai / chi nhanh)
        if (ph)
        {
            bool trongDs = false;
            for (auto &x : ds)
                if (x->layMaPhieu() == ph->layMaPhieu())
                {
                    trongDs = true;
                    break;
                }
            if (!trongDs)
                ph = nullptr;
        }
        if (!ph)
        {
            UI::ghiTrucTiep(string("  ") + UI::DO +
                            "[!] Khong tim thay phieu dat phong phu hop." + UI::RESET + "\n");
            if (!xacNhanEnterEsc("BAN CO MUON TIM TIEP KHONG?"))
                return; // ESC -> ve menu truoc
            continue;   // ENTER -> nhap ma khac
        }
        inChiTietPhieu(ph);
        if (!xacNhanEnterEsc("BAN CO MUON TIM TIEP KHONG?"))
            return;
    }
}

void ChuongTrinh::xemLichSuDatTra()
{
    auto cn = chonChiNhanh(true);
    vector<shared_ptr<PhieuDatPhong>> ds;
    for (auto &ph : ks.layDanhSachPhieu())
        if (!cn || ks.chiNhanhCuaPhong(ph->layPhong()->layMaPhong()) == cn)
            ds.push_back(ph);
    sort(ds.begin(), ds.end(), [](const shared_ptr<PhieuDatPhong> &a, const shared_ptr<PhieuDatPhong> &b)
         { return a->layThoiGianDat() > b->layThoiGianDat(); });
    manHinh("LICH SU DAT & TRA PHONG" + string(cn ? " - " + cn->layTen() : " - TAT CA CHI NHANH"),
            "Moi phieu 1 dong, moi nhat o tren. Ngay mau xam = ngay du kien (chua dien ra).");
    inLichSu(ds);
    if (ds.empty())
        return;

    // Buoc 2: hoi co muon xem chi tiet khong? ENTER = co, ESC = thoat
    if (!xacNhanEnterEsc("Ban co muon xem chi tiet phieu khong?"))
        return;

    // Buoc 3-6: nhap ma phieu -> chi tiet -> hoi xem phieu khac
    while (true)
    {
        shared_ptr<PhieuDatPhong> ph;
        while (!ph)
        {
            tieuDeNho("NHAP MA PHIEU CAN XEM CHI TIET");
            string ma = nhapMa("Ma phieu: ", "DP");
            if (ma == "0")
            {
                // ESC: quay lai cau hoi xem chi tiet / thoat
                if (!xacNhanEnterEsc("Ban co muon xem chi tiet phieu khong?"))
                    return;
                continue;
            }
            ph = ks.timPhieu(ma);
            if (!ph)
            {
                UI::ghiTrucTiep(string("  ") + UI::DO + "[!] Khong tim thay ma phieu! Vui long kiem tra va nhap lai." + UI::RESET + "\n");
                continue;
            }
            // Chi cho xem phieu thuoc danh sach dang loc (theo chi nhanh)
            bool trongDs = false;
            for (auto &x : ds)
                if (x->layMaPhieu() == ph->layMaPhieu())
                {
                    trongDs = true;
                    break;
                }
            if (!trongDs)
            {
                UI::ghiTrucTiep(string("  ") + UI::DO + "[!] Phieu khong thuoc danh sach dang xem (chi nhanh dang loc)." + UI::RESET + "\n");
                ph = nullptr;
                continue;
            }
        }

        // Buoc 4: hien thi chi tiet day du
        manHinh("CHI TIET PHIEU DAT PHONG - " + ph->layMaPhieu());
        inChiTietPhieu(ph);

        // Buoc 5-6: xem phieu khac?
        if (!xacNhanEnterEsc("Ban co muon xem chi tiet phieu khac khong?"))
            return;
        // ENTER -> vong lap, nhap ma khac (khong ve lai toan bo danh sach)
    }
}

void ChuongTrinh::traCuuPhieu()
{
    // Tra cuu phieu: nhap ma -> chi tiet -> hoi xem them (ENTER) / thoat (ESC)
    // Nhap sai: bao loi duoi o nhap, cho nhap lai ngay, khong quay menu.
    while (true)
    {
        shared_ptr<PhieuDatPhong> ph;
        while (!ph)
        {
            tieuDeNho("TRA CUU PHIEU - NHAP MA PHIEU");
            string ma = nhapMa("Ma phieu: ", "DP");
            if (ma == "0")
                return; // ESC -> ve man hinh truoc
            ph = ks.timPhieu(ma);
            if (!ph)
            {
                UI::ghiTrucTiep(string("  ") + UI::DO +
                                "[!] Khong tim thay ma phieu! Vui long nhap lai." + UI::RESET + "\n");
                continue;
            }
        }

        manHinh("CHI TIET PHIEU DAT PHONG - " + ph->layMaPhieu());
        inChiTietPhieu(ph);

        // ENTER: xem them (nhap ma khac) | ESC: quay lai
        if (!xacNhanEnterEsc("Co muon xem them khong?"))
            return;
        // ENTER -> vong lap, nhap ma phieu khac
    }
}

// Chuyen phieu sang phong khac: CAP NHAT phong cua chinh phieu hien tai (khong tao phieu moi, khong xoa phieu cu).
// Giu nguyen ma phieu, khach hang, ngay nhan, ngay tra, tien coc, dich vu, trang thai -> chi doi phong.
void ChuongTrinh::chuyenPhieuSangPhongKhac()
{
    auto canXuLy = ks.phieuBiChanBoiKhachDangO();
    if (!canXuLy.empty())
    {
        canhBao("Cac phieu den ngay nhan nhung phong van bi khach truoc chua check-out chiem:");
        inBangPhieu(canXuLy);
        cout << "\n";
    }
    // ---- Buoc 1: nhap ma phieu (dung dinh dang + phai ton tai) ----
    shared_ptr<PhieuDatPhong> ph;
    while (!ph)
    {
        string ma = nhapMa("Nhap ma phieu can chuyen: ", "DP");
        if (ma == "0")
            return;
        ph = ks.timPhieu(ma);
        if (!ph)
            loi("Loi: Khong tim thay phieu " + ma + ". Vui long nhap lai.");
    }

    // ---- Buoc 2: thong tin phieu ----
    auto cu = ph->layPhong();
    inKhoi("THONG TIN PHIEU",
           {{"Ma phieu", ph->layMaPhieu()},
            {"Khach", ph->layKhachHang()->layHoTen() + " (" + ph->layKhachHang()->layMa() + ")"},
            {"Chi nhanh", tenChiNhanhCuaPhong(*cu)},
            {"Phong hien tai", cu->laySoPhong() + " - " + loaiPhongToString(cu->layLoaiPhong()) + " (" + cu->layMaPhong() + ")"},
            {"Ngay nhan", ph->layNgayNhan().toString()},
            {"Ngay tra", ph->layNgayTra().toString() + "  (" + to_string(ph->laySoDem()) + " dem, " +
                             to_string(ph->laySoNguoi()) + " khach)"},
            {"Trang thai", trangThaiPhieuToString(ph->layTrangThai())},
            {"Tien coc", ph->layTienCoc() > 0 ? dinhDangTien(ph->layTienCoc()) + " VND (giu nguyen)" : "Khong coc"}});
    if (auto dangO = ks.phieuDangOCuaPhong(cu->layMaPhong()); dangO && dangO != ph)
        canhBao("Phong " + cu->laySoPhong() + " dang co khach " + dangO->layKhachHang()->layHoTen() + " chua check-out" +
                (dangO->dangOverstay() ? " (qua han " + to_string(dangO->soNgayOverstay()) + " ngay)." : "."));
    string ly = ks.kiemTraChuyenPhong(ph->layMaPhieu());
    if (!ly.empty())
    {
        loi(ly);
        return;
    }

    // ---- Buoc 3: chi hien thi phong phu hop cho TOAN BO khoang ngay cua phieu ----
    bool cungLoai = true;
    auto ds = ks.phongCoTheChuyen(ph->layMaPhieu(), true);
    if (ds.empty())
    {
        canhBao("Khong con phong " + loaiPhongToString(cu->layLoaiPhong()) + " nao trong chi nhanh trong khoang " +
                ph->layNgayNhan().toString() + " -> " + ph->layNgayTra().toString() + ".");
        if (!nhapCoKhong("Xem them phong thuoc loai khac (gia phong se tinh theo phong moi)?"))
            return;
        cungLoai = false;
        ds = ks.phongCoTheChuyen(ph->layMaPhieu(), false);
    }
    if (ds.empty())
    {
        canhBao("Khong co phong phu hop de chuyen. Vui long cho khach dang o check-out hoac huy phieu.");
        return;
    }
    vector<string> muc;
    for (auto &x : ds)
        muc.push_back("Phong " + x->laySoPhong() + " - " + loaiPhongToString(x->layLoaiPhong()) + " - " +
                      to_string(x->laySucChua()) + " nguoi" +
                      (cungLoai ? "" : "  (" + dinhDangTien(x->layGiaPhong()) + "d/dem)"));
    cout << "\n";
    int c = chonMenuTuongTac("PHONG CO THE CHUYEN", "Du suc chua, khong co khach, khong trung phieu khac", muc, "Huy");
    if (c == 0)
    {
        canhBao("Da huy thao tac chuyen phong.");
        return;
    }
    auto moi = ds[(size_t)c - 1];

    // ---- Buoc 4: xac nhan ----
    if (moi->layLoaiPhong() != cu->layLoaiPhong() || fabs(moi->layGiaPhong() - cu->layGiaPhong()) > 0.5)
        thongTin("Tien phong du kien: " + dinhDangTien(ph->tinhTienPhong()) + " -> " +
                 dinhDangTien(moi->tinhGiaPhong(ph->laySoDem(), ph->layMua())) + " VND (tinh theo phong moi).");
    if (!nhapCoKhong("Ban co chac chan muon chuyen phieu " + ph->layMaPhieu() + " tu phong " + cu->laySoPhong() +
                     " sang phong " + moi->laySoPhong() + " khong?"))
    {
        canhBao("Da huy thao tac chuyen phong.");
        return;
    }
    ks.chuyenPhongPhieu(ph->layMaPhieu(), moi->layMaPhong());
    cout << "\n";
    thanhCong("Chuyen phong thanh cong!");
    dongThongTin("Phieu", ph->layMaPhieu() + " (giu nguyen ma, khach, ngay o, coc, trang thai)");
    dongThongTin("Phong cu", cu->laySoPhong() + " (" + cu->layMaPhong() + ")");
    dongThongTin("Phong moi", moi->laySoPhong() + " (" + moi->layMaPhong() + ")", DAM + XANH_LA);
}
