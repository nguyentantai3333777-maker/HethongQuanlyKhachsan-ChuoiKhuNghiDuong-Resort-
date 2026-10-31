// MenuKhachHang.cpp - menuQuanLyKhachHang (xanh ngoc nhat) & menuNhanSuChiNhanh (tim nhat)
#include "ChuongTrinh.h"
#include "InputHelper.h"
#include "TienIch.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>

using namespace std;
using namespace InputHelper;
using namespace UI;
// =====================================================================
void ChuongTrinh::menuQuanLyKhachHang()
{
    chayMenu(ChuDe::KHACH_HANG, "KHACH HANG",
             "Tich diem / hang thanh vien. ESC / Quay lai de thoat menu.",
             {{"Danh sach khach hang", false, [this]
               { xemDanhSachKhachHang(); }},
              {"Tim kiem khach hang", false, [this]
               { timKiemKhachHang(); }},
              {"Them khach hang", false, [this]
               { themKhachHang(); }},
              {"Cap nhat thong tin & hang thanh vien", false, [this]
               { suaKhachHang(); }},
              {"Lich su dat & tra phong cua khach", false, [this]
               { xemLichSuKhachHang(); }}});
}
void ChuongTrinh::xemDanhSachKhachHang()
{
    const auto &dsCN = ks.layDanhSachChiNhanh();
    vector<string> loc = {"Tat ca chi nhanh"};
    for (auto &cn : dsCN)
        loc.push_back(tenChiNhanh(*cn));
    thongTin("Loc danh sach khach hang theo chi nhanh:");
    int c = chonMenuTuongTac("LOC THEO CHI NHANH", "", loc, "Huy");
    if (c == 0)
        return;
    vector<shared_ptr<KhachHang>> ds;
    string tieuDeLoc;
    // Luu cnDangChon de thong tin chi tiet khach hien thi dung chi nhanh dang xem
    auto cnTruoc = cnDangChon;
    if (c == 1)
    {
        cnDangChon = nullptr;
        ds = ks.layDanhSachKhachHang();
        tieuDeLoc = "Khach hang - tat ca chi nhanh";
    }
    else
    {
        auto cn = dsCN[(size_t)c - 2];
        cnDangChon = cn; // ngu canh chi nhanh dang thao tac
        for (auto &kh : ks.layDanhSachKhachHang())
        {
            // Uu tien loc theo maChiNhanh gan cho khach (phan chia CN01/CN02)
            if (!kh->layMaChiNhanh().empty())
            {
                if (kh->layMaChiNhanh() == cn->layMa())
                    ds.push_back(kh);
            }
            else
            {
                // Tuong thich du lieu cu: loc theo lich su dat phong
                for (auto &ph : kh->layLichSuDatPhong())
                    if (ks.maChiNhanhCuaPhong(ph->layPhong()) == cn->layMa())
                    {
                        ds.push_back(kh);
                        break;
                    }
            }
        }
        tieuDeLoc = "Khach hang - " + tenChiNhanh(*cn);
    }
    while (true)
    {
        tieuDeNho(tieuDeLoc);
        inBangKhachHang(ds);
        if (ds.empty())
        {
            thongTin("Khong co khach hang.");
            cnDangChon = cnTruoc;
            return;
        }

        // Bo sung: cho phep nhap ten de tim, hoac chon tu danh sach nhu cu
        vector<string> luaChon;
        luaChon.reserve(ds.size() + 1);
        luaChon.push_back(">> Tim theo ten khach hang");
        for (const auto &kh : ds)
            luaChon.push_back(kh->layMa() + " - " + kh->layHoTen() + " - " + kh->laySoDienThoai());

        const int chon = chonMenuTuongTac(
            "CHON KHACH HANG DE XEM CHI TIET",
            tieuDeLoc + " | ESC: quay lai | chon 1 de nhap ten",
            luaChon,
            "Quay lai");
        if (chon == 0)
        {
            cnDangChon = cnTruoc;
            return; // ESC
        }

        shared_ptr<KhachHang> kh;
        if (chon == 1)
        {
            // Tim theo ten trong danh sach hien tai
            string ten = nhapDongCoEsc("Nhap ten khach hang can xem: ");
            if (ten == "\x1b" || ten.empty())
                continue;
            // Tim trong ds (khong phan biet hoa thuong)
            string tenLower = ten;
            transform(tenLower.begin(), tenLower.end(), tenLower.begin(), ::tolower);
            vector<shared_ptr<KhachHang>> khop;
            for (const auto &k : ds)
            {
                string ht = k->layHoTen();
                string htLower = ht;
                transform(htLower.begin(), htLower.end(), htLower.begin(), ::tolower);
                if (htLower.find(tenLower) != string::npos)
                    khop.push_back(k);
            }
            if (khop.empty())
            {
                loi("Khong tim thay khach hang co ten phu hop trong danh sach.");
                continue;
            }
            if (khop.size() == 1)
                kh = khop.front();
            else
            {
                vector<string> luaKhop;
                for (const auto &k : khop)
                    luaKhop.push_back(k->layMa() + " - " + k->layHoTen() + " - " + k->laySoDienThoai());
                int ck = chonMenuTuongTac("KET QUA TIM THEO TEN", ten, luaKhop, "Quay lai");
                if (ck == 0)
                    continue;
                kh = khop[(size_t)ck - 1];
            }
        }
        else
        {
            kh = ds[(size_t)chon - 2];
        }
        inKhoi("CHI TIET KHACH HANG - " + kh->layMa(), thongTinChiTietKhachHang(*kh), 24, true);
        // ENTER: xem khach khac (quay lai danh sach) | ESC: thoat chuc nang
        if (!xacNhanEnterEsc("Ban co muon xem khach hang khac khong?"))
        {
            cnDangChon = cnTruoc;
            return;
        }
        // ENTER -> ve vong ngoai, ve lai bang + menu chon
    }
}

