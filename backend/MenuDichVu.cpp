// MenuDichVu.cpp - menuQuanLyDichVu (mau hong nhat)
#include "ChuongTrinh.h"
#include "InputHelper.h"
#include "TienIch.h"
#include <algorithm>
#include <iostream>

using namespace std;
using namespace InputHelper;
using namespace UI;

void ChuongTrinh::menuQuanLyDichVu()
{
    chayMenu(ChuDe::DICH_VU, "QUAN LY DICH VU",
             "Am thuc - Spa - Tour. ESC / Quay lai de thoat menu.",
             {{"Danh muc dich vu", false, [this]
               { xemDanhMucDichVu(); }},
              {"Ghi dich vu (khach dang o / dat truoc)", false, [this]
               { ghiNhanDichVu(); }},
              {"Dich vu da dung theo phieu", false, [this]
               { xemDichVuCuaPhieu(); }},
              {"Them dich vu", true, [this]
               { themDichVu(); }},
              {"Sua ten / don gia dich vu", true, [this]
               { suaDichVu(); }},
              {"Xoa dich vu", true, [this]
               { xoaDichVu(); }}});
}

void ChuongTrinh::xemDanhMucDichVu()
{
    inBangDichVu(ks.layDanhSachDichVu());
    cout << "\n";
    thongTin("Tour: doan tu " + to_string(TourTuyen::SO_NGUOI_GIAM_GIA) + " nguoi tro len duoc giam 10%.");
}

void ChuongTrinh::ghiNhanDichVu()
{
    auto dsDangO = ks.locPhieu(TrangThaiPhieu::DA_NHAN_PHONG);
    for (auto &ph : ks.locPhieu(TrangThaiPhieu::DA_DAT)) // dich vu dat truoc
        dsDangO.push_back(ph);
    if (dsDangO.empty())
    {
        canhBao("Hien khong co phieu nao dang o hoac cho nhan phong.");
        return;
    }
    auto ph = chonPhieu(dsDangO, "Ma phieu cua khach: ");
    if (!ph)
        return;
    tieuDeNho("Dich vu cho " + ph->layKhachHang()->layHoTen() + " - phong " + tenPhong(*ph->layPhong()) + " (" +
              ph->layPhong()->layChiNhanh() + ")");
    auto dv = chonDichVu();
    if (!dv)
        return;
    string nhanSL = dv->layLoai() == "Tour" ? "So nguoi" : (dv->layLoai() == "Spa" ? "So lan" : "So suat");
    int sl = 0;
    try
    {
        PhamViHuyBangEsc choPhepHuy;
        sl = nhapSoNguyenHopLe(nhanSL + ": ", 1, 100);
    }
    catch (const HuyNhap &)
    {
        canhBao("Da huy ghi nhan dich vu.");
        return;
    }
    thongTin("Thanh tien: " + dinhDangTien(dv->tinhChiPhi(sl)) + " VND");
    if (!xacNhanEnterEsc("XAC NHAN GHI DICH VU?"))
        return;
    ks.ghiDichVu(ph->layMaPhieu(), dv->layMa(), sl);
    thanhCong("Da ghi " + dv->layTen() + " x" + to_string(sl) + " vao phieu " + ph->layMaPhieu() +
              ". Tien dich vu hien tai: " + dinhDangTien(ph->tinhTienDichVu()) + " VND.");
}

void ChuongTrinh::xemDichVuCuaPhieu()
{
    vector<shared_ptr<PhieuDatPhong>> ds;
    for (auto &ph : ks.layDanhSachPhieu())
        if (!ph->layDichVuSuDung().empty())
            ds.push_back(ph);
    if (ds.empty())
    {
        canhBao("Chua co phieu nao su dung dich vu.");
        return;
    }
    auto ph = chonPhieu(ds, "Ma phieu: ");
    if (!ph)
        return;
    manHinh("DICH VU CUA PHIEU " + ph->layMaPhieu(),
            ph->layKhachHang()->layHoTen() + " - phong " + tenPhong(*ph->layPhong()) + " - " +
                ph->layPhong()->layChiNhanh());
    Bang b({{"Thoi gian", 16, false}, {"Dich vu", 22, false}, {"Loai", 8, false}, {"SL", 3, true},
            {"Don gia", 11, true}, {"Thanh tien", 12, true}});
    for (auto &sd : ph->layDichVuSuDung())
        b.themDong({dinhDangThoiGian(sd.thoiGian), sd.dichVu->layTen(), sd.dichVu->layLoai(), to_string(sd.soLuong),
                    dinhDangTienVN(sd.dichVu->layDonGia()), dinhDangTienVN(sd.thanhTien())},
                   {MO, "", MO, "", MO, DAM + CHU});
    b.in();
    cout << "\n";
    dongTien("TONG TIEN DICH VU", ph->tinhTienDichVu(), DAM + CHU, 60);
}

