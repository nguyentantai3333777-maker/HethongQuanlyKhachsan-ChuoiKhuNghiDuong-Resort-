// QuanLyKhachSan.cpp
#include "QuanLyKhachSan.h"
#include "InputHelper.h"
#include "PhongCuThe.h"
#include "PhongDecorator.h"
#include "TrangThaiCuThe.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>
#include <stdexcept>

using namespace std;
using InputHelper::chuanHoaMa;

QuanLyKhachSan::QuanLyKhachSan()
    : demChiNhanh(0), demPhong(0), demKhachHang(0), demNhanVien(0), demPhieu(0), demHoaDon(0), demDichVu(0) {}

string QuanLyKhachSan::taoMa(const string &tienTo, int so)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%02d", so);
    return tienTo + buf;
}

// =====================================================================
//                              CHI NHANH
// =====================================================================
shared_ptr<ChiNhanhResort> QuanLyKhachSan::themChiNhanh(const string &ten, const string &dc, const string &hl)
{
    for (auto &cn : danhSachChiNhanh)
        if (InputHelper::chuHoa(cn->layTen()) == InputHelper::chuHoa(InputHelper::trim(ten)))
            throw invalid_argument("Ten chi nhanh da ton tai.");
    auto cn = make_shared<ChiNhanhResort>(taoMa("CN", demChiNhanh + 1), ten, dc, hl);
    ++demChiNhanh;
    danhSachChiNhanh.push_back(cn);
    return cn;
}

void QuanLyKhachSan::damBaoNhanSuMacDinh()
{
    static const vector<string> viTri = {"Ky thuat", "Phuc vu", "An ninh", "Buong phong", "Bep"};
    // Ten Viet Nam tu nhien, da dang — khong con ghep chuc vu + so thu tu
    static const vector<string> tenMau = {
        "Nguyen Minh Tuan", "Tran Thi Ngoc Anh", "Le Hoang Nam", "Pham Thi Thu Ha",
        "Hoang Van Khoa", "Vu Thi Mai", "Doan Thanh Phong", "Bui Thi Lan",
        "Ngo Quang Huy", "Dang Thi Huong", "Phan Van Duc", "Ly Thi Kim Anh",
        "Truong Minh Hai", "Mai Thi Hong", "Lam Van Khoa", "Vo Thi Thanh",
        "Huynh Quoc Bao", "Cao Thi Nga", "Dinh Van Son", "Nguyen Thi My Linh",
        "Tran Quoc Viet", "Le Thi Phuong", "Pham Minh Chau", "Hoang Thi Dieu",
        "Vu Van Tai", "Do Thi Yen", "Bui Quang Minh", "Ngo Thi Cam",
        "Dang Van Long", "Phan Thi Thu", "Ly Minh Dat", "Truong Thi Hoa"};
    long long soDienThoai = 998000000;
    size_t chiSoTen = 0;
    auto laNhanVienKenhOnline = [](const NhanVien &nv)
    {
        return nv.layChucVu() == "Kênh Đặt Phòng Online" || nv.layChucVu() == "Kenh Dat Phong Online";
    };
    auto taoNhanVien = [&](const shared_ptr<ChiNhanhResort> &cn, const string &ten, const string &chucVu, double luong)
    {
        string sdt;
        const auto nhanVienHienTai = tatCaNhanVien(true);
        do
        {
            sdt = "0" + to_string(soDienThoai++);
        } while (any_of(nhanVienHienTai.begin(), nhanVienHienTai.end(),
                        [&](const shared_ptr<NhanVien> &nv) { return nv->laySoDienThoai() == sdt; }));
        return themNhanVien(cn->layMa(), ten, sdt, chucVu, luong);
    };

    for (const auto &cn : danhSachChiNhanh)
    {
        int quanLy = 0, leTan = 0, khac = 0;
        for (const auto &nv : cn->layDanhSachNhanVien())
        {
            if (!nv->layDangLam() || laNhanVienKenhOnline(*nv))
                continue;
            const string &cv = nv->layChucVu();
            if (cv.find("Quan ly") != string::npos || cv.find("Quản lý") != string::npos)
                ++quanLy;
            else if (cv.find("Le tan") != string::npos || cv.find("Lễ tân") != string::npos)
                ++leTan;
            else
                ++khac;
        }
        if (quanLy == 0)
        {
            const string &ten = tenMau[chiSoTen % tenMau.size()];
            ++chiSoTen;
            taoNhanVien(cn, ten, "Quan ly", 15000000);
            ++quanLy;
        }
        while (leTan < 2)
        {
            const string &ten = tenMau[chiSoTen % tenMau.size()];
            ++chiSoTen;
            taoNhanVien(cn, ten, "Le tan", 8000000);
            ++leTan;
        }
        while (khac < 17)
        {
            const size_t index = (size_t)khac;
            const string &viTriNV = viTri[index % viTri.size()];
            const string &ten = tenMau[chiSoTen % tenMau.size()];
            ++chiSoTen;
            taoNhanVien(cn, ten, viTriNV, 5500000);
            ++khac;
        }
    }
}

shared_ptr<ChiNhanhResort> QuanLyKhachSan::timChiNhanh(const string &ma) const
{
    string m = chuanHoaMa(ma);
    for (auto &cn : danhSachChiNhanh)
        if (cn->layMa() == m)
            return cn;
    return nullptr;
}

shared_ptr<ChiNhanhResort> QuanLyKhachSan::chiNhanhCuaPhong(const string &maPhong) const
{
    string m = chuanHoaMa(maPhong);
    for (auto &cn : danhSachChiNhanh)
        if (cn->timPhong(m))
            return cn;
    return nullptr;
}

string QuanLyKhachSan::maChiNhanhCuaPhong(const shared_ptr<Phong> &p) const
{
    if (!p)
        return "";
    for (auto &cn : danhSachChiNhanh)
        for (auto &x : cn->layDanhSachPhong())
            if (x == p)
                return cn->layMa();
    for (auto &x : danhSachPhongDaXoa) // phong da xoa nhung con lich su
        if (x.second == p)
            return x.first;
    return "";
}

vector<string> QuanLyKhachSan::lyDoKhongXoaDuocChiNhanh(const string &ma) const
{
    vector<string> lyDo;
    auto cn = timChiNhanh(ma);
    if (!cn)
    {
        lyDo.push_back("Khong tim thay chi nhanh " + chuanHoaMa(ma) + ".");
        return lyDo;
    }
    // 1. Phong dang hoat dong (va cac phieu dat / khach dang o trong cac phong do)
    size_t soPhong = cn->layDanhSachPhong().size();
    if (soPhong > 0)
    {
        int phieuHoatDong = 0;
        for (auto &ph : danhSachPhieu)
            if (ph->dangHoatDong() && maChiNhanhCuaPhong(ph->layPhong()) == cn->layMa())
                ++phieuHoatDong;
        lyDo.push_back("Chi nhanh con " + to_string(soPhong) + " phong" +
                       (phieuHoatDong ? " (trong do co " + to_string(phieuHoatDong) + " phieu dat / khach dang o)" : "") +
                       ". Hay xoa hoac xu ly het phong truoc.");
    }
    // 2. Lich su phieu / hoa don (phong da xoa van con duoc phieu cu tham chieu)
    int phongDaXoa = 0;
    for (auto &x : danhSachPhongDaXoa)
        if (x.first == cn->layMa())
            ++phongDaXoa;
    int soHoaDon = (int)hoaDonCuaChiNhanh(cn->layMa()).size();
    if (phongDaXoa > 0 || (soPhong == 0 && soHoaDon > 0))
        lyDo.push_back("Chi nhanh con lich su luu tru (" + to_string(soHoaDon) + " hoa don, " + to_string(phongDaXoa) +
                       " phong da xoa). Xoa chi nhanh se lam hong phieu / hoa don cu.");
    // 3. Nhan vien (ke ca nguoi da nghi viec: ten van duoc phieu / hoa don cu tham chieu)
    int dangLam = 0, daNghi = 0;
    for (auto &nv : cn->layDanhSachNhanVien())
        (nv->layDangLam() ? dangLam : daNghi)++;
    if (dangLam + daNghi > 0)
        lyDo.push_back("Chi nhanh con " + to_string(dangLam) + " nhan vien dang lam" +
                       (daNghi ? " va " + to_string(daNghi) + " nhan vien da nghi (con trong lich su phieu / hoa don)" : "") +
                       ". Hay chuyen / cho nghi viec va dam bao khong con lich su truoc.");
    return lyDo;
}

void QuanLyKhachSan::xoaChiNhanh(const string &ma)
{
    auto ly = lyDoKhongXoaDuocChiNhanh(ma);
    if (!ly.empty())
    {
        string s = "Khong the xoa chi nhanh " + chuanHoaMa(ma) + ":";
        for (auto &x : ly)
            s += "\n - " + x;
        throw runtime_error(s);
    }
    string m = chuanHoaMa(ma);
    for (size_t i = 0; i < danhSachChiNhanh.size(); ++i)
        if (danhSachChiNhanh[i]->layMa() == m)
        {
            danhSachChiNhanh.erase(danhSachChiNhanh.begin() + (long)i);
            return; // demChiNhanh giu nguyen -> ma chi nhanh khong bi dung lai
        }
}

vector<shared_ptr<PhieuDatPhong>> QuanLyKhachSan::phieuCuaChiNhanh(const string &maCN) const
{
    if (maCN.empty())
        return danhSachPhieu;
    string m = chuanHoaMa(maCN);
    vector<shared_ptr<PhieuDatPhong>> kq;
    for (auto &ph : danhSachPhieu)
        if (maChiNhanhCuaPhong(ph->layPhong()) == m)
            kq.push_back(ph);
    return kq;
}

bool QuanLyKhachSan::hoaDonThuocChiNhanh(const HoaDon &hd, const string &maCN) const
{
    return maCN.empty() || maChiNhanhCuaPhong(hd.layPhieu()->layPhong()) == chuanHoaMa(maCN);
}

vector<shared_ptr<HoaDon>> QuanLyKhachSan::hoaDonCuaChiNhanh(const string &maCN) const
{
    vector<shared_ptr<HoaDon>> kq;
    for (auto &hd : danhSachHoaDon)
        if (hoaDonThuocChiNhanh(*hd, maCN))
            kq.push_back(hd);
    return kq;
}

// =====================================================================
//                                PHONG
// =====================================================================
string QuanLyKhachSan::taoMaPhong() const { return taoMa("P", demPhong + 1); }

void QuanLyKhachSan::themPhong(const string &maCN, const shared_ptr<Phong> &p)
{
    if (!p)
        throw invalid_argument("Phong null.");
    auto cn = timChiNhanh(maCN);
    if (!cn)
        throw runtime_error("Khong tim thay chi nhanh " + maCN + ".");
    if (timPhong(p->layMaPhong())) // ma phong duy nhat tren toan he thong
        throw invalid_argument("Ma phong " + p->layMaPhong() + " da ton tai.");
    cn->themPhong(p);
    demPhong = max(demPhong, atoi(p->layMaPhong().c_str() + 1));
}