void ChuongTrinh::timKiemKhachHang()
{
    while (true)
    {
        thongTin("Huong dan: nhap ten, so dien thoai, CCCD/ho chieu hoac ma khach hang; ESC de quay lai.");
        string kw = nhapDongCoEsc("Tu khoa (ten / SDT / CCCD / ma): ");
        if (kw == "\x1b")
            return;
        if (kw.empty())
        {
            loi("Vui long nhap tu khoa tim kiem.");
            continue;
        }

        const auto ds = ks.timKhachTheoTuKhoa(kw);
        if (ds.empty())
        {
            loi("Khong tim thay khach hang phu hop.");
            hienThi();
            while (true)
            {
                const Phim phim = docPhim();
                if (phim == Phim::ENTER) break;
                if (phim == Phim::ESC) return;
            }
            continue;
        }

        shared_ptr<KhachHang> kh;
        if (ds.size() == 1)
        {
            // Neu chi co mot ket qua khop, hien chi tiet ngay, khong in them bang ket qua.
            kh = ds.front();
        }
        else
        {
            vector<string> luaChon;
            luaChon.reserve(ds.size());
            for (const auto &item : ds)
                luaChon.push_back(item->layMa() + " - " + item->layHoTen() + " - " + item->laySoDienThoai());
            const int c = chonMenuTuongTac("KET QUA TIM KHACH HANG", kw, luaChon, "Quay lai tim kiem");
            if (c == 0)
                continue;
            kh = ds[(size_t)c - 1];
        }

        inKhoi("CHI TIET KHACH HANG - " + kh->layMa(), thongTinChiTietKhachHang(*kh), 24, true);
        thongTin("ESC: quay lai.");
        hienThi(); // flush buffer: hien bang chi tiet ngay, khong can ENTER them
        while (true)
        {
            const Phim phim = docPhim();
            if (phim == Phim::ENTER) break;
            if (phim == Phim::ESC) return;
        }
    }
}

void ChuongTrinh::themKhachHang()
{
    bool luotDau = true;
    while (true)
    {
        if (!luotDau)
            tieuDeNho("Them khach hang moi"); // luot moi: bat dau lai tu dau, khong dung lai du lieu cu
        luotDau = false;
        auto kh = nhapKhachHangMoi();
        if (!kh) // da huy / khong luu -> ve menu quan ly khach hang
            return;
        inKhoi("KHACH HANG " + kh->layMa(), thongTinChiTietKhachHang(*kh));
        luuDuLieu(); // du lieu da xu ly xong -> ghi file truoc khi hoi tiep tuc
        if (!xacNhanEnterEsc("Ban co muon them khach hang nua khong?"))
            return;
    }
}

