// ChuongTrinh.cpp - Khoi dong, doc/ghi du lieu, dang nhap, menu chinh, tai khoan, ham hien thi dung chung
#include "ChuongTrinh.h"
#include "InputHelper.h"
#include "TienIch.h"
#include <algorithm>
#include <filesystem>
#include <ctime>
#include <iostream>

using namespace std;
using namespace InputHelper;
using namespace UI;

static string thongBaoKhoiDong;

ChuongTrinh::ChuongTrinh(const string &thuMuc) : roleHienTai(Role::USER), thuMucDuLieu(thuMuc) {}

// =====================================================================
//                         DOC / GHI DU LIEU
// =====================================================================
// Nguyen tac bao ve du lieu:
//  - CHI doc tu cac file CSV hien co. Khong tao du lieu mau / du lieu rong thay the.
//  - Neu doc that bai (thieu file, dong loi, tai khoan trong): bao ro duong dan + nguyen nhan,
//    dung chuong trinh va KHONG ghi bat ky file nao (khoaGhi).
//  - Truoc lan ghi dau tien cua moi phien chay: sao luu toan bo *.csv vao data/backup/<thoi gian>/.
//  - Moi file CSV duoc ghi qua file tam roi doi ten (xem LuuTruFile.cpp).
bool ChuongTrinh::docDuLieu(string &baoCao)
{
    baoCao.clear();
    int loi = 0;
    bool coFile = false;
    string ngoaiLe;
    try
    {
        coFile = ks.docFile(thuMucDuLieu, loi);
    }
    catch (const exception &e)
    {
        ngoaiLe = e.what();
    }
    for (const auto &dong : ks.nhatKyDoc)
        baoCao += "  " + dong + "\n";
    if (!ngoaiLe.empty())
        baoCao += "  LOI NGOAI LE khi doc: " + ngoaiLe + "\n";

    const filesystem::path fileTKNhap = filesystem::path(thuMucDuLieu) / "taikhoan.csv";
    const filesystem::path fileTKCu = filesystem::path(thuMucDuLieu) / "taikhoan.txt";
    error_code ec;
    const filesystem::path fileTK = filesystem::exists(fileTKNhap, ec) ? fileTKNhap : fileTKCu;
    bool tkOk = false;
    if (!filesystem::exists(fileTK, ec))
        baoCao += "  taikhoan.csv: KHONG TIM THAY FILE " + fileTKNhap.string() + "\n";
    else if (!qlTaiKhoan.docFile(fileTK.string()))
        baoCao += "  " + fileTK.filename().string() + ": KHONG DOC DUOC TAI KHOAN NAO tu " + fileTK.string() + "\n";
    else
    {
        tkOk = true;
        baoCao += "  " + fileTK.filename().string() + ": doc " + to_string(qlTaiKhoan.layDanhSach().size()) +
                  " tai khoan\n";
    }
    return coFile && ngoaiLe.empty() && loi == 0 && tkOk;
}

bool ChuongTrinh::napDuLieu()
{
    string baoCao;
    if (!docDuLieu(baoCao))
    {
        khoaGhi = true; // khong ghi gi xuong dia trong phien nay
        baoCaoLoiKhoiDong = "KHONG NAP DUOC DU LIEU TU THU MUC:\n  " + thuMucDuLieu +
                            "\n\nChi tiet:\n" + baoCao +
                            "\nChuong trinh KHONG tao du lieu mau va KHONG ghi de bat ky file CSV nao.\n"
                            "Hay sua file/duong dan roi chay lai. Chi dinh thu muc: main <duong dan thu muc data> "
                            "hoac dat bien moi truong OCEANPEARL_DATA.\n";
        return false;
    }
    thongBaoKhoiDong = "Da doc du lieu tu '" + thuMucDuLieu + "' (" + to_string(ks.tatCaPhong().size()) +
                       " phong, " + to_string(ks.layDanhSachKhachHang().size()) + " khach, " +
                       to_string(ks.layDanhSachPhieu().size()) + " phieu, " +
                       to_string(ks.layDanhSachHoaDon().size()) + " hoa don).";

    // Khoi dong CHI DOC: khong tu them nhan vien mau (damBaoNhanSuMacDinh), khong tu tao / xoa tai khoan
    // (damBaoTaiKhoanNhanSu van chay khi quan tri sua nhan su). Chi dong bo trang thai theo ngay;
    // CSV chi duoc ghi khi co phieu bi tu dong huy (no-show) hoac khi nguoi dung them / sua / xoa du lieu.
    if (ks.dongBoTheoNgay() > 0)
        luuDuLieu();
    return true;
}

int ChuongTrinh::kiemTraDuLieu()
{
    string baoCao;
    const bool ok = docDuLieu(baoCao);
    cout << "Thu muc du lieu: " << thuMucDuLieu << "\n" << baoCao;
    cout << "Tong: " << ks.layDanhSachChiNhanh().size() << " chi nhanh, " << ks.tatCaPhong().size() << " phong, "
         << ks.tatCaNhanVien(true).size() << " nhan vien, " << qlTaiKhoan.layDanhSach().size() << " tai khoan, "
         << ks.layDanhSachKhachHang().size() << " khach hang, " << ks.layDanhSachDichVu().size() << " dich vu, "
         << ks.layDanhSachPhieu().size() << " phieu dat phong, " << ks.layDanhSachHoaDon().size() << " hoa don\n";
    cout << (ok ? "KET QUA: OK - khong co dong loi (khong ghi file nao)." : "KET QUA: CO LOI (xem chi tiet o tren).") << "\n";
    return ok ? 0 : 1;
}

void ChuongTrinh::damBaoTaiKhoanNhanSu()
{
    qlTaiKhoan.damBaoTaiKhoanNhanSu(ks.tatCaNhanVien());
}

void ChuongTrinh::saoLuuTruocKhiGhi()
{
    if (daSaoLuu)
        return;
    namespace fs = filesystem;
    error_code ec;
    if (!fs::exists(thuMucDuLieu, ec))
    {
        daSaoLuu = true; // chua co gi de sao luu
        return;
    }
    time_t now = time(nullptr);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y%m%d-%H%M%S", localtime(&now));
    fs::path dich = fs::path(thuMucDuLieu) / "backup" / buf;
    for (int i = 2; fs::exists(dich, ec); ++i)
        dich = fs::path(thuMucDuLieu) / "backup" / (string(buf) + "-" + to_string(i));
    fs::create_directories(dich);
    int soFile = 0;
    for (const auto &e : fs::directory_iterator(thuMucDuLieu))
    {
        if (!e.is_regular_file())
            continue;
        const string ext = e.path().extension().string();
        if (ext != ".csv" && ext != ".txt")
            continue;
        fs::copy_file(e.path(), dich / e.path().filename(), fs::copy_options::none);
        ++soFile;
    }
    (void)soFile;
    daSaoLuu = true;
}

