// MenuPhong.cpp - menuQuanLyPhong (mau vang nhat)
#include "ChuongTrinh.h"
#include "InputHelper.h"
#include "PhongCuThe.h"
#include "TienIch.h"
#include "TrangThaiCuThe.h"
#include <algorithm>
#include <iostream>
#include <map>

using namespace std;
using namespace InputHelper;
using namespace UI;

// QUAN LY PHONG: chon chi nhanh truoc -> chi hien phong cua chi nhanh do (khong tron du lieu cac chi nhanh)
void ChuongTrinh::menuQuanLyPhong()
{
    while (true)
    {
        datChuDe(ChuDe::PHONG);
        shared_ptr<ChiNhanhResort> cn;
        if (!chonChiNhanhManHinh("CHON CHI NHANH", cn, false, "Quan ly phong - chon chi nhanh can xem"))
            break;
        cnDangChon = cn;
        menuPhongCuaChiNhanh();
        cnDangChon = nullptr; // quay lai buoc chon chi nhanh
    }
    cnDangChon = nullptr;
    datChuDe(ChuDe::CHINH);
}

void ChuongTrinh::menuPhongCuaChiNhanh()
{
    string ten = tenChiNhanh(*cnDangChon);
    auto cn = cnDangChon;
    // 6 chuc nang, khong co muc "Quay lai" (ESC de quay lai)
    chayMenu(ChuDe::PHONG, "PHONG - " + chuHoa(ten),
             cn->layDiaChi() + "  |  Hotline " + cn->layHotline(),
             {{"Them phong", true, [this]
               { themPhong(); }},
              {"Tim kiem phong", false, [this]
               { timKiemPhong(); }},
              {"Sua phong", true, [this]
               { suaPhong(); }},
              {"Doi trang thai phong", false, [this]
               { capNhatTrangThaiPhong(); }},
              {"Xem chi tiet phong", false, [this]
               { xemChiTietPhong(); }}},
             [this, cn]
             {
                 inBangPhong(cn->layDanhSachPhong(), false);
             },
             "Chi nhanh: " + ten);
}

void ChuongTrinh::xemTatCaPhong()
{
    // Chi hien bang phong; khong in dong chu thich trang thai ben duoi bang.
    inBangPhong(phongDangXem(), !cnDangChon);
}