void ChuongTrinh::suaKhachHang()
{
    while (true)
    {
        auto kh = chonKhachHang(); // moi luot deu chon lai khach hang
        if (!kh)
            return;

        // Thay doi dang cho trong phien (chua ghi vao doi tuong / file)
        map<int, string> moi;
        int hangMoi = -1;
        // Thu tu menu (khong gom ma KH - ma la khoa, khong cho doi tai day)
        const vector<string> tenTruong = {
            "Ho va ten",
            "So dien thoai",
            "Email",
            "Gioi tinh",
            "CCCD / Ho chieu",
            "Quoc tich",
            "Dia chi thuong tru",
            "Hang thanh vien"
        };
        auto hienTai = [&](int c) -> string
        {
            switch (c)
            {
            case 1: return kh->layHoTen();
            case 2: return kh->laySoDienThoai();
            case 3: return kh->layEmail();
            case 4: return kh->layGioiTinh();
            case 5: return kh->layCCCD();
            case 6: return kh->layQuocTich();
            case 7: return kh->layDiaChiThuongTru();
            case 8: return kh->hangThanhVien();
            default: return "";
            }
        };

        bool daLuu = false;
        bool thoatPhien = false;
        while (!thoatPhien)
        {
            inKhoi(kh->layHoTen() + " (" + kh->layMa() + ")", thongTinChiTietKhachHang(*kh), 20, true);
            cout << "\n";
            UI::hienThi();

            vector<string> muc;
            for (int i = 1; i <= 8; ++i)
            {
                string nhan = tenTruong[(size_t)i - 1];
                if (i == 8 && hangMoi >= 0)
                    nhan += "  ->  " + hangToString((HangThanhVien)hangMoi);
                else if (i != 8 && moi.count(i))
                    nhan += "  ->  " + moi[i];
                muc.push_back(nhan);
            }
            muc.push_back("Hoan tat chinh sua");

            const int c = chonMenuTuongTac(
                "MENU CHINH SUA KHACH HANG",
                "Ma KH: " + kh->layMa() + " (khong doi) | ↑↓ + ENTER | ESC: huy phien",
                muc);

            // ESC o menu thuoc tinh = huy toan bo phien (khong luu)
            if (c == 0)
            {
                if (!moi.empty() || hangMoi >= 0)
                    canhBao("Da huy phien chinh sua. Khong co thay doi nao duoc luu.");
                thoatPhien = true;
                break;
            }
            // Hoan tat -> xac nhan luu / huy
            if (c == 9)
            {
                if (moi.empty() && hangMoi < 0)
                {
                    thongTin("Khong co thay doi nao de luu.");
                    thoatPhien = true;
                    break;
                }
                int xn = chonMenuTuongTac(
                    "XAC NHAN CHINH SUA",
                    "Ban co muon luu cac thay doi khong?",
                    {"Co, luu thay doi", "Khong, huy thay doi"});
                if (xn == 0)
                {
                    // ESC: quay lai menu chinh sua, giu thay doi dang cho
                    continue;
                }
                if (xn == 2)
                {
                    canhBao("Da huy cac thay doi. Du lieu cu duoc giu nguyen.");
                    thoatPhien = true;
                    break;
                }
                // xn == 1: luu
                const string cuTen = kh->layHoTen(), cuSdt = kh->laySoDienThoai(), cuCccd = kh->layCCCD(),
                             cuEmail = kh->layEmail(), cuQt = kh->layQuocTich(), cuGt = kh->layGioiTinh(),
                             cuDc = kh->layDiaChiThuongTru();
                const HangThanhVien cuHang = kh->layHang();
                try
                {
                    if (moi.count(1)) kh->datHoTen(moi[1]);
                    if (moi.count(2)) kh->datSoDienThoai(moi[2]);
                    if (moi.count(3)) kh->datEmail(moi[3]);
                    if (moi.count(4)) kh->datGioiTinh(moi[4]);
                    if (moi.count(5)) kh->datCCCD(moi[5]);
                    if (moi.count(6)) kh->datQuocTich(moi[6]);
                    if (moi.count(7)) kh->datDiaChiThuongTru(moi[7]);
                    if (hangMoi >= 0) kh->datHang((HangThanhVien)hangMoi);
                    daLuu = true;
                }
                catch (const exception &e)
                {
                    kh->datHoTen(cuTen);
                    kh->datSoDienThoai(cuSdt);
                    kh->datCCCD(cuCccd);
                    kh->datEmail(cuEmail);
                    kh->datQuocTich(cuQt);
                    kh->datHang(cuHang);
                    kh->datGioiTinh(cuGt);
                    kh->datDiaChiThuongTru(cuDc);
                    loi(string("Khong the cap nhat: ") + e.what() + " Khong co thay doi nao duoc luu.");
                    thoatPhien = true;
                    break;
                }
                luuDuLieu();
                thanhCong("Da cap nhat khach " + kh->layMa() + ".");
                // Cap nhat xong: hien thi lai chi tiet voi du lieu moi nhat (khong can thoat chuong trinh)
                inKhoi("CHI TIET KHACH HANG - " + kh->layMa(), thongTinChiTietKhachHang(*kh), 24, true);
                inBangKhachHang({kh});
                thoatPhien = true;
                break;
            }

            // Sua tung thuoc tinh (1..8)
            PhamViHuyBangEsc choPhepHuy;
            try
            {
                tieuDeNho("Sua " + tenTruong[(size_t)c - 1]);
                thongTin("Hien tai: " + (hienTai(c).empty() ? string("(trong)") : hienTai(c)) + "  |  ESC: huy nhap truong nay.");
                switch (c)
                {
                case 1:
                {
                    string v = nhapHoTen("Ho ten moi: ");
                    if (v == kh->layHoTen()) moi.erase(c); else moi[c] = v;
                    break;
                }
                case 2:
                {
                    string sdt = nhapSoDienThoai("SDT moi: ");
                    ks.kiemTraTrungKhach(sdt, "", kh->layMa());
                    if (sdt == kh->laySoDienThoai()) moi.erase(c); else moi[c] = sdt;
                    break;
                }
                case 3:
                {
                    string v = nhapEmail("Email moi: ");
                    if (v == kh->layEmail()) moi.erase(c); else moi[c] = v;
                    break;
                }
                case 4:
                {
                    string gt = nhapGioiTinh();
                    if (gt == kh->layGioiTinh()) moi.erase(c); else moi[c] = gt;
                    break;
                }
                case 5:
                {
                    string so = nhapCCCD("CCCD / Ho chieu moi: ");
                    ks.kiemTraTrungKhach("", so, kh->layMa());
                    if (so == kh->layCCCD()) moi.erase(c); else moi[c] = so;
                    break;
                }
                case 6:
                {
                    string v = nhapChuoiChu("Quoc tich moi: ");
                    if (v == kh->layQuocTich()) moi.erase(c); else moi[c] = v;
                    break;
                }
                case 7:
                {
                    string diaChi;
                    while (true)
                    {
                        diaChi = nhapDong("Dia chi thuong tru moi: ");
                        if (diaChi.empty() || laDiaChi(diaChi))
                            break;
                        loi("Dia chi khong hop le! Cho phep chu, so va , . / - ( ) #, dai 5-120 ky tu. Hay nhap lai.");
                    }
                    if (diaChi == kh->layDiaChiThuongTru()) moi.erase(c); else moi[c] = diaChi;
                    break;
                }
                case 8:
                {
                    Bang b({{"#", 2, true}, {"Hang", 10, false}, {"Giam gia", 8, true}, {"Diem toi thieu", 14, true}});
                    for (int h = 0; h <= 3; ++h)
                        b.themDong({to_string(h + 1), hangToString((HangThanhVien)h),
                                    to_string((int)(tyLeGiamCuaHang((HangThanhVien)h) * 100 + 0.5)) + "%",
                                    to_string(diemToiThieuCuaHang((HangThanhVien)h))},
                                   {DAM + CHU, (int)kh->layHang() == h ? DAM + XANH_LA : "", "", MO});
                    b.in();
                    thongTin("Hien tai: " + kh->hangThanhVien() + ", " + to_string(kh->layDiemTichLuy()) + " diem.");
                    int h = nhapSoNguyenHopLe("Chon hang moi: ", 1, 4) - 1;
                    hangMoi = (h == (int)kh->layHang()) ? -1 : h;
                    break;
                }
                }
            }
            catch (const HuyNhap &)
            {
                canhBao("Da huy nhap truong nay. Cac thay doi truoc do van duoc giu.");
            }
            catch (const exception &e)
            {
                loi(e.what());
            }
        }

        if (!daLuu)
        {
            // Da huy hoac khong co thay doi -> co the chon khach khac hoac thoat
            if (!xacNhanEnterEsc("Ban co muon sua khach hang khac khong?"))
                return;
            continue;
        }
        if (!xacNhanEnterEsc("Ban co muon cap nhat khach hang khac khong?"))
            return;
    }
}

