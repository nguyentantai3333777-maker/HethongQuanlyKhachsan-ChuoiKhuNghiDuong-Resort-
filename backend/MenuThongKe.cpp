// MenuThongKe.cpp - menuThongKeBaoCao (mau nau; moi cot bieu do mot sac nau khac nhau)
#include "ChuongTrinh.h"
#include "InputHelper.h"
#include "TienIch.h"
#include <iostream>

using namespace std;
using namespace InputHelper;
using namespace UI;

// THONG KE: chon pham vi truoc (mot chi nhanh hoac tat ca) -> moi bao cao chi tinh trong pham vi do
void ChuongTrinh::menuThongKeBaoCao()
{
    while (true)
    {
        datChuDe(ChuDe::THONG_KE);
        shared_ptr<ChiNhanhResort> cn;
        if (!chonChiNhanhManHinh("PHAM VI THONG KE", cn, true, "Chon chi nhanh, hoac tong hop toan he thong"))
            break;
        cnThongKe = cn;
        menuThongKeTheoPhamVi();
    }
    cnThongKe = nullptr;
    datChuDe(ChuDe::CHINH);
}

void ChuongTrinh::menuThongKeTheoPhamVi()
{
    string pv = cnThongKe ? chuHoa(tenChiNhanh(*cnThongKe)) : "TAT CA CHI NHANH";
    chayMenu(ChuDe::THONG_KE, "THONG KE - " + pv, "Doanh thu tinh tren hoa don da thanh toan (sau giam gia, gom VAT)",
             {{"Tong quan", true, [this]
               { thongKeTongQuan(); }},
              {"Doanh thu", true, [this]
               { baoCaoDoanhThu(); }},
              {"Cong suat phong", true, [this]
               { thongKeCongSuat(); }},
              {"Dich vu", true, [this]
               { thongKeDichVu(); }},
              {"Khach hang chi tieu nhieu nhat", true, [this]
               { thongKeKhachHang(); }}},
             nullptr, "Pham vi: " + (cnThongKe ? tenChiNhanh(*cnThongKe) : string("Tat ca chi nhanh")));
}

void ChuongTrinh::thongKeTongQuan()
{
    string maCN = cnThongKe ? cnThongKe->layMa() : "";
    auto phong = cnThongKe ? cnThongKe->layDanhSachPhong() : ks.tatCaPhong();
    auto phieu = ks.phieuCuaChiNhanh(maCN);
    size_t soKhach = 0;
    for (auto &kh : ks.layDanhSachKhachHang())
    {
        bool co = !cnThongKe;
        for (auto &ph : kh->layLichSuDatPhong())
            co = co || ks.maChiNhanhCuaPhong(ph->layPhong()) == maCN;
        soKhach += co ? 1 : 0;
    }
    const double doanhThuThang = ks.doanhThuThangHienTai(maCN);
    const double chiPhiLuongThang = ks.chiPhiLuongThang(maCN);
    const double thueDoanhThu = doanhThuThang * 0.10; // 10% thue doanh thu (bao cao quan tri)
    const double doanhThuThuc = doanhThuThang - chiPhiLuongThang - thueDoanhThu; // = DT*90% - luong
    // Khong hien thi muc tieu 3x (quy uoc noi bo, khong phai chi tieu quan tri)
    // Loi nhuan = DT thang - Luong NV - Thue 10% (dong bo voi bao cao Doanh thu)
    inChiSo({{"Tong doanh thu", dinhDangTienVN(ks.tongDoanhThu(maCN))},
             {"Doanh thu thang nay", dinhDangTienVN(doanhThuThang)},
             {"Tong luong nhan vien", dinhDangTienVN(chiPhiLuongThang)},
             {"Thue doanh thu 10%", dinhDangTienVN(thueDoanhThu)},
             {"Loi nhuan", dinhDangTienVN(doanhThuThuc)},
             {"Cong suat hien tai", dinhDangSoThuc(ks.congSuatPhong(maCN), 1) + " %"},
             {"Doanh thu tu phong", dinhDangTienVN(ks.doanhThuPhong(maCN))},
             {"Doanh thu dich vu", dinhDangTienVN(ks.doanhThuDichVu(maCN))},
             {"Tong so phong", to_string(phong.size()) + (cnThongKe ? "" : " (" + to_string(ks.layDanhSachChiNhanh().size()) + " CN)")},
             {"Tong khach hang", to_string(soKhach)},
             {"Tong phieu dat", to_string(phieu.size())},
             {"Hoa don con no", to_string(ks.soHoaDonChuaThanhToan(maCN))}});

    vector<pair<string, double>> tt;
    for (int i = 0; i <= (int)MaTrangThai::BAO_TRI; ++i)
    {
        int d = 0;
        for (auto &p : phong)
            if ((int)p->layMaTrangThai() == i)
                ++d;
        tt.push_back({tenTrangThai((MaTrangThai)i), (double)d});
    }
    veBieuDoCot("Phong theo trang thai", tt, false, "phong");

    vector<pair<string, double>> pt;
    for (int i = 0; i <= (int)TrangThaiPhieu::DA_HUY; ++i)
    {
        int d = 0;
        for (auto &ph : phieu)
            d += (int)ph->layTrangThai() == i ? 1 : 0;
        pt.push_back({trangThaiPhieuToString((TrangThaiPhieu)i), (double)d});
    }
    veBieuDoCot("Phieu dat phong theo trang thai", pt, false, "phieu");
}

