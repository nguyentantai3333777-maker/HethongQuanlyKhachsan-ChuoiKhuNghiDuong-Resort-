// ChuongTrinh.cpp
#include "ChuongTrinh.h"
#include "InputHelper.h"
#include "PhongCuThe.h"
#include "PhongDecorator.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace std;
using namespace InputHelper;

// =====================================================================
//  Ham giao dien dung chung
// =====================================================================

// In menu trong khung, tieu de can giua
static void inMenu(const string& tieuDe, const vector<string>& muc)
{
    size_t rong = tieuDe.size() + 8;
    for (size_t i = 0; i < muc.size(); ++i) rong = max(rong, muc[i].size() + 4);
    size_t le = (rong - tieuDe.size()) / 2;
    string vienDoi = "+" + string(rong, '=') + "+";
    string vienDon = "+" + string(rong, '-') + "+";

    cout << "\n" << vienDoi << "\n|" << string(le, ' ') << tieuDe
         << string(rong - le - tieuDe.size(), ' ') << "|\n" << vienDon << "\n";
    for (size_t i = 0; i < muc.size(); ++i)
        cout << "| " << left << setw((int)rong - 2) << muc[i] << " |\n";
    cout << vienDoi << "\n";
}

// Lap lai thao tac cho den khi nguoi dung chon dung. f tra ve false -> dung ngay.
// Loi (nhap sai ma...) chi in thong bao roi hoi tiep, khong bi day ra menu.
static void lapLai(const string& hoiTiep, const function<bool()>& f)
{
    while (true)
    {
        try { if (!f()) return; }
        catch (const HetDuLieuNhap&) { throw; }
        catch (const exception& e) { cout << "[LOI] " << e.what() << "\n"; }
        if (!nhapCoKhong(hoiTiep)) return;
    }
}

// Tu thong tin chung da nhap, hoi them thuoc tinh rieng roi tao dung lop phong cu the
static shared_ptr<Phong> taoPhongCuThe(const Phong& p)
{
    shared_ptr<Phong> kq;
    switch (p.layLoaiPhong())
    {
    case LoaiPhong::STANDARD:
    {
        // Thong nhat: Standard co huong nhin Bien
        kq = make_shared<StandardRoom>(p.layMaPhong(), p.laySoPhong(), p.layTang(), p.layGiaPhong(),
                                       p.laySucChua(), p.layDienTich(), "Bien");
        break;
    }
    case LoaiPhong::DELUXE:
    {
        // Thong nhat: Deluxe co ban cong va huong nhin Bien
        kq = make_shared<DeluxeRoom>(p.layMaPhong(), p.laySoPhong(), p.layTang(), p.layGiaPhong(),
                                     p.laySucChua(), p.layDienTich(), true, "Bien");
        break;
    }
    case LoaiPhong::SUITE:
    {
        int spn = nhapSoNguyen("So phong ngu (1-10): ", 1, 10);
        bool pk = nhapCoKhong("Co phong khach?");
        kq = make_shared<Suite>(p.layMaPhong(), p.laySoPhong(), p.layTang(), p.layGiaPhong(),
                                p.laySucChua(), p.layDienTich(), spn, pk);
        break;
    }
    default:
    {
        bool hb = nhapCoKhong("Co ho boi?");
        double sanVuon = nhapSoThucDuong("Dien tich san vuon m2 (>0): ", 100000);
        int spn = nhapSoNguyen("So phong ngu (1-10): ", 1, 10);
        kq = make_shared<Villa>(p.layMaPhong(), p.laySoPhong(), p.layTang(), p.layGiaPhong(),
                                p.laySucChua(), p.layDienTich(), hb, sanVuon, spn);
        break;
    }
    }
    kq->datChiNhanh(p.layChiNhanh());
    return kq;
}