void ChuongTrinh::luuDuLieu()
{
    // Dong bo toan bo du lieu ra CSV (chinhanh, phong, nhanvien, khachhang, dichvu,
    // phieudatphong, dichvusudung, hoadon, chitiethoadon, taikhoan).
    // Neu ghi that bai: bao loi ro, khong im lang de tranh hieu nham "da luu".
    if (khoaGhi)
    {
        loi("KHONG GHI FILE: du lieu CSV khong duoc nap thanh cong nen duoc bao ve, khong bi ghi de.");
        return;
    }
    try
    {
        saoLuuTruocKhiGhi(); // that bai -> khong ghi gi ca
        ks.ghiFile(thuMucDuLieu);
        qlTaiKhoan.ghiFile((filesystem::path(thuMucDuLieu) / "taikhoan.csv").string());
    }
    catch (const exception &e)
    {
        loi(string("LOI LUU FILE CSV: ") + e.what() + " — Du lieu trong bo nho van giu, nhung CHUA duoc ghi xuong dia.");
    }
}

bool ChuongTrinh::napLaiDuLieuTuFile(string *thongBaoLoi)
{
    // Doc lai CSV vao cung doi tuong ks / qlTaiKhoan — Console va Web luon nhin cung mot nguon.
    int soLoi = 0;
    string baoCao;
    bool okKs = ks.docFile(thuMucDuLieu, soLoi);
    const filesystem::path fileTK = filesystem::path(thuMucDuLieu) / "taikhoan.csv";
    bool okTk = qlTaiKhoan.docFile(fileTK.string());
    if (!okKs)
    {
        if (thongBaoLoi)
            *thongBaoLoi = "Khong doc duoc du lieu tu '" + thuMucDuLieu + "'.";
        return false;
    }
    if (!okTk && thongBaoLoi)
        *thongBaoLoi = "Canh bao: khong doc duoc taikhoan.csv (giu tai khoan trong bo nho neu co).";
    (void)baoCao;
    (void)soLoi;
    return true;
}

// =====================================================================
//                         KHUNG CHUONG TRINH
// =====================================================================
// Man hinh dau tien: chon chay Console hoac Web. Hai che do dung CHUNG doi tuong ks / qlTaiKhoan
// (cung logic nghiep vu C++, cung thu muc du lieu data/).
void ChuongTrinh::chay()
{
    khoiTaoConsole();
    batDauCanGiua(); // moi man hinh duoc ve thanh MOT khoi o giua cua so console
    if (!napDuLieu())
    {
        ketThucCanGiua();
        cleanupDoubleBuffer();
        cout << "\n" << baoCaoLoiKhoiDong << "\nNhan Enter de thoat...";
        cout.flush();
        string bo;
        getline(cin, bo);
        return;
    }
    try
    {
        while (true)
        {
            datChuDe(ChuDe::CHINH);
            xoaManHinh();
            inBanner();
            cout << "\n";
            int c = chonMenuTuongTac("CHON CHE DO CHAY",
                        "Hai che do dung CHUNG mot thu muc CSV (Single Source of Truth): " + thuMucDuLieu,
                        {"Chay tren CONSOLE", "Chay tren WEB (mo trinh duyet)"}, "Thoat");
            if (c == 0)
                break;
            // Truoc khi vao che do: ghi bo nho xuong dia, roi nap lai de khop file
            // (dam bao Console <-> Web luon thay cung du lieu, ke ca sau nhieu thao tac).
            luuDuLieu();
            {
                string loiNap;
                if (!napLaiDuLieuTuFile(&loiNap) && !loiNap.empty())
                    canhBao(loiNap);
            }
            if (c == 1)
                chayConsole();
            else
                chayWeb();
            // Sau khi thoat che do: ghi lai lan nua (Web/Console deu co the da thay doi bo nho).
            luuDuLieu();
        }
    }
    catch (const HetDuLieuNhap &e)
    {
        cout << "\n";
        canhBao(e.what());
    }
    luuDuLieu();
    datChuDe(ChuDe::CHINH);
    cout << "\n";
    thanhCong("Du lieu da duoc luu vao thu muc '" + thuMucDuLieu + "/'. Hen gap lai!");
    cout << "\n";
    ketThucCanGiua();
    cleanupDoubleBuffer();
}

void ChuongTrinh::chayConsole()
{
    while (dangNhap())
        hienThiMenuChinh();
}

void ChuongTrinh::thucHien(const function<void()> &hanhDong)
{
    try
    {
        hanhDong();
    }
    catch (const HetDuLieuNhap &)
    {
        throw;
    }
    catch (const HuyNhap &)
    {
        canhBao("Da huy thao tac.");
        return;
    }
    catch (const exception &e)
    {
        cout << "\n";
        loi(e.what());
    }
    luuDuLieu(); // moi thao tac deu duoc ghi xuong file ngay
    pauseScreen();
}

void ChuongTrinh::inThanhTrangThai() const
{
    string trai = "Ocean Pearl Resort";
    if (nvHienTai)
    {
        auto cn = ks.timChiNhanh(nvHienTai->layMaChiNhanh());
        trai += " | " + nvHienTai->layHoTen() + " (" + userHienTai + ")" + (cn ? " - " + cn->layTen() : "");
    }
    string phai = dinhDangThoiGian(time(nullptr));
    trai = catNgan(trai, RONG - doRong(phai) - 2); // khong de thanh trang thai tran dong
    // KHOI: thanh trang thai rong bang khung tieu de -> nam thang hang ngay tren khung, giua man hinh
    cout << KHOI << MO << trai << string((size_t)max(1, RONG - doRong(trai) - doRong(phai)), ' ') << phai << RESET
         << "\n";
}

void ChuongTrinh::manHinh(const string &tieu, const string &phu) const
{
    xoaManHinh();
    inThanhTrangThai();
    tieuDe(tieu, phu);
}

void ChuongTrinh::chayMenu(ChuDe cd, const string &tieu, const string &phu, const vector<MucMenu> &muc,
                           const function<void()> &noiDung, const string &phuHanhDong)
{
    vector<const MucMenu *> hien;
    for (auto &m : muc)
        if (!m.chiAdmin || laAdmin())
            hien.push_back(&m);
    vector<string> ten;
    for (auto m : hien)
        ten.push_back(m->ten);

    int chon = 0;
    while (true)
    {
        datChuDe(cd);

        auto veHeader = [&]() {
            xoaManHinh();
            inThanhTrangThai();
            // Luon in tieu de + phu de nguoi dung biet dang o man hinh nao
            tieuDe(tieu, phu);
            if (noiDung)
            {
                noiDung();
                cout << "\n";
            }
            else
                cout << "\n";
        };
        // Khi co noiDung (bang danh sach), van truyen tieu de de menu sequential co tieu de ro
        int c = chonMenuTuongTac(tieu, phu, ten, "Quay lai", {}, chon, veHeader);
        if (c == 0)
        {
            datChuDe(ChuDe::CHINH);
            return;
        }
        chon = c - 1;
        const MucMenu &m = *hien[(size_t)c - 1];
        if (m.laMenuCon)
        {
            m.hanhDong();
            continue;
        }
        manHinh(chuHoa(m.ten), phuHanhDong);
        thucHien(m.hanhDong);
    }
}

bool ChuongTrinh::chonChiNhanhManHinh(const string &tieu, shared_ptr<ChiNhanhResort> &kq, bool coTatCa,
                                      const string &phu)
{
    while (true)
    {
        const auto &ds = ks.layDanhSachChiNhanh(); // danh sach that cua he thong, khong hard-code
        vector<string> ten;
        for (auto &cn : ds)
            ten.push_back(tenChiNhanh(*cn));
        if (coTatCa)
            ten.push_back("Tat ca chi nhanh");
        xoaManHinh();
        inThanhTrangThai();
        cout << "\n";
        if (ds.empty())
        {
            tieuDe(tieu);
            canhBao("He thong chua co chi nhanh nao. Hay them chi nhanh truoc (menu Quan ly chi nhanh).");
            pauseScreen();
            return false;
        }
        int c = chonMenuTuongTac(tieu, phu, ten, "Quay lai");
        if (c == 0)
            return false;
        kq = (size_t)c <= ds.size() ? ds[(size_t)c - 1] : nullptr;
        return true;
    }
}