void ChuongTrinh::xemLichSuKhachHang()
{
    while (true)
    {
        auto kh = chonKhachHang();
        if (!kh)
            return;
        manHinh("LICH SU - " + kh->layHoTen() + " (" + kh->layMa() + ")",
                kh->laySoDienThoai() + "  |  Hang " + kh->hangThanhVien() + "  |  " + to_string(kh->layDiemTichLuy()) +
                    " diem");
        auto ds = kh->layLichSuDatPhong();
        sort(ds.begin(), ds.end(), [](const shared_ptr<PhieuDatPhong> &a, const shared_ptr<PhieuDatPhong> &b)
             { return a->layThoiGianDat() > b->layThoiGianDat(); });
        inLichSu(ds);
        double tongChi = 0;
        for (auto &ph : ds)
            if (auto hd = ks.timHoaDon(ph->layMaHoaDon()))
                if (hd->layDaThanhToan())
                    tongChi += hd->tongThanhToan();
        cout << "\n";
        dongTien("Tong chi tieu", tongChi, DAM + CHU, 60);
        // ENTER: xem khach khac | ESC: thoat
        if (!xacNhanEnterEsc("Ban co muon xem lich su khach hang khac khong?"))
            return;
    }
}

// =====================================================================
//                          NHAN SU & CHI NHANH
// =====================================================================
void ChuongTrinh::menuNhanSuChiNhanh()
{
    chayMenu(ChuDe::NHAN_SU, "QUAN LY CHI NHANH & NHAN SU",
             "Chi ADMIN. ESC / Quay lai de thoat menu.",
             {{"Danh sach chi nhanh", true, [this]
               { xemChiNhanh(); }},
              {"Them chi nhanh", true, [this]
               { themChiNhanh(); }},
              {"Danh sach nhan vien", true, [this]
               { xemNhanVien(); }},
              {"Them nhan vien", true, [this]
               { themNhanVien(); }},
              {"Chinh sua thong tin nhan vien", true, [this]
               { suaNhanVien(); }},
              {"Cho nhan vien nghi viec", true, [this]
               { xoaNhanVien(); }}});
}

void ChuongTrinh::xemChiNhanh() { inBangChiNhanh(); }