// Sua cac dich vu / thuoc tinh rieng cua tung loai phong
static void suaDichVuPhong(const shared_ptr<Phong>& p)
{
    shared_ptr<StandardRoom> st = dynamic_pointer_cast<StandardRoom>(p);
    shared_ptr<DeluxeRoom>   dl = dynamic_pointer_cast<DeluxeRoom>(p);
    shared_ptr<Suite>        su = dynamic_pointer_cast<Suite>(p);
    shared_ptr<Villa>        vi = dynamic_pointer_cast<Villa>(p);
    if (st || dl)
        throw runtime_error("Dich vu cua phong Standard/Deluxe la co dinh (huong nhin Bien, Deluxe co ban cong), khong sua duoc.");
    if (!su && !vi) throw runtime_error("Phong nay khong co dich vu de sua.");

    while (true)
    {
        cout << *p;
        int c = 0;
        if (st)
        {
            inMenu("SUA DICH VU", {"1. Huong nhin", "0. Xong"});
            c = nhapLuaChon(0, 1);
        }
        else if (dl)
        {
            inMenu("SUA DICH VU", {"1. Ban cong", "2. Huong nhin", "0. Xong"});
            c = nhapLuaChon(0, 2);
        }
        else if (su)
        {
            inMenu("SUA DICH VU", {"1. So phong ngu", "2. Phong khach", "0. Xong"});
            c = nhapLuaChon(0, 2);
        }
        else
        {
            inMenu("SUA DICH VU", {"1. Ho boi", "2. San vuon", "3. So phong ngu", "0. Xong"});
            c = nhapLuaChon(0, 3);
        }
        if (c == 0) return;

        try
        {
            if (st) st->datHuongNhin(nhapChuoiChu("Huong nhin moi: "));
            else if (dl)
            {
                if (c == 1) dl->datCoBanCong(nhapCoKhong("Co ban cong?"));
                else dl->datHuongNhin(nhapChuoiChu("Huong nhin moi: "));
            }
            else if (su)
            {
                if (c == 1) su->datSoPhongNgu(nhapSoNguyen("So phong ngu (1-10): ", 1, 10));
                else su->datCoPhongKhach(nhapCoKhong("Co phong khach?"));
            }
            else
            {
                if (c == 1) vi->datCoHoBoi(nhapCoKhong("Co ho boi?"));
                else if (c == 2) vi->datDienTichSanVuon(nhapSoThucDuong("Dien tich san vuon m2 (>0): ", 100000));
                else vi->datSoPhongNgu(nhapSoNguyen("So phong ngu (1-10): ", 1, 10));
            }
            cout << "Cap nhat dich vu thanh cong!\n";
        }
        catch (const HetDuLieuNhap&) { throw; }
        catch (const exception& e) { cout << "[LOI] " << e.what() << "\n"; }
    }
}

// ---- Hoa don ----
static void inVienHD() { cout << "+" << string(58, '-') << "+\n"; }
static void inDongHD(const string& s) { cout << "| " << left << setw(56) << s << " |\n"; }
// coDau = true: in kem dau +/- (dung cho cac khoan tang / giam gia)
static void inTienHD(const string& ten, double tien, bool coDau = false)
{
    string so = dinhDangTien(fabs(tien));
    if (coDau) so = string(tien < 0 ? "-" : "+") + so;
    ostringstream os;
    os << left << setw(36) << ten << right << setw(12) << so << " VND";
    inDongHD(os.str());
}

struct MucHoaDon { string ten; double tien; bool coDau; };

static string ghiChuMua(double heSo)
{
    long long pt = llround(fabs(heSo - 1.0) * 100);
    if (pt == 0) return "gia goc";
    return string(heSo < 1.0 ? "giam " : "tang ") + to_string(pt) + "%";
}

// =====================================================================
//  ChuongTrinh
// =====================================================================
ChuongTrinh::ChuongTrinh() : roleHienTai(Role::USER) { khoiTaoDuLieuMau(); }

// ===== Du lieu mau (ma phong / so phong deu la so duong) =====
void ChuongTrinh::khoiTaoDuLieuMau()
{
    qlTaiKhoan.themTaiKhoan("admin", "admin123", Role::ADMIN);
    qlTaiKhoan.themTaiKhoan("user", "user123", Role::USER);

    qlPhong.themPhong(make_shared<StandardRoom>("1001", "101", 1, 500000, 2, 20, "Bien"));
    qlPhong.themPhong(make_shared<StandardRoom>("1002", "102", 1, 550000, 3, 24, "Bien"));
    qlPhong.themPhong(make_shared<DeluxeRoom>("2001", "201", 2, 900000, 3, 32, true, "Bien"));
    qlPhong.themPhong(make_shared<DeluxeRoom>("2002", "202", 2, 950000, 4, 35, true, "Bien"));
    qlPhong.themPhong(make_shared<Suite>("3001", "301", 3, 1800000, 4, 55, 2, true));
    qlPhong.themPhong(make_shared<Suite>("3002", "302", 3, 2000000, 5, 60, 3, true));
    qlPhong.themPhong(make_shared<Villa>("9001", "901", 0, 5000000, 8, 150, true, 120, 4));
    qlPhong.themPhong(make_shared<Villa>("9002", "902", 0, 6500000, 10, 200, true, 250, 5));

    // Cho vai phong o cac trang thai khac nhau de de test
    qlPhong.chuyenTrangThai("1002", HanhDong::NHAN_PHONG);   // Dang o
    qlPhong.chuyenTrangThai("2002", HanhDong::GIU_CHO);      // Dang giu cho
    qlPhong.chuyenTrangThai("3001", HanhDong::BAO_TRI);      // Dang bao tri
}