bool ChuongTrinh::dangNhap()
{
    datChuDe(ChuDe::CHINH);
    for (int lan = 1; lan <= 3; ++lan)
    {
        xoaManHinh();
        inBanner();
        tieuDe("DANG NHAP HE THONG");
        if (lan > 1)
            canhBao("Dang nhap sai. Con " + to_string(4 - lan) + " lan thu.");
        thongTin("Nhap tai khoan va mat khau. ESC de thoat.");
        cout << "\n";
        // Bat buoc nhap day du tai khoan + mat khau, khong chap nhan de trong
        string user = nhapTaiKhoan("Tai khoan : ", true, false);
        if (user == "\x1b")
            return false;
        string pass = nhapMatKhau("Mat khau  : ", true, false);
        if (pass == "\x1b")
            return false;
        try
        {
            const TaiKhoan &tk = qlTaiKhoan.dangNhap(user, pass);
            auto nv = ks.timNhanVien(tk.layMaNhanVien());
            if (!nv || !nv->layDangLam())
                throw runtime_error("Tai khoan khong gan voi nhan vien dang lam viec.");
            if (!chucVuKhopVaiTro(nv->layChucVu(), tk.layRole()))
                throw runtime_error("Tai khoan chi duoc dang nhap dung vai tro theo chuc vu nhan vien.");
            userHienTai = tk.layUsername();
            roleHienTai = tk.layRole();
            nvHienTai = nv;
            thongBaoKhoiDong.clear();
            thanhCong("Dang nhap thanh cong. Xin chao " + nv->layHoTen() + "!");
            pauseScreen();
            return true;
        }
        catch (const exception &e)
        {
            cout << "\n";
            loi(e.what());
            pauseScreen();
        }
    }
    loi("Dang nhap sai qua 3 lan. Chuong trinh se thoat.");
    return false;
}

// =====================================================================
//                              MENU CHINH
// =====================================================================
void ChuongTrinh::hienThiMenuChinh()
{
    struct Muc
    {
        string ten;
        ChuDe cd;
        bool chiAdmin;
        void (ChuongTrinh::*ham)();
    };
    // chiAdmin = true  : chi ADMIN (Quan ly) thay
    // chiAdmin = false : ca ADMIN lan USER (Le tan) deu thay
    const vector<Muc> tatCa = {
        {"Quan ly phong", ChuDe::PHONG, false, &ChuongTrinh::menuQuanLyPhong},
        {"Quan ly khach hang", ChuDe::KHACH_HANG, false, &ChuongTrinh::menuQuanLyKhachHang},
        {"Dat phong & Check-in", ChuDe::DAT_PHONG, false, &ChuongTrinh::menuDatPhongCheckIn},
        {"Thanh toan & Hoa don", ChuDe::THANH_TOAN, false, &ChuongTrinh::menuThanhToanHoaDon},
        {"Quan ly dich vu", ChuDe::DICH_VU, false, &ChuongTrinh::menuQuanLyDichVu},
        {"Quan ly chi nhanh & Nhan su", ChuDe::NHAN_SU, true, &ChuongTrinh::menuNhanSuChiNhanh},
        {"Thong ke & Bao cao", ChuDe::THONG_KE, true, &ChuongTrinh::menuThongKeBaoCao},
        {"Tai khoan", ChuDe::TAI_KHOAN, false, &ChuongTrinh::menuTaiKhoan} // Le tan cung can doi MK / xem TT
    };
    vector<const Muc *> hien;
    for (auto &m : tatCa)
        if (laAdmin() || !m.chiAdmin)
            hien.push_back(&m);

    while (true)
    {
        int huy = ks.dongBoTheoNgay();
        if (huy > 0)
            luuDuLieu();
        datChuDe(ChuDe::CHINH);
        xoaManHinh();
        inBanner();
        inThanhTrangThai();
        cout << "\n";

        auto ds = ks.tatCaPhong();
        int trong = 0, dangO = 0, denHomNay = 0;
        for (auto &p : ds)
        {
            if (p->layMaTrangThai() == MaTrangThai::TRONG)
                ++trong;
            else if (p->layMaTrangThai() == MaTrangThai::DANG_O)
                ++dangO;
        }
        for (auto &ph : ks.locPhieu(TrangThaiPhieu::DA_DAT))
            if (ph->layNgayNhan() == Ngay::homNay())
                ++denHomNay;
        // Tong quan nhanh: khung 2 hang x 2 cot, thang hang theo cot
        const int cotTrai = (RONG - 4) / 2;
        auto dongTQ = [&](const string &a, const string &mauA, const string &b, const string &mauB)
        {
            khungDong2(mauA + canTrai(a, cotTrai) + RESET + mauB + b, "", RONG);
        };
        khungTren(RONG, "TONG QUAN");
        dongTQ("Phong trong " + to_string(trong) + "/" + to_string(ds.size()), CHU, "Dang o " + to_string(dangO), CHU);
        dongTQ("Khach den hom nay " + to_string(denHomNay), CHU,
               "Hoa don chua thanh toan " + to_string(ks.soHoaDonChuaThanhToan()), CHU);
        size_t quaHan = ks.phieuQuaHan().size(), biChan = ks.phieuBiChanBoiKhachDangO().size();
        if (quaHan || biChan)
            dongTQ("Khach o qua han " + to_string(quaHan), HONG, "Phieu can chuyen phong " + to_string(biChan), HONG);
        khungDuoi(RONG);
        if (huy > 0)
            canhBao("Da tu dong huy " + to_string(huy) + " phieu qua ngay nhan phong ma khach khong den.");
        cout << "\n";

        vector<string> ten, mauSo;
        for (auto m : hien)
        {
            ten.push_back(m->ten);
            mauSo.push_back(mauCua(m->cd));
        }
        // Menu tuong tac: ↑↓ + Enter chon, ESC = dang xuat
        string phuMenu = laAdmin() ? "ADMIN - Day du chuc nang quan tri"
                                   : "LE TAN - Dat phong, khach hang, thanh toan, dich vu";
        int c = chonMenuTuongTac("QUAN LY KHACH SAN", phuMenu, ten, "Dang xuat", mauSo);
        if (c == 0)
            return;
        (this->*(hien[(size_t)c - 1]->ham))();
    }
}

// =====================================================================
//                         HIEN THI DUNG CHUNG
// =====================================================================
string ChuongTrinh::mauTrangThaiPhong(MaTrangThai tt)
{
    switch (tt)
    {
    case MaTrangThai::TRONG:
        return XANH_LA;
    case MaTrangThai::GIU_CHO:
        return VANG;
    case MaTrangThai::DANG_O:
        return XANH_TROI;
    case MaTrangThai::DANG_DON:
        return TIM;
    case MaTrangThai::BAO_TRI:
        return HONG;
    }
    return CHU;
}