void ChuongTrinh::themChiNhanh()
{
    while (true)
    {
        try
        {
            PhamViHuyBangEsc choPhepHuy;
            // Moi truong kiem tra ngay; sai -> bao loi duoi truong, nhap lai
            string ten;
            while (true)
            {
                ten = nhapTenChiNhanh("Ten chi nhanh: ");
                bool trung = false;
                for (auto &cn : ks.layDanhSachChiNhanh())
                    trung = trung || chuHoa(cn->layTen()) == chuHoa(ten);
                if (!trung)
                    break;
                UI::ghiTrucTiep(string("  ") + UI::DO + "[!] Ten chi nhanh '" + ten + "' da ton tai." + UI::RESET + "\n");
            }
            string dc = nhapDiaChi("Dia chi: ");
            string hl = nhapSoDienThoai("Hotline (10 so): ");
            cout << "\n";
            inKhoi("CHI NHANH SE DUOC THEM", {{"Ten chi nhanh", ten}, {"Dia chi", dc}, {"Hotline", hl}});
            cout << "\n";
            // Buoc 1: xac nhan them (ENTER/ESC, mau vang, khung)
            if (xacNhanEnterEsc("Ban co chac chan muon xac nhan thuc hien hay khong?"))
            {
                auto cn = ks.themChiNhanh(ten, dc, hl);
                ks.damBaoNhanSuMacDinh();
                damBaoTaiKhoanNhanSu();
                cout << "\n";
                thanhCong("Da them chi nhanh thanh cong! (" + cn->layMa() + " - " + tenChiNhanh(*cn) + ")");
            }
            else
                canhBao("Da huy thao tac. Khong them chi nhanh.");
        }
        catch (const HuyNhap &)
        {
            canhBao("Da huy them chi nhanh.");
        }
        // Buoc 2 bat buoc: hoi them nua?
        if (!xacNhanEnterEsc("Ban co muon them nua khong?"))
            return;
    }
}

void ChuongTrinh::xoaChiNhanh()
{
    // 1. Danh sach chi nhanh
    inBangChiNhanh();
    cout << "\n";
    // 2-4. Nhap ma, kiem tra dinh dang & ton tai; sai thi bao loi va nhap lai
    shared_ptr<ChiNhanhResort> cn;
    while (!cn)
    {
        string ma = nhapMa("Nhap ma chi nhanh can xoa: ", "CN");
        if (ma == "0")
        {
            canhBao("Da huy thao tac xoa chi nhanh.");
            return;
        }
        cn = ks.timChiNhanh(ma);
        if (!cn)
            loi("Loi: Ma chi nhanh " + ma + " khong ton tai. Vui long nhap lai.");
    }
    // 5. Thong tin chi nhanh
    int nv = 0;
    for (auto &x : cn->layDanhSachNhanVien())
        nv += x->layDangLam() ? 1 : 0;
    cout << "\n";
    inKhoi("CHI NHANH " + cn->layMa(),
           {{"Ten", tenChiNhanh(*cn)},
            {"Dia chi", cn->layDiaChi()},
            {"Hotline", cn->layHotline()},
            {"So phong", to_string(cn->layDanhSachPhong().size())},
            {"Nhan vien", to_string(nv) + " dang lam"},
            {"Hoa don", to_string(ks.hoaDonCuaChiNhanh(cn->layMa()).size())}});
    // Kiem tra quan he du lieu TRUOC khi hoi xac nhan (tranh lam hong phieu / hoa don / nhan vien)
    auto ly = ks.lyDoKhongXoaDuocChiNhanh(cn->layMa());
    if (!ly.empty())
    {
        cout << "\n";
        loi("Khong the xoa chi nhanh " + tenChiNhanh(*cn) + " vi con du lieu lien quan:");
        for (auto &x : ly)
            thongTin(x);
        return;
    }
    // 6-8. Xac nhan
    cout << "\n";
    if (!nhapXacNhanYN())
    {
        canhBao("Da huy thao tac. Chi nhanh khong bi xoa.");
        return;
    }
    string ten = tenChiNhanh(*cn);
    ks.xoaChiNhanh(cn->layMa());
    cout << "\n";
    thanhCong("Da xoa chi nhanh thanh cong! (" + ten + ")");
}