shared_ptr<Phong> QuanLyKhachSan::timPhong(const string &maPhong) const
{
    string m = chuanHoaMa(maPhong);
    for (auto &cn : danhSachChiNhanh)
        if (auto p = cn->timPhong(m))
            return p;
    return nullptr;
}

vector<shared_ptr<Phong>> QuanLyKhachSan::tatCaPhong() const
{
    vector<shared_ptr<Phong>> kq;
    for (auto &cn : danhSachChiNhanh)
        for (auto &p : cn->layDanhSachPhong())
            kq.push_back(p);
    return kq;
}

vector<shared_ptr<Phong>> QuanLyKhachSan::locPhong(const function<bool(const Phong &)> &dk) const
{
    vector<shared_ptr<Phong>> kq;
    for (auto &p : tatCaPhong())
        if (dk(*p))
            kq.push_back(p);
    return kq;
}

void QuanLyKhachSan::doiSoPhong(const string &maPhong, const string &soMoi)
{
    auto p = timPhong(maPhong);
    if (!p)
        throw runtime_error("Khong tim thay phong.");
    auto cn = chiNhanhCuaPhong(maPhong);
    string so = InputHelper::trim(soMoi);
    if (InputHelper::laSoDuong(so))
        so = InputHelper::chuanHoaSoDuong(so);
    if (cn->coSoPhong(so, p->layMaPhong()))
        throw invalid_argument("So phong " + so + " da ton tai trong chi nhanh.");
    p->datSoPhong(so);
}

void QuanLyKhachSan::kiemTraCoTheDoiGia(const string &maPhong) const
{
    auto ds = lichDatCuaPhong(maPhong);
    if (!ds.empty())
        throw runtime_error("Phong con " + to_string(ds.size()) +
                            " phieu dat/dang o. Chi doi gia, suc chua, phu phi khi phong khong con phieu "
                            "(de giu dung gia da bao cho khach).");
}

// =====================================================================
//                     LICH DAT PHONG - CHONG TRUNG NGAY
// =====================================================================
vector<shared_ptr<PhieuDatPhong>> QuanLyKhachSan::lichDatCuaPhong(const string &maPhong) const
{
    string m = chuanHoaMa(maPhong);
    vector<shared_ptr<PhieuDatPhong>> kq;
    for (auto &ph : danhSachPhieu)
        if (ph->dangHoatDong() && ph->layPhong()->layMaPhong() == m)
            kq.push_back(ph);
    sort(kq.begin(), kq.end(), [](const shared_ptr<PhieuDatPhong> &a, const shared_ptr<PhieuDatPhong> &b)
         { return a->layNgayNhan() < b->layNgayNhan(); });
    return kq;
}

shared_ptr<PhieuDatPhong> QuanLyKhachSan::timPhieuTrungLich(const string &maPhong, const Ngay &ngayNhan,
                                                            int soDem) const
{
    for (auto &ph : lichDatCuaPhong(maPhong))
        if (ph->trungLich(ngayNhan, soDem))
            return ph;
    return nullptr;
}

shared_ptr<PhieuDatPhong> QuanLyKhachSan::phieuHienTaiCuaPhong(const string &maPhong) const
{
    Ngay hn = Ngay::homNay();
    shared_ptr<PhieuDatPhong> homNay;
    for (auto &ph : lichDatCuaPhong(maPhong))
    {
        if (ph->layTrangThai() == TrangThaiPhieu::DA_NHAN_PHONG)
            return ph;
        if (ph->layNgayNhan() == hn)
            homNay = ph;
    }
    return homNay;
}

shared_ptr<PhieuDatPhong> QuanLyKhachSan::phieuSapToiCuaPhong(const string &maPhong) const
{
    Ngay hn = Ngay::homNay();
    for (auto &ph : lichDatCuaPhong(maPhong)) // da sap xep theo ngay nhan
        if (ph->layTrangThai() == TrangThaiPhieu::DA_DAT && hn < ph->layNgayNhan())
            return ph;
    return nullptr;
}

string QuanLyKhachSan::lyDoKhongDatDuoc(const Phong &p, const Ngay &ngayNhan, int soDem, int soNguoi) const
{
    if (soNguoi > p.laySucChua())
        return "Toi da " + to_string(p.laySucChua()) + " khach";
    // Bao tri: khong cho dat bat ky khoang ngay nao (khong chi hom nay)
    if (p.layMaTrangThai() == MaTrangThai::BAO_TRI)
        return "Dang bao tri";
    // Khach DANG O (da check-in, chua check-out): phong bi chiem vo thoi han, ke ca khi da qua ngay tra du kien
    for (auto &ph : lichDatCuaPhong(p.layMaPhong()))
        if (ph->chiemVoThoiHan() && ph->trungLich(ngayNhan, soDem))
        {
            string s = "Dang co khach luu tru (" + ph->layMaPhieu() + ", chua check-out";
            if (ph->dangOverstay())
                s += ", qua han " + to_string(ph->soNgayOverstay()) + " ngay";
            else
                s += ", du kien tra " + ph->layNgayTra().toString().substr(0, 5);
            return s + ")";
        }
    // Dang don: chua san sang nhan khach trong ngay
    if (ngayNhan == Ngay::homNay() && p.layMaTrangThai() == MaTrangThai::DANG_DON)
        return "Dang don dep";
    // Dang o / giu cho: khong cho dat them cung ngay (lich chi tiet van do trungLich xu ly)
    if (ngayNhan == Ngay::homNay() &&
        (p.layMaTrangThai() == MaTrangThai::DANG_O || p.layMaTrangThai() == MaTrangThai::GIU_CHO))
        return "Phong dang co khach / dang giu cho";
    if (auto ph = timPhieuTrungLich(p.layMaPhong(), ngayNhan, soDem))
    {
        // Khong lo ten khach (privacy); admin van thay ma phieu + khoang ngay
        string nn = ph->layNgayNhan().toString(), nt = ph->layNgayTra().toString();
        return "Trung " + ph->layMaPhieu() + " (" + nn.substr(0, 5) + "-" + nt.substr(0, 5) + ")";
    }
    return "";
}

void QuanLyKhachSan::chuyenBaoTri(const string &maPhong, const string &lyDo)
{
    auto p = timPhong(maPhong);
    if (!p)
        throw runtime_error("Khong tim thay phong " + chuanHoaMa(maPhong) + ".");
    string ly = InputHelper::trim(lyDo);
    if (ly.empty())
        throw invalid_argument("Can nhap ly do bao tri.");
    if (ly.size() > 100 || InputHelper::coKyTuDieuKhien(ly))
        throw invalid_argument("Ly do bao tri toi da 100 ky tu va khong chua ky tu dac biet.");
    dongBoTheoNgay(); // phieu qua han khong den da duoc tu dong huy -> khong bi tinh la xung dot
    auto ds = lichDatCuaPhong(p->layMaPhong()); // chi gom phieu DA_DAT / DA_NHAN_PHONG
    if (!ds.empty())
    {
        string chiTiet;
        for (auto &ph : ds)
            chiTiet += "\n - " + ph->layMaPhieu() + ": " + ph->layNgayNhan().toString() + " -> " +
                       ph->layNgayTra().toString() + " (" + trangThaiPhieuToString(ph->layTrangThai()) + ")";
        throw runtime_error("Khong the chuyen phong " + p->layMaPhong() + " sang bao tri: phong con " +
                            to_string(ds.size()) + " phieu dat/dang o:" + chiTiet +
                            "\nHay huy hoac chuyen cac phieu nay sang phong khac (hoac cho khach tra phong) truoc.");
    }
    p->baoTri(ly);
}

double QuanLyKhachSan::tranCocToiDa(const Phong &p, int soDem, MuaDuLich mua, double tyLeGiam) const
{
    return floor(HoaDon::tinhTien(p.tinhGiaPhong(soDem, mua), tyLeGiam).tong);
}

vector<shared_ptr<PhieuDatPhong>> QuanLyKhachSan::phieuQuaHan() const
{
    vector<shared_ptr<PhieuDatPhong>> kq;
    for (auto &ph : danhSachPhieu)
        if (ph->dangOverstay())
            kq.push_back(ph);
    return kq;
}

shared_ptr<PhieuDatPhong> QuanLyKhachSan::phieuDangOCuaPhong(const string &maPhong) const
{
    for (auto &ph : lichDatCuaPhong(maPhong))
        if (ph->layTrangThai() == TrangThaiPhieu::DA_NHAN_PHONG)
            return ph;
    return nullptr;
}

string QuanLyKhachSan::thongBaoPhongDangCoKhach(const Phong &p, const PhieuDatPhong &dangO) const
{
    // Quy tac: khach da check-in ma chua check-out -> phong bi chiem VO THOI HAN,
    // khong dua vao ngay tra du kien de xac dinh phong trong.
    string s = "Khong the dat phong " + p.laySoPhong() + " (" + p.layMaPhong() + ").\n" +
               "Phong dang co khach " + dangO.layKhachHang()->layHoTen() + " (phieu " + dangO.layMaPhieu() +
               ") chua check-out.\n" + "Ngay tra du kien: " + dangO.layNgayTra().toString();
    if (dangO.dangOverstay())
        s += " (da qua han " + to_string(dangO.soNgayOverstay()) + " ngay)";
    return s + ".\nPhong chi duoc giai phong sau khi khach thuc hien check-out.";
}

vector<shared_ptr<PhieuDatPhong>> QuanLyKhachSan::phieuBiChanBoiKhachDangO() const
{
    Ngay hn = Ngay::homNay();
    vector<shared_ptr<PhieuDatPhong>> kq;
    for (auto &ph : danhSachPhieu)
        if (ph->layTrangThai() == TrangThaiPhieu::DA_DAT && ph->layNgayNhan() <= hn)
            if (auto o = phieuDangOCuaPhong(ph->layPhong()->layMaPhong()); o && o != ph)
                kq.push_back(ph);
    return kq;
}

// =====================================================================
//                    CHUYEN PHIEU SANG PHONG KHAC
// =====================================================================
string QuanLyKhachSan::kiemTraChuyenPhong(const string &maPhieu) const
{
    auto ph = timPhieu(maPhieu);
    if (!ph)
        return "Khong tim thay phieu " + chuanHoaMa(maPhieu) + ".";
    if (ph->layTrangThai() != TrangThaiPhieu::DA_DAT)
        return "Chi chuyen phong duoc phieu 'Cho nhan phong'. Phieu " + ph->layMaPhieu() + " dang '" +
               trangThaiPhieuToString(ph->layTrangThai()) + "'.";
    if (ph->layNgayNhan() < Ngay::homNay())
        return "Phieu " + ph->layMaPhieu() + " da qua ngay nhan phong (" + ph->layNgayNhan().toString() +
               "). Hay huy phieu (co the hoan coc) va dat lai cho khach.";
    return "";
}