string ChuongTrinh::mauTrangThaiPhieu(TrangThaiPhieu tt)
{
    switch (tt)
    {
    case TrangThaiPhieu::DA_DAT:
        return VANG;
    case TrangThaiPhieu::DA_NHAN_PHONG:
        return XANH_TROI;
    case TrangThaiPhieu::DA_TRA_PHONG:
        return XANH_LA;
    case TrangThaiPhieu::DA_HUY:
        return HONG;
    }
    return CHU;
}

string ChuongTrinh::tenPhong(const Phong &p) { return p.layMaPhong() + " - " + p.laySoPhong(); }

// Ten hien thi = thuong hieu + ten chi nhanh trong du lieu (khong hard-code danh sach chi nhanh)
string ChuongTrinh::tenChiNhanh(const ChiNhanhResort &cn)
{
    const string &t = cn.layTen();
    return chuHoa(t).rfind("OCEAN PEARL", 0) == 0 ? t : "Ocean Pearl Resort " + t;
}

string ChuongTrinh::tenChiNhanhCuaPhong(const Phong &p) const
{
    auto cn = ks.chiNhanhCuaPhong(p.layMaPhong());
    return cn ? tenChiNhanh(*cn) : p.layChiNhanh();
}

vector<shared_ptr<Phong>> ChuongTrinh::phongDangXem() const
{
    return cnDangChon ? cnDangChon->layDanhSachPhong() : ks.tatCaPhong();
}

static string ngayNgan(const Ngay &n) { return n.toString().substr(0, 5); } // dd/mm

string ChuongTrinh::lichGanNhat(const Phong &p) const
{
    if (auto ph = ks.phieuHienTaiCuaPhong(p.layMaPhong()))
        return ph->layMaPhieu() + " " + ph->layKhachHang()->layHoTen() + " den " + ngayNgan(ph->layNgayTra());
    if (auto ph = ks.phieuSapToiCuaPhong(p.layMaPhong()))
        return "Dat " + ngayNgan(ph->layNgayNhan()) + ": " + ph->layMaPhieu() + " " + ph->layKhachHang()->layHoTen();
    if (p.layMaTrangThai() == MaTrangThai::BAO_TRI)
        return p.ghiChuTrangThai();
    return "";
}

void ChuongTrinh::inBangPhong(const vector<shared_ptr<Phong>> &ds, bool hienChiNhanh) const
{
    // Danh sach gon: chi thong tin quan trong (chi tiet xem o "Xem chi tiet phong")
    if (!hienChiNhanh)
    {
        Bang b({{"Ma phong", 8, false},
                {"So phong", 8, false},
                {"Loai", 10, false},
                {"Suc chua", 8, true},
                {"Gia/dem", 12, true},
                {"Trang thai", 12, false}});
        for (auto &p : ds)
            b.themDong({p->layMaPhong(), p->laySoPhong(), loaiPhongToString(p->layLoaiPhong()),
                        to_string(p->laySucChua()), dinhDangTienVN(p->layGiaPhong()), p->layTenTrangThai()},
                       {DAM + CHU, "", "", MO, "", mauTrangThaiPhong(p->layMaTrangThai())});
        b.in("(Chi nhanh chua co phong nao)", false);
        return;
    }
    Bang b({{"Ma", 6, false},
            {"Chi nhanh", 12, false},
            {"So", 6, false},
            {"Loai", 10, false},
            {"Gia/dem", 12, true},
            {"SC", 4, true},
            {"Trang thai", 12, false}});
    for (auto &p : ds)
        b.themDong({p->layMaPhong(), p->layChiNhanh(), p->laySoPhong(), loaiPhongToString(p->layLoaiPhong()),
                    dinhDangTienVN(p->layGiaPhong()), to_string(p->laySucChua()), p->layTenTrangThai()},
                   {DAM + CHU, "", "", "", "", MO, mauTrangThaiPhong(p->layMaTrangThai())});
    b.in("(Khong co phong phu hop)", false);
}

void ChuongTrinh::inBangPhieu(const vector<shared_ptr<PhieuDatPhong>> &ds, const string &khiRong) const
{
    // Danh sach gon: ma, khach, phong, ngay, trang thai
    Bang b({{"Ma phieu", 8, false},
            {"Khach hang", 16, false},
            {"Phong", 10, false},
            {"Nhan", 10, false},
            {"Tra", 10, false},
            {"Dem", 4, true},
            {"Trang thai", 14, false}});
    for (auto &ph : ds)
    {
        auto kh = ph->layKhachHang();
        b.themDong({ph->layMaPhieu(), kh->layHoTen(), tenPhong(*ph->layPhong()),
                    ph->layNgayNhan().toString(), ph->layNgayTra().toString(),
                    to_string(ph->laySoDem()), trangThaiPhieuToString(ph->layTrangThai())},
                   {DAM + CHU, KHACH, "", "", "", MO, mauTrangThaiPhieu(ph->layTrangThai())});
    }
    b.in(khiRong);
}

void ChuongTrinh::inChiTietPhieu(const shared_ptr<PhieuDatPhong> &ph) const
{
    auto kh = ph->layKhachHang();
    auto p = ph->layPhong();
    vector<pair<string, string>> dong = {
        {"Trang thai", trangThaiPhieuToString(ph->layTrangThai())},
        {"Khach hang", kh->layHoTen() + " (" + kh->layMa() + ") - " + kh->laySoDienThoai() + " - hang " +
                           kh->hangThanhVien()},
        {"Chi nhanh", p->layChiNhanh()},
        {"Phong", tenPhong(*p) + " (" + loaiPhongToString(p->layLoaiPhong()) + ")"},
        {"Ngay o", ph->layNgayNhan().toString() + " -> " + ph->layNgayTra().toString() + "  (" +
                       to_string(ph->laySoDem()) + " dem, " + to_string(ph->laySoNguoi()) + " khach)"},
        {"Mua", muaToString(ph->layMua()) + " (x" + dinhDangSoThuc(heSoMua(ph->layMua()), 1) + ")"},
        {"Dat luc", dinhDangThoiGian(ph->layThoiGianDat()) + " - NV " + ph->layNhanVien()->layHoTen()},
        {"Tien coc", ph->layTienCoc() > 0 ? dinhDangTien(ph->layTienCoc()) + " VND (" + ph->layPhuongThucCoc() + ")" +
                                                 (ph->layDaHoanCoc() ? " - da hoan" : "")
                                           : "Khong coc"}};
    if (ph->layThoiGianCheckIn())
        dong.push_back({"Check-in", dinhDangThoiGian(ph->layThoiGianCheckIn())});
    if (ph->layThoiGianCheckOut())
        dong.push_back({"Check-out", dinhDangThoiGian(ph->layThoiGianCheckOut()) + "  (hoa don " +
                                         ph->layMaHoaDon() + ")"});
    if (ph->layTrangThai() == TrangThaiPhieu::DA_HUY)
        dong.push_back({"Da huy", dinhDangThoiGian(ph->layThoiGianHuy()) + " - " + ph->layLyDoHuy()});
    if (!ph->layGhiChu().empty())
        dong.push_back({"Ghi chu", ph->layGhiChu()});
    inKhoi("PHIEU " + ph->layMaPhieu(), dong);
    cout << "\n";
    dongTien("Tien phong", ph->tinhTienPhong(), DAM + CHU, 60);
    dongTien("Tien dich vu", ph->tinhTienDichVu(), DAM + CHU, 60);
}