void ChuongTrinh::timKiemPhong()
{
    // Dang o man hinh mot chi nhanh -> chi tim trong chi nhanh do
    const auto nguon = phongDangXem();
    auto loc = [&](const function<bool(const Phong &)> &dk)
    {
        vector<shared_ptr<Phong>> v;
        for (auto &p : nguon)
            if (dk(*p))
                v.push_back(p);
        return v;
    };
    while (true)
    {
        inKhoi("TIM KIEM / LOC PHONG", {{"Pham vi", cnDangChon ? tenChiNhanh(*cnDangChon) : "Tat ca chi nhanh"},
                                          {"So phong hien co", to_string(nguon.size())},
                                          {"Huong dan", "Chon tieu chi trong menu de loc ket qua"}});
        vector<string> tieuChi = {"Theo chi nhanh", "Theo loai phong", "Theo khoang gia moi dem", "Theo trang thai",
                                  "Theo suc chua toi thieu", "Phong con trong trong khoang ngay",
                                  "Theo ma / so phong"};
        if (cnDangChon)
            tieuChi[0] = "Tat ca phong cua chi nhanh";
        cout << "\n";
        UI::hienThi();
        int c = chonMenuTuongTac("TIM KIEM PHONG",
                                 "Nhan phim 1-7 hoac mui ten + ENTER de chon tieu chi. ESC de quay lai.", tieuChi);
        vector<shared_ptr<Phong>> kq;
        PhamViHuyBangEsc choPhepHuy; // ESC o buoc nhap tieu chi -> ve lai menu tim kiem
        try
        {
            switch (c)
            {
            case 0:
                return;
            case 1:
            {
                if (cnDangChon)
                {
                    kq = nguon;
                    break;
                }
                auto cn = chonChiNhanh();
                if (!cn)
                    return;
                kq = cn->layDanhSachPhong();
                break;
            }
            case 7:
            {
                string k = chuHoa(nhapChuoi("Ma phong hoac so phong: "));
                string ma = chuanHoaMa(k);
                kq = loc([&](const Phong &p)
                         { return p.layMaPhong() == ma || p.laySoPhong() == k; });
                break;
            }
            case 2:
            {
                LoaiPhong loai = nhapLoaiPhong();
                kq = loc([loai](const Phong &p)
                         { return p.layLoaiPhong() == loai; });
                break;
            }
            case 3:
            {
                const double GIA_MIN = 500000, GIA_MAX = 20000000;
                inKhoi("TIM KIEM THEO KHOANG GIA MOI DEM", {{"Gia toi thieu", dinhDangTien(GIA_MIN) + " VND"},
                                                              {"Gia toi da", dinhDangTien(GIA_MAX) + " VND"},
                                                              {"Dinh dang nhap", "500000 hoac 500,000 (chi so, dau phay ngan nhom 3 so)"},
                                                              {"Dieu kien", "Gia tu -> Gia den (Gia tu <= Gia den); hai dau mut deu hop le"},
                                                              {"Huong dan", "Nhap Gia tu truoc, sau do Gia den. ESC: ve menu tim kiem"}});
                double tu = nhapGiaTimKiem("Gia tu (VND): ", GIA_MIN, GIA_MAX);
                double den = nhapGiaTimKiem("Gia den (VND): ", GIA_MIN, GIA_MAX, tu);
                kq = loc([tu, den](const Phong &p)
                         { return p.layGiaPhong() >= tu && p.layGiaPhong() <= den; });
                break;
            }
            case 4:
            {
                // Vong lap xem trang thai: ENTER xem trang thai khac, ESC thoat
                while (true)
                {
                    thongTin("1. Trong   2. Dang giu cho   3. Dang o   4. Dang don dep   5. Dang bao tri");
                    MaTrangThai tt = static_cast<MaTrangThai>(nhapSoNguyenHopLe("Chon: ", 1, 5) - 1);
                    kq = loc([tt](const Phong &p)
                             { return p.layMaTrangThai() == tt; });
                    tieuDeNho("Ket qua (" + to_string(kq.size()) + " phong)");
                    inBangPhong(kq, !cnDangChon);
                    if (!xacNhanEnterEsc("Co muon xem trang thai phong khac khong ?"))
                        return; // ESC -> ve menu
                    // ENTER -> chon trang thai khac
                }
            }
            case 5:
            {
                int sc = nhapSoNguyenHopLe("So khach toi thieu: ", 1, 50);
                kq = loc([sc](const Phong &p)
                         { return p.laySucChua() >= sc; });
                break;
            }
            case 6:
            {
                inKhoi("TIM PHONG CON TRONG THEO NGAY", {{"Ngay nhan phong", "DD/MM/YYYY (de trong = hom nay)"},
                                                           {"Ngay tra phong", "DD/MM/YYYY, phai sau ngay nhan phong"},
                                                           {"Vi du", "15/11/2026 (cung chap nhan 15-11-2026)"},
                                                           {"Gioi han", "Toi da 60 dem"},
                                                           {"Huong dan", "Nhap ngay can tim. ESC: ve menu tim kiem"}});
                Ngay nn = nhapNgay("Ngay nhan phong (DD/MM/YYYY): ");
                int dem = 0;
                while (true)
                {
                    Ngay tra = nhapNgay("Ngay tra phong (DD/MM/YYYY): ", false);
                    dem = nn.khoangCachDen(tra);
                    if (dem < 1)
                    {
                        UI::ghiTrucTiep(string("  ") + UI::DO + "[!] Ngay tra phong phai sau ngay nhan phong." + UI::RESET + "\n");
                        continue;
                    }
                    if (dem > 60)
                    {
                        loi("Khoang o toi da 60 dem. Vui long nhap lai ngay tra phong.");
                        continue;
                    }
                    break;
                }
                kq = loc([&](const Phong &p)
                         { return ks.lyDoKhongDatDuoc(p, nn, dem, 1).empty(); });
                break;
            }
            default:
                continue;
            }
        }
        catch (const HuyNhap &)
        {
            canhBao("Da huy tim kiem. Quay lai menu tim kiem.");
            continue;
        }
        tieuDeNho("Ket qua (" + to_string(kq.size()) + " phong)");
        inBangPhong(kq, !cnDangChon);
        if (kq.empty())
            thongTin("Khong tim thay phong phu hop.");
        if (!xacNhanEnterEsc("Ban co muon tim kiem tiep khong?"))
            return;
    }
}