void ChuongTrinh::xemNhanVien()
{
    // Bat buoc chon chi nhanh truoc khi xem danh sach nhan vien
    auto cn = chonChiNhanh();
    if (!cn)
        return;
    vector<shared_ptr<NhanVien>> ds;
    for (auto &nv : cn->layDanhSachNhanVien())
        ds.push_back(nv);
    while (true)
    {
        tieuDeNho("Nhan vien chi nhanh " + cn->layTen());
        inBangNhanVien(ds);
        if (ds.empty())
        {
            thongTin("Chi nhanh nay chua co nhan vien dang lam viec.");
            return;
        }
        thongTin("Co muon xem chi tiet khong? ENTER: xem | ESC: quay lai");
        hienThi();
        const Phim phimXemChiTiet = docPhim();
        if (phimXemChiTiet == Phim::ESC)
            return;
        if (phimXemChiTiet != Phim::ENTER)
            continue;
        while (true)
        {
            string ma = nhapMa("Nhap ma nhan vien can xem chi tiet: ", "NV");
            if (ma == "0")
                return;
            auto nv = ks.timNhanVien(ma);
            if (!nv || find(ds.begin(), ds.end(), nv) == ds.end())
            {
                loi("Ma nhan vien khong thuoc danh sach chi nhanh dang xem. Vui long nhap lai.");
                continue;
            }
            auto info = nv->layThongTin();
            for (auto &x : info)
                if (x.first == "Chi nhanh")
                    x.second = cn->layTen();
            const auto tk = qlTaiKhoan.timTheoNhanVien(nv->layMa());
            info.push_back({"Tai khoan", tk ? "Co tai khoan dang nhap (" + tk->layUsername() + ")" : "Khong co tai khoan dang nhap"});
            inKhoi("CHI TIET NHAN VIEN - " + nv->layMa(), info);
            cout << "\n";
            break;
        }
    }
}

void ChuongTrinh::themNhanVien()
{
    auto cn = chonChiNhanh();
    if (!cn)
        return;
    PhamViHuyBangEsc choPhepHuy;
    string ten = nhapHoTen("Ho ten: ");
    string sdt = nhapSoDienThoai("So dien thoai: ");
    string email = nhapEmail("Email: ");
    string cccd = nhapDong("CCCD/CMND: ");
    while (!cccd.empty() && !((cccd.size() == 9 || cccd.size() == 12) && laSoThuan(cccd, cccd.size(), cccd.size())))
    {
        loi("CCCD/CMND khong hop le.");
        cccd = nhapDong("CCCD/CMND: ");
    }
    const string gioiTinh = nhapGioiTinh();
    string ngaySinh = nhapDong("Ngay sinh (dd/mm/yyyy): ");
    while (!ngaySinh.empty())
    {
        Ngay parsed;
        if (Ngay::docChuoi(ngaySinh, parsed))
            break;
        loi("Ngay sinh khong hop le.");
        ngaySinh = nhapDong("Ngay sinh (dd/mm/yyyy): ");
    }
    // Chuc vu duoc chon tu danh sach dong bo voi cac vai tro he thong,
    // tranh nhap sai chinh ta va tranh tao chuc vu khong duoc ho tro.
    const vector<string> chucVu = {
        "Quan ly",
        "Le tan",
        "Buong phong",
        "Ky thuat",
        "Phuc vu"
    };
    int chonCV = chonMenuTuongTac("CHON CHUC VU NHAN VIEN", "Chon mot chuc vu trong danh sach", chucVu, "Huy");
    if (chonCV == 0)
        return;
    string cv = chucVu.at(static_cast<size_t>(chonCV - 1));
    inKhoi("CHUC VU DA CHON", {{"Chuc vu", cv}});
    double luong = nhapSoThucHopLe("Luong co ban (toi thieu 7.000.000 VND): ",
                                   NhanVien::LUONG_CO_BAN_TOI_THIEU, 500000000);
    const bool quanLy = laChucVuQuanLy(cv);
    const bool leTan = laChucVuLeTan(cv);
    const bool taoTaiKhoan = (quanLy || leTan) && nhapCoKhong("Tao tai khoan dang nhap cho nhan vien nay?");
    string user, pass;
    const Role role = quanLy ? Role::ADMIN : Role::USER;
    bool seTaoTK = taoTaiKhoan;
    if (seTaoTK)
    {
        // choPhepEsc=true: ESC huy o moi lan nhap lai (ke ca sau khi username/mat khau sai)
        while (true)
        {
            user = nhapTaiKhoan("Username (3-20 ky tu): ", true);
            if (user == "\x1b")
            {
                canhBao("Da huy tao tai khoan.");
                seTaoTK = false;
                break;
            }
            try
            {
                QuanLyTaiKhoan::kiemTraUsername(user);
                if (qlTaiKhoan.timTheoUsername(user))
                    throw invalid_argument("Username '" + user + "' da ton tai.");
                break;
            }
            catch (const invalid_argument &e)
            {
                loi(e.what());
            }
        }
        while (seTaoTK)
        {
            pass = nhapMatKhau("Mat khau (6-32 ky tu): ", true);
            if (pass == "\x1b")
            {
                canhBao("Da huy tao tai khoan.");
                seTaoTK = false;
                break;
            }
            try
            {
                QuanLyTaiKhoan::kiemTraMatKhau(pass);
                break;
            }
            catch (const invalid_argument &e)
            {
                loi(e.what());
            }
        }
    }
    if (!xacNhanEnterEsc("Co chac muon them khong ?"))
    {
        canhBao("Da huy them nhan vien.");
        return;
    }
    auto nv = ks.themNhanVien(cn->layMa(), ten, sdt, cv, luong, email, cccd, gioiTinh, ngaySinh);
    thanhCong("Da them nhan vien " + nv->layMa() + " - " + nv->layHoTen() + " (" + cn->layTen() + ").");
    if (seTaoTK)
    {
        qlTaiKhoan.themTaiKhoan(user, pass, role, nv->layMa());
        thanhCong("Da tao tai khoan '" + user + "'.");
    }
}