void ChuongTrinh::themDichVu()
{
    // Vong lap: them xong (hoac huy) van hoi co muon them tiep khong
    while (true)
    {
        bool daThem = false;
        try
        {
            PhamViHuyBangEsc choPhepHuyBangEsc;
            tieuDeNho("THEM DICH VU MOI");
            thongTin("Loai dich vu:  1. Am thuc   2. Spa   3. Tour tuyen");
            int loai = nhapSoNguyenHopLe("Chon loai: ", 1, 3);
            string ten;
            while (true)
            {
                ten = nhapDong("Ten dich vu: ");
                if (!InputHelper::laChuThuan(ten))
                {
                    UI::ghiTrucTiep(string("  ") + UI::DO + "[!] Ten dich vu chi duoc chua chu cai va khoang trang." + UI::RESET + "\n");
                    continue;
                }
                auto trung = find_if(ks.layDanhSachDichVu().begin(), ks.layDanhSachDichVu().end(),
                    [&](const auto &d) { return chuHoa(d->layTen()) == chuHoa(ten); });
                if (trung == ks.layDanhSachDichVu().end())
                    break;
                UI::ghiTrucTiep(string("  ") + UI::DO + "[!] Ten dich vu da ton tai. Vui long nhap ten khac." + UI::RESET + "\n");
            }
            shared_ptr<DichVu> dv;
            string ma = ks.taoMaDichVu();
            if (loai == 1)
            {
                double gia = nhapSoThucHopLe("Don gia moi suat (VND): ", 1000, 50000000);
                string bua;
                while (true)
                {
                    bua = nhapDong("Loai bua an: ");
                    // De trong -> mac dinh "Set menu"; neu co nhap thi chi duoc chu cai (nhap sai -> bao loi, nhap lai)
                    if (bua.empty() || InputHelper::laChuThuan(bua))
                        break;
                    UI::ghiTrucTiep(string("  ") + UI::DO + "[!] Loai bua an chi duoc chua chu cai va khoang trang." + UI::RESET + "\n");
                }
                dv = make_shared<AmThuc>(ma, ten, gia, bua);
            }
            else if (loai == 2)
            {
                double gia = nhapSoThucHopLe("Don gia moi lan (VND): ", 1000, 50000000);
                int phut = nhapSoNguyenHopLe("Thoi luong (phut): ", 15, 480);
                dv = make_shared<Spa>(ma, ten, gia, phut);
            }
            else
            {
                double gia = nhapSoThucHopLe("Don gia moi nguoi (VND): ", 1000, 50000000);
                string dd;
                while (true)
                {
                    dd = nhapDong("Diem den: ");
                    if (InputHelper::laTenRieng(dd, 50))
                        break;
                    UI::ghiTrucTiep(string("  ") + UI::DO + "[!] Diem den phai co chu cai; chi dung chu, so, khoang trang va - ' ." + UI::RESET + "\n");
                }
                int soNgay = nhapSoNguyenHopLe("So ngay: ", 1, 15);
                dv = make_shared<TourTuyen>(ma, ten, gia, dd, soNgay);
            }

            // Buoc xac nhan them
            if (xacNhanEnterEsc("BAN CO MUON THEM DICH VU NAY KHONG?"))
            {
                ks.themDichVu(dv);
                thanhCong("Da them dich vu " + dv->layMa() + " - " + dv->layTen() + ".");
                daThem = true;
            }
            else
                canhBao("Da huy them dich vu.");
        }
        catch (const HuyNhap &)
        {
            canhBao("Da huy them dich vu.");
        }

        // Bat buoc hoi tiep tuc (ke ca khi huy xac nhan / ESC giua form)
        if (!xacNhanEnterEsc("BAN CO MUON THEM DICH VU TIEP KHONG?"))
            return;
        (void)daThem;
    }
}