vector<shared_ptr<Phong>> QuanLyKhachSan::phongCoTheChuyen(const string &maPhieu, bool cungLoai) const
{
    vector<shared_ptr<Phong>> kq;
    if (!kiemTraChuyenPhong(maPhieu).empty())
        return kq;
    auto ph = timPhieu(maPhieu);
    auto cu = ph->layPhong();
    auto cn = chiNhanhCuaPhong(cu->layMaPhong());
    if (!cn)
        return kq;
    for (auto &p : cn->layDanhSachPhong())
    {
        if (p == cu || (cungLoai && p->layLoaiPhong() != cu->layLoaiPhong()))
            continue;
        // lyDoKhongDatDuoc: suc chua, bao tri, khach dang o (vo thoi han), dang don / giu cho hom nay, trung phieu khac
        if (lyDoKhongDatDuoc(*p, ph->layNgayNhan(), ph->laySoDem(), ph->laySoNguoi()).empty())
            kq.push_back(p);
    }
    // Phong cung loai, gia gan nhat len truoc
    double giaCu = cu->layGiaPhong();
    stable_sort(kq.begin(), kq.end(), [&](const shared_ptr<Phong> &a, const shared_ptr<Phong> &b)
                {
                    bool la = a->layLoaiPhong() == cu->layLoaiPhong(), lb = b->layLoaiPhong() == cu->layLoaiPhong();
                    if (la != lb)
                        return la;
                    return fabs(a->layGiaPhong() - giaCu) < fabs(b->layGiaPhong() - giaCu); });
    return kq;
}

shared_ptr<Phong> QuanLyKhachSan::chuyenPhongPhieu(const string &maPhieu, const string &maPhongMoi)
{
    string ly = kiemTraChuyenPhong(maPhieu);
    if (!ly.empty())
        throw runtime_error(ly);
    auto ph = layPhieuBatBuoc(maPhieu);
    auto cu = ph->layPhong();
    auto moi = timPhong(maPhongMoi);
    if (!moi)
        throw runtime_error("Khong tim thay phong " + chuanHoaMa(maPhongMoi) + ".");
    if (moi == cu)
        throw invalid_argument("Phong moi trung voi phong hien tai cua phieu.");
    if (chiNhanhCuaPhong(moi->layMaPhong()) != chiNhanhCuaPhong(cu->layMaPhong()))
        throw invalid_argument("Chi chuyen sang phong cung chi nhanh voi phieu.");
    string lyDo = lyDoKhongDatDuoc(*moi, ph->layNgayNhan(), ph->laySoDem(), ph->laySoNguoi());
    if (!lyDo.empty())
        throw runtime_error("Khong chuyen duoc sang phong " + moi->laySoPhong() + " (" + moi->layMaPhong() + "): " +
                            lyDo + ".");
    Ngay hn = Ngay::homNay();
    // State Pattern: phieu nhan phong hom nay dang giu cho phong cu -> tra lai phong cu, giu cho phong moi
    if (ph->layNgayNhan() == hn && cu->layMaTrangThai() == MaTrangThai::GIU_CHO)
        cu->huyGiuCho();
    ph->doiPhong(moi); // giu nguyen ma phieu, khach, ngay nhan/tra, coc, dich vu, trang thai
    if (ph->layNgayNhan() == hn && moi->layMaTrangThai() == MaTrangThai::TRONG)
        moi->giuCho(hn.toTimeT(23, 59, 59));
    dongBoTheoNgay(); // phong cu co the can giu cho cho phieu khac den hom nay
    return cu;
}

// =====================================================================
//                               NHAN VIEN
// =====================================================================
shared_ptr<NhanVien> QuanLyKhachSan::themNhanVien(const string &maCN, const string &ten, const string &sdt,
                                                  const string &cv, double luong, const string &email,
                                                  const string &cccd, const string &gioiTinh, const string &ngaySinh)
{
    auto cn = timChiNhanh(maCN);
    if (!cn)
        throw runtime_error("Khong tim thay chi nhanh " + maCN + ".");
    for (auto &nv : tatCaNhanVien())
        if (nv->laySoDienThoai() == InputHelper::trim(sdt))
            throw invalid_argument("So dien thoai da duoc nhan vien " + nv->layMa() + " su dung.");
    auto nv = make_shared<NhanVien>(taoMa("NV", demNhanVien + 1), ten, sdt, cv, luong, cn->layMa(), email,
                                    cccd, gioiTinh, ngaySinh);
    ++demNhanVien;
    cn->themNhanVien(nv);
    return nv;
}

shared_ptr<NhanVien> QuanLyKhachSan::timNhanVien(const string &ma) const
{
    string m = chuanHoaMa(ma);
    for (auto &cn : danhSachChiNhanh)
        if (auto nv = cn->timNhanVien(m))
            return nv;
    return nullptr;
}

vector<shared_ptr<NhanVien>> QuanLyKhachSan::tatCaNhanVien(bool caNguoiDaNghi) const
{
    vector<shared_ptr<NhanVien>> kq;
    for (auto &cn : danhSachChiNhanh)
        for (auto &nv : cn->layDanhSachNhanVien())
            if (caNguoiDaNghi || nv->layDangLam())
                kq.push_back(nv);
    return kq;
}

void QuanLyKhachSan::xoaNhanVien(const string &ma)
{
    auto nv = timNhanVien(ma);
    if (!nv || !nv->layDangLam())
        throw runtime_error("Khong tim thay nhan vien dang lam viec.");
    nv->datDangLam(false); // phieu/hoa don cu van giu ten nguoi lap
}

// =====================================================================
//                              KHACH HANG
// =====================================================================
void QuanLyKhachSan::kiemTraTrungKhach(const string &sdt, const string &cccd, const string &boQua) const
{
    for (auto &kh : danhSachKhachHang)
    {
        if (kh->layMa() == boQua)
            continue;
        if (!sdt.empty() && kh->laySoDienThoai() == InputHelper::trim(sdt))
            throw invalid_argument("So dien thoai da thuoc khach " + kh->layMa() + " - " + kh->layHoTen() + ".");
        if (!cccd.empty() && kh->layCCCD() == InputHelper::chuHoa(InputHelper::trim(cccd)))
            throw invalid_argument("CCCD/Ho chieu da thuoc khach " + kh->layMa() + " - " + kh->layHoTen() + ".");
    }
}

shared_ptr<KhachHang> QuanLyKhachSan::themKhachHang(const string &ten, const string &sdt, const string &cccd,
                                                    const string &email, const string &qt,
                                                    const string &diaChi, const string &gioiTinh)
{
    kiemTraTrungKhach(sdt, cccd);
    auto kh = make_shared<KhachHang>(taoMa("KH", demKhachHang + 1), ten, sdt, cccd, email, qt,
                                     diaChi, gioiTinh);
    ++demKhachHang;
    danhSachKhachHang.push_back(kh);
    return kh;
}

shared_ptr<KhachHang> QuanLyKhachSan::timKhachHang(const string &ma) const
{
    string m = chuanHoaMa(ma);
    for (auto &kh : danhSachKhachHang)
        if (kh->layMa() == m)
            return kh;
    return nullptr;
}

shared_ptr<KhachHang> QuanLyKhachSan::timKhachTheoSDT(const string &sdt) const
{
    string s = InputHelper::trim(sdt);
    for (auto &kh : danhSachKhachHang)
        if (kh->laySoDienThoai() == s)
            return kh;
    return nullptr;
}

shared_ptr<KhachHang> QuanLyKhachSan::timKhachTheoCCCD(const string &cccd) const
{
    string s = InputHelper::chuHoa(InputHelper::trim(cccd));
    for (auto &kh : danhSachKhachHang)
        if (kh->layCCCD() == s)
            return kh;
    return nullptr;
}

vector<shared_ptr<KhachHang>> QuanLyKhachSan::timKhachTheoTuKhoa(const string &tuKhoa) const
{
    string k = InputHelper::chuHoa(InputHelper::trim(tuKhoa));
    string ma = chuanHoaMa(tuKhoa);
    vector<shared_ptr<KhachHang>> kq;
    for (auto &kh : danhSachKhachHang)
    {
        if (InputHelper::chuHoa(kh->layHoTen()).find(k) != string::npos ||
            kh->laySoDienThoai().find(k) != string::npos || kh->layCCCD().find(k) != string::npos ||
            kh->layMa() == ma)
            kq.push_back(kh);
    }
    return kq;
}

// =====================================================================
//                                DICH VU
// =====================================================================
string QuanLyKhachSan::taoMaDichVu() const { return taoMa("DV", demDichVu + 1); }

void QuanLyKhachSan::themDichVu(const shared_ptr<DichVu> &dv)
{
    if (!dv)
        throw invalid_argument("Dich vu null.");
    if (timDichVu(dv->layMa()))
        throw invalid_argument("Ma dich vu da ton tai.");
    for (auto &d : danhSachDichVu)
        if (InputHelper::chuHoa(d->layTen()) == InputHelper::chuHoa(dv->layTen()))
            throw invalid_argument("Ten dich vu da ton tai.");
    danhSachDichVu.push_back(dv);
    demDichVu = max(demDichVu, atoi(dv->layMa().c_str() + 2));
}

shared_ptr<DichVu> QuanLyKhachSan::timDichVu(const string &ma) const
{
    string m = chuanHoaMa(ma);
    for (auto &d : danhSachDichVu)
        if (d->layMa() == m)
            return d;
    return nullptr;
}

void QuanLyKhachSan::xoaDichVu(const string &ma)
{
    string m = chuanHoaMa(ma);
    for (size_t i = 0; i < danhSachDichVu.size(); ++i)
        if (danhSachDichVu[i]->layMa() == m)
        {
            // Cac lan su dung truoc do da duoc clone (chot gia) nen hoa don cu khong bi anh huong
            danhSachDichVu.erase(danhSachDichVu.begin() + (long)i);
            return;
        }
    throw runtime_error("Khong tim thay dich vu " + m + ".");
}

// =====================================================================
//                          NGHIEP VU LUU TRU
// =====================================================================
shared_ptr<PhieuDatPhong> QuanLyKhachSan::layPhieuBatBuoc(const string &ma) const
{
    auto p = timPhieu(ma);
    if (!p)
        throw runtime_error("Khong tim thay phieu " + chuanHoaMa(ma) + ".");
    return p;
}

shared_ptr<PhieuDatPhong> QuanLyKhachSan::taoPhieu(const string &maKH, const string &maPhong,
                                                   const shared_ptr<NhanVien> &nv, const Ngay &ngayNhan,
                                                   int soDem, int soNguoi, MuaDuLich mua, const string &ghiChu)
{
    auto kh = timKhachHang(maKH);
    if (!kh)
        throw runtime_error("Khong tim thay khach hang " + maKH + ".");
    auto p = timPhong(maPhong);
    if (!p)
        throw runtime_error("Khong tim thay phong " + maPhong + ".");
    auto phieu = make_shared<PhieuDatPhong>(taoMa("DP", demPhieu + 1), kh, p, nv, ngayNhan, soDem, soNguoi,
                                            mua, ghiChu);
    ++demPhieu;
    return phieu;
}

