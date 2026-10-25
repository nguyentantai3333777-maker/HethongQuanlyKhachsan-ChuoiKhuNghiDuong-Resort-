// ChuongTrinh.cpp
#include "ChuongTrinh.h"
#include "InputHelper.h"
#include "PhongCuThe.h"
#include "PhongDecorator.h"
#include <iostream>

using namespace std;
using namespace InputHelper;

ChuongTrinh::ChuongTrinh() : roleHienTai(Role::USER)
{
    khoiTaoDuLieuMau();
}

void ChuongTrinh::khoiTaoDuLieuMau()
{
    qlTaiKhoan.themTaiKhoan("admin", "admin123", Role::ADMIN);
    qlTaiKhoan.themTaiKhoan("user", "user123", Role::USER);

    qlPhong.themPhong(make_shared<StandardRoom>("1001", "101", 1, 500000, 2, 20, "Bien"));
    qlPhong.themPhong(make_shared<DeluxeRoom>("2001", "201", 2, 900000, 3, 32, true, "Bien"));
}

void ChuongTrinh::yeuCauAdmin() const
{
    if (roleHienTai != Role::ADMIN)
        throw runtime_error("Chuc nang danh rieng cho ADMIN.");
}

bool ChuongTrinh::dangNhap()
{
    cout << "=== DANG NHAP HE THONG RESORT ===\n";
    string user = nhapDong("Username: ");
    string pass = nhapDong("Password: ");
    try
    {
        const TaiKhoan &tk = qlTaiKhoan.dangNhap(user, pass);
        userHienTai = tk.layUsername();
        roleHienTai = tk.layRole();
        cout << "Xin chao " << userHienTai << " (" << roleToString(roleHienTai) << ")!\n";
        return true;
    }
    catch (const exception &e)
    {
        cout << e.what() << "\n";
        return false;
    }
}

void ChuongTrinh::chay()
{
    if (dangNhap())
    {
        if (roleHienTai == Role::ADMIN)
            menuAdmin();
        else
            menuUser();
    }
}

void ChuongTrinh::menuAdmin()
{
    while (true)
    {
        cout << "\n================ MENU ADMIN ================\n";
        cout << "1. Xem danh sach tat ca phong\n";
        cout << "2. Dat phong cho khach hang\n";
        cout << "3. Tra phong & Xuat hoa don thanh toan\n";
        cout << "4. Xem bao cao doanh thu\n";
        cout << "5. Luu nhat ky/Giao dich ra File\n";
        cout << "0. Dang xuat / Thoat\n";
        cout << "============================================\n";

        int lc = nhapLuaChon(0, 5);
        if (lc == 0)
            break;

        try
        {
            switch (lc)
            {
            case 1:
                qlPhong.hienThiTatCa();
                break;
            case 2:
                xuLyDatPhong();
                break;
            case 3:
                xuLyTraPhongVaThanhToan();
                break;
            case 4:
                xuLyXemBaoCaoDoanhThu();
                break;
            case 5:
                xuLyLuuLichSuFile();
                break;
            }
        }
        catch (const exception &e)
        {
            cout << "[LOI] " << e.what() << "\n";
        }
    }
}

void ChuongTrinh::menuUser()
{
    while (true)
    {
        cout << "\n================ MENU USER ================\n";
        cout << "1. Xem danh sach phong\n";
        cout << "2. Dat phong cho khach\n";
        cout << "0. Dang xuat / Thoat\n";
        cout << "===========================================\n";

        int lc = nhapLuaChon(0, 2);
        if (lc == 0)
            break;

        try
        {
            switch (lc)
            {
            case 1:
                qlPhong.hienThiTatCa();
                break;
            case 2:
                xuLyDatPhong();
                break;
            }
        }
        catch (const exception &e)
        {
            cout << "[LOI] " << e.what() << "\n";
        }
    }
}

// ===== XỬ LÝ CÁC PHÂN HỆ MỚI =====