void ChuongTrinh::yeuCauAdmin() const
{
    if (roleHienTai != Role::ADMIN)
        throw runtime_error("Ban khong co quyen thuc hien chuc nang nay (chi danh cho ADMIN).");
}

// ===== Vong lap chinh =====
void ChuongTrinh::chay()
{
    try
    {
        while (dangNhap())
        {
            if (roleHienTai == Role::ADMIN) menuAdmin();
            else menuUser();
        }
    }
    catch (const HetDuLieuNhap& e)
    {
        cout << "\n" << e.what() << "\n";
    }
    cout << "Tam biet!\n";
}

bool ChuongTrinh::dangNhap()
{
    inMenu("HE THONG QUAN LY PHONG", {"1. Dang nhap", "0. Thoat chuong trinh"});
    if (nhapLuaChon(0, 1) == 0) return false;

    for (int lan = 1; lan <= 3; ++lan)
    {
        string user = nhapDong("Username: ");
        string pass = nhapDong("Password: ");
        try
        {
            const TaiKhoan& tk = qlTaiKhoan.dangNhap(user, pass);
            userHienTai = tk.layUsername();
            roleHienTai = tk.layRole();
            cout << "Dang nhap thanh cong! Xin chao " << userHienTai
                 << " (" << roleToString(roleHienTai) << ")!\n";
            return true;
        }
        catch (const HetDuLieuNhap&) { throw; }
        catch (const exception& e)
        {
            cout << "[LOI] " << e.what() << " (lan " << lan << "/3)\n";
        }
    }
    cout << "Ban da nhap sai 3 lan. Quay lai man hinh dau.\n";
    return dangNhap();
}

// ===== Menu ADMIN =====
void ChuongTrinh::menuAdmin()
{
    while (true)
    {
        inMenu("ADMIN MENU",
               {"1. Them phong", "2. Xoa phong", "3. Sua thong tin phong",
                "4. Xem danh sach tat ca phong", "5. Tim kiem phong",
                "6. Xem phong theo trang thai", "7. Chuyen trang thai phong",
                "8. Xem thong tin chi tiet phong", "9. Quan ly tai khoan",
                "0. Dang xuat"});
        int c = nhapLuaChon(0, 9);
        if (c == 0) { cout << "Da dang xuat.\n"; return; }
        try
        {
            switch (c)
            {
            case 1: xuLyThemPhong(); break;
            case 2: xuLyXoaPhong(); break;
            case 3: xuLySuaPhong(); break;
            case 4: xuLyXemTatCa(); break;
            case 5: xuLyTimKiem(); break;
            case 6: xuLyXemTheoTrangThai(); break;
            case 7: xuLyChuyenTrangThai(); break;
            case 8: xuLyXemChiTiet(); break;
            case 9: xuLyQuanLyTaiKhoan(); break;
            }
        }
        catch (const HetDuLieuNhap&) { throw; }
        catch (const exception& e) { cout << "[LOI] " << e.what() << "\n"; }
    }
}

// ===== Menu USER =====
void ChuongTrinh::menuUser()
{
    while (true)
    {
        inMenu("USER MENU",
               {"1. Xem danh sach phong", "2. Tim phong", "3. Xem phong trong",
                "4. Xem thong tin chi tiet phong", "5. Dat phong",
                "6. Tra phong", "0. Dang xuat"});
        int c = nhapLuaChon(0, 6);
        if (c == 0) { cout << "Da dang xuat.\n"; return; }
        try
        {
            switch (c)
            {
            case 1: xuLyXemTatCa(); break;
            case 2: xuLyTimKiem(); break;
            case 3: xuLyXemPhongTrong(); break;
            case 4: xuLyXemChiTiet(); break;
            case 5: xuLyDatPhong(); break;
            case 6: xuLyTraPhong(); break;
            }
        }
        catch (const HetDuLieuNhap&) { throw; }
        catch (const exception& e) { cout << "[LOI] " << e.what() << "\n"; }
    }
}