shared_ptr<PhieuDatPhong> QuanLyKhachSan::datPhong(const string &maKH, const string &maPhong,
                                                   const shared_ptr<NhanVien> &nv, const Ngay &ngayNhan,
                                                   int soDem, int soNguoi, MuaDuLich mua, const string &ghiChu,
                                                   double tienCoc, const string &ptCoc)
{
    Ngay homNay = Ngay::homNay();
    if (ngayNhan < homNay)
        throw invalid_argument("Ngay nhan phong khong duoc o qua khu.");
    if (homNay.khoangCachDen(ngayNhan) > 365)
        throw invalid_argument("Chi nhan dat phong truoc toi da 365 ngay.");
    auto p = timPhong(maPhong);
    if (!p)
        throw runtime_error("Khong tim thay phong " + maPhong + ".");
    // Khach da check-in ma chua check-out: tu choi MOI khoang ngay sau ngay nhan cua ho,
    // KE CA khi khoang ngay moi nam sau ngay tra du kien (phong chi giai phong khi check-out)
    if (auto dangO = phieuDangOCuaPhong(p->layMaPhong()); dangO && dangO->trungLich(ngayNhan, soDem))
        throw runtime_error(thongBaoPhongDangCoKhach(*p, *dangO));
    string lyDo = lyDoKhongDatDuoc(*p, ngayNhan, soDem, soNguoi);
    if (!lyDo.empty())
        throw runtime_error("Khong dat duoc phong " + p->layMaPhong() + ": " + lyDo + ".");
    if (tienCoc < 0)
        throw invalid_argument("Tien coc khong duoc am.");
    auto khDat = timKhachHang(maKH);
    if (!khDat)
        throw runtime_error("Khong tim thay khach hang " + maKH + ".");
    // Tran coc = tong tien phong phai tra (sau giam gia thanh vien, gom VAT) -> coc khong bao gio vuot hoa don phong
    double maxCoc = tranCocToiDa(*p, soDem, mua, khDat->tyLeGiamGia());
    if (tienCoc > maxCoc + 0.5)
        throw invalid_argument("Tien coc khong duoc vuot tong tien phong phai tra (" +
                               dinhDangTien(maxCoc) + " VND, da tinh giam gia thanh vien va VAT).");

    auto phieu = taoPhieu(maKH, maPhong, nv, ngayNhan, soDem, soNguoi, mua, ghiChu);
    phieu->datCoc(tienCoc, ptCoc);
    danhSachPhieu.push_back(phieu);
    phieu->layKhachHang()->themLichSu(phieu);
    // State: khach nhan phong hom nay -> Trong -> Giu cho (den 23:59)
    if (ngayNhan == homNay && p->layMaTrangThai() == MaTrangThai::TRONG)
        p->giuCho(ngayNhan.toTimeT(23, 59, 59));
    return phieu;
}

int QuanLyKhachSan::dongBoTheoNgay()
{
    Ngay hn = Ngay::homNay();
    time_t bayGio = time(nullptr);
    int daHuy = 0;
    // 1. Huy phieu qua ngay nhan ma khach khong den
    // Chinh sach no-show: giu lai tien coc (hoanCoc=false) — khach bo coc khi khong den
    for (auto &ph : danhSachPhieu)
        if (ph->layTrangThai() == TrangThaiPhieu::DA_DAT && ph->layNgayNhan() < hn)
        {
            // Phong van bi khach truoc (chua check-out / qua han) chiem -> khach nay KHONG phai no-show:
            // giu nguyen phieu de le tan xu ly (cho khach truoc check-out, hoac huy / dat lai)
            if (auto o = phieuDangOCuaPhong(ph->layPhong()->layMaPhong()); o && o != ph)
                continue;
            // Neu dang giu cho phong nay thi giai phong state
            auto p = ph->layPhong();
            if (p && p->layMaTrangThai() == MaTrangThai::GIU_CHO)
                p->huyGiuCho();
            ph->huy(bayGio, "Qua han nhan phong - he thong tu dong huy (giu coc)", false);
            ++daHuy;
        }
    // 2. Giai phong phong dang giu cho da het han
    for (auto &p : tatCaPhong())
    {
        auto gc = dynamic_pointer_cast<PhongGiuCho>(p->layTrangThai());
        if (gc && gc->daHetHan())
            p->huyGiuCho();
    }
    // 3. Giu cho phong cho khach se den trong hom nay
    for (auto &ph : danhSachPhieu)
        if (ph->layTrangThai() == TrangThaiPhieu::DA_DAT && ph->layNgayNhan() == hn &&
            ph->layPhong()->layMaTrangThai() == MaTrangThai::TRONG)
            ph->layPhong()->giuCho(hn.toTimeT(23, 59, 59));
    return daHuy;
}

void QuanLyKhachSan::checkIn(const string &maPhieu)
{
    dongBoTheoNgay();
    auto phieu = layPhieuBatBuoc(maPhieu);
    if (phieu->layTrangThai() != TrangThaiPhieu::DA_DAT)
        throw runtime_error("Phieu " + phieu->layMaPhieu() + " dang '" +
                            trangThaiPhieuToString(phieu->layTrangThai()) + "', khong the check-in.");
    if (Ngay::homNay() < phieu->layNgayNhan())
        throw runtime_error("Chua den ngay nhan phong (" + phieu->layNgayNhan().toString() + ").");
    auto p = phieu->layPhong();
    // Phieu dat truoc KHONG bi huy khi phong dang co khach; nhung khach moi khong duoc nhan phong
    // cho den khi khach cu check-out (hoac le tan chuyen phieu sang phong khac).
    if (auto khac = phieuDangOCuaPhong(p->layMaPhong()); khac && khac != phieu)
    {
        string a = khac->layKhachHang()->layHoTen(), b = phieu->layKhachHang()->layHoTen();
        string s = "Khong the check-in cho khach " + b + " (phieu " + phieu->layMaPhieu() + ").\n" + "Phong " +
                   p->laySoPhong() + " (" + p->layMaPhong() + ") hien dang co khach " + a + " (phieu " +
                   khac->layMaPhieu() + ") chua check-out";
        if (khac->dangOverstay())
            s += "\nva khach " + a + " da qua ngay tra du kien (" + khac->layNgayTra().toString() + ", qua han " +
                 to_string(khac->soNgayOverstay()) + " ngay).";
        else
            s += " (ngay tra du kien " + khac->layNgayTra().toString() + ").";
        throw runtime_error(s + "\nVui long:\n 1. Cho khach " + a + " check-out\n hoac\n 2. Xu ly chuyen phong cho khach " +
                            b + " (chuc nang 'Chuyen phieu sang phong khac').");
    }
    if (Ngay::homNay() > phieu->layNgayNhan())
        throw runtime_error("Da qua ngay nhan phong (" + phieu->layNgayNhan().toString() +
                            "). Phieu no-show se bi huy tu dong.");
    if (p->layMaTrangThai() == MaTrangThai::DANG_DON)
        throw runtime_error("Phong " + p->layMaPhong() + " dang don dep. Hay 'Hoan tat don dep' truoc khi check-in.");
    try
    {
        p->nhanPhong(); // State: Giu cho -> Dang o
    }
    catch (...)
    {
        if (p->layMaTrangThai() == MaTrangThai::TRONG && phieu->layTrangThai() == TrangThaiPhieu::DA_DAT &&
            phieu->layNgayNhan() < Ngay::homNay())
            phieu->huy(time(nullptr), "Qua han giu cho");
        throw;
    }
    phieu->xacNhanCheckIn(time(nullptr));
}

void QuanLyKhachSan::huyDatPhong(const string &maPhieu, const string &lyDo, bool hoanCoc)
{
    auto phieu = layPhieuBatBuoc(maPhieu);
    if (phieu->layTrangThai() != TrangThaiPhieu::DA_DAT)
        throw runtime_error("Chi huy duoc phieu dang 'Cho nhan phong'.");
    auto p = phieu->layPhong();
    if (phieu->layNgayNhan() == Ngay::homNay() && p->layMaTrangThai() == MaTrangThai::GIU_CHO)
        p->huyGiuCho(); // State: Giu cho -> Trong
    phieu->huy(time(nullptr), lyDo, hoanCoc);
}

string QuanLyKhachSan::kiemTraGiaHan(const string &maPhieu, int soDemThem) const
{
    auto phieu = timPhieu(maPhieu);
    if (!phieu)
        return "Khong tim thay phieu " + chuanHoaMa(maPhieu) + ".";
    if (phieu->layTrangThai() != TrangThaiPhieu::DA_NHAN_PHONG)
        return "Chi gia han duoc phieu dang o (da check-in, chua check-out). Phieu " + phieu->layMaPhieu() + " dang '" +
               trangThaiPhieuToString(phieu->layTrangThai()) + "'.";
    if (soDemThem <= 0 || soDemThem > 30)
        return "So dem gia han phai tu 1 den 30.";
    if (phieu->laySoDem() + soDemThem > 60)
        return "Tong so dem toi da 60 (hien o " + to_string(phieu->laySoDem()) + " dem).";
    Ngay hn = Ngay::homNay(), traCu = phieu->layNgayTra(), traMoi = traCu.congNgay(soDemThem);
    if (!(hn < traMoi))
        return "Khach da qua han " + to_string(phieu->soNgayOverstay()) + " ngay: gia han phai keo ngay tra moi (" +
               traMoi.toString() + ") sau hom nay. Can gia han it nhat " + to_string(traCu.khoangCachDen(hn) + 1) + " dem.";
    // Xung dot voi phieu khac dat cung phong trong khoang gia han
    for (auto &ph : lichDatCuaPhong(phieu->layPhong()->layMaPhong()))
    {
        if (ph == phieu)
            continue;
        if (ph->layNgayNhan() < traMoi)
        {
            // Khong de hai phieu cung chiem mot phong trong cung khoang thoi gian
            int toiDa = traCu.khoangCachDen(ph->layNgayNhan());
            auto p = phieu->layPhong();
            return "Khong the gia han.\nPhong " + p->laySoPhong() + " (" + p->layMaPhong() + ") da co phieu dat cua khach " +
                   ph->layKhachHang()->layHoTen() + " (phieu " + ph->layMaPhieu() + ")\ntu ngay " +
                   ph->layNgayNhan().toString() + ".\n" +
                   (toiDa > 0 ? "Chi co the gia han toi da " + to_string(toiDa) + " dem (den " + ph->layNgayNhan().toString() + ").\n"
                              : "") +
                   "Vui long chon ngay khac hoac thuc hien chuyen phong.";
        }
    }
    return "";
}

shared_ptr<PhieuDatPhong> QuanLyKhachSan::giaHan(const string &maPhieu, int soDemThem)
{
    string ly = kiemTraGiaHan(maPhieu, soDemThem);
    if (!ly.empty())
        throw runtime_error(ly);
    auto phieu = layPhieuBatBuoc(maPhieu);
    phieu->giaHan(soDemThem); // khong tao phieu moi: cung phieu, cung phong, so dem tang
    return phieu;
}

