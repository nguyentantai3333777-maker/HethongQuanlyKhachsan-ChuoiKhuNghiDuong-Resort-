// Nguoi.cpp
#include "Nguoi.h"
#include "PhieuDatPhong.h"
#include "InputHelper.h"
#include "TienIch.h"
#include <stdexcept>

using namespace std;

const int KhachHang::TIEN_MOI_DIEM;

string hangToString(HangThanhVien h)
{
    switch (h)
    {
    case HangThanhVien::THANH_VIEN:
        return "Thanh vien";
    case HangThanhVien::BAC:
        return "Bac";
    case HangThanhVien::VANG:
        return "Vang";
    case HangThanhVien::KIM_CUONG:
        return "Kim cuong";
    }
    return "Thanh vien";
}

double tyLeGiamCuaHang(HangThanhVien h)
{
    static const double tl[] = {0.0, 0.05, 0.10, 0.15};
    return tl[(int)h];
}

int diemToiThieuCuaHang(HangThanhVien h)
{
    static const int d[] = {0, 100, 400, 1000};
    return d[(int)h];
}

// ============================ Nguoi ============================
Nguoi::Nguoi(const string &m, const string &ten, const string &sdt, const string &e, const string &gt) : ma(m)
{
    if (InputHelper::trim(m).empty())
        throw invalid_argument("Ma khong duoc rong.");
    datHoTen(ten);
    datSoDienThoai(sdt);
    datEmail(e);
    datGioiTinh(gt);
}

void Nguoi::datHoTen(const string &ten)
{
    if (!InputHelper::laChuThuan(ten) || InputHelper::trim(ten).size() < 2 || InputHelper::trim(ten).size() > 50)
        throw invalid_argument("Ho ten chi duoc chua chu cai va khoang trang (2-50 ky tu), khong duoc co so.");
    hoTen = InputHelper::chuanHoaHoTen(ten);
}

void Nguoi::datSoDienThoai(const string &sdt)
{
    string s = InputHelper::trim(sdt);
    if (!InputHelper::laSoDienThoai(s))
        throw invalid_argument("So dien thoai phai gom 10 chu so va bat dau bang 0.");
    soDienThoai = s;
}

void Nguoi::datEmail(const string &e)
{
    string s = InputHelper::trim(e);
    if (!s.empty() && (s.size() > 80 || !InputHelper::laEmail(s) || InputHelper::coKyTuDieuKhien(s)))
        throw invalid_argument("Email khong hop le.");
    email = s;
}

vector<pair<string, string>> Nguoi::layThongTin() const
{
    return {{"Ma", ma},
            {"Ho ten", hoTen},
            {"Vai tro", layVaiTro()},
            {"So dien thoai", soDienThoai},
            {"Email", email.empty() ? "Chua cap nhat" : email},
            {"Gioi tinh", gioiTinh.empty() ? "Chua cap nhat" : gioiTinh}};
}

void Nguoi::datGioiTinh(const string &gt)
{
    const string s = InputHelper::trim(gt);
    if (!s.empty() && s != "Nam" && s != "Nu" && s != "Nữ" && s != "Khac" && s != "Khác")
        throw invalid_argument("Gioi tinh phai la Nam, Nu hoac Khac.");
    gioiTinh = s;
}

// ============================ KhachHang ============================
KhachHang::KhachHang(const string &m, const string &ten, const string &sdt, const string &so,
                     const string &e, const string &qt, const string &diaChi, const string &gt,
                     const string &maCN)
    : Nguoi(m, ten, sdt, e, gt), diemTichLuy(0), hang(HangThanhVien::THANH_VIEN), maChiNhanh(maCN)
{
    datCCCD(so);
    datQuocTich(qt);
    datDiaChiThuongTru(diaChi);
}

void KhachHang::datCCCD(const string &so)
{
    string s = InputHelper::trim(so);
    if (!InputHelper::laCCCD(s))
        throw invalid_argument("Giay to phai la CMND/CCCD 9-12 chu so hoac ho chieu hop le.");
    cccd = InputHelper::chuHoa(s);
}

void KhachHang::datQuocTich(const string &qt)
{
    string s = InputHelper::trim(qt);
    if (s.empty())
        s = "Viet Nam";
    if (!InputHelper::laChuThuan(s))
        throw invalid_argument("Quoc tich chi duoc chua chu cai.");
    quocTich = InputHelper::chuanHoaHoTen(s);
}

void KhachHang::datDiaChiThuongTru(const string &diaChi)
{
    const string s = InputHelper::trim(diaChi);
    if (!s.empty() && !InputHelper::laDiaChi(s))
        throw invalid_argument("Dia chi thuong tru khong hop le.");
    diaChiThuongTru = s;
}

void KhachHang::datDiem(int diem) { diemTichLuy = diem < 0 ? 0 : diem; }

void KhachHang::congDiem(int diem)
{
    if (diem <= 0)
        return;
    diemTichLuy += diem;
    // Tu dong nang hang khi dat nguong diem
    for (int h = (int)HangThanhVien::KIM_CUONG; h > (int)hang; --h)
        if (diemTichLuy >= diemToiThieuCuaHang((HangThanhVien)h))
        {
            hang = (HangThanhVien)h;
            break;
        }
}