// ===== Chuc nang ADMIN =====
void ChuongTrinh::xuLyThemPhong()
{
    yeuCauAdmin();
    lapLai("Ban co muon them phong khac?", [this]() -> bool { themMotPhong(); return true; });
}

void ChuongTrinh::themMotPhong()
{
    // Ma phong: so duong, trung -> bao va nhap lai ngay
    string ma;
    while (true)
    {
        ma = nhapSoDuongChuoi("Ma phong (so duong): ");
        if (!qlPhong.daCoMaPhong(ma)) break;
        cout << "[LOI] Ma phong '" << ma << "' da ton tai, vui long nhap ma khac.\n";
    }
    // So phong: so duong, trung -> bao va nhap lai ngay
    string so;
    while (true)
    {
        so = nhapSoDuongChuoi("So phong (so duong): ");
        if (so == ma)
        {
            cout << "[LOI] So phong phai khac ma phong ('" << ma << "'), vui long nhap so khac.\n";
            continue;
        }
        if (!qlPhong.daCoSoPhong(so)) break;
        cout << "[LOI] So phong '" << so << "' da ton tai, vui long nhap so khac.\n";
    }
    int tang = nhapSoNguyen("Tang (0-100): ", 0, 100);
    LoaiPhong loai = nhapLoaiPhong();
    double gia = nhapSoThucDuong("Gia co ban VND/dem (>0): ", 1e10);
    int sc = nhapSoNguyen("Suc chua (1-50): ", 1, 50);
    double dt = nhapSoThucDuong("Dien tich m2 (>0): ", 10000);
    string cn = nhapChuoiChu("Chi nhanh (chi nhap chu): ");

    Phong chung(ma, so, tang, loai, gia, sc, dt);
    chung.datChiNhanh(cn);
    shared_ptr<Phong> p = taoPhongCuThe(chung);   // hoi them thuoc tinh rieng cua loai phong
    qlPhong.themPhong(p);
    cout << "Them phong thanh cong!\n" << *p;
}

void ChuongTrinh::xuLyXoaPhong()
{
    yeuCauAdmin();
    if (qlPhong.layTatCa().empty()) throw runtime_error("Danh sach phong dang rong.");
    lapLai("Ban co muon xoa phong khac?", [this]() -> bool
    {
        shared_ptr<Phong> p = qlPhong.timTheoMa(nhapSoDuongChuoi("Nhap ma phong can xoa (so duong): "));
        cout << *p;
        if (nhapCoKhong("Ban chac chan muon xoa?"))
        {
            qlPhong.xoaPhong(p->layMaPhong());
            cout << "Da xoa phong.\n";
        }
        return true;
    });
}

void ChuongTrinh::xuLySuaPhong()
{
    yeuCauAdmin();
    if (qlPhong.layTatCa().empty()) throw runtime_error("Danh sach phong dang rong.");
    lapLai("Ban co muon sua phong khac?", [this]() -> bool
    {
        shared_ptr<Phong> p = qlPhong.timTheoMa(nhapSoDuongChuoi("Nhap ma phong can sua (so duong): "));
        suaMotPhong(p);
        return true;
    });
}