void ChuongTrinh::xemChiTietPhong()
{
    // 1) Hien danh sach phong (giao dien hien co)
    inBangPhong(phongDangXem(), !cnDangChon);

    // 2) Xac nhan xem chi tiet
    if (!xacNhanEnterEsc("Ban co muon xem chi tiet phong khong?"))
        return;

    // 3) Vong lap xem lien tiep nhieu phong; khong ve lai danh sach giua cac lan xem
    auto ds = phongDangXem();
    while (true)
    {
        shared_ptr<Phong> p;
        while (!p)
        {
            string s = nhapMa("Ma phong can xem chi tiet: ", "P");
            if (s == "0") // ESC
                return;
            auto tim = ks.timPhong(s);
            if (!tim)
            {
                loi("Khong co ma nay! Hay nhap lai.");
                continue;
            }
            if (find(ds.begin(), ds.end(), tim) == ds.end())
            {
                loi("Phong " + tim->layMaPhong() + " khong nam trong danh sach chi nhanh nay. Hay nhap lai.");
                continue;
            }
            p = tim;
        }

        // Hien chi tiet (khong xoa man hinh danh sach lan dau; cac lan sau chi cap nhat chi tiet)
        manHinh("PHONG " + tenPhong(*p) + "  -  " + p->layChiNhanh());
        vector<pair<string, string>> dong = {
            {"Loai phong", loaiPhongToString(p->layLoaiPhong()) + ", tang " + to_string(p->layTang())},
            {"Gia co ban", dinhDangTien(p->layGiaPhong()) + " VND / dem"},
            {"Suc chua", to_string(p->laySucChua()) + " khach, " + dinhDangSoThuc(p->layDienTich()) + " m2"},
            {"Trang thai", p->layTenTrangThai() + (p->ghiChuTrangThai().empty() ? "" : " - " + p->ghiChuTrangThai())}};
        for (auto &x : p->layThongTinRieng())
            dong.push_back(x);
        string dv;
        for (auto &s : p->layDichVuDiKem())
            dv += (dv.empty() ? "" : ", ") + s;
        dong.push_back({"Tien nghi", dv});
        inKhoi("", dong);

        tieuDeNho("Gia theo mua (da gom phu phi cua loai phong)");
        Bang b({{"Mua", 12, false}, {"He so", 6, true}, {"1 dem", 12, true}, {"7 dem (-10%)", 14, true}});
        for (int m = 1; m <= 4; ++m)
        {
            MuaDuLich mua = static_cast<MuaDuLich>(m);
            b.themDong({muaToString(mua), "x" + dinhDangSoThuc(heSoMua(mua), 1), dinhDangTienVN(p->tinhGiaPhong(1, mua)),
                        dinhDangTienVN(p->tinhGiaPhong(7, mua))});
        }
        b.in();

        tieuDeNho("Lich dat phong (dang o / sap toi)");
        inBangPhieu(ks.lichDatCuaPhong(p->layMaPhong()), "(Chua co ai dat - phong trong o moi ngay)");

        // 4) Tiep tuc xem phong khac hay ESC quay lai menu phong
        if (!xacNhanEnterEsc("Co muon xem tiep khong?"))
            return;
        // ENTER: nhap ma phong tiep, khong ve lai danh sach
    }
}