void ChuongTrinh::inLichSu(const vector<shared_ptr<PhieuDatPhong>> &ds) const
{
    // Bang rong, de doc: uu tien hien du ten khach / chi nhanh / trang thai
    Bang b({{"Ma phieu", 8, false},
            {"Khach hang", 20, false},
            {"Chi nhanh", 16, false},
            {"Phong", 12, false},
            {"Ngay dat", 16, false},
            {"Ngay nhan", 12, false},
            {"Ngay tra", 12, false},
            {"Trang thai", 14, false}});
    for (auto &ph : ds)
    {
        string nhan, tra, mauNhan = CHU, mauTra = CHU;
        if (ph->layThoiGianCheckIn())
            nhan = dinhDangThoiGianNgan(ph->layThoiGianCheckIn());
        else
        {
            nhan = ph->layNgayNhan().toString(); // dd/mm/yyyy day du
            mauNhan = MO;
        }
        if (ph->layThoiGianCheckOut())
            tra = dinhDangThoiGianNgan(ph->layThoiGianCheckOut());
        else
        {
            tra = ph->layNgayTra().toString();
            mauTra = MO;
        }
        if (ph->layTrangThai() == TrangThaiPhieu::DA_HUY)
        {
            nhan = tra = "-";
            mauNhan = mauTra = MO;
        }
        b.themDong({ph->layMaPhieu(), ph->layKhachHang()->layHoTen(), ph->layPhong()->layChiNhanh(),
                    tenPhong(*ph->layPhong()), dinhDangThoiGianNgan(ph->layThoiGianDat()), nhan, tra,
                    trangThaiPhieuToString(ph->layTrangThai())},
                   {DAM + CHU, "", "", "", MO, mauNhan, mauTra, mauTrangThaiPhieu(ph->layTrangThai())});
    }
    b.in("(Chua co lich su)");
}

void ChuongTrinh::inBangKhachHang(const vector<shared_ptr<KhachHang>> &ds) const
{
    // Danh sach gon; chi tiet day du khi xem chi tiet khach
    Bang b({{"Ma", 6, false},
            {"Ho ten", 18, false},
            {"SDT", 12, false},
            {"Hang", 12, false},
            {"Diem", 8, true},
            {"Email", 22, false}});
    for (auto &kh : ds)
        b.themDong({kh->layMa(), kh->layHoTen(), kh->laySoDienThoai(),
                    kh->hangThanhVien(), to_string(kh->layDiemTichLuy()),
                    kh->layEmail().empty() ? "-" : kh->layEmail()},
                   vector<string>(6, KHACH));
    b.mauTieuDe = KHACH;
    b.in("(Khong co khach hang)");
}

vector<pair<string, string>> ChuongTrinh::thongTinChiTietKhachHang(const KhachHang &kh) const
{
    auto info = kh.layThongTin();
    // Hien thi chi nhanh: uu tien maChiNhanh cua khach; neu trong thi dung chi nhanh dang chon (cnDangChon)
    // Khong ghi de maChiNhanh trong ho so — chi hien thi dung ngu canh.
    {
        string hienThiCN;
        if (!kh.layMaChiNhanh().empty())
        {
            if (kh.layMaChiNhanh() == "CN01") hienThiCN = "Nha Trang (CN01)";
            else if (kh.layMaChiNhanh() == "CN02") hienThiCN = "Quy Nhon (CN02)";
            else if (auto cn = ks.timChiNhanh(kh.layMaChiNhanh()))
                hienThiCN = tenChiNhanh(*cn) + " (" + cn->layMa() + ")";
            else
                hienThiCN = kh.layMaChiNhanh();
        }
        else if (cnDangChon)
            hienThiCN = tenChiNhanh(*cnDangChon) + " (" + cnDangChon->layMa() + ")";
        else
            hienThiCN = "Chua gan chi nhanh";

        bool daThay = false;
        for (auto &p : info)
            if (p.first == "Chi nhanh")
            {
                p.second = hienThiCN;
                daThay = true;
                break;
            }
        if (!daThay)
            info.push_back({"Chi nhanh", hienThiCN});
    }

    vector<string> phongDaO, chiNhanhDaO;
    for (const auto &ph : kh.layLichSuDatPhong())
    {
        if (ph->layTrangThai() != TrangThaiPhieu::DA_TRA_PHONG)
            continue;
        const auto phong = ph->layPhong();
        const string tenPhong = phong->layChiNhanh() + " - " + phong->laySoPhong();
        const string chiNhanh = phong->layChiNhanh();
        if (find(phongDaO.begin(), phongDaO.end(), tenPhong) == phongDaO.end())
            phongDaO.push_back(tenPhong);
        if (find(chiNhanhDaO.begin(), chiNhanhDaO.end(), chiNhanh) == chiNhanhDaO.end())
            chiNhanhDaO.push_back(chiNhanh);
    }
    double tongChi = 0;
    for (const auto &hd : ks.layDanhSachHoaDon())
        if (hd->layDaThanhToan() && hd->layPhieu()->layKhachHang()->layMa() == kh.layMa())
            tongChi += hd->tongThanhToan();
    auto danhSach = [](const vector<string> &ds)
    {
        string kq;
        for (const auto &x : ds)
            kq += (kq.empty() ? "" : ", ") + x;
        return kq.empty() ? string("Chua co lich su") : kq;
    };
    info.push_back({"Loai khach hang", kh.layHang() == HangThanhVien::THANH_VIEN ? "Thuong" : "VIP - " + kh.hangThanhVien()});
    info.push_back({"So phong da o", to_string(phongDaO.size())});
    info.push_back({"Phong / chi nhanh", danhSach(phongDaO)});
    info.push_back({"Chi nhanh da luu tru", danhSach(chiNhanhDaO)});
    info.push_back({"Tong chi tieu", dinhDangTienVN(tongChi)});
    return info;
}

void ChuongTrinh::inBangNhanVien(const vector<shared_ptr<NhanVien>> &ds) const
{
    // Ma NV | Ho ten | Chuc vu | Chi nhanh | Luong
    Bang b({{"Ma NV", 8, false},
            {"Ho ten", 20, false},
            {"Chuc vu", 14, false},
            {"Chi nhanh", 14, false},
            {"Luong", 14, true}});
    for (const auto &nv : ds)
    {
        string tenCN = "-";
        if (auto cn = ks.timChiNhanh(nv->layMaChiNhanh()))
            tenCN = cn->layTen();
        b.themDong({nv->layMa(), nv->layHoTen(), nv->layChucVu(), tenCN, dinhDangTienVN(nv->layLuongCoBan())});
    }
    b.in("(Khong co nhan vien)");
}

void ChuongTrinh::inBangDichVu(const vector<shared_ptr<DichVu>> &ds) const
{
    // Danh sach gon: ma, ten, loai, don gia
    Bang b({{"Ma", 6, false},
            {"Ten dich vu", 24, false},
            {"Loai", 10, false},
            {"Don gia", 14, true},
            {"Don vi", 10, false}});
    for (auto &dv : ds)
        b.themDong({dv->layMa(), dv->layTen(), dv->layLoai(), dinhDangTienVN(dv->layDonGia()), dv->layDonViTinh()},
                   {DAM + CHU, "", "", "", MO});
    b.in("(Chua co dich vu)");
}