void KhachHang::themLichSu(const shared_ptr<PhieuDatPhong> &phieu)
{
    if (phieu)
        lichSuDatPhong.push_back(phieu);
}

vector<shared_ptr<PhieuDatPhong>> KhachHang::layLichSuDatPhong() const
{
    vector<shared_ptr<PhieuDatPhong>> kq;
    for (size_t i = 0; i < lichSuDatPhong.size(); ++i)
        if (auto p = lichSuDatPhong[i].lock())
            kq.push_back(p);
    return kq;
}

int KhachHang::soLanLuuTru() const
{
    int dem = 0;
    for (auto &p : layLichSuDatPhong())
        if (p->layTrangThai() == TrangThaiPhieu::DA_TRA_PHONG)
            ++dem;
    return dem;
}

vector<pair<string, string>> KhachHang::layThongTin() const
{
    auto ds = Nguoi::layThongTin();
    ds.push_back({"CCCD/Ho chieu", cccd});
    ds.push_back({"Quoc tich", quocTich});
    ds.push_back({"Dia chi thuong tru", diaChiThuongTru.empty() ? "Chua cap nhat" : diaChiThuongTru});
    string tenCN = maChiNhanh;
    if (maChiNhanh == "CN01") tenCN = "Nha Trang (CN01)";
    else if (maChiNhanh == "CN02") tenCN = "Quy Nhon (CN02)";
    else if (maChiNhanh.empty()) tenCN = "Chua gan chi nhanh";
    ds.push_back({"Chi nhanh", tenCN});
    ds.push_back({"Hang thanh vien", hangThanhVien() + " (giam " +
                                         to_string((int)(tyLeGiamGia() * 100)) + "%)"});
    ds.push_back({"Diem tich luy", to_string(diemTichLuy)});
    ds.push_back({"So lan dat phong", to_string(layLichSuDatPhong().size())});
    ds.push_back({"So lan luu tru", to_string(soLanLuuTru())});
    return ds;
}

// ============================ NhanVien ============================
NhanVien::NhanVien(const string &m, const string &ten, const string &sdt, const string &cv,
                   double luong, const string &cn, const string &e, const string &soCCCD,
                   const string &gt, const string &ns)
    : Nguoi(m, ten, sdt, e, gt), luongCoBan(0), maChiNhanh(cn), dangLam(true), tamNghi(false), lyDoNghi("")
{
    datChucVu(cv);
    datLuongCoBan(luong);
    datCCCD(soCCCD);
    datNgaySinh(ns);
}

void NhanVien::datChucVu(const string &cv)
{
    string s = InputHelper::trim(cv);
    if (!InputHelper::laChuThuan(s) || s.size() > 30)
        throw invalid_argument("Chuc vu chi duoc chua chu cai va khoang trang (toi da 30 ky tu).");
    chucVu = s;
}

void NhanVien::datLuongCoBan(double l)
{
    if (!(l >= LUONG_CO_BAN_TOI_THIEU) || l > 1000000000.0)
        throw invalid_argument("Luong co ban phai tu 5.500.000 den 1.000.000.000 VND.");
    luongCoBan = l;
}

void NhanVien::datCCCD(const string &so)
{
    const string s = InputHelper::trim(so);
    if (!s.empty() && !((s.size() == 9 || s.size() == 12) && InputHelper::laSoThuan(s, s.size(), s.size())))
        throw invalid_argument("CCCD/CMND nhan vien khong hop le.");
    cccd = InputHelper::chuHoa(s);
}

void NhanVien::datNgaySinh(const string &ngay)
{
    const string s = InputHelper::trim(ngay);
    if (!s.empty())
    {
        Ngay parsed;
        if (!Ngay::docChuoi(s, parsed))
            throw invalid_argument("Ngay sinh phai theo dinh dang dd/mm/yyyy.");
    }
    ngaySinh = s;
}

double NhanVien::heSoLuong() const
{
    return chucVu.find("Quan ly") != string::npos || chucVu.find("Quản lý") != string::npos ||
                   chucVu.find("QUAN LY") != string::npos || chucVu.find("QUẢN LÝ") != string::npos ||
                   chucVu.find("quản lý") != string::npos
               ? 3.0
               : 2.0;
}

vector<pair<string, string>> NhanVien::layThongTin() const
{
    auto ds = Nguoi::layThongTin();
    ds.push_back({"Chuc vu", chucVu});
    ds.push_back({"Chi nhanh", maChiNhanh});
    ds.push_back({"Luong co ban", dinhDangTien(luongCoBan) + " VND"});
    ds.push_back({"He so luong", to_string(heSoLuong())});
    ds.push_back({"Luong thuc nhan", dinhDangTien(luongThucNhan()) + " VND"});
    ds.push_back({"CCCD/CMND", cccd.empty() ? "Chua cap nhat" : cccd});
    ds.push_back({"Ngay sinh", ngaySinh.empty() ? "Chua cap nhat" : ngaySinh});
    ds.push_back({"Tinh trang", !dangLam ? "Da nghi viec" : tamNghi ? "Tam nghi" : "Dang lam viec"});
    if (!dangLam && !lyDoNghi.empty())
        ds.push_back({"Ly do nghi", lyDoNghi});
    return ds;
}