// Bao gia: minh hoa Decorator - moi dich vu chon them se boc phong bang mot lop
void ChuongTrinh::baoGiaPhong()
{
    inBangPhong(phongDangXem(), !cnDangChon);
    auto goc = chonPhong(phongDangXem(), "Ma phong can bao gia");
    if (!goc)
        return;
    Ngay ngayNhan = nhapNgay("Ngay nhan du kien (dd/mm/yyyy): ");
    int soDem = nhapSoNguyenHopLe("So dem: ", 1, 60);
    MuaDuLich mua = xacDinhMua(ngayNhan);

    shared_ptr<Phong> p = goc;
    vector<pair<string, double>> buoc;
    while (nhapCoKhong("Them dich vu vao bao gia?"))
    {
        auto dv = chonDichVu();
        if (!dv)
            break;
        string nhanSL = dv->layLoai() == "Tour" ? "So nguoi" : (dv->layLoai() == "Spa" ? "So lan" : "So suat");
        int sl = nhapSoNguyenHopLe(nhanSL + ": ", 1, 50);
        double truoc = p->tinhGiaPhong(soDem, mua);
        p = QuanLyKhachSan::bocDichVu(p, dv, sl); // them mot lop Decorator
        buoc.push_back({dv->layTen() + " x" + to_string(sl), p->tinhGiaPhong(soDem, mua) - truoc});
    }

    manHinh("BAO GIA PHONG " + tenPhong(*goc) + "  -  " + goc->layChiNhanh());
    dongThongTin("Ngay o", ngayNhan.toString() + " -> " + ngayNhan.congNgay(soDem).toString() + " (" +
                               to_string(soDem) + " dem)");
    dongThongTin("Mua", muaToString(mua) + " (x" + dinhDangSoThuc(heSoMua(mua), 1) + ")");
    string lyDo = ks.lyDoKhongDatDuoc(*goc, ngayNhan, soDem, 1);
    dongThongTin("Lich phong", lyDo.empty() ? "Con trong" : lyDo, lyDo.empty() ? XANH_LA : HONG);
    cout << "\n";
    const int W = RONG_KHOI;
    double tienDV = 0;
    for (auto &x : buoc)
        tienDV += x.second;
    double tong = p->tinhGiaPhong(soDem, mua);
    auto kqGia = HoaDon::tinhTien(tong, 0); // bao gia khong biet khach -> chua giam; cung cong thuc voi hoa don
    khungTren(W, "BAO GIA");
    khungTien("1. TIEN PHONG", goc->tinhGiaPhong(soDem, mua), W, DAM + CARAMEL);
    khungDong("", W);
    khungTien("2. TIEN DICH VU", tienDV, W, DAM + HONG);
    for (auto &x : buoc)
        khungTien("     " + x.first, x.second, W, MO);
    khungNgang(W);
    khungTien("Tam tinh", tong, W, CHU);
    khungTien("VAT 10%", kqGia.tienThue, W, CHU, "+");
    khungNgang(W);
    khungTien("TONG DU KIEN", kqGia.tong, W, DAM + CHU);
    khungDuoi(W);
    if (string cb = canhBaoNhieuMua(ngayNhan, soDem); !cb.empty())
        canhBao(cb);
    cout << "\n";
    thongTin("Chua tinh giam gia hang thanh vien.");
}