void QuanLyKhachSan::ghiDichVu(const string &maPhieu, const string &maDV, int soLuong)
{
    auto phieu = layPhieuBatBuoc(maPhieu);
    auto dv = timDichVu(maDV);
    if (!dv)
        throw runtime_error("Khong tim thay dich vu " + chuanHoaMa(maDV) + ".");
    phieu->themDichVu(dv->clone(), soLuong, time(nullptr)); // clone -> chot gia
}

// Decorator Pattern: boc them mot lop dich vu quanh phong (chon decorator theo loai dich vu)
// Ho tro day du 4 nhom: Am thuc, Spa, Tour, Tien ich (khop dichvu.csv)
shared_ptr<Phong> QuanLyKhachSan::bocDichVu(const shared_ptr<Phong> &p, const shared_ptr<DichVu> &dv, int soLuong)
{
    if (auto at = dynamic_pointer_cast<AmThuc>(dv))
        return make_shared<DecoratorAmThuc>(p, at, soLuong);
    if (auto spa = dynamic_pointer_cast<Spa>(dv))
        return make_shared<DecoratorSpa>(p, spa, soLuong);
    if (auto tour = dynamic_pointer_cast<TourTuyen>(dv))
        return make_shared<DecoratorTour>(p, tour, soLuong);
    if (auto ti = dynamic_pointer_cast<DichVuTienIch>(dv))
        return make_shared<DecoratorTienIch>(p, ti, soLuong);
    // Fallback: dich vu hop le nhung khong thuoc 4 lop tren (tuong thich du lieu cu)
    // Van cong dung thanh tien vao gia de khong mat tam tinh / hoa don
    if (dv)
    {
        // Boc bang DecoratorTienIch neu clone thanh DichVuTienIch duoc; neu khong thi bao ro
        auto tam = make_shared<DichVuTienIch>(dv->layMa(), dv->layTen(), dv->layDonGia());
        return make_shared<DecoratorTienIch>(p, tam, soLuong);
    }
    throw runtime_error("Loai dich vu khong ho tro.");
}

// Boc phong bang tung dich vu khach da su dung: Tour(Spa(AmThuc(Phong)))
shared_ptr<Phong> QuanLyKhachSan::taoPhongCoDichVu(const PhieuDatPhong &phieu) const
{
    shared_ptr<Phong> p = phieu.layPhong();
    for (auto &sd : phieu.layDichVuSuDung())
        p = bocDichVu(p, sd.dichVu, sd.soLuong);
    return p;
}

shared_ptr<HoaDon> QuanLyKhachSan::lapHoaDon(const shared_ptr<PhieuDatPhong> &phieu,
                                             const shared_ptr<NhanVien> &nv, const string &ma) const
{
    // Mot nguon so dem duy nhat (khong cong trung dat + gia han + thuc te):
    // - Dang o: soDemThucTe() = ngay nhan -> hom nay (gom qua han, KHONG bat buoc gia han tren he thong).
    // - Da check-out: laySoDem() da chot luc check-out.
    const bool dangO = phieu->layTrangThai() == TrangThaiPhieu::DA_NHAN_PHONG;
    const int demDat = phieu->laySoDem();
    const int demQuaHan = dangO ? phieu->soNgayOverstay() : 0;
    const bool som = dangO && phieu->traSom();
    int tongDem = dangO ? phieu->soDemThucTe() : demDat;
    if (tongDem < 1)
        tongDem = 1;

    double tongDecorator = taoPhongCoDichVu(*phieu)->tinhGiaPhong(tongDem, phieu->layMua());
    auto hd = make_shared<HoaDon>(ma, phieu, nv, tongDecorator, phieu->layKhachHang()->tyLeGiamGia(),
                                  phieu->layTienCoc());

    auto p = phieu->layPhong();
    // tinhTienPhong(soDemThem): gia = phong.tinhGiaPhong(soDem + soDemThem)
    // -> soDemThem = tongDem - demDat de ra dung tongDem dem (am khi tra som, duong khi qua han)
    double tienPhong = phieu->tinhTienPhong(tongDem - demDat);
    string noiDung = loaiPhongToString(p->layLoaiPhong()) + " " + p->laySoPhong() + " - mua " +
                     muaToString(phieu->layMua()) + " x" + dinhDangSoThuc(heSoMua(phieu->layMua()), 1);
    if (tongDem >= Phong::SO_DEM_UU_DAI)
        noiDung += ", uu dai dai ngay -10%";
    if (som)
        noiDung += ", tra som (du kien " + to_string(demDat) + " dem, tinh " + to_string(tongDem) + " dem)";
    if (demQuaHan > 0)
        noiDung += ", gom " + to_string(demQuaHan) + " dem o qua han (khong can gia han tren he thong)";
    hd->themChiTiet(ChiTietHoaDon("Phong", p->layMaPhong(), noiDung, tongDem, tienPhong / tongDem, tienPhong));

    for (auto &sd : phieu->layDichVuSuDung())
    {
        string nd = sd.dichVu->layTen();
        if (sd.thanhTien() + 0.5 < sd.dichVu->layDonGia() * sd.soLuong)
            nd += " (uu dai doan -10%)"; // TourTuyen::tinhChiPhi override
        hd->themChiTiet(ChiTietHoaDon("Dich vu", sd.dichVu->layMa(), nd, sd.soLuong, sd.dichVu->layDonGia(),
                                      sd.thanhTien()));
        hd->themDichVu(sd.dichVu);
    }
    return hd;
}

shared_ptr<HoaDon> QuanLyKhachSan::xemTamTinh(const string &maPhieu, const shared_ptr<NhanVien> &nv) const
{
    auto phieu = layPhieuBatBuoc(maPhieu);
    if (phieu->layTrangThai() != TrangThaiPhieu::DA_NHAN_PHONG)
        throw runtime_error("Chi xem tam tinh cho phieu dang o.");
    return lapHoaDon(phieu, nv, "TAM TINH");
}

shared_ptr<HoaDon> QuanLyKhachSan::checkOut(const string &maPhieu, const shared_ptr<NhanVien> &nv)
{
    auto phieu = layPhieuBatBuoc(maPhieu);
    if (phieu->layTrangThai() != TrangThaiPhieu::DA_NHAN_PHONG)
        throw runtime_error("Phieu " + phieu->layMaPhieu() + " khong o trang thai 'Dang o'.");
    // Chot hoa don TRUOC khi doi soDem: lapHoaDon dung soDemThucTe (gom qua han / tra som)
    auto hd = lapHoaDon(phieu, nv, taoMa("HD", demHoaDon + 1));
    // Dong bo soDem tren phieu voi so dem da tinh phi (khong tinh trung lan sau)
    const int demTinhPhi = phieu->soDemThucTe();
    if (int quaHan = phieu->soNgayOverstay())
        phieu->giaHan(quaHan, false); // mo rong han -> soDem = demTinhPhi
    else if (phieu->traSom())
        phieu->rutNganSoDem(demTinhPhi); // rut ngan ve so dem thuc te da o
    // Tra dung han: soDem giu nguyen (= demTinhPhi)
    phieu->layPhong()->traPhong(); // State: Dang o -> Dang don dep
    ++demHoaDon;
    phieu->xacNhanCheckOut(time(nullptr), hd->layMa());
    danhSachHoaDon.push_back(hd);
    return hd;
}

void QuanLyKhachSan::thanhToanHoaDon(const string &maHD, const string &pt)
{
    auto hd = timHoaDon(maHD);
    if (!hd)
        throw runtime_error("Khong tim thay hoa don " + chuanHoaMa(maHD) + ".");
    hd->thanhToan(pt, time(nullptr));
    hd->layPhieu()->layKhachHang()->congDiem((int)(hd->tongThanhToan() / KhachHang::TIEN_MOI_DIEM));
}

// =====================================================================
//                                TRA CUU
// =====================================================================
shared_ptr<PhieuDatPhong> QuanLyKhachSan::timPhieu(const string &ma) const
{
    string m = chuanHoaMa(ma);
    for (auto &p : danhSachPhieu)
        if (p->layMaPhieu() == m)
            return p;
    return nullptr;
}

vector<shared_ptr<PhieuDatPhong>> QuanLyKhachSan::locPhieu(TrangThaiPhieu tt) const
{
    vector<shared_ptr<PhieuDatPhong>> kq;
    for (auto &p : danhSachPhieu)
        if (p->layTrangThai() == tt)
            kq.push_back(p);
    return kq;
}

shared_ptr<HoaDon> QuanLyKhachSan::timHoaDon(const string &ma) const
{
    string m = chuanHoaMa(ma);
    for (auto &h : danhSachHoaDon)
        if (h->layMa() == m)
            return h;
    return nullptr;
}

// =====================================================================
//                                THONG KE
// =====================================================================
double QuanLyKhachSan::tongDoanhThu(const string &maCN) const
{
    double t = 0;
    for (auto &h : danhSachHoaDon)
        if (h->layDaThanhToan() && hoaDonThuocChiNhanh(*h, maCN))
            t += h->tongThanhToan();
    return t;
}

double QuanLyKhachSan::doanhThuThangHienTai(const string &maCN) const
{
    const auto thang = doanhThuTheoThang(1, maCN);
    return thang.empty() ? 0 : thang.front().second;
}

double QuanLyKhachSan::chiPhiLuongThang(const string &maCN) const
{
    double tong = 0;
    for (const auto &nv : tatCaNhanVien())
    {
        if (!nv->layDangLam() || (!maCN.empty() && nv->layMaChiNhanh() != maCN) ||
            nv->layChucVu() == "Kênh Đặt Phòng Online" || nv->layChucVu() == "Kenh Dat Phong Online")
            continue;
        tong += nv->luongThucNhan();
    }
    return tong;
}

// Doanh thu phong / dich vu: phan bo theo ty le sau giam gia + VAT
double QuanLyKhachSan::doanhThuPhong(const string &maCN) const
{
    double t = 0;
    for (auto &h : danhSachHoaDon)
        if (h->layDaThanhToan() && h->tamTinh() > 0 && hoaDonThuocChiNhanh(*h, maCN))
            t += h->tongThanhToan() * h->tienPhong() / h->tamTinh();
    return t;
}

double QuanLyKhachSan::doanhThuDichVu(const string &maCN) const { return tongDoanhThu(maCN) - doanhThuPhong(maCN); }

int QuanLyKhachSan::soHoaDonChuaThanhToan(const string &maCN) const
{
    int d = 0;
    for (auto &h : danhSachHoaDon)
        if (!h->layDaThanhToan() && hoaDonThuocChiNhanh(*h, maCN))
            ++d;
    return d;
}

double QuanLyKhachSan::tongCocGiuLai() const
{
    double t = 0;
    for (auto &ph : danhSachPhieu)
        if (ph->layTrangThai() == TrangThaiPhieu::DA_HUY && !ph->layDaHoanCoc())
            t += ph->layTienCoc();
    return t;
}

double QuanLyKhachSan::tongCocDangGiu() const
{
    double t = 0;
    for (auto &ph : danhSachPhieu)
        if (ph->dangHoatDong())
            t += ph->layTienCoc();
    return t;
}