void ChuongTrinh::suaMotPhong(const shared_ptr<Phong>& p)
{
    while (true)
    {
        cout << *p;
        inMenu("SUA PHONG " + p->layMaPhong(),
               {"1. So phong", "2. Tang", "3. Loai", "4. Gia", "5. Suc chua",
                "6. Dien tich", "7. Chi nhanh", "8. Dich vu", "0. Xong"});
        int c = nhapLuaChon(0, 8);
        if (c == 0) return;
        try
        {
            switch (c)
            {
            case 1:
                while (true)
                {
                    string so = nhapSoDuongChuoi("So phong moi (so duong): ");
                    if (so == p->layMaPhong())
                    {
                        cout << "[LOI] So phong phai khac ma phong ('" << so << "'), vui long nhap so khac.\n";
                        continue;
                    }
                    if (qlPhong.daCoSoPhong(so, p->layMaPhong()))
                    {
                        cout << "[LOI] So phong '" << so << "' da ton tai, vui long nhap so khac.\n";
                        continue;
                    }
                    p->datSoPhong(so);
                    break;
                }
                break;
            case 2: p->datTang(nhapSoNguyen("Tang moi (0-100): ", 0, 100)); break;
            case 3:
                // Moi loai phong la 1 lop rieng (thuoc tinh rieng) nen khong doi loai tai cho
                cout << "Khong the doi loai phong. Hay xoa phong va them lai voi loai moi.\n";
                continue;
            case 4: p->datGiaCoBan(nhapSoThucDuong("Gia moi (>0): ", 1e10)); break;
            case 5: p->datSucChua(nhapSoNguyen("Suc chua moi (1-50): ", 1, 50)); break;
            case 6: p->datDienTich(nhapSoThucDuong("Dien tich moi (>0): ", 10000)); break;
            case 7: p->datChiNhanh(nhapChuoiChu("Chi nhanh moi (chi nhap chu): ")); break;
            case 8: suaDichVuPhong(p); continue;
            }
            cout << "Cap nhat thanh cong!\n";
        }
        catch (const HetDuLieuNhap&) { throw; }
        catch (const exception& e) { cout << "[LOI] " << e.what() << "\n"; }
    }
}

void ChuongTrinh::xuLyXemTheoTrangThai()
{
    yeuCauAdmin();
    lapLai("Ban co muon xem trang thai khac?", [this]() -> bool
    {
        inMenu("XEM THEO TRANG THAI",
               {"1. Trong", "2. Dang giu cho", "3. Dang o", "4. Dang don dep", "5. Dang bao tri", "0. Quay lai"});
        int c = nhapLuaChon(0, 5);
        if (c == 0) return false;
        const char* ten[] = {"Trong", "Dang giu cho", "Dang o", "Dang don dep", "Dang bao tri"};
        QuanLyPhong::hienThiDanhSach(qlPhong.timTheoTrangThai(ten[c - 1]));
        return true;
    });
}

void ChuongTrinh::xuLyChuyenTrangThai()
{
    yeuCauAdmin();
    lapLai("Ban co muon chuyen trang thai phong khac?", [this]() -> bool
    {
        shared_ptr<Phong> p = qlPhong.timTheoMa(nhapSoDuongChuoi("Nhap ma phong (so duong): "));
        cout << "Trang thai hien tai: " << p->layTenTrangThai() << "\n";
        inMenu("CHUYEN TRANG THAI",
               {"1. Giu cho", "2. Nhan phong", "3. Tra phong", "4. Don phong",
                "5. Bao tri", "6. Hoan tat", "0. Huy"});
        int c = nhapLuaChon(0, 6);
        if (c == 0) return false;
        HanhDong ds[] = {HanhDong::GIU_CHO, HanhDong::NHAN_PHONG, HanhDong::TRA_PHONG,
                         HanhDong::DON_PHONG, HanhDong::BAO_TRI, HanhDong::HOAN_TAT};
        qlPhong.chuyenTrangThai(p->layMaPhong(), ds[c - 1]);
        cout << "Thanh cong. Trang thai moi: " << p->layTenTrangThai() << "\n";
        return true;
    });
}

void ChuongTrinh::xuLyQuanLyTaiKhoan()
{
    yeuCauAdmin();
    while (true)
    {
        inMenu("QUAN LY TAI KHOAN", {"1. Xem", "2. Them", "3. Xoa", "4. Doi mat khau", "0. Quay lai"});
        int c = nhapLuaChon(0, 4);
        if (c == 0) return;
        try
        {
            if (c == 1) qlTaiKhoan.hienThi();
            else if (c == 2)
            {
                string u = nhapDong("Username moi: ");
                string pw = nhapDong("Password: ");
                cout << "Role: 1.ADMIN  2.USER\n";
                Role r = (nhapLuaChon(1, 2) == 1) ? Role::ADMIN : Role::USER;
                qlTaiKhoan.themTaiKhoan(u, pw, r);
                cout << "Them tai khoan thanh cong!\n";
            }
            else if (c == 3)
            {
                string u = nhapChuoi("Username can xoa: ");
                if (u == userHienTai) throw logic_error("Khong the tu xoa tai khoan dang dang nhap.");
                qlTaiKhoan.xoaTaiKhoan(u);
                cout << "Da xoa tai khoan.\n";
            }
            else
            {
                string u = nhapChuoi("Username: ");
                qlTaiKhoan.doiMatKhau(u, nhapDong("Mat khau moi: "));
                cout << "Doi mat khau thanh cong!\n";
            }
        }
        catch (const HetDuLieuNhap&) { throw; }
        catch (const exception& e) { cout << "[LOI] " << e.what() << "\n"; }
    }
}