void ChuongTrinh::capNhatTrangThaiPhong()
{
    while (true)
    {
        int c = chonMenuTuongTac("", "", {"Hoan tat don dep    (Dang don dep -> Trong)", "Chuyen sang bao tri (Trong / Dang don -> Bao tri)",
                "Hoan tat bao tri    (Bao tri -> Trong)"});
        if (c == 0)
            return;
        vector<shared_ptr<Phong>> ds;
        for (auto &p : phongDangXem())
        {
            MaTrangThai tt = p->layMaTrangThai();
            if ((c == 1 && tt == MaTrangThai::DANG_DON) ||
                (c == 2 && (tt == MaTrangThai::TRONG || tt == MaTrangThai::DANG_DON)) ||
                (c == 3 && tt == MaTrangThai::BAO_TRI))
                ds.push_back(p);
        }
        cout << "\n";
        inBangPhong(ds, !cnDangChon);
        if (ds.empty())
        {
            canhBao("Khong co phong phu hop voi thao tac nay.");
            if (!xacNhanEnterEsc("Co muon chon thao tac khac khong ?"))
                return;
            continue;
        }
        auto p = chonPhong(ds, "Ma phong");
        if (!p)
            return;
        string truoc = p->layTenTrangThai();
        try
        {
            if (c == 1)
                p->donPhong();
            else if (c == 2)
                ks.chuyenBaoTri(p->layMaPhong(), nhapChuoi("Ly do bao tri: "));
            else
                p->hoanTat();
            ks.dongBoTheoNgay();
            thanhCong("Phong " + tenPhong(*p) + ": " + truoc + " -> " + p->layTenTrangThai() + ".");
        }
        catch (const exception &ex)
        {
            loi(string("Khong the doi trang thai: ") + ex.what());
        }
        if (!xacNhanEnterEsc("Co muon tiep tuc doi trang thai phong khac khong ?"))
            return; // ESC -> ve menu
        // ENTER -> vong lap tiep
    }
}

void ChuongTrinh::themPhong()
{
    while (true)
    {
        bool daThem = false;
        try
        {
            inKhoi("THEM PHONG MOI", {{"Huong dan", "Nhap thong tin theo tung buoc; ESC de huy."},
                                       {"Loai phong", "Chon tu menu Standard / Deluxe / Suite / Villa"}});
            auto cn = cnDangChon ? cnDangChon : chonChiNhanh();
            if (!cn)
            {
                if (!xacNhanEnterEsc("BAN CO MUON THEM PHONG TIEP KHONG?"))
                    return;
                continue;
            }
            PhamViHuyBangEsc choPhepHuy;
            string ma = ks.taoMaPhong();
            dongThongTin("Chi nhanh", tenChiNhanh(*cn), DAM + CHU);
            dongThongTin("Ma phong moi", ma);
            cout << "\n";
            LoaiPhong loai = nhapLoaiPhong();
            string so;
            while (true)
            {
                so = nhapSoDuongChuoi("So phong: ");
                if (!cn->coSoPhong(so))
                    break;
                UI::ghiTrucTiep(string("  ") + UI::DO + "[!] So phong " + so + " da co trong chi nhanh " + cn->layTen() + "." + UI::RESET + "\n");
            }
            int tang = nhapSoNguyenHopLe("Tang (1-5): ", 1, 5);
            double gia = nhapSoThucHopLe("Gia co ban moi dem (VND): ", 100000, 100000000);
            int scMin = 1, scMax = 20;
            if (loai == LoaiPhong::STANDARD) { scMin = 1; scMax = 2; }
            else if (loai == LoaiPhong::DELUXE || loai == LoaiPhong::SUITE) { scMin = 2; scMax = 6; }
            else if (loai == LoaiPhong::VILLA) { scMin = 8; scMax = 20; }
            int sc = nhapSoNguyenHopLe("Suc chua (khach, " + to_string(scMin) + "-" + to_string(scMax) + "): ", scMin, scMax);
            double dt = nhapSoThucHopLe("Dien tich (m2): ", 5, 5000);

            auto chonHuongNhin = []()
            {
                int c = chonMenuTuongTac("HUONG NHIN", "ESC de huy", {"Huong bien", "Huong thanh pho"}, "Huy");
                if (c == 0)
                    throw HuyNhap();
                return c == 1 ? string("Huong bien") : string("Huong thanh pho");
            };

            shared_ptr<Phong> p;
            switch (loai)
            {
            case LoaiPhong::STANDARD:
                p = make_shared<StandardRoom>(ma, so, tang, gia, sc, dt, chonHuongNhin());
                break;
            case LoaiPhong::DELUXE:
            {
                bool bc = nhapCoKhong("Co ban cong?");
                p = make_shared<DeluxeRoom>(ma, so, tang, gia, sc, dt, bc, chonHuongNhin());
                break;
            }
            case LoaiPhong::SUITE:
            {
                int spn = nhapSoNguyenHopLe("So phong ngu: ", 1, 10);
                bool pk = nhapCoKhong("Co phong khach?");
                double buffet = nhapSoThucHopLe("Phi buffet sang moi dem (VND): ", 1, 10000000);
                p = make_shared<Suite>(ma, so, tang, gia, sc, dt, spn, pk, buffet);
                break;
            }
            case LoaiPhong::VILLA:
            {
                bool hb = nhapCoKhong("Co ho boi rieng?");
                double sv = nhapSoThucHopLe("Dien tich san vuon (m2): ", 1, 100000);
                int spn = nhapSoNguyenHopLe("So phong ngu: ", 1, 20);
                double qg = nhapSoThucHopLe("Phi quan gia moi dem (VND): ", 1, 10000000);
                p = make_shared<Villa>(ma, so, tang, gia, sc, dt, hb, sv, spn, qg);
                break;
            }
            }

            if (xacNhanEnterEsc("BAN CO MUON THEM PHONG NAY KHONG?"))
            {
                ks.themPhong(cn->layMa(), p);
                thanhCong("Da them phong " + tenPhong(*p) + " vao chi nhanh " + tenChiNhanh(*cn) + ".");
                daThem = true;
            }
            else
                canhBao("Da huy them phong.");
        }
        catch (const HuyNhap &)
        {
            canhBao("Da huy them phong.");
        }

        if (!xacNhanEnterEsc("BAN CO MUON THEM PHONG TIEP KHONG?"))
            return;
        (void)daThem;
    }
}