double QuanLyKhachSan::congSuatPhong(const string &maCN) const
{
    auto cnLoc = maCN.empty() ? nullptr : timChiNhanh(maCN);
    auto ds = cnLoc ? cnLoc->layDanhSachPhong() : tatCaPhong();
    if (ds.empty())
        return 0;
    int dangO = 0;
    for (auto &p : ds)
        if (p->layMaTrangThai() == MaTrangThai::DANG_O)
            ++dangO;
    return 100.0 * dangO / ds.size();
}

vector<pair<string, double>> QuanLyKhachSan::doanhThuTheoChiNhanh() const
{
    // Tra ve (ten chi nhanh, doanh thu) — dung ma de ghep chinh xac, tranh lech ten
    vector<pair<string, double>> kq;
    vector<string> maList;
    for (auto &cn : danhSachChiNhanh)
    {
        kq.push_back({cn->layTen(), 0});
        maList.push_back(cn->layMa());
    }
    for (auto &h : danhSachHoaDon)
    {
        if (!h->layDaThanhToan())
            continue;
        string ma = maChiNhanhCuaPhong(h->layPhieu()->layPhong());
        for (size_t i = 0; i < maList.size(); ++i)
            if (maList[i] == ma)
            {
                kq[i].second += h->tongThanhToan();
                break;
            }
    }
    return kq;
}

vector<pair<string, double>> QuanLyKhachSan::doanhThuTheoLoaiPhong(const string &maCN) const
{
    vector<pair<string, double>> kq = {{"Standard", 0}, {"Deluxe", 0}, {"Suite", 0}, {"Villa", 0}};
    for (auto &h : danhSachHoaDon)
    {
        if (!h->layDaThanhToan() || !hoaDonThuocChiNhanh(*h, maCN))
            continue;
        int idx = (int)h->layPhieu()->layPhong()->layLoaiPhong();
        kq[(size_t)idx].second += h->tongThanhToan();
    }
    return kq;
}

vector<pair<string, double>> QuanLyKhachSan::doanhThuTheoThang(int soThang, const string &maCN) const
{
    Ngay hn = Ngay::homNay();
    int thang = hn.layThang(), nam = hn.layNam();
    vector<pair<string, double>> kq;
    vector<pair<int, int>> khoa;
    for (int i = soThang - 1; i >= 0; --i)
    {
        int m = thang - i, y = nam;
        while (m <= 0)
        {
            m += 12;
            --y;
        }
        char buf[48];
        snprintf(buf, sizeof(buf), "Thang %02d/%04d", m, y);
        kq.push_back({buf, 0});
        khoa.push_back({m, y});
    }
    for (auto &h : danhSachHoaDon)
    {
        if (!h->layDaThanhToan() || !hoaDonThuocChiNhanh(*h, maCN))
            continue;
        Ngay n = Ngay::tuTimeT(h->layThoiGianThanhToan());
        for (size_t i = 0; i < khoa.size(); ++i)
            if (khoa[i].first == n.layThang() && khoa[i].second == n.layNam())
                kq[i].second += h->tongThanhToan();
    }
    return kq;
}

vector<ThongKeDichVu> QuanLyKhachSan::thongKeDichVu(const string &maCN) const
{
    map<string, ThongKeDichVu> gom;
    for (auto &h : danhSachHoaDon)
    {
        if (!h->layDaThanhToan() || !hoaDonThuocChiNhanh(*h, maCN))
            continue;
        for (auto &dv : h->layDichVuSuDung())
        {
            auto &tk = gom[dv->layMa()];
            tk.ma = dv->layMa();
            tk.ten = dv->layTen();
            tk.loai = dv->layLoai();
        }
        for (auto &ct : h->layChiTiet())
        {
            if (ct.layLoai() == "Phong")
                continue;
            auto &tk = gom[ct.layMaThamChieu()];
            tk.soLuong += ct.laySoLuong();
            tk.doanhThu += ct.layThanhTien();
        }
    }
    vector<ThongKeDichVu> kq;
    for (auto &x : gom)
        kq.push_back(x.second);
    sort(kq.begin(), kq.end(), [](const ThongKeDichVu &a, const ThongKeDichVu &b)
         { return a.doanhThu > b.doanhThu; });
    return kq;
}

vector<ThongKeKhachHang> QuanLyKhachSan::topKhachHang(size_t n, const string &maCN) const
{
    vector<ThongKeKhachHang> kq;
    for (auto &kh : danhSachKhachHang)
    {
        ThongKeKhachHang tk{kh, 0, 0};
        for (auto &h : danhSachHoaDon)
            if (h->layDaThanhToan() && h->layPhieu()->layKhachHang() == kh && hoaDonThuocChiNhanh(*h, maCN))
            {
                ++tk.soHoaDon;
                tk.tongChi += h->tongThanhToan();
            }
        if (tk.soHoaDon > 0)
            kq.push_back(tk);
    }
    sort(kq.begin(), kq.end(), [](const ThongKeKhachHang &a, const ThongKeKhachHang &b)
         { return a.tongChi > b.tongChi; });
    if (kq.size() > n)
        kq.resize(n);
    return kq;
}

// =====================================================================
//                               DU LIEU MAU
// =====================================================================
// Mot luot luu tru da hoan tat trong qua khu (dat -> check-in -> dich vu -> check-out -> thanh toan)
void QuanLyKhachSan::taoLichSuMau(const string &maKH, const string &maPhong, const string &maNV,
                                  int lechNgay, int soDem, int soNguoi,
                                  const vector<pair<string, int>> &dichVu, const string &pt, bool donXong,
                                  bool daThanhToan)
{
    Ngay ngayNhan = Ngay::homNay().congNgay(lechNgay);
    Ngay ngayTra = ngayNhan.congNgay(soDem);
    auto nv = timNhanVien(maNV);
    auto phieu = taoPhieu(maKH, maPhong, nv, ngayNhan, soDem, soNguoi, xacDinhMua(ngayNhan), "");
    phieu->datThoiGianDat(ngayNhan.congNgay(-5).toTimeT(9, 30));
    auto p = phieu->layPhong();
    p->giuCho(time(nullptr) + 3600);
    p->nhanPhong();
    phieu->xacNhanCheckIn(ngayNhan.toTimeT(14, 10));
    for (size_t i = 0; i < dichVu.size(); ++i)
        phieu->themDichVu(timDichVu(dichVu[i].first)->clone(), dichVu[i].second,
                          ngayNhan.toTimeT(18, 0) + (time_t)i * 3600);
    danhSachPhieu.push_back(phieu);
    phieu->layKhachHang()->themLichSu(phieu);

    string maHD = taoMa("HD", demHoaDon + 1);
    phieu->xacNhanCheckOut(ngayTra.toTimeT(11, 30), maHD); // check-out truoc khi lap hoa don (khong tinh dem qua han)
    auto hd = lapHoaDon(phieu, nv, maHD);
    ++demHoaDon;
    hd->datThoiGianLap(ngayTra.toTimeT(11, 30));
    p->traPhong();
    if (donXong)
        p->donPhong();
    if (daThanhToan)
    {
        hd->thanhToan(pt, ngayTra.toTimeT(11, 35));
        phieu->layKhachHang()->congDiem((int)(hd->tongThanhToan() / KhachHang::TIEN_MOI_DIEM));
    }
    danhSachHoaDon.push_back(hd);
}