void ChuongTrinh::xuLyDatPhong()
{
    cout << "\n--- TAO PHIEU DAT PHONG ---\n";
    string maP = nhapChuoi("Nhap ma phieu dat: ");
    string tenKH = nhapChuoi("Ten khach hang: ");
    string cccd = nhapChuoi("CCCD/CMND: ");
    string sdt = nhapChuoi("So dien thoai: ");
    KhachHang kh("KH_" + cccd, tenKH, cccd, sdt);

    string maPhong = nhapSoDuongChuoi("Nhap ma phong muon dat: ");
    auto phong = qlPhong.timTheoMa(maPhong);

    if (phong->layTenTrangThai() != "Trong")
    {
        cout << "Phong hien khong o trang thai trong đe dat!\n";
        return;
    }

    int soDem = nhapSoNguyen("Nhap so dem luu tru: ", 1, 365);
    MuaDuLich mua = nhapMuaDuLich();
    double tienCoc = nhapSoThucDuong("Nhap tien dat coc (VND): ", 100000000);

    PhieuDatPhong pd(maP, kh, maPhong, "2026-10-22", "2026-10-25", soDem, mua, tienCoc);
    danhSachPhieuDat.push_back(pd);

    // Chuyển trạng thái phòng sang Đang ở / Nhận phòng
    qlPhong.chuyenTrangThai(maPhong, HanhDong::NHAN_PHONG);
    cout << "=> Dat phong thanh cong cho khach hang " << tenKH << "!\n";
}

void ChuongTrinh::xuLyTraPhongVaThanhToan()
{
    cout << "\n--- TRA PHONG & THANH TOAN HOA DON ---\n";
    string maPhong = nhapSoDuongChuoi("Nhap ma phong can tra: ");
    auto phong = qlPhong.timTheoMa(maPhong);

    // Tìm phiếu đặt phòng tương ứng
    int idx = -1;
    for (size_t i = 0; i < danhSachPhieuDat.size(); ++i)
    {
        if (danhSachPhieuDat[i].layMaPhong() == phong->layMaPhong() &&
            danhSachPhieuDat[i].layTrangThai() == TrangThaiDatPhong::DA_DAT)
        {
            idx = (int)i;
            break;
        }
    }

    if (idx == -1)
    {
        cout << "Khong tim thay phieu dat phong hop le cho phong nay!\n";
        return;
    }

    PhieuDatPhong pd = danhSachPhieuDat[idx];
    double tienPhong = phong->tinhGiaPhong(pd.laySoDem(), pd.layMua());

    // Nhập thêm dịch vụ sử dụng
    bool themAnSang = nhapCoKhong("Khach co dung An sang khong?");
    bool themMinibar = nhapCoKhong("Khach co dung Minibar khong?");
    int soSuatAT = nhapSoNguyen("So suat am thuc goi them (0 neu khong): ", 0, 50);

    double tienDV = 0;
    if (themAnSang)
        tienDV += 150000.0 * pd.laySoDem();
    if (themMinibar)
        tienDV += 200000.0 * pd.laySoDem();
    if (soSuatAT > 0)
        tienDV += AmThuc().tinhChiPhi(soSuatAT);

    // Tạo hóa đơn
    HoaDon hd("HD_" + maPhong, pd, tienPhong, tienDV);
    hd.inHoaDon();

    // Thêm vào hệ thống báo cáo
    qlBaoCao.themHoaDon(hd);
    danhSachPhieuDat[idx].datTrangThai(TrangThaiDatPhong::DA_TRA_PHONG);

    // Chuyển trạng thái phòng về Đang dọn dẹp
    qlPhong.chuyenTrangThai(maPhong, HanhDong::TRA_PHONG);
}

void ChuongTrinh::xuLyXemBaoCaoDoanhThu()
{
    yeuCauAdmin();
    qlBaoCao.inBaoCaoDoanhThu();
}

void ChuongTrinh::xuLyLuuLichSuFile()
{
    yeuCauAdmin();
    bool ok = QuanLyLuuTruFile::ghiLogHoaDon("LichSuGiaoDich.txt", "Da cap nhat va dong bo giao dich luc 2026-10-22");
    if (ok)
    {
        cout << "=> Luu lich su giao dich ra file 'LichSuGiaoDich.txt' thanh cong!\n";
    }
    else
    {
        cout << "=> Loi khi ghi file!\n";
    }
}