void ChuongTrinh::suaPhong()
{
    // Chinh sua phong theo thuoc tinh: ma / loai / trang thai.
    // Sau MOI lan thao tac (thanh cong hoac that bai) deu hoi co muon tiep tuc sua khong.
    while (true)
    {
        inBangPhong(phongDangXem(), !cnDangChon);
        auto p = chonPhong(phongDangXem(), "Ma phong can sua");
        if (!p)
        {
            if (!xacNhanEnterEsc("BAN CO MUON SUA PHONG KHAC KHONG?"))
                return;
            continue;
        }

        bool daThayDoi = false;
        bool tiepTucSua = true;

        while (tiepTucSua)
        {
            // Hien thi thong tin hien tai
            inKhoi("CHINH SUA PHONG - " + tenPhong(*p), {
                {"Ma phong", p->layMaPhong()},
                {"Loai phong", loaiPhongToString(p->layLoaiPhong())},
                {"Trang thai", p->layTenTrangThai()},
                {"Gia / dem", dinhDangTien(p->layGiaPhong()) + " VND"},
                {"Suc chua", to_string(p->laySucChua()) + " nguoi"},
                {"Huong dan", "Chon thuoc tinh can sua. Co the sua nhieu lan lien tiep."}
            });

            // Menu chon thuoc tinh (dung helper mui ten cua du an)
            int c = chonMenuTuongTac(
                "CHINH SUA PHONG",
                "Chon thuoc tinh can sua | ESC: hoan tat",
                {"Sua ma phong", "Sua loai phong", "Sua trang thai phong", "Sua gia phong", "Hoan tat chinh sua"});

            // ESC hoac "Hoan tat" -> ket thuc sua phong nay
            if (c == 0 || c == 5)
            {
                tiepTucSua = false;
                break;
            }

            // Thuc hien sua thuoc tinh; bat ky ket qua nao (thanh cong / loi) deu hoi tiep tuc
            try
            {
                if (c == 1)
                {
                    // ----- Sua ma phong -----
                    tieuDeNho("Sua ma phong");
                    thongTin("Ma hien tai: " + p->layMaPhong());
                    string maMoi;
                    while (true)
                    {
                        maMoi = nhapMa("Ma phong moi: ", "P");
                        if (maMoi == "0")
                        {
                            canhBao("Da huy sua ma phong.");
                            break;
                        }
                        if (maMoi == p->layMaPhong())
                        {
                            canhBao("Ma moi trung ma hien tai. Khong thay doi.");
                            break;
                        }
                        if (ks.timPhong(maMoi))
                        {
                            loi("Ma phong " + maMoi + " da ton tai. Hay nhap ma khac.");
                            continue; // cho nhap lai
                        }
                        // Hop le
                        string maCu = p->layMaPhong();
                        p->datMaPhong(maMoi);
                        daThayDoi = true;
                        thanhCong("Da doi ma phong: " + maCu + " -> " + maMoi);
                        break;
                    }
                }
                else if (c == 2)
                {
                    // ----- Sua loai phong -----
                    tieuDeNho("Sua loai phong");
                    thongTin("Loai hien tai: " + loaiPhongToString(p->layLoaiPhong()));
                    LoaiPhong loaiMoi = nhapLoaiPhong();
                    if (loaiMoi == p->layLoaiPhong())
                    {
                        canhBao("Loai phong khong thay doi.");
                    }
                    else
                    {
                        // Cap nhat loai; gia / suc chua giu nguyen (nguoi dung co the chinh o module khac neu can)
                        // Luu y: cac thuoc tinh rieng cua Standard/Deluxe/Suite/Villa khong tu dong chuyen doi.
                        p->datLoaiPhong(loaiMoi);
                        daThayDoi = true;
                        thanhCong("Da doi loai phong thanh " + loaiPhongToString(loaiMoi) + ".");
                        canhBao("Luu y: gia, suc chua va thuoc tinh rieng (ban cong, so phong ngu...) khong tu dong doi theo loai.");
                    }
                }
                else if (c == 3)
                {
                    // ----- Sua trang thai phong -----
                    tieuDeNho("Sua trang thai phong");
                    thongTin("Trang thai hien tai: " + p->layTenTrangThai());
                    // Chi cho phep cac chuyen doi an toan thong qua logic hien co
                    int chonTT = chonMenuTuongTac(
                        "CHON TRANG THAI MOI",
                        "Mot so trang thai bi han che neu phong dang co khach / dang dat",
                        {"Trong", "Dang don dep", "Bao tri", "Huy (khong doi)"});
                    if (chonTT == 0 || chonTT == 4)
                    {
                        canhBao("Da huy sua trang thai.");
                    }
                    else
                    {
                        string truoc = p->layTenTrangThai();
                        MaTrangThai hienTai = p->layMaTrangThai();
                        if (chonTT == 1) // Trong
                        {
                            if (hienTai == MaTrangThai::DANG_DON)
                                p->donPhong();
                            else if (hienTai == MaTrangThai::BAO_TRI)
                                p->hoanTat();
                            else if (hienTai == MaTrangThai::TRONG)
                                canhBao("Phong da o trang thai Trong.");
                            else
                                throw runtime_error("Khong the chuyen truc tiep sang Trong khi dang " + truoc +
                                                    ". Hay dung chuc nang Doi trang thai phong hoac xu ly phieu dat truoc.");
                        }
                        else if (chonTT == 2) // Dang don dep
                        {
                            // Chi cho phep neu dang Trong (sau khi tra phong thuong la Dang don)
                            if (hienTai == MaTrangThai::TRONG)
                            {
                                // Tao trang thai Dang don (neu co constructor)
                                p->datTrangThai(make_shared<PhongDangDon>());
                            }
                            else if (hienTai == MaTrangThai::DANG_DON)
                                canhBao("Phong da o trang thai Dang don dep.");
                            else
                                throw runtime_error("Chi co the chuyen sang Dang don dep khi phong dang Trong.");
                        }
                        else if (chonTT == 3) // Bao tri
                        {
                            if (hienTai == MaTrangThai::BAO_TRI)
                                canhBao("Phong da o trang thai Bao tri.");
                            else
                            {
                                string lyDo = nhapChuoi("Ly do bao tri: ");
                                ks.chuyenBaoTri(p->layMaPhong(), lyDo);
                            }
                        }
                        ks.dongBoTheoNgay();
                        if (p->layTenTrangThai() != truoc)
                        {
                            daThayDoi = true;
                            thanhCong("Phong " + tenPhong(*p) + ": " + truoc + " -> " + p->layTenTrangThai() + ".");
                        }
                    }
                }
                else if (c == 4)
                {
                    // ----- Sua gia phong (khoang bat buoc 500.000 - 2.000.000 VND) -----
                    tieuDeNho("Sua gia phong");
                    const double giaMin = 500000;
                    const double giaMax = 2000000;

                    thongTin("Loai: " + loaiPhongToString(p->layLoaiPhong()) +
                             " | Gia hien tai: " + dinhDangTien(p->layGiaPhong()) + " VND/dem");
                    thongTin("Khoang hop le: " + dinhDangTien(giaMin) + " - " + dinhDangTien(giaMax) + " VND");
                    try
                    {
                        PhamViHuyBangEsc choPhepHuy;
                        double giaMoi = nhapSoThucHopLe("Gia moi (VND/dem): ", giaMin, giaMax);
                        if (giaMoi == p->layGiaPhong())
                        {
                            canhBao("Gia khong thay doi.");
                        }
                        else
                        {
                            double giaCu = p->layGiaPhong();
                            p->datGiaCoBan(giaMoi);
                            daThayDoi = true;
                            thanhCong("Da doi gia phong " + tenPhong(*p) + ": " +
                                      dinhDangTien(giaCu) + " -> " + dinhDangTien(giaMoi) + " VND/dem.");
                        }
                    }
                    catch (const HuyNhap &)
                    {
                        canhBao("Da huy sua gia phong.");
                    }
                }
            }
            catch (const HuyNhap &)
            {
                canhBao("Da huy nhap lieu cho thuoc tinh nay.");
            }
            catch (const exception &ex)
            {
                loi(string("Khong the cap nhat: ") + ex.what());
            }

            // ===== BAT BUOC: hoi tiep tuc sau MOI lan thao tac (ke ca khi loi) =====
            if (!xacNhanEnterEsc("BAN CO MUON TIEP TUC SUA PHONG KHONG?"))
                tiepTucSua = false;
        }

        if (daThayDoi)
        {
            luuDuLieu();
            thanhCong("Da luu thay doi cho phong " + tenPhong(*p) + " (" + tenChiNhanhCuaPhong(*p) + ").");
            inBangPhong({p}, !cnDangChon);
        }
        else
        {
            canhBao("Khong co thay doi nao duoc luu.");
        }

        if (!xacNhanEnterEsc("Ban co muon sua phong khac khong?"))
            return;
    }
}

void ChuongTrinh::xoaPhong()
{
    while (true)
    {
        inKhoi("XOA PHONG", {{"Dieu kien", "Chi xoa duoc phong khong con phieu dat / khach dang o"},
                              {"Lich su", "Phieu dat cu van duoc giu lai"}});
        inBangPhong(phongDangXem(), !cnDangChon);
        auto p = chonPhong(phongDangXem(), "Ma phong can xoa");
        if (!p)
        {
            if (!xacNhanEnterEsc("BAN CO MUON XOA PHONG KHAC KHONG?"))
                return;
            continue;
        }
        if (xacNhanEnterEsc("BAN CO MUON XOA PHONG " + tenPhong(*p) + " KHONG?"))
        {
            try
            {
                string ten = tenPhong(*p);
                ks.xoaPhong(p->layMaPhong());
                thanhCong("Da xoa phong " + ten + ".");
            }
            catch (const exception &ex)
            {
                loi(string("Khong the xoa: ") + ex.what());
            }
        }
        else
            canhBao("Da huy thao tac xoa phong.");

        if (!xacNhanEnterEsc("BAN CO MUON XOA PHONG KHAC KHONG?"))
            return;
    }
}