void ChuongTrinh::baoCaoDoanhThu()
{
    string maCN = cnThongKe ? cnThongKe->layMa() : "";
    string tenPhamVi = cnThongKe ? ("OCEAN PEARL RESORT - " + chuHoa(cnThongKe->layTen()))
                                 : "OCEAN PEARL RESORT - TOAN HE THONG";

    // 4 chi tieu tai chinh bat buoc (cung cong thuc toan he thong)
    // Loi nhuan = Tong DT - Tong luong NV - Thue 10% DT
    auto inKhungDoanhThu = [&](const string &tieuPhu, double tongDT, double tongLuong) {
        const double thue = tongDT * 0.10;
        const double loiNhuan = tongDT - tongLuong - thue;
        const int trong = max(48, RONG - 8);
        auto dongTienHang = [&](const string &nhan, const string &giaTri, bool noiBat) {
            const string n = nhan;
            const string g = giaTri + " d";
            int cach = max(1, trong - 2 - doRong(n) - doRong(g));
            if (noiBat)
                cout << "  | " << VANG << DAM << n << string((size_t)cach, ' ') << g << RESET << " |\n";
            else
                cout << "  | " << CHU << n << string((size_t)cach, ' ') << DAM << CHU << g << RESET << " |\n";
        };
        cout << "\n";
        cout << "  +" << string(trong, '-') << "+\n";
        // Tieu de can giua
        {
            string t1 = "DOANH THU";
            int l1 = max(0, (trong - doRong(t1)) / 2);
            cout << "  |" << VANG << DAM << string(l1, ' ') << t1
                 << string((size_t)max(0, trong - l1 - doRong(t1)), ' ') << RESET << "|\n";
            int l2 = max(0, (trong - doRong(tieuPhu)) / 2);
            cout << "  |" << MO << string(l2, ' ') << tieuPhu
                 << string((size_t)max(0, trong - l2 - doRong(tieuPhu)), ' ') << RESET << "|\n";
        }
        cout << "  +" << string(trong, '-') << "+\n";
        cout << "  |" << string(trong, ' ') << "|\n";
        dongTienHang("Tong doanh thu:", dinhDangTienVN(tongDT), false);
        cout << "  |" << string(trong, ' ') << "|\n";
        dongTienHang("Tong luong nhan vien:", dinhDangTienVN(tongLuong), false);
        cout << "  |" << string(trong, ' ') << "|\n";
        dongTienHang("Thue doanh thu (10%):", dinhDangTienVN(thue), false);
        cout << "  |" << string(trong, ' ') << "|\n";
        cout << "  +" << string(trong, '-') << "+\n";
        cout << "  |" << string(trong, ' ') << "|\n";
        dongTienHang("LOI NHUAN:", dinhDangTienVN(loiNhuan), true);
        cout << "  |" << string(trong, ' ') << "|\n";
        cout << "  +" << string(trong, '-') << "+\n";
        cout << "\n";
    };

    if (cnThongKe)
    {
        // Pham vi 1 chi nhanh: DT thang hien tai + luong chi nhanh
        inKhungDoanhThu(tenPhamVi, ks.doanhThuThangHienTai(maCN), ks.chiPhiLuongThang(maCN));
    }
    else
    {
        // Toan he thong: tong hop + bang theo tung chi nhanh (cung 4 cot)
        inKhungDoanhThu(tenPhamVi, ks.doanhThuThangHienTai(""), ks.chiPhiLuongThang(""));

        Bang taiChinh({{"Chi nhanh", 18, false},
                       {"Tong DT", 14, true},
                       {"Luong NV", 14, true},
                       {"Thue 10%", 12, true},
                       {"Loi nhuan", 14, true}});
        for (const auto &cn : ks.layDanhSachChiNhanh())
        {
            const double doanhThu = ks.doanhThuThangHienTai(cn->layMa());
            const double chiPhi = ks.chiPhiLuongThang(cn->layMa());
            const double thue = doanhThu * 0.10;
            const double loiNhuan = doanhThu - chiPhi - thue;
            taiChinh.themDong({cn->layTen(), dinhDangTienVN(doanhThu), dinhDangTienVN(chiPhi),
                               dinhDangTienVN(thue), dinhDangTienVN(loiNhuan)});
        }
        const double doanhThu = ks.doanhThuThangHienTai();
        const double chiPhi = ks.chiPhiLuongThang();
        const double thue = doanhThu * 0.10;
        const double loiNhuan = doanhThu - chiPhi - thue;
        taiChinh.themDong({"TOAN HE THONG", dinhDangTienVN(doanhThu), dinhDangTienVN(chiPhi),
                           dinhDangTienVN(thue), dinhDangTienVN(loiNhuan)},
                          {DAM + CHU, DAM + CHU, DAM + CHU, DAM + CHU, VANG + DAM});
        taiChinh.in();
        veBieuDoCot("Theo chi nhanh", ks.doanhThuTheoChiNhanh());
    }

    veBieuDoCot("Theo loai phong", ks.doanhThuTheoLoaiPhong(maCN));
    veBieuDoCot("6 thang gan nhat", ks.doanhThuTheoThang(6, maCN));
}