void QuanLyKhachSan::napDuLieuMau()
{
    // ---- Chi nhanh (2 chi nhanh chinh) ----
    auto cn1 = themChiNhanh("Nha Trang", "86 Tran Phu, Loc Tho, Nha Trang", "0258388999");
    auto cn2 = themChiNhanh("Quy Nhon", "Bai Xep, Nhon Ly, Quy Nhon", "0256384567");

    // ---- Nhan vien ----
    themNhanVien(cn1->layMa(), "Nguyen Minh Quan", "0901000001", "Quan ly", 15000000, "quan.nm@oceanpearl.vn");
    themNhanVien(cn1->layMa(), "Tran Thu Ha", "0901000002", "Le tan", 10000000, "ha.tt@oceanpearl.vn");
    themNhanVien(cn2->layMa(), "Le Hoang Nam", "0901000003", "Le tan", 10000000);
    themNhanVien(cn1->layMa(), "Pham Bao Ngoc", "0901000004", "Buong phong", 8000000);
    themNhanVien(cn2->layMa(), "Vo Thanh Tung", "0901000005", "Quan ly", 15000000, "tung.vt@oceanpearl.vn");
    themNhanVien(cn2->layMa(), "Doan Mai Linh", "0901000006", "Buong phong", 8000000);

    // ---- Phong: moi chi nhanh co du 4 loai, MOI LOAI TU 5 DEN 10 PHONG (ma P01.. duy nhat toan he thong) ----
    // So phong duy nhat trong tung chi nhanh. Gia / dien tich / huong nhin thay doi nhe giua cac phong.
    const char *huong[] = {"Thanh pho", "Vuon", "Bien", "Ho boi"};
    auto taoNhom = [&](const shared_ptr<ChiNhanhResort> &cn, int giaChuan, int soStd, int soDlx, int soSuite, int soVilla)
    {
        // Suc chua theo loai: Standard 1-2, Deluxe/Suite 2-6, Villa 8-20
        const int stdCap[] = {2, 2, 1, 2, 2, 2, 1, 2};
        const int dlxCap[] = {2, 3, 4, 5, 6, 3, 4, 6};
        const int suiteCap[] = {2, 4, 6, 3, 4, 6};
        const int villaCap[] = {8, 10, 12, 14, 16, 17, 18, 20};
        for (int i = 0; i < soStd; ++i) // Standard: tang 1, so 101..
            themPhong(cn->layMa(), make_shared<StandardRoom>(taoMaPhong(), to_string(101 + i), 1,
                                                             giaChuan + 10000 * (i % 4), stdCap[i % 8],
                                                             22 + 2 * (i % 3), huong[i % 4]));
        for (int i = 0; i < soDlx; ++i) // Deluxe: tang 2, so 201..
            themPhong(cn->layMa(), make_shared<DeluxeRoom>(taoMaPhong(), to_string(201 + i), 2,
                                                           giaChuan * 17 / 10 + 50000 * (i % 4), dlxCap[i % 8], 30 + 2 * (i % 4),
                                                           i % 5 != 4, huong[(i + 2) % 4]));
        for (int i = 0; i < soSuite; ++i) // Suite: tang 3, so 301..
            themPhong(cn->layMa(), make_shared<Suite>(taoMaPhong(), to_string(301 + i), 3,
                                                      giaChuan * 34 / 10 + 100000 * (i % 3), suiteCap[i % 6], 55 + 3 * (i % 3),
                                                      2 + (i % 3 == 2 ? 1 : 0), true, 300000 + 10000 * (i % 3)));
        for (int i = 0; i < soVilla; ++i) // Villa: tang 0, so 501..
            themPhong(cn->layMa(), make_shared<Villa>(taoMaPhong(), to_string(501 + i), 1,
                                                      giaChuan * 8 + 350000 * (i % 4), villaCap[i % 8], 180 + 20 * (i % 4),
                                                      true, 90 + 30 * (i % 4), 2 + (i % 3), 400000 + 50000 * (i % 3)));
    };
    taoNhom(cn1, 650000, 8, 7, 6, 5); // Nha Trang: 8 Standard, 7 Deluxe, 6 Suite, 5 Villa = 26 phong
    taoNhom(cn2, 600000, 7, 8, 5, 6); // Quy Nhon : 7 Standard, 8 Deluxe, 5 Suite, 6 Villa = 26 phong
    auto phongSo = [&](const shared_ptr<ChiNhanhResort> &cn, const string &so)
    {
        for (auto &p : cn->layDanhSachPhong())
            if (p->laySoPhong() == so)
                return p->layMaPhong();
        throw runtime_error("Du lieu mau: khong co phong " + so);
    };
    auto NT = [&](const string &so) { return phongSo(cn1, so); };
    auto QN = [&](const string &so) { return phongSo(cn2, so); };

    // ---- Dich vu ----
    themDichVu(make_shared<AmThuc>(taoMaDichVu(), "Buffet sang", 250000, "Buffet"));
    themDichVu(make_shared<AmThuc>(taoMaDichVu(), "Set menu hai san", 650000, "Hai san"));
    themDichVu(make_shared<Spa>(taoMaDichVu(), "Massage da nong", 450000, 60));
    themDichVu(make_shared<Spa>(taoMaDichVu(), "Tri lieu thao duoc", 700000, 90));
    themDichVu(make_shared<TourTuyen>(taoMaDichVu(), "Tour 4 dao", 550000, "Vinh Nha Trang", 1));
    themDichVu(make_shared<TourTuyen>(taoMaDichVu(), "Lan ngam san ho", 850000, "Hon Mun", 1));
    themDichVu(make_shared<TourTuyen>(taoMaDichVu(), "Tour Ky Co Eo Gio", 600000, "Ky Co - Eo Gio", 1));

    // ---- Khach hang ----
    themKhachHang("Nguyen Van An", "0905123456", "079201001234", "an.nguyen@gmail.com");
    themKhachHang("Tran Thi Bich", "0912345678", "001302004567", "bich.tran@gmail.com");
    themKhachHang("Le Quoc Cuong", "0938765432", "048099007788");
    themKhachHang("Pham Thu Dung", "0977111222", "056301009900", "dung.pham@yahoo.com");
    themKhachHang("John Smith", "0868222333", "C1234567", "john.smith@mail.com", "Anh");
    themKhachHang("Hoang Gia Bao", "0966333444", "052202003344", "bao.hoang@gmail.com");
    themKhachHang("Vu Ngoc Lan", "0944555666", "064198005566");
    themKhachHang("Dang Quang Huy", "0355777888", "077200007788", "huy.dang@outlook.com");

    // ---- Lich su luu tru da hoan tat (co hoa don) ----
    taoLichSuMau("KH01", NT("201"), "NV02", -150, 3, 2, {{"DV01", 6}, {"DV03", 2}}, "Chuyen khoan");
    taoLichSuMau("KH05", NT("202"), "NV02", -75, 4, 3, {{"DV03", 3}}, "Chuyen khoan");
    taoLichSuMau("KH03", NT("101"), "NV02", -118, 2, 2, {{"DV01", 4}}, "Tien mat");
    taoLichSuMau("KH01", QN("501"), "NV03", -95, 4, 6, {{"DV02", 6}, {"DV06", 6}}, "The tin dung");
    taoLichSuMau("KH08", QN("503"), "NV05", -50, 5, 5, {{"DV02", 5}, {"DV07", 5}}, "The tin dung");
    taoLichSuMau("KH04", NT("301"), "NV02", -64, 2, 3, {{"DV04", 2}}, "Chuyen khoan");
    taoLichSuMau("KH05", QN("201"), "NV03", -40, 7, 2, {{"DV01", 14}, {"DV05", 2}}, "The tin dung");
    taoLichSuMau("KH06", NT("502"), "NV02", -30, 3, 4, {{"DV01", 6}, {"DV05", 4}}, "Chuyen khoan");
    taoLichSuMau("KH02", QN("301"), "NV03", -21, 3, 4, {{"DV02", 4}, {"DV03", 2}}, "Tien mat");
    taoLichSuMau("KH07", QN("102"), "NV03", -12, 2, 2, {{"DV07", 2}}, "Vi dien tu");
    taoLichSuMau("KH01", NT("101"), "NV02", -9, 2, 2, {{"DV03", 1}}, "Chuyen khoan");
    taoLichSuMau("KH03", NT("102"), "NV02", -3, 2, 1, {}, "Tien mat", false); // NT 102 dang don dep
    taoLichSuMau("KH07", NT("302"), "NV02", -4, 3, 3, {{"DV03", 2}}, "", true, false); // check-out hom qua, HOA DON CHUA THANH TOAN

    auto nvHa = timNhanVien("NV02"), nvNam = timNhanVien("NV03");
    auto datTruoc = [&](const string &maKH, const string &maPhong, const shared_ptr<NhanVien> &nv, int lech, int dem,
                        int nguoi, const string &ghiChu = "", double coc = 0, int ngayDat = 2)
    {
        Ngay nn = Ngay::homNay().congNgay(lech);
        auto ph = datPhong(maKH, maPhong, nv, nn, dem, nguoi, xacDinhMua(nn), ghiChu, coc, coc > 0 ? "Chuyen khoan" : "");
        ph->datThoiGianDat(time(nullptr) - ngayDat * 86400);
        return ph;
    };

    // ---- Phieu da huy: 1 phieu hoan coc, 1 phieu GIU coc (no-show / huy sat ngay) ----
    {
        auto ph = datTruoc("KH04", NT("203"), nvHa, 5, 2, 2, "", 500000, 3);
        huyDatPhong(ph->layMaPhieu(), "Khach doi lich cong tac", true);
    }
    {
        auto ph = datTruoc("KH03", QN("103"), nvNam, 6, 2, 2, "", 400000, 1);
        huyDatPhong(ph->layMaPhieu(), "Khach huy sat ngay - theo chinh sach giu coc", false);
    }

    // ---- Phong bao tri (khong co phieu dat) ----
    timPhong(NT("207"))->baoTri("Sua dieu hoa");
    timPhong(QN("107"))->baoTri("Thay noi that phong");

    // ---- Phieu dat truoc (tao TRUOC khi khach vao o cac phong nay, de co lich tuong lai that su) ----
    datTruoc("KH08", NT("201"), nvHa, 4, 2, 2, "Dat sau khi khach hien tai tra phong");   // 201: sau khach dang o -> thu gia han bi xung dot
    datTruoc("KH03", NT("105"), nvHa, 1, 1, 2, "Se bi anh huong neu khach 105 o qua han"); // 105: khach dang overstay
    datTruoc("KH03", NT("301"), nvHa, 2, 2, 3, "Ky niem ngay cuoi", 1500000);            // thu chuyen bao tri se bi tu choi
    datTruoc("KH01", NT("501"), nvHa, 4, 3, 4, "", 3000000);
    datTruoc("KH08", QN("502"), nvNam, 7, 5, 6, "Nghi he cung gia dinh");
    datTruoc("KH06", QN("101"), nvNam, 3, 2, 2, "", 300000);
    // Khach den nhan phong hom nay (phong chuyen sang Dang giu cho)
    datTruoc("KH04", NT("103"), nvHa, 0, 1, 2, "", 0, 2);
    datTruoc("KH07", QN("202"), nvNam, 0, 2, 2, "", 0, 1);

    // ---- Khach dang o (da check-in) ----
    auto dangO = [&](const string &maKH, const string &maPhong, const shared_ptr<NhanVien> &nv, int lech, int dem,
                     int nguoi, const string &ghiChu, double coc, const vector<pair<string, int>> &dv)
    {
        Ngay nn = Ngay::homNay().congNgay(lech);
        auto ph = taoPhieu(maKH, maPhong, nv, nn, dem, nguoi, xacDinhMua(nn), ghiChu);
        if (coc > 0)
            ph->datCoc(coc, "Chuyen khoan");
        ph->datThoiGianDat(time(nullptr) - (3 - lech) * 86400);
        ph->layPhong()->giuCho(time(nullptr) + 3600);
        ph->layPhong()->nhanPhong();
        ph->xacNhanCheckIn(nn.toTimeT(14, 20));
        for (size_t i = 0; i < dv.size(); ++i)
            ph->themDichVu(timDichVu(dv[i].first)->clone(), dv[i].second, nn.congNgay(1).toTimeT(8, 0) + (time_t)i * 3600);
        danhSachPhieu.push_back(ph);
        ph->layKhachHang()->themLichSu(ph);
    };
    dangO("KH02", NT("201"), nvHa, -1, 3, 2, "Phong tang cao, yen tinh", 1000000, {{"DV03", 2}});
    dangO("KH05", QN("501"), nvNam, -2, 4, 5, "Don san bay", 0, {{"DV01", 10}, {"DV05", 5}});
    dangO("KH06", QN("302"), nvNam, -1, 2, 3, "", 0, {{"DV01", 3}});
    dangO("KH01", NT("105"), nvHa, -4, 2, 2, "Khach chua tra phong du da het han (overstay)", 0, {{"DV01", 2}});

    // ---- Bo sung khach hang mau (nhieu khach, thong tin day du) ----
    const char *tenThem[] = {
        "Nguyen Thi Hong", "Tran Van Khoa", "Le Minh Tue", "Pham Quang Duc", "Hoang Thi Mai",
        "Vu Duc Anh", "Dang Thi Huong", "Bui Van Son", "Do Thi Lan", "Ngo Minh Tri",
        "Phan Van Hieu", "Truong Thi Nga", "Ly Quoc Bao", "Cao Thi Yen", "Mai Van Phong",
        "Dinh Thi Cuc", "Ta Quang Vinh", "Luu Thi Dao", "Chu Van Hai", "Ong Thi Kim",
        "Ha Minh Chau", "Kieu Van Loc", "Tong Thi My", "Ung Van Toan", "Au Thi Bich"
    };
    for (int i = 0; i < 25; ++i)
    {
        string sdt = "09" + to_string(20000000 + i * 137);
        string cccd = "0" + to_string(79000000000LL + i * 100003LL);
        string email = string("khach") + to_string(i + 10) + "@mail.com";
        themKhachHang(tenThem[i], sdt, cccd, email, "Viet Nam");
    }

    // ---- Bo sung lich su dat/tra da hoan tat: ~20+ luot/chi nhanh trong 30 ngay gan day ----
    // Dung cac phong Standard/Deluxe xoay vong, lech ngay am (qua khu), so dem 1-3, tranh trung lich cung phong.
    auto phongNT = cn1->layDanhSachPhong();
    auto phongQN = cn2->layDanhSachPhong();
    const char *ptTT[] = {"Chuyen khoan", "Tien mat", "The tin dung"};
    const char *dvPool[] = {"DV01", "DV02", "DV03", "DV05"};
    int demNT = 0, demQN = 0;
    // Nha Trang: 30 ngay x xoay phong, moi ngay 1 phieu (co the nhieu phong song song)
    for (int day = 5; day <= 55 && demNT < 45; ++day)
    {
        int idxP = (day * 3) % (int)phongNT.size();
        auto &p = phongNT[(size_t)idxP];
        // Chi dung phong Standard/Deluxe de tranh trung voi dangO/datTruoc dang chiem
        if (p->layLoaiPhong() == LoaiPhong::VILLA || p->layLoaiPhong() == LoaiPhong::SUITE)
            continue;
        string so = p->laySoPhong();
        // Tranh cac phong dang co khach/dat truoc trong seed (105, 201, 103, 301, 501...)
        if (so == "105" || so == "201" || so == "103" || so == "301" || so == "501" || so == "502")
            continue;
        int lech = -day;
        int dem = 1 + (day % 3);
        int khIdx = (day * 7) % 25; // KH10.. tu bo them
        string maKH = "KH" + string(khIdx + 10 < 10 ? "0" : "") + to_string(khIdx + 10);
        // KH bat dau tu KH09 sau 8 khach cu + 25 them => ma thuc te la KH09, KH10...
        // themKhachHang cap ma tu dong; 8 khach cu -> KH01..KH08, 25 them -> KH09..KH33
        maKH = "KH" + to_string(9 + (day % 25));
        if (maKH.size() == 3)
            maKH = "KH0" + maKH.substr(2); // KH9 -> khong; to_string(9)="9" -> KH09
        {
            int soKH = 9 + (day % 25);
            maKH = soKH < 10 ? ("KH0" + to_string(soKH)) : ("KH" + to_string(soKH));
        }
        vector<pair<string, int>> dvs;
        if (day % 2 == 0)
            dvs.push_back({dvPool[day % 4], 1 + day % 3});
        try
        {
            taoLichSuMau(maKH, NT(so), "NV02", lech, dem, min(2, p->laySucChua()), dvs, ptTT[day % 3]);
            ++demNT;
        }
        catch (...)
        {
            // Trung lich / loi lien ket -> bo qua ban ghi nay
        }
    }
    for (int day = 5; day <= 55 && demQN < 45; ++day)
    {
        int idxP = (day * 5 + 1) % (int)phongQN.size();
        auto &p = phongQN[(size_t)idxP];
        if (p->layLoaiPhong() == LoaiPhong::VILLA)
            continue;
        string so = p->laySoPhong();
        if (so == "501" || so == "302" || so == "202" || so == "101" || so == "502" || so == "503" || so == "201" || so == "301")
            continue;
        int lech = -day;
        int dem = 1 + (day % 2);
        int soKH = 9 + ((day + 3) % 25);
        string maKH = soKH < 10 ? ("KH0" + to_string(soKH)) : ("KH" + to_string(soKH));
        vector<pair<string, int>> dvs;
        if (day % 3 == 0)
            dvs.push_back({dvPool[(day + 1) % 4], 1 + day % 2});
        try
        {
            taoLichSuMau(maKH, QN(so), "NV03", lech, dem, min(2, p->laySucChua()), dvs, ptTT[(day + 1) % 3]);
            ++demQN;
        }
        catch (...)
        {
        }
    }
    // Mot so phieu huy (khong tinh doanh thu)
    try
    {
        auto phHuy = taoPhieu("KH10", NT("106"), nvHa, Ngay::homNay().congNgay(-12), 2, 2, MuaDuLich::BINH_THUONG, "Khach huy");
        phHuy->datThoiGianDat(Ngay::homNay().congNgay(-15).toTimeT(10, 0));
        phHuy->huy(Ngay::homNay().congNgay(-11).toTimeT(9, 0), "Khach doi lich", false);
        danhSachPhieu.push_back(phHuy);
        phHuy->layKhachHang()->themLichSu(phHuy);
    }
    catch (...)
    {
    }
}