void ChuongTrinh::suaDichVu()
{
    while (true)
    {
        bool daSua = false;
        try
        {
            auto dv = chonDichVu();
            if (!dv)
            {
                // ESC o nhap ma / quay lai
                if (!xacNhanEnterEsc("BAN CO MUON SUA DICH VU KHAC KHONG?"))
                    return;
                continue;
            }

            PhamViHuyBangEsc choPhepHuy;
            tieuDeNho("SUA DICH VU " + dv->layMa() + " - " + dv->layTen());
            dongThongTin("Don gia hien tai", dinhDangTien(dv->layDonGia()) + " VND");
            int c = chonMenuTuongTac("CHON THUOC TINH", "ESC de quay lai", {"Ten dich vu", "Don gia"}, "Quay lai");
            if (c == 0)
            {
                // ESC o menu thuoc tinh -> van hoi sua tiep
            }
            else if (c == 1)
            {
                string ten;
                while (true)
                {
                    ten = nhapDong("Ten moi: ");
                    if (!InputHelper::laChuThuan(ten))
                    {
                        UI::ghiTrucTiep(string("  ") + UI::DO + "[!] Ten dich vu chi duoc chua chu cai va khoang trang." + UI::RESET + "\n");
                        continue;
                    }
                    auto trung = find_if(ks.layDanhSachDichVu().begin(), ks.layDanhSachDichVu().end(),
                        [&](const auto &d) { return d != dv && chuHoa(d->layTen()) == chuHoa(ten); });
                    if (trung == ks.layDanhSachDichVu().end())
                        break;
                    UI::ghiTrucTiep(string("  ") + UI::DO + "[!] Ten dich vu da ton tai. Vui long nhap ten khac." + UI::RESET + "\n");
                }
                if (xacNhanEnterEsc("BAN CO MUON SUA DICH VU NAY KHONG?"))
                {
                    dv->datTen(ten);
                    thanhCong("Da cap nhat ten dich vu " + dv->layMa() + ".");
                    daSua = true;
                }
                else
                    canhBao("Da huy sua dich vu. Du lieu cu duoc giu nguyen.");
            }
            else if (c == 2)
            {
                double gia = nhapSoThucHopLe("Don gia moi (VND): ", 1000, 50000000);
                if (xacNhanEnterEsc("BAN CO MUON SUA DICH VU NAY KHONG?"))
                {
                    dv->datDonGia(gia);
                    thanhCong("Da cap nhat don gia " + dv->layMa() + ". Cac lan su dung truoc giu gia cu.");
                    daSua = true;
                }
                else
                    canhBao("Da huy sua dich vu. Du lieu cu duoc giu nguyen.");
            }
        }
        catch (const HuyNhap &)
        {
            canhBao("Da huy sua dich vu.");
        }

        if (!xacNhanEnterEsc("BAN CO MUON SUA DICH VU KHAC KHONG?"))
            return;
        (void)daSua;
    }
}

void ChuongTrinh::xoaDichVu()
{
    // Vong lap xoa: nhap ma -> xac nhan -> hoi xoa tiep (bat buoc)
    while (true)
    {
        shared_ptr<DichVu> dv;
        // Buoc 1: nhap ma (sai -> nhap lai, ESC -> hoi tiep tuc)
        while (!dv)
        {
            string ma = nhapMa("Nhap ma dich vu can xoa: ", "DV");
            if (ma == "0")
            {
                // ESC: van hoi co muon xoa dich vu khac khong
                if (!xacNhanEnterEsc("BAN CO MUON XOA DICH VU KHAC KHONG?"))
                    return;
                break; // quay lai vong ngoai de nhap ma moi
            }
            dv = ks.timDichVu(ma);
            if (!dv)
            {
                UI::ghiTrucTiep(string("  ") + UI::DO +
                                "[!] Ma dich vu khong ton tai! Vui long nhap lai." + UI::RESET + "\n");
                continue;
            }
        }
        if (!dv)
            continue; // vua ESC o buoc nhap ma va chon tiep tuc

        // Buoc 2: hien thi thong tin dich vu
        manHinh("XOA DICH VU - " + dv->layMa());
        inKhoi("CHI TIET DICH VU CAN XOA",
               {{"Ma dich vu", dv->layMa()},
                {"Ten dich vu", dv->layTen()},
                {"Loai", dv->layLoai()},
                {"Don gia", dinhDangTien(dv->layDonGia()) + " VND / " + dv->layDonViTinh()},
                {"Ghi chu", "Hoa don / phieu da dung dich vu nay van giu nguyen lich su (gia da chot)."}});

        // Buoc 3: xac nhan xoa
        if (xacNhanEnterEsc("BAN CO CHAC CHAN MUON XOA DICH VU NAY KHONG?"))
        {
            try
            {
                string ten = dv->layTen();
                string ma = dv->layMa();
                ks.xoaDichVu(ma);
                thanhCong("Da xoa dich vu " + ma + " - " + ten + " khoi danh muc.");
                thongTin("Lich su hoa don cu khong bi anh huong.");
            }
            catch (const exception &ex)
            {
                loi(string("Khong the xoa: ") + ex.what());
            }
        }
        else
            canhBao("Da huy xoa. Dich vu duoc giu nguyen.");

        // Buoc 4: bat buoc hoi xoa tiep
        if (!xacNhanEnterEsc("BAN CO MUON XOA DICH VU KHAC KHONG?"))
            return;
    }
}