// ===== Chuc nang dung chung =====
void ChuongTrinh::xuLyXemTatCa()
{
    if (qlPhong.layTatCa().empty()) { cout << "Danh sach phong dang rong.\n"; return; }
    qlPhong.hienThiTatCa();
    if (!nhapCoKhong("Ban co muon xem chi tiet mot phong?")) return;
    xuLyXemChiTiet();
}

void ChuongTrinh::xuLyTimKiem()
{
    while (true)
    {
        inMenu("TIM KIEM PHONG",
               {"1. Theo ma phong", "2. Theo so phong", "3. Theo loai phong",
                "4. Theo khoang gia", "0. Quay lai"});
        int c = nhapLuaChon(0, 4);
        if (c == 0) return;

        // Moi muc tu lap lai: nhap dung hay sai deu hoi co nhap tiep khong
        switch (c)
        {
        case 1:
            lapLai("Ban co muon tim ma phong khac?", [this]() -> bool
            {
                QuanLyPhong::DanhSachPhong kq;
                kq.push_back(qlPhong.timTheoMa(nhapSoDuongChuoi("Ma phong (so duong): ")));
                QuanLyPhong::hienThiDanhSach(kq);
                return true;
            });
            break;
        case 2:
            lapLai("Ban co muon tim so phong khac?", [this]() -> bool
            {
                QuanLyPhong::DanhSachPhong kq;
                kq.push_back(qlPhong.timTheoSoPhong(nhapSoDuongChuoi("So phong (so duong): ")));
                QuanLyPhong::hienThiDanhSach(kq);
                return true;
            });
            break;
        case 3:
            lapLai("Ban co muon tim loai phong khac?", [this]() -> bool
            {
                QuanLyPhong::hienThiDanhSach(qlPhong.timTheoLoai(nhapLoaiPhong()));
                return true;
            });
            break;
        default:
            lapLai("Ban co muon tim khoang gia khac?", [this]() -> bool
            {
                double tu = nhapSoThucDuong("Gia tu (VND): ", 1e10);
                double den = nhapSoThucDuong("Gia den (VND): ", 1e10);
                QuanLyPhong::hienThiDanhSach(qlPhong.timTheoKhoangGia(tu, den));
                return true;
            });
            break;
        }
    }
}

void ChuongTrinh::xuLyXemPhongTrong()
{
    QuanLyPhong::hienThiDanhSach(qlPhong.timPhongTrong());
}

void ChuongTrinh::xuLyXemChiTiet()
{
    lapLai("Ban co muon xem phong khac?", [this]() -> bool
    {
        cout << *qlPhong.timTheoMa(nhapSoDuongChuoi("Nhap ma phong (so duong): "));
        return true;
    });
}

// ===== Chuc nang USER =====
void ChuongTrinh::xuLyDatPhong()
{
    lapLai("Ban co muon dat phong khac?", [this]() -> bool { datMotPhong(); return true; });
}