void ChuongTrinh::thongKeCongSuat()
{
    Bang b({{"Chi nhanh", 12, false},
            {"Tong", 4, true},
            {"Trong", 5, true},
            {"Giu cho", 7, true},
            {"Dang o", 6, true},
            {"Don dep", 7, true},
            {"Bao tri", 7, true},
            {"Cong suat", 9, true}});
    vector<pair<string, double>> cs;
    for (auto &cn : ks.layDanhSachChiNhanh())
    {
        if (cnThongKe && cn != cnThongKe)
            continue;
        int tong = (int)cn->layDanhSachPhong().size();
        int o = cn->demPhong(MaTrangThai::DANG_O);
        double pct = tong ? 100.0 * o / tong : 0;
        b.themDong({cn->layTen(), to_string(tong), to_string(cn->demPhong(MaTrangThai::TRONG)),
                    to_string(cn->demPhong(MaTrangThai::GIU_CHO)), to_string(o),
                    to_string(cn->demPhong(MaTrangThai::DANG_DON)), to_string(cn->demPhong(MaTrangThai::BAO_TRI)),
                    dinhDangSoThuc(pct, 1) + " %"},
                   {DAM + CHU, "", XANH_LA, VANG, XANH_TROI, TIM, HONG, DAM + CHU});
        cs.push_back({cn->layTen(), pct});
    }
    b.in();
    veBieuDoCot("Cong suat (% phong dang co khach)", cs, false, "%");
}

void ChuongTrinh::thongKeDichVu()
{
    auto ds = ks.thongKeDichVu(cnThongKe ? cnThongKe->layMa() : "");
    Bang b({{"Ma", 4, false}, {"Dich vu", 22, false}, {"Loai", 8, false}, {"So luong", 8, true},
            {"Doanh thu", 13, true}});
    vector<pair<string, double>> bd;
    for (auto &x : ds)
    {
        b.themDong({x.ma, x.ten, x.loai, to_string(x.soLuong), dinhDangTienVN(x.doanhThu)},
                   {DAM + CHU, "", MO, "", DAM + CHU});
        bd.push_back({x.ten, x.doanhThu});
    }
    b.in("(Chua co dich vu nao duoc thanh toan)");
    veBieuDoCot("Doanh thu tung dich vu", bd);
}

void ChuongTrinh::thongKeKhachHang()
{
    auto ds = ks.topKhachHang(10, cnThongKe ? cnThongKe->layMa() : "");
    Bang b({{"#", 2, true}, {"Ma", 4, false}, {"Ho ten", 20, false}, {"Hang", 10, false}, {"Diem", 5, true},
            {"So lan", 6, true}, {"Tong chi tieu", 14, true}});
    vector<pair<string, double>> bd;
    int stt = 1;
    for (auto &x : ds)
    {
        b.themDong({to_string(stt++), x.khach->layMa(), x.khach->layHoTen(), x.khach->hangThanhVien(),
                    to_string(x.khach->layDiemTichLuy()), to_string(x.soHoaDon), dinhDangTienVN(x.tongChi)},
                   {MO, DAM + CHU, "", "", MO, "", DAM + CHU});
        bd.push_back({x.khach->layHoTen(), x.tongChi});
    }
    b.in("(Chua co khach hang thanh toan)");
    veBieuDoCot("Chi tieu cua khach", bd);
}