void QuanLyKhachSan::capNhatBoDem()
{
    auto soCuoi = [](const string &ma)
    {
        size_t i = ma.find_first_of("0123456789");
        return i == string::npos ? 0 : atoi(ma.c_str() + i);
    };
    demChiNhanh = demPhong = demKhachHang = demNhanVien = demPhieu = demHoaDon = demDichVu = 0;
    for (auto &cn : danhSachChiNhanh)
    {
        demChiNhanh = max(demChiNhanh, soCuoi(cn->layMa()));
        for (auto &p : cn->layDanhSachPhong())
            demPhong = max(demPhong, soCuoi(p->layMaPhong()));
        for (auto &nv : cn->layDanhSachNhanVien())
            demNhanVien = max(demNhanVien, soCuoi(nv->layMa()));
    }
    for (auto &x : danhSachPhongDaXoa)
        demPhong = max(demPhong, soCuoi(x.second->layMaPhong()));
    for (auto &kh : danhSachKhachHang)
        demKhachHang = max(demKhachHang, soCuoi(kh->layMa()));
    for (auto &ph : danhSachPhieu)
        demPhieu = max(demPhieu, soCuoi(ph->layMaPhieu()));
    for (auto &hd : danhSachHoaDon)
        demHoaDon = max(demHoaDon, soCuoi(hd->layMa()));
    for (auto &dv : danhSachDichVu)
        demDichVu = max(demDichVu, soCuoi(dv->layMa()));
}

// =====================================================================
//                    BO SUNG: XOA PHONG, SU KIEN, THONG KE THEO KY
// =====================================================================
void QuanLyKhachSan::xoaPhong(const string &maPhong)
{
    string m = chuanHoaMa(maPhong);
    auto p = timPhong(m);
    if (!p)
        throw runtime_error("Khong tim thay phong " + m + ".");
    auto ds = lichDatCuaPhong(m);
    if (!ds.empty())
        throw runtime_error("Phong " + m + " con " + to_string(ds.size()) +
                            " phieu dat/dang o. Hay huy hoac tra phong truoc khi xoa.");
    if (p->layMaTrangThai() == MaTrangThai::DANG_O || p->layMaTrangThai() == MaTrangThai::GIU_CHO)
        throw runtime_error("Phong dang co khach / dang giu cho, khong the xoa.");
    auto cn = chiNhanhCuaPhong(m);
    bool coLichSu = false;
    for (auto &ph : danhSachPhieu)
        if (ph->layPhong() == p)
            coLichSu = true;
    cn->xoaPhong(m);
    if (coLichSu) // giu lai de phieu/hoa don cu van doc duoc
        danhSachPhongDaXoa.push_back({cn->layMa(), p});
}

shared_ptr<Phong> QuanLyKhachSan::timPhongKeCaDaXoa(const string &maPhong) const
{
    if (auto p = timPhong(maPhong))
        return p;
    string m = chuanHoaMa(maPhong);
    for (auto &x : danhSachPhongDaXoa)
        if (x.second->layMaPhong() == m)
            return x.second;
    return nullptr;
}

vector<SuKienLuuTru> QuanLyKhachSan::layLichSuSuKien() const
{
    vector<SuKienLuuTru> ds;
    for (auto &p : danhSachPhieu)
    {
        ds.push_back({p->layThoiGianDat(), "DAT PHONG", p});
        if (p->layThoiGianCheckIn())
            ds.push_back({p->layThoiGianCheckIn(), "CHECK-IN", p});
        if (p->layThoiGianCheckOut())
            ds.push_back({p->layThoiGianCheckOut(), "CHECK-OUT", p});
        if (p->layThoiGianHuy())
            ds.push_back({p->layThoiGianHuy(), "HUY", p});
    }
    reverse(ds.begin(), ds.end()); // cung thoi diem -> su kien sau hien truoc
    stable_sort(ds.begin(), ds.end(), [](const SuKienLuuTru &a, const SuKienLuuTru &b)
                { return a.thoiGian > b.thoiGian; });
    return ds;
}

vector<ThongKeNhanVien> QuanLyKhachSan::thongKeNhanVien() const
{
    vector<ThongKeNhanVien> kq;
    for (auto &nv : tatCaNhanVien(true))
    {
        ThongKeNhanVien tk{nv, 0, 0, 0};
        for (auto &ph : danhSachPhieu)
            if (ph->layNhanVien() == nv)
                ++tk.soPhieuLap;
        for (auto &h : danhSachHoaDon)
            if (h->layNhanVienLap() == nv)
            {
                ++tk.soHoaDonLap;
                if (h->layDaThanhToan())
                    tk.doanhThu += h->tongThanhToan();
            }
        kq.push_back(tk);
    }
    return kq;
}

double QuanLyKhachSan::doanhThuTrongKhoang(time_t tu, time_t den, const string &maCN) const
{
    double t = 0;
    for (auto &h : danhSachHoaDon)
        if (h->layDaThanhToan() && h->layThoiGianThanhToan() >= tu && h->layThoiGianThanhToan() < den && hoaDonThuocChiNhanh(*h, maCN))
            t += h->tongThanhToan();
    return t;
}

vector<pair<string, double>> QuanLyKhachSan::doanhThuTheoKy(const string &ky, int soKy, const string &maCN) const
{
    Ngay hn = Ngay::homNay();
    vector<pair<string, double>> kq;
    char buf[48];
    for (int i = soKy - 1; i >= 0; --i)
    {
        Ngay tu, den;
        if (ky == "ngay")
        {
            tu = hn.congNgay(-i);
            den = tu.congNgay(1);
            snprintf(buf, sizeof(buf), "%02d/%02d", tu.layNgay(), tu.layThang());
        }
        else if (ky == "tuan")
        {
            int thu = layTm(hn.toTimeT(12)).tm_wday;   // 0 = Chu nhat
            Ngay thuHai = hn.congNgay(-((thu + 6) % 7)); // dau tuan = thu Hai
            tu = thuHai.congNgay(-7 * i);
            den = tu.congNgay(7);
            snprintf(buf, sizeof(buf), "Tuan %02d/%02d", tu.layNgay(), tu.layThang());
        }
        else if (ky == "quy")
        {
            int q = (hn.layThang() - 1) / 3 - i, y = hn.layNam();
            while (q < 0)
            {
                q += 4;
                --y;
            }
            tu = Ngay(1, q * 3 + 1, y);
            den = q == 3 ? Ngay(1, 1, y + 1) : Ngay(1, q * 3 + 4, y);
            snprintf(buf, sizeof(buf), "Q%d/%d", q + 1, y);
        }
        else if (ky == "nam")
        {
            int y = hn.layNam() - i;
            tu = Ngay(1, 1, y);
            den = Ngay(1, 1, y + 1);
            snprintf(buf, sizeof(buf), "%d", y);
        }
        else // thang
        {
            int m = hn.layThang() - i, y = hn.layNam();
            while (m <= 0)
            {
                m += 12;
                --y;
            }
            tu = Ngay(1, m, y);
            den = m == 12 ? Ngay(1, 1, y + 1) : Ngay(1, m + 1, y);
            snprintf(buf, sizeof(buf), "%02d/%d", m, y);
        }
        kq.push_back({buf, doanhThuTrongKhoang(tu.toTimeT(), den.toTimeT(), maCN)});
    }
    return kq;
}