void ChuongTrinh::suaNhanVien()
{
    // Chinh sua NV hien co: chon NV -> chon truong -> xac nhan -> luu. Khong doi ma NV, khong tao NV moi.
    while (true)
    {
        auto ds = ks.tatCaNhanVien();
        inBangNhanVien(ds);
        if (ds.empty())
        {
            thongTin("Khong co nhan vien dang lam viec.");
            return;
        }
        string ma = nhapMa("Ma nhan vien can sua: ", "NV");
        if (ma == "0")
            return;
        auto nv = ks.timNhanVien(ma);
        if (!nv || !nv->layDangLam())
        {
            loi("Khong tim thay nhan vien dang lam viec " + ma + ".");
            if (!xacNhanEnterEsc("Ban co muon chon nhan vien khac khong?"))
                return;
            continue;
        }

        bool tiepTucSua = true;
        while (tiepTucSua)
        {
            inKhoi("CHINH SUA NHAN VIEN - " + nv->layMa(), {
                {"Ho ten", nv->layHoTen()},
                {"Gioi tinh", nv->layGioiTinh()},
                {"Ngay sinh", nv->layNgaySinh().empty() ? "(Chua cap nhat)" : nv->layNgaySinh()},
                {"So dien thoai", nv->laySoDienThoai()},
                {"Email", nv->layEmail().empty() ? "(Khong)" : nv->layEmail()},
                {"CCCD", nv->layCCCD()},
                {"Chuc vu", nv->layChucVu()},
                {"Chi nhanh", nv->layMaChiNhanh()},
                {"Luong co ban", dinhDangTien(nv->layLuongCoBan()) + " VND"},
                {"Huong dan", "Chon truong can sua. Ma nhan vien khong doi."}
            });

            int c = chonMenuTuongTac(
                "CHINH SUA NHAN VIEN",
                "Chon truong | ESC: hoan tat",
                {"Ho va ten", "Gioi tinh", "Ngay sinh", "So dien thoai", "Email", "CCCD",
                 "Chuc vu", "Chi nhanh lam viec", "Luong co ban", "Hoan tat chinh sua"});
            if (c == 0 || c == 10)
            {
                tiepTucSua = false;
                break;
            }
            try
            {
                PhamViHuyBangEsc choPhepHuy;
                if (c == 1)
                {
                    string ten = nhapHoTen("Ho ten moi: ");
                    nv->datHoTen(ten);
                    thanhCong("Da cap nhat ho ten.");
                }
                else if (c == 2)
                {
                    string gt = nhapGioiTinh();
                    if (!gt.empty())
                    {
                        nv->datGioiTinh(gt);
                        thanhCong("Da cap nhat gioi tinh.");
                    }
                }
                else if (c == 3)
                {
                    string ns = nhapDong("Ngay sinh moi (dd/mm/yyyy): ");
                    while (!ns.empty())
                    {
                        Ngay parsed;
                        if (Ngay::docChuoi(ns, parsed))
                            break;
                        loi("Ngay sinh khong hop le.");
                        ns = nhapDong("Ngay sinh moi (dd/mm/yyyy): ");
                    }
                    nv->datNgaySinh(ns);
                    thanhCong("Da cap nhat ngay sinh.");
                }
                else if (c == 4)
                {
                    string sdt = nhapSoDienThoai("So dien thoai moi: ");
                    nv->datSoDienThoai(sdt);
                    thanhCong("Da cap nhat so dien thoai.");
                }
                else if (c == 5)
                {
                    string email = nhapEmail("Email moi: ");
                    nv->datEmail(email);
                    thanhCong("Da cap nhat email.");
                }
                else if (c == 6)
                {
                    string cccd = nhapCCCD("CCCD moi: ");
                    nv->datCCCD(cccd);
                    thanhCong("Da cap nhat CCCD.");
                }
                else if (c == 7)
                {
                    const vector<string> chucVu = {"Quan ly", "Le tan", "Buong phong", "Ky thuat", "Phuc vu"};
                    int chonCV = chonMenuTuongTac("CHON CHUC VU MOI", "", chucVu, "Huy");
                    if (chonCV > 0)
                    {
                        nv->datChucVu(chucVu[(size_t)chonCV - 1]);
                        thanhCong("Da cap nhat chuc vu: " + nv->layChucVu());
                    }
                    else
                        canhBao("Da huy doi chuc vu.");
                }
                else if (c == 8)
                {
                    auto cn = chonChiNhanh();
                    if (cn)
                    {
                        nv->datMaChiNhanh(cn->layMa());
                        thanhCong("Da chuyen nhan vien sang chi nhanh " + tenChiNhanh(*cn) + ".");
                    }
                    else
                        canhBao("Da huy doi chi nhanh.");
                }
                else if (c == 9)
                {
                    double luong = nhapSoThucHopLe("Luong co ban moi (toi thieu 7.000.000 VND): ",
                                                   NhanVien::LUONG_CO_BAN_TOI_THIEU, 500000000);
                    nv->datLuongCoBan(luong);
                    thanhCong("Da cap nhat luong: " + dinhDangTien(luong) + " VND.");
                }
            }
            catch (const HuyNhap &)
            {
                canhBao("Da huy nhap lieu cho truong nay.");
            }
            catch (const exception &ex)
            {
                loi(string("Khong the cap nhat: ") + ex.what());
            }

            if (!xacNhanEnterEsc("BAN CO MUON TIEP TUC SUA NHAN VIEN NAY KHONG?"))
                tiepTucSua = false;
        }

        if (!xacNhanEnterEsc("Ban co muon sua nhan vien khac khong?"))
            return;
    }
}