void ChuongTrinh::inBangHoaDon(const vector<shared_ptr<HoaDon>> &ds, bool hienChiNhanh) const
{
    if (!hienChiNhanh) // HOA DON - <chi nhanh>: Ma HD | Khach hang | Ngay | Tong tien | Trang thai
    {
        Bang b({{"Ma HD", 5, false},
                {"Khach hang", 14, false},
                {"Phong", 7, false},
                {"Ngay", 9, false},
                {"Tong tien", 12, true},
                {"Trang thai", 13, false}});
        for (auto &hd : ds)
        {
            auto ph = hd->layPhieu();
            time_t t = hd->layDaThanhToan() ? hd->layThoiGianThanhToan() : hd->layThoiGianLap();
            b.themDong({hd->layMa(), ph->layKhachHang()->layHoTen(), tenPhong(*ph->layPhong()), Ngay::tuTimeT(t).toString(),
                        dinhDangTienVN(hd->tongThanhToan()),
                        hd->layDaThanhToan() ? hd->layPhuongThuc() : "Chua thanh toan"},
                       {DAM + CHU, "", MO, "", DAM + CHU, hd->layDaThanhToan() ? XANH_LA : VANG});
        }
        b.in("(Chi nhanh chua co hoa don)");
        return;
    }
    Bang b({{"Ma HD", 5, false},
            {"Ma phieu", 7, false},
            {"Khach hang", 14, false},
            {"Chi nhanh", 9, false},
            {"Phong", 7, false},
            {"Ngay lap", 12, false},
            {"Tong tien", 12, true},
            {"Trang thai", 13, false}});
    for (auto &hd : ds)
    {
        auto ph = hd->layPhieu();
        b.themDong({hd->layMa(), ph->layMaPhieu(), ph->layKhachHang()->layHoTen(), ph->layPhong()->layChiNhanh(),
                    tenPhong(*ph->layPhong()), dinhDangThoiGianNgan(hd->layThoiGianLap()),
                    dinhDangTienVN(hd->tongThanhToan()), hd->layDaThanhToan() ? hd->layPhuongThuc() : "Chua thanh toan"},
                   {DAM + CHU, MO, "", "", "", MO, DAM + CHU, hd->layDaThanhToan() ? XANH_LA : VANG});
    }
    b.in("(Chua co hoa don)");
}

void ChuongTrinh::inBangChiNhanh() const
{
    Bang b({{"Ma", 6, false},
            {"Chi nhanh", 22, false},
            {"Hotline", 12, false},
            {"Phong", 6, true},
            {"Trong", 6, true},
            {"Nhan vien", 10, true}});
    for (auto &cn : ks.layDanhSachChiNhanh())
    {
        int nv = 0;
        for (auto &x : cn->layDanhSachNhanVien())
            if (x->layDangLam())
                ++nv;
        b.themDong({cn->layMa(), tenChiNhanh(*cn), cn->layHotline(),
                    to_string(cn->layDanhSachPhong().size()), to_string(cn->demPhong(MaTrangThai::TRONG)),
                    to_string(nv)},
                   {DAM + CHU, DAM + CHU, "", "", XANH_LA, ""});
    }
    b.in();
}

// Hoa don: khung ASCII, noi bat 2 khoan TIEN PHONG va TIEN DICH VU
void ChuongTrinh::inHoaDon(const HoaDon &hd) const
{
    const int W = RONG_KHOI;
    auto ph = hd.layPhieu();
    auto kh = ph->layKhachHang();
    auto p = ph->layPhong();
    bool tamTinh = hd.layMa() == "TAM TINH";

    string trai = tamTinh ? "TAM TINH" : "HOA DON " + hd.layMa();
    string phai = tamTinh ? "(chua thanh toan)" : (hd.layDaThanhToan() ? "Da thanh toan" : "Chua thanh toan");
    cout << "\n";
    khungTren(W, "OCEAN PEARL RESORT");
    khungDong2(trai, phai, W, DAM + CHU, hd.layDaThanhToan() ? XANH_LA : VANG);
    khungNgang(W);
    khungNhan("Khach hang", kh->layHoTen() + " (" + kh->layMa() + ")", W);
    khungNhan("Phong", tenPhong(*p) + " " + loaiPhongToString(p->layLoaiPhong()) + "  -  chi nhanh " + p->layChiNhanh(), W);
    khungNhan("Luu tru", ph->layNgayNhan().toString() + " -> " + ph->layNgayTra().toString() + "  (" +
                             to_string(ph->laySoDem()) + " dem)", W);
    khungNgang(W);

    // ---- 1. Tien phong ----
    khungTien("1. TIEN PHONG", hd.tienPhong(), W, DAM + CARAMEL);
    for (auto &ct : hd.layChiTiet())
        if (ct.layLoai() == "Phong")
            for (auto &d : ngatDong(to_string(ct.laySoLuong()) + " dem x " + dinhDangTien(ct.layDonGia()) + "  (" +
                                        ct.layNoiDung() + ")", W - 10))
                khungDong("     " + d, W, MO);

    // ---- 2. Tien dich vu ----
    khungDong("", W);
    khungTien("2. TIEN DICH VU", hd.tienDichVu(), W, DAM + HONG);
    bool coDV = false;
    for (auto &ct : hd.layChiTiet())
        if (ct.layLoai() != "Phong")
        {
            coDV = true;
            string trai2 = "     " + canTrai(ct.layNoiDung(), 30) + "  " + canPhai(to_string(ct.laySoLuong()), 3) + " x " +
                           dinhDangTien(ct.layDonGia());
            khungDong2(trai2, dinhDangTien(ct.layThanhTien()), W, MO, MO);
        }
    if (!coDV)
        khungDong("     (khong su dung dich vu)", W, MO);

    // ---- Tong ket ----
    khungNgang(W);
    khungTien("Tam tinh", hd.tamTinh(), W, CHU);
    if (hd.tienGiam() > 0)
        khungTien("Giam gia thanh vien " + to_string((int)(hd.layTyLeGiamGia() * 100 + 0.5)) + "%", hd.tienGiam(), W,
                  XANH_LA, "-");
    khungTien("VAT 10%", hd.tienThue(), W, CHU, "+");
    khungTien("TONG CONG", hd.tongThanhToan(), W, DAM + CHU);
    if (hd.layTienCoc() > 0)
        khungTien("Da dat coc", hd.layTienCoc(), W, XANH_LA, "-");
    khungNgang(W);
    khungTien("CON PHAI THANH TOAN", hd.conLaiPhaiTra(), W, DAM + CHU);
    if (hd.tienCocDu() > 0)
        khungTien("COC DU - HOAN LAI CHO KHACH", hd.tienCocDu(), W, DAM + XANH_LA);
    if (hd.layDaThanhToan())
        for (auto &d : ngatDong("Thanh toan " + hd.layPhuongThuc() + " luc " + dinhDangThoiGian(hd.layThoiGianThanhToan()) +
                                    (hd.layNhanVienLap() ? "  -  NV " + hd.layNhanVienLap()->layHoTen() : ""), W - 4))
            khungDong(d, W, MO);
    khungDuoi(W);
    if (!hd.khopVoiDecorator()) // chi bao khi co sai lech (binh thuong khong hien)
        loi("Canh bao: tong tinh qua Decorator (" + dinhDangTien(hd.layTongTheoDecorator()) + ") khong khop!");
}