// Dat phong = chon hinh thuc (thuong/tour) -> chon phong -> dich vu -> hoa don -> giu cho / nhan phong
void ChuongTrinh::datMotPhong()
{
    const double GIA_SANG = 150000, GIA_TRUA = 200000, GIA_TOI = 300000;   // VND / nguoi / bua

    inMenu("DAT PHONG",
           {"1. Dat thuong (tien phong + dich vu tu chon)",
            "2. Dat tour (tron goi: phong + an uong ca ngay)",
            "0. Quay lai"});
    int hinhThuc = nhapLuaChon(0, 2);
    if (hinhThuc == 0) return;
    bool laTour = (hinhThuc == 2);

    // Chi phong Trong moi dat duoc (phong dang giu cho / dang o / don dep / bao tri thi khong)
    QuanLyPhong::DanhSachPhong coThe = qlPhong.timPhongTrong();
    if (coThe.empty()) throw runtime_error("Hien khong con phong trong de dat.");
    cout << "\nCac phong co the dat:\n";
    QuanLyPhong::hienThiDanhSach(coThe);

    shared_ptr<Phong> p;
    while (true)
    {
        string ma = nhapDong("Nhap ma phong muon dat (so duong, 0 de quay lai): ");
        if (ma == "0") return;
        if (!laSoDuong(ma))
        {
            cout << "[LOI] Chi duoc nhap so duong (hoac 0 de quay lai).\n";
            continue;
        }
        try
        {
            p = qlPhong.timTheoMa(ma);
            string tt = p->layTenTrangThai();
            if (tt != "Trong")
                throw runtime_error("Phong " + p->layMaPhong() + " dang o trang thai '" + tt
                                    + "', khong the dat. Vui long chon phong Trong.");
            break;
        }
        catch (const HetDuLieuNhap&) { throw; }
        catch (const exception& e) { cout << "[LOI] " << e.what() << "\n"; }
    }

    cout << "\n" << *p;
    int soDem = nhapSoNguyen("So dem (1-365, tu 7 dem tro len giam 10% tien phong): ", 1, 365);
    MuaDuLich mua = nhapMuaDuLich();

    // Dich vu da kem san trong phong thi khong hoi them
    vector<string> dvSan = p->layDichVuDiKem();
    auto coSan = [&](const string& ten) { return find(dvSan.begin(), dvSan.end(), ten) != dvSan.end(); };
    bool coBuffet = coSan("Buffet sang");
    bool coSang = coBuffet || coSan("An sang");
    bool coMinibarSan = coSan("Minibar");

    cout << "\n-- Dich vu di kem (tra loi 'k' hoac nhap 0 neu khong dung) --\n";
    bool an = false, mini = false;
    int suatTrua = 0, suatToi = 0, nguoiTour = 0;
    if (laTour)
    {
        nguoiTour = nhapSoNguyen("So nguoi di tour (1-" + to_string(p->laySucChua()) + "): ", 1, p->laySucChua());
        cout << "(Tour da gom an uong tron goi nen khong chon them an sang / am thuc.)\n";
    }
    else
    {
        if (coSang) cout << "Phong da kem san " << (coBuffet ? "buffet sang" : "an sang") << ".\n";
        else an = nhapCoKhong("Them an sang (+" + dinhDangTien(GIA_SANG) + " VND/dem)?");
        suatTrua = nhapSoNguyen("So suat bua trua, " + dinhDangTien(GIA_TRUA) + " VND/suat (0-50): ", 0, 50);
        suatToi = nhapSoNguyen("So suat bua toi, " + dinhDangTien(GIA_TOI) + " VND/suat (0-50): ", 0, 50);
    }
    if (coMinibarSan) cout << "Phong da kem san minibar.\n";
    else mini = nhapCoKhong("Them minibar (+200,000 VND/dem)?");
    int lanSpa = nhapSoNguyen("So lan spa, " + dinhDangTien(Spa().layDonGia()) + " VND/lan (0-20): ", 0, 20);

    // Gia an uong tron goi cho 1 nguoi / 1 ngay (khong tinh sang neu phong da kem)
    double giaAnNgay = GIA_TRUA + GIA_TOI + (coSang ? 0 : GIA_SANG);

    // ---- Tinh tien tung muc bang Decorator ----
    vector<MucHoaDon> muc;
    shared_ptr<Phong> cur = p;
    double tong = cur->tinhGiaPhong(soDem, mua);

    // Tach tien phong thanh tung khoan de khach thay ro: gia goc, mua, giam o dai ngay, tien ich
    double heSo = p->layHeSoMua(mua);
    double giaGoc = p->layGiaPhong() * soDem;
    double sauMua = giaGoc * heSo;
    double sauGiam = (soDem >= 7) ? sauMua * 0.9 : sauMua;
    double tienIch = tong - sauGiam;
    muc.push_back(MucHoaDon{"Gia phong " + to_string(soDem) + " dem x " + dinhDangTien(p->layGiaPhong()), giaGoc, false});
    if (heSo != 1.0)
        muc.push_back(MucHoaDon{"Mua " + muaToString(mua) + " (" + ghiChuMua(heSo) + ")", sauMua - giaGoc, true});
    if (soDem >= 7)
        muc.push_back(MucHoaDon{"Giam o tu 7 dem (-10%)", sauGiam - sauMua, true});
    if (tienIch > 0.5)
        muc.push_back(MucHoaDon{"Tien ich phong x " + to_string(soDem) + " dem", tienIch, false});

    auto them = [&](const shared_ptr<Phong>& moi, const string& ten)
    {
        double t = moi->tinhGiaPhong(soDem, mua);
        muc.push_back(MucHoaDon{ten, t - tong, false});
        tong = t;
        cur = moi;
    };
    if (an)   them(make_shared<AnSangDecorator>(cur), "An sang x" + to_string(soDem) + " dem");
    if (mini) them(make_shared<MinibarDecorator>(cur), "Minibar x" + to_string(soDem) + " dem");
    if (suatTrua > 0)
        them(make_shared<DecoratorAmThuc>(cur, make_shared<AmThuc>("Bua trua", GIA_TRUA), suatTrua),
             "Bua trua x" + to_string(suatTrua) + " suat");
    if (suatToi > 0)
        them(make_shared<DecoratorAmThuc>(cur, make_shared<AmThuc>("Bua toi", GIA_TOI), suatToi),
             "Bua toi x" + to_string(suatToi) + " suat");
    if (lanSpa > 0)
        them(make_shared<DecoratorSpa>(cur, make_shared<Spa>(), lanSpa), "Spa x" + to_string(lanSpa) + " lan");
    if (laTour)
        them(make_shared<DecoratorTour>(cur, make_shared<TourTuyen>("Tour tron goi", giaAnNgay), nguoiTour),
             "An uong x" + to_string(nguoiTour) + " nguoi x" + to_string(soDem) + " dem");

    // ---- In hoa don ----
    string tieuDe = "HOA DON DAT PHONG";
    cout << "\n";
    inVienHD();
    inDongHD(string((56 - tieuDe.size()) / 2, ' ') + tieuDe);
    inVienHD();
    inDongHD("Phong      : " + p->layMaPhong() + " (" + loaiPhongToString(p->layLoaiPhong()) + ")");
    inDongHD(string("Hinh thuc  : ") + (laTour ? "DAT TOUR (tron goi phong + an uong)" : "DAT THUONG"));
    inDongHD("So dem     : " + to_string(soDem));
    inDongHD("Mua        : " + muaToString(mua) + " (" + ghiChuMua(heSo) + ")");
    if (laTour)
    {
        inDongHD("So nguoi   : " + to_string(nguoiTour));
        inDongHD("Gia an     : " + dinhDangTien(giaAnNgay) + " VND/nguoi/ngay ("
                 + (coSang ? "trua + toi, phong da kem sang" : "sang + trua + toi") + ")");
    }
    inVienHD();
    for (size_t i = 0; i < muc.size(); ++i) inTienHD(muc[i].ten, muc[i].tien, muc[i].coDau);
    inVienHD();
    inTienHD("TONG CONG", tong);
    if (laTour) inTienHD("Chia deu moi nguoi (" + to_string(nguoiTour) + " nguoi)", tong / nguoiTour);
    inVienHD();

    if (!nhapCoKhong("Xac nhan dat phong?")) { cout << "Da huy dat phong.\n"; return; }

    // ---- Giu cho / nhan phong ----
    cout << "1. Giu cho (nhan phong sau)   2. Nhan phong ngay\n";
    HanhDong hd = (nhapLuaChon(1, 2) == 1) ? HanhDong::GIU_CHO : HanhDong::NHAN_PHONG;
    qlPhong.chuyenTrangThai(p->layMaPhong(), hd);
    cout << "Dat phong thanh cong! Phong " << p->layMaPhong()
         << " - trang thai: " << p->layTenTrangThai() << "\n";
}

void ChuongTrinh::xuLyTraPhong()
{
    lapLai("Ban co muon tra phong khac?", [this]() -> bool
    {
        string ma = nhapSoDuongChuoi("Nhap ma phong (so duong): ");
        qlPhong.chuyenTrangThai(ma, HanhDong::TRA_PHONG);
        shared_ptr<Phong> p = qlPhong.timTheoMa(ma);
        cout << "Tra phong thanh cong! Phong " << p->layMaPhong()
             << " - trang thai: " << p->layTenTrangThai() << "\n";
        return true;
    });
}