void ChuongTrinh::xoaNhanVien()
{
    // Vong lap xoa/nghi nhieu nhan vien:
    // - Moi nhan vien: nhap ma -> xac nhan rieng -> (neu dong y) nhap ly do rieng -> luu
    // - ESC o xac nhan / o ly do: huy NHAN VIEN HIEN TAI, roi hoi co muon tiep tuc xoa ai khong
    // - ENTER o cau hoi tiep tuc: quay lai Buoc 1 (nhap ma moi)
    // - ESC o cau hoi tiep tuc: ket thuc chuc nang, ve menu
    // - Khong tu dong xoa lien tiep, khong bo qua xac nhan tung nguoi, khong gioi han so luot
    while (true)
    {
        inBangNhanVien(ks.tatCaNhanVien());
        string ma;
        shared_ptr<NhanVien> nv;
        // Buoc 1: nhap ma, lap khi sai / khong ton tai / da nghi
        while (true)
        {
            ma = nhapMa("Ma nhan vien: ", "NV");
            if (ma == "0")
                return; // ESC o nhap ma -> thoat chuc nang
            nv = ks.timNhanVien(ma);
            if (!nv || !nv->layDangLam())
            {
                loi("Khong tim thay nhan vien dang lam viec " + ma + ".");
                continue;
            }
            if (nv == nvHienTai)
            {
                loi("Khong the cho chinh minh nghi viec.");
                continue;
            }
            break;
        }

        auto tk = qlTaiKhoan.timTheoNhanVien(nv->layMa());
        string user = tk ? tk->layUsername() : "";

        // Buoc 2: xac nhan xoa (ENTER = co, ESC = huy nhan vien nay roi hoi tiep tuc)
        string cauHoi = "Co chac muon xoa nguoi nay khong ?";
        if (tk)
            cauHoi += " (se xoa ca tai khoan '" + user + "')";
        if (!xacNhanEnterEsc(cauHoi))
        {
            canhBao("Da huy thao tac voi nhan vien " + nv->layHoTen() + ".");
            // Khong return: chuyen sang cau hoi tiep tuc
            if (!xacNhanEnterEsc("Co muon tiep tuc xoa ai khong ?"))
                return; // ESC -> ve menu
            continue;   // ENTER -> nhap ma moi
        }

        // Buoc 3: nhap ly do cho nghi (bat buoc, khong rong; ESC huy nhan vien nay roi hoi tiep tuc)
        string lyDo;
        bool huyLyDo = false;
        while (true)
        {
            string s = trim(nhapDongCoEsc("Nhap ly do cho nghi: "));
            if (s == "\x1b")
            {
                canhBao("Da huy thao tac voi nhan vien " + nv->layHoTen() + ".");
                huyLyDo = true;
                break;
            }
            if (s.empty())
            {
                loi("Ly do cho nghi khong duoc de trong. Hay nhap lai.");
                continue;
            }
            if (s.size() > 200)
            {
                loi("Ly do toi da 200 ky tu. Hay nhap lai.");
                continue;
            }
            lyDo = s;
            break;
        }
        if (huyLyDo)
        {
            if (!xacNhanEnterEsc("Co muon tiep tuc xoa ai khong ?"))
                return;
            continue;
        }

        // Buoc 4: luu an toan (soft-delete + ly do + xoa tai khoan neu co)
        try
        {
            nv->datLyDoNghi(lyDo);
            if (tk)
                qlTaiKhoan.xoaTaiKhoan(user);
            ks.xoaNhanVien(nv->layMa());
            thanhCong("Da cho " + nv->layHoTen() + " nghi viec. Ly do: " + lyDo);
            thanhCong("Phieu/hoa don cu van giu ten nhan vien.");
        }
        catch (const exception &ex)
        {
            loi(string("Luu that bai: ") + ex.what());
            // Khong tiep tuc sang nhan vien khac khi loi he thong
            return;
        }

        // Buoc 5: hoi tiep tuc xoa ai khong (sau khi xoa thanh cong)
        if (!xacNhanEnterEsc("Co muon tiep tuc xoa ai khong ?"))
            return; // ESC -> ve menu
        // ENTER -> vong lap tiep, nhap ma moi
    }
}