// =====================================================================
//                          CHON DOI TUONG
// =====================================================================
shared_ptr<Phong> ChuongTrinh::chonPhong(const vector<shared_ptr<Phong>> &ds, const string &nhan)
{
    while (true)
    {
        string s = nhapMa(nhan, "P");
        if (s == "0")
            return nullptr;
        auto p = ks.timPhong(s);
        if (!p)
        {
            loi("Khong co ma nay! Hay nhap lai.");
            continue;
        }
        if (find(ds.begin(), ds.end(), p) == ds.end())
        {
            loi("Phong " + p->layMaPhong() + " khong nam trong danh sach o tren. Hay nhap lai.");
            continue;
        }
        return p;
    }
}

shared_ptr<PhieuDatPhong> ChuongTrinh::chonPhieu(const vector<shared_ptr<PhieuDatPhong>> &ds, const string &nhan)
{
    inBangPhieu(ds);
    if (ds.empty())
        return nullptr;
    while (true)
    {
        string s = nhapMa(nhan, "DP");
        if (s == "0")
            return nullptr;
        auto ph = ks.timPhieu(s);
        if (ph && find(ds.begin(), ds.end(), ph) != ds.end())
            return ph;
        loi(ph ? "Phieu " + ph->layMaPhieu() + " khong nam trong danh sach tren. Hay nhap lai."
               : "Khong co ma nay! Hay nhap lai.");
    }
}

shared_ptr<KhachHang> ChuongTrinh::chonKhachHang()
{
    while (true)
    {
        string s = nhapDongCoEsc("Nhap SDT / CCCD / ten / ma khach: ");
        if (s == "\x1b")
            return nullptr;
        if (s.empty())
        {
            loi("Loi: Khong duoc de trong! Nhap SDT, CCCD, ten hoac ma khach.");
            continue;
        }
        auto ds = ks.timKhachTheoTuKhoa(s);
        if (ds.empty())
        {
            loi("Khong tim thay khach hang phu hop.");
            continue;
        }
        if (ds.size() == 1)
        {
            thongTin(KHACH + "Da chon: " + ds[0]->layHoTen() + " (" + ds[0]->layMa() + ") - " + ds[0]->laySoDienThoai());
            return ds[0];
        }
        inBangKhachHang(ds);
        while (true)
        {
            string ma = nhapMa("Nhap ma khach trong danh sach: ", "KH");
            if (ma == "0")
                break;
            auto kh = ks.timKhachHang(ma);
            if (kh && find(ds.begin(), ds.end(), kh) != ds.end())
                return kh;
            loi("Ma khach khong co trong danh sach.");
        }
    }
}

shared_ptr<KhachHang> ChuongTrinh::nhapKhachHangMoi()
{
    PhamViHuyBangEsc choPhepHuy;
    string ten = nhapHoTen("Ho ten: ");
    string sdt = nhapSoDienThoai("So dien thoai: ");
    if (auto kh = ks.timKhachTheoSDT(sdt))
    {
        canhBao("SDT nay da thuoc khach " + kh->layHoTen() + " (" + kh->layMa() + ").");
        return nhapCoKhong("Dung khach hang nay?") ? kh : nullptr;
    }
    string cccd = nhapCCCD("CCCD (12 so) / Ho chieu: ");
    if (auto kh = ks.timKhachTheoCCCD(cccd))
    {
        canhBao("CCCD/Ho chieu nay da thuoc khach " + kh->layHoTen() + " (" + kh->layMa() + ").");
        return nhapCoKhong("Dung khach hang nay?") ? kh : nullptr;
    }
    string email = nhapEmail("Email: ");
    string qt = nhapDong("Quoc tich: ");
    while (!qt.empty() && !laChuThuan(qt))
    {
        UI::ghiTrucTiep(string("  ") + UI::DO + "[!] Quoc tich chi gom chu cai." + UI::RESET + "\n");
        qt = nhapDong("Quoc tich: ");
    }
    const string gioiTinh = nhapGioiTinh();
    string diaChi;
    while (true)
    {
        diaChi = nhapDong("Dia chi thuong tru: ");
        if (diaChi.empty() || laDiaChi(diaChi))
            break;
        UI::ghiTrucTiep(string("  ") + UI::DO + "[!] Dia chi khong hop le! Cho phep chu, so va , . / - ( ) #, dai 5-120 ky tu." + UI::RESET + "\n");
    }
    if (!xacNhanEnterEsc("CO CHAC MUON THEM KHONG?"))
    {
        canhBao("Da huy them khach hang.");
        return nullptr;
    }
    auto kh = ks.themKhachHang(ten, sdt, cccd, email, qt.empty() ? "Viet Nam" : qt, diaChi, gioiTinh);
    // Tu dong gan chi nhanh theo ngu canh dang thao tac (cn dang chon hoac chi nhanh cua NV dang nhap)
    // Luu that vao ho so (maChiNhanh), khong chi sua chuoi hien thi.
    string maCN;
    if (cnDangChon)
        maCN = cnDangChon->layMa();
    else if (nvHienTai && !nvHienTai->layMaChiNhanh().empty())
        maCN = nvHienTai->layMaChiNhanh();
    if (!maCN.empty())
    {
        kh->datMaChiNhanh(maCN);
        if (auto cn = ks.timChiNhanh(maCN))
            thongTin("Da gan chi nhanh: " + tenChiNhanh(*cn) + " (" + maCN + ").");
    }
    thanhCong("Da tao khach hang " + kh->layMa() + " - " + kh->layHoTen() + ".");
    return kh;
}

shared_ptr<KhachHang> ChuongTrinh::chonHoacTaoKhachHang()
{
    const vector<string> luaChon = {
        "Khach cu (tim theo SDT / CCCD / ten / ma)",
        "Khach moi"};
    int c = chonMenuTuongTac("CHON KHACH HANG", "ESC de quay lai", luaChon, "Quay lai");
    if (c == 1)
        return chonKhachHang();
    if (c == 2)
        return nhapKhachHangMoi();
    return nullptr;
}

shared_ptr<DichVu> ChuongTrinh::chonDichVu()
{
    inBangDichVu(ks.layDanhSachDichVu());
    if (ks.layDanhSachDichVu().empty())
        return nullptr;
    while (true)
    {
        // An dong "ESC: Quay lai" duoi o nhap ma (tranh thua dong); ESC van huy duoc
        string s = nhapMa("Nhap ma dich vu: ", "DV", true, false);
        if (s == "0")
            return nullptr;
        if (auto dv = ks.timDichVu(s))
            return dv;
        loi("Loi: Ma dich vu " + chuanHoaMa(s) + " khong ton tai. Vui long nhap lai.");
    }
}

// Chon chi nhanh trong man hinh hien tai: danh sach danh so lay tu du lieu that.
// choPhepTatCa: muc cuoi "Tat ca chi nhanh" -> nullptr; ESC dung de quay lai.
shared_ptr<ChiNhanhResort> ChuongTrinh::chonChiNhanh(bool choPhepTatCa)
{
    const auto &ds = ks.layDanhSachChiNhanh();
    if (ds.empty())
    {
        canhBao("He thong chua co chi nhanh nao.");
        return nullptr;
    }
    vector<string> ten;
    for (auto &cn : ds)
        ten.push_back(tenChiNhanh(*cn) + "  (" + cn->layMa() + ")");
    if (choPhepTatCa)
        ten.push_back("Tat ca chi nhanh");
    int c = chonMenuTuongTac("CHON CHI NHANH", "", ten, "Huy");
    if (c == 0 || (size_t)c > ds.size())
        return nullptr;
    return ds[(size_t)c - 1];
}

shared_ptr<HoaDon> ChuongTrinh::chonHoaDon(const vector<shared_ptr<HoaDon>> &ds)
{
    inBangHoaDon(ds);
    if (ds.empty())
        return nullptr;
    while (true)
    {
        string s = nhapMa("Nhap ma hoa don: ", "HD");
        if (s == "0")
            return nullptr;
        auto hd = ks.timHoaDon(s);
        if (hd && find(ds.begin(), ds.end(), hd) != ds.end())
            return hd;
        loi(hd ? "Loi: Hoa don " + hd->layMa() + " khong nam trong danh sach tren (thuoc chi nhanh / trang thai khac)."
               : "Loi: Ma hoa don " + chuanHoaMa(s) + " khong ton tai. Vui long nhap lai.");
    }
}

string ChuongTrinh::chonPhuongThuc(const string &mucThoat)
{
    static const vector<string> pt = {"Tien mat", "Chuyen khoan", "The tin dung", "Vi dien tu"};
    cout << "\n";
    int c = chonMenuTuongTac("", "", pt, mucThoat);
    return c == 0 ? "" : pt[(size_t)c - 1];
}

// =====================================================================
//                               TAI KHOAN
// =====================================================================
void ChuongTrinh::menuTaiKhoan()
{
    // Muc ca nhan: ca Le tan lan Quan ly deu dung duoc
    // Muc quan tri: chi ADMIN
    chayMenu(ChuDe::TAI_KHOAN, "TAI KHOAN",
             "Thong tin ca nhan + quan tri (ADMIN). ESC / Quay lai de thoat.",
             {{"Thong tin tai khoan cua toi", false, [this]
               { xemTaiKhoanCuaToi(); }},
              {"Doi mat khau", false, [this]
               { doiMatKhau(); }},
              {"Danh sach tai khoan", true, [this]
               { xemDanhSachTaiKhoan(); }},
              {"Tao tai khoan cho nhan vien", true, [this]
               { taoTaiKhoan(); }},
              {"Xoa tai khoan", true, [this]
               { xoaTaiKhoan(); }}});
}

void ChuongTrinh::xemTaiKhoanCuaToi()
{
    vector<pair<string, string>> dong = {{"Username", userHienTai}, {"Quyen", roleToString(roleHienTai)}};
    for (auto &x : nvHienTai->layThongTin())
        dong.push_back(x);
    inKhoi("", dong);
}

void ChuongTrinh::doiMatKhau()
{
    // choPhepEsc=true: ESC huy o moi lan nhap (ke ca sau khi de trong / sai dinh dang)
    string cu = nhapMatKhau("Mat khau hien tai: ", true);
    if (cu == "\x1b")
    {
        canhBao("Da huy doi mat khau.");
        return;
    }
    string moi = nhapMatKhau("Mat khau moi (6-32 ky tu): ", true);
    if (moi == "\x1b")
    {
        canhBao("Da huy doi mat khau.");
        return;
    }
    if (moi.size() < 6 || moi.size() > 32)
        throw runtime_error("Mat khau moi phai tu 6 den 32 ky tu.");
    string lai = nhapMatKhau("Nhap lai mat khau moi: ", true);
    if (lai == "\x1b")
    {
        canhBao("Da huy doi mat khau.");
        return;
    }
    if (moi != lai)
        throw runtime_error("Mat khau nhap lai khong khop.");
    qlTaiKhoan.doiMatKhau(userHienTai, cu, moi);
    thanhCong("Doi mat khau thanh cong.");
}

void ChuongTrinh::xemDanhSachTaiKhoan()
{
    Bang b({{"Username", 14, false}, {"Quyen", 16, false}, {"Nhan vien", 26, false}, {"Chuc vu", 12, false}});
    for (auto &tk : qlTaiKhoan.layDanhSach())
    {
        auto nv = ks.timNhanVien(tk.layMaNhanVien());
        b.themDong({tk.layUsername(), roleToString(tk.layRole()),
                    nv ? nv->layMa() + " - " + nv->layHoTen() : tk.layMaNhanVien(), nv ? nv->layChucVu() : "-"},
                   {DAM + CHU, CHU, "", MO});
    }
    b.in();
}

void ChuongTrinh::taoTaiKhoan()
{
    vector<shared_ptr<NhanVien>> chuaCo;
    for (auto &nv : ks.tatCaNhanVien())
        if (!qlTaiKhoan.timTheoNhanVien(nv->layMa()) &&
            (laChucVuQuanLy(nv->layChucVu()) || laChucVuLeTan(nv->layChucVu())))
            chuaCo.push_back(nv);
    if (chuaCo.empty())
    {
        canhBao("Khong co quan ly hoac le tan nao can tao tai khoan.");
        return;
    }
    tieuDeNho("Nhan vien chua co tai khoan");
    inBangNhanVien(chuaCo);
    shared_ptr<NhanVien> nv;
    while (!nv)
    {
        string ma = nhapMa("Ma nhan vien: ", "NV");
        if (ma == "0")
            return;
        auto x = ks.timNhanVien(ma);
        if (x && find(chuaCo.begin(), chuaCo.end(), x) != chuaCo.end())
            nv = x;
        else
            loi("Ma nhan vien khong hop le.");
    }
    string user, pass;
    while (true)
    {
        user = nhapTaiKhoan("Username (3-20 ky tu): ", true);
        if (user == "\x1b")
        {
            canhBao("Da huy tao tai khoan.");
            return;
        }
        if (!laUsername(user))
        {
            loi("Username chi gom chu, so, dau _ . va dai 3-20 ky tu.");
            continue;
        }
        if (qlTaiKhoan.timTheoUsername(user))
        {
            loi("Username '" + user + "' da ton tai.");
            continue;
        }
        break;
    }
    while (true)
    {
        pass = nhapMatKhau("Mat khau (6-32 ky tu): ", true);
        if (pass == "\x1b")
        {
            canhBao("Da huy tao tai khoan.");
            return;
        }
        if (pass.size() < 6 || pass.size() > 32)
        {
            loi("Mat khau phai tu 6 den 32 ky tu.");
            continue;
        }
        break;
    }
    Role r = laChucVuQuanLy(nv->layChucVu()) ? Role::ADMIN : Role::USER;
    qlTaiKhoan.themTaiKhoan(user, pass, r, nv->layMa());
    thanhCong("Da tao tai khoan '" + user + "' cho " + nv->layHoTen() + ".");
}

void ChuongTrinh::xoaTaiKhoan()
{
    xemDanhSachTaiKhoan();
    cout << "\n";
    thongTin("Nhap username can xoa. ESC de huy.");
    string user = nhapTaiKhoan("Username can xoa: ", true);
    if (user == "\x1b")
        return;
    if (user == userHienTai)
        throw runtime_error("Khong the xoa tai khoan dang dang nhap.");
    if (!nhapCoKhong("Xac nhan xoa tai khoan '" + user + "'?"))
        return;
    qlTaiKhoan.xoaTaiKhoan(user);
    thanhCong("Da xoa tai khoan '" + user + "'.");
}
