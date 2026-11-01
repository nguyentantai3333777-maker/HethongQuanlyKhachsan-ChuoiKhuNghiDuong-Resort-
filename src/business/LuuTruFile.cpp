// LuuTruFile.cpp - Doc/ghi toan bo du lieu QuanLyKhachSan ra file CSV (UTF-8)
// Dinh dang: dong dau la tieu de cot (CSV), moi dong sau la 1 ban ghi, phan tach bang dau phay.
// Ky tu dac biet (, " xuong dong) duoc escape dung chuan CSV.
// Doc: uu tien file .csv; neu chua co thi doc ban .txt cu (tuong thich nguoc).
// Ghi: luon ghi .csv (chinhanh, nhanvien, phong, dichvu, khachhang, phieudatphong,
//      dichvusudung, hoadon, chitiethoadon). Tai khoan: taikhoan.csv (QuanLyTaiKhoan).
#include "QuanLyKhachSan.h"
#include "InputHelper.h"
#include "PhongCuThe.h"
#include "TrangThaiCuThe.h"
#include <cstdlib>
#include <map>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

using namespace std;
namespace fs = std::filesystem;

namespace
{
    // ---------------- Tien ich doc/ghi ----------------
    string csvEscaped(string s)
    {
        for (char &c : s)
            if (c == '\n' || c == '\r')
                c = ' ';
        const bool quote = s.find_first_of(",\"") != string::npos;
        if (!quote)
            return s;
        string out = "\"";
        for (char c : s)
        {
            if (c == '"')
                out += "\"\"";
            else
                out += c;
        }
        return out + "\"";
    }

    string dongCSV(const vector<string> &fields)
    {
        string out;
        for (size_t i = 0; i < fields.size(); ++i)
        {
            if (i)
                out += ',';
            out += csvEscaped(fields[i]);
        }
        return out;
    }

    string so(double v)
    {
        ostringstream os;
        os << setprecision(15) << v;
        return os.str();
    }

    vector<string> tach(const string &dong)
    {
        vector<string> kq;
        string o;
        for (char c : dong)
        {
            if (c == '|')
            {
                kq.push_back(o);
                o.clear();
            }
            else if (c != '\r')
                o += c;
        }
        kq.push_back(o);
        return kq;
    }

    bool tachCSV(const string &line, vector<string> &fields)
    {
        fields.clear();
        string field;
        bool quoted = false;
        for (size_t i = 0; i < line.size(); ++i)
        {
            const char c = line[i];
            if (quoted)
            {
                if (c == '"' && i + 1 < line.size() && line[i + 1] == '"')
                {
                    field += '"';
                    ++i;
                }
                else if (c == '"')
                    quoted = false;
                else
                    field += c;
            }
            else if (c == ',' )
            {
                fields.push_back(field);
                field.clear();
            }
            else if (c == '"' && field.empty())
                quoted = true;
            else
                field += c;
        }
        if (quoted)
            return false;
        fields.push_back(field);
        return true;
    }

    double docSo(const string &s) { return strtod(s.c_str(), nullptr); }
    long long docNguyen(const string &s) { return strtoll(s.c_str(), nullptr, 10); }

    // Ghi vao "<file>.tmp" roi doi ten de de len file that khi hoan tat -> mat dien / loi giua chung
    // khong bao gio lam hong hay cat ngan file CSV dang co.
    class FileGhi
    {
        ofstream f;
        string dich, tam;

    public:
        FileGhi(const FileGhi &) = delete;
        FileGhi &operator=(const FileGhi &) = delete;
        ~FileGhi()
        {
            const bool tot = f.is_open() && f.good();
            if (f.is_open())
                f.close();
            error_code ec;
            if (tot)
            {
                fs::rename(tam, dich, ec);
                if (ec) // doi ten that bai -> thu sao chep de len
                {
                    ec.clear();
                    fs::copy_file(tam, dich, fs::copy_options::overwrite_existing, ec);
                    error_code ec2;
                    fs::remove(tam, ec2);
                }
            }
            else
            {
                error_code ec2;
                fs::remove(tam, ec2); // bo ban tam hong, giu nguyen file that
            }
        }
        FileGhi(const string &duongDan, const string &tieuDe) : f(duongDan + ".tmp", ios::binary | ios::trunc),
                                                                  dich(duongDan), tam(duongDan + ".tmp")
        {
            if (!f)
                throw runtime_error("Khong ghi duoc file " + duongDan);
            // UTF-8 BOM: Excel Windows mo dung tieng Viet; cac trinh doc CSV khac van doc binh thuong
            static const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
            f.write(reinterpret_cast<const char *>(bom), 3);
            f << dongCSV(tach(tieuDe)) << "\r\n"; // CRLF: giong dinh dang file CSV hien co
        }
        void dong(const vector<string> &truong)
        {
            // Ghi DUNG MOT dong CSV (truoc day lap theo so cot -> trung ban ghi)
            f << dongCSV(truong) << "\r\n";
        }
    };

    vector<string> *gNhatKy = nullptr; // nhat ky cua lan docFile dang chay

    void ghiNhatKy(const string &s)
    {
        if (gNhatKy)
            gNhatKy->push_back(s);
    }

    // Doc tung dong cua file, goi xuLy(cac truong). Dong loi KHONG bi bo qua trong im lang:
    // moi dong loi duoc dem (loi) va ghi vao nhat ky kem so dong + ly do.
    template <class F>
    void docTungDong(const string &duongDan, size_t soTruongToiThieu, int &loi, F xuLy)
    {
        const string tenFile = fs::path(duongDan).filename().string();
        ifstream f(duongDan, ios::binary);
        if (!f)
        {
            ++loi;
            ghiNhatKy(tenFile + ": KHONG MO DUOC FILE " + duongDan);
            return;
        }
        int soDoc = 0, soLoi = 0, soDong = 0;
        vector<string> chiTiet;
        auto baoLoi = [&](const string &lyDo)
        {
            ++loi;
            ++soLoi;
            if (chiTiet.size() < 5)
                chiTiet.push_back("    dong " + to_string(soDong) + ": " + lyDo);
        };
        const bool csv = duongDan.size() >= 4 && duongDan.substr(duongDan.size() - 4) == ".csv";
        bool daXuLyHeaderCSV = false;
        string dong;
        bool daBoBOM = false;
        while (getline(f, dong))
        {
            ++soDong;
            if (!dong.empty() && dong.back() == '\r')
                dong.pop_back();
            // Bo UTF-8 BOM o dau file (neu co)
            if (!daBoBOM)
            {
                daBoBOM = true;
                if (dong.size() >= 3 &&
                    (unsigned char)dong[0] == 0xEF &&
                    (unsigned char)dong[1] == 0xBB &&
                    (unsigned char)dong[2] == 0xBF)
                    dong.erase(0, 3);
            }
            if (dong.empty() || (!csv && dong[0] == '#'))
                continue;
            vector<string> t;
            if (csv)
            {
                if (!daXuLyHeaderCSV)
                {
                    daXuLyHeaderCSV = true;
                    continue;
                }
                if (!tachCSV(dong, t))
                {
                    baoLoi("dau ngoac kep khong dong");
                    continue;
                }
            }
            else
                t = tach(dong);
            try
            {
                if (t.size() < soTruongToiThieu)
                    throw runtime_error("thieu truong");
                xuLy(t);
                ++soDoc;
            }
            catch (const exception &e)
            {
                baoLoi(string(t.empty() ? "" : "[" + t[0] + "] ") + e.what());
            }
        }
        ghiNhatKy(tenFile + ": doc " + to_string(soDoc) + " ban ghi, loi " + to_string(soLoi));
        for (auto &c : chiTiet)
            ghiNhatKy(c);
        if (soLoi > (int)chiTiet.size())
            ghiNhatKy("    ... va " + to_string(soLoi - (int)chiTiet.size()) + " dong loi khac");
    }

    // ---------------- Dich vu <-> truong ----------------
    vector<string> dichVuThanhTruong(const DichVu &dv)
    {
        vector<string> t = {dv.layMa(), dv.layLoai(), dv.layTen(), so(dv.layDonGia()), "", ""};
        if (auto a = dynamic_cast<const AmThuc *>(&dv))
            t[4] = a->layLoaiBuaAn();
        else if (auto s = dynamic_cast<const Spa *>(&dv))
            t[4] = to_string(s->layThoiLuong());
        else if (auto r = dynamic_cast<const TourTuyen *>(&dv))
        {
            t[4] = r->layDiemDen();
            t[5] = to_string(r->laySoNgay());
        }
        return t;
    }

    shared_ptr<DichVu> truongThanhDichVu(const vector<string> &t, size_t i)
    {
        const string &ma = t[i], &loai = t[i + 1], &ten = t[i + 2];
        double gia = docSo(t[i + 3]);
        if (loai == "Am thuc")
            return make_shared<AmThuc>(ma, ten, gia, t[i + 4]);
        if (loai == "Spa")
            return make_shared<Spa>(ma, ten, gia, (int)docNguyen(t[i + 4]));
        if (loai == "Tour")
            return make_shared<TourTuyen>(ma, ten, gia, t[i + 4], (int)docNguyen(t[i + 5]));
        if (loai == "Tien ich")
            return make_shared<DichVuTienIch>(ma, ten, gia);
        throw runtime_error("Loai dich vu la: " + loai);
    }

    LoaiPhong docLoaiPhong(const string &s)
    {
        if (s == "Standard")
            return LoaiPhong::STANDARD;
        if (s == "Deluxe")
            return LoaiPhong::DELUXE;
        if (s == "Suite")
            return LoaiPhong::SUITE;
        if (s == "Villa")
            return LoaiPhong::VILLA;
        throw runtime_error("Loai phong la");
    }
}

// =====================================================================
//                                GHI FILE
// =====================================================================
void QuanLyKhachSan::ghiFile(const string &thuMuc) const
{
    fs::create_directories(thuMuc);
    auto dd = [&](const string &ten)
    { return (fs::path(thuMuc) / ten).string(); };

    {
        FileGhi f(dd("chinhanh.csv"), "ma|ten|diaChi|hotline");
        for (auto &cn : danhSachChiNhanh)
            f.dong({cn->layMa(), cn->layTen(), cn->layDiaChi(), cn->layHotline()});
    }
    {
        FileGhi f(dd("nhanvien.csv"), "ma|hoTen|sdt|email|chucVu|luongCoBan|maChiNhanh|dangLam(1/0)|cccd|gioiTinh|ngaySinh|tamNghi(1/0)|lyDoNghi");
        for (auto &nv : tatCaNhanVien(true))
            f.dong({nv->layMa(), nv->layHoTen(), nv->laySoDienThoai(), nv->layEmail(), nv->layChucVu(),
                so(nv->layLuongCoBan()), nv->layMaChiNhanh(), nv->layDangLam() ? "1" : "0", nv->layCCCD(),
                nv->layGioiTinh(), nv->layNgaySinh(), nv->layTamNghi() ? "1" : "0", nv->layLyDoNghi()});
    }
    {
        FileGhi f(dd("phong.csv"), "ma|maChiNhanh|loai|soPhong|tang|giaCoBan|sucChua|dienTich|trangThai(0-4)|"
                                   "hanGiuCho|lyDoBaoTri|rieng1|rieng2|rieng3|rieng4|daXoa(1/0)");
        vector<pair<string, shared_ptr<Phong>>> tatCa;
        for (auto &cn : danhSachChiNhanh)
            for (auto &p : cn->layDanhSachPhong())
                tatCa.push_back({cn->layMa(), p});
        size_t soDangDung = tatCa.size();
        for (auto &x : danhSachPhongDaXoa)
            tatCa.push_back(x);
        for (size_t i = 0; i < tatCa.size(); ++i)
            {
                auto &p = tatCa[i].second;
                string maCN = tatCa[i].first;
                string han, lyDo;
                if (auto gc = dynamic_pointer_cast<PhongGiuCho>(p->layTrangThai()))
                    han = to_string((long long)gc->layHanGiuCho());
                if (p->layMaTrangThai() == MaTrangThai::BAO_TRI)
                    lyDo = p->ghiChuTrangThai();
                vector<string> t = {p->layMaPhong(), maCN, loaiPhongToString(p->layLoaiPhong()),
                                    p->laySoPhong(), to_string(p->layTang()), so(p->layGiaPhong()),
                                    to_string(p->laySucChua()), so(p->layDienTich()),
                                    to_string((int)p->layMaTrangThai()), han, lyDo, "", "", "", "",
                                    i < soDangDung ? "0" : "1"};
                if (auto s = dynamic_pointer_cast<StandardRoom>(p))
                    t[11] = s->layHuongNhin();
                else if (auto d = dynamic_pointer_cast<DeluxeRoom>(p))
                {
                    t[11] = d->layCoBanCong() ? "1" : "0";
                    t[12] = d->layHuongNhin();
                }
                else if (auto su = dynamic_pointer_cast<Suite>(p))
                {
                    t[11] = to_string(su->laySoPhongNgu());
                    t[12] = su->layCoPhongKhach() ? "1" : "0";
                    t[13] = so(su->layPhiBuffetSang());
                }
                else if (auto v = dynamic_pointer_cast<Villa>(p))
                {
                    t[11] = v->layCoHoBoi() ? "1" : "0";
                    t[12] = so(v->layDienTichSanVuon());
                    t[13] = to_string(v->laySoPhongNgu());
                    t[14] = so(v->layPhiQuanGia());
                }
                f.dong(t);
            }
    }
    {
        FileGhi f(dd("dichvu.csv"), "ma|loai|ten|donGia|rieng1(loaiBua/phut/diemDen)|rieng2(soNgay)");
        for (auto &dv : danhSachDichVu)
            f.dong(dichVuThanhTruong(*dv));
    }
    {
        FileGhi f(dd("khachhang.csv"), "ma|hoTen|sdt|cccd|email|quocTich|diemTichLuy|hang(0-3)|gioiTinh|diaChiThuongTru|maChiNhanh");
        for (auto &kh : danhSachKhachHang)
            f.dong({kh->layMa(), kh->layHoTen(), kh->laySoDienThoai(), kh->layCCCD(), kh->layEmail(),
                kh->layQuocTich(), to_string(kh->layDiemTichLuy()), to_string((int)kh->layHang()),
                kh->layGioiTinh(), kh->layDiaChiThuongTru(), kh->layMaChiNhanh()});
    }
    {
        FileGhi f(dd("phieudatphong.csv"),
                  "ma|maKH|maPhong|maNV|tgDat|tgCheckIn|tgCheckOut|tgHuy|ngayNhan|soDem|soNguoi|mua|"
                  "trangThai|tienCoc|phuongThucCoc|daHoanCoc|maHoaDon|lyDoHuy|ghiChu");
        FileGhi g(dd("dichvusudung.csv"), "maPhieu|thoiGian|soLuong|maDV|loai|ten|donGia|rieng1|rieng2");
        for (auto &ph : danhSachPhieu)
        {
            f.dong({ph->layMaPhieu(), ph->layKhachHang()->layMa(), ph->layPhong()->layMaPhong(),
                    ph->layNhanVien()->layMa(), to_string((long long)ph->layThoiGianDat()),
                    to_string((long long)ph->layThoiGianCheckIn()), to_string((long long)ph->layThoiGianCheckOut()),
                    to_string((long long)ph->layThoiGianHuy()), ph->layNgayNhan().toString(),
                    to_string(ph->laySoDem()), to_string(ph->laySoNguoi()), to_string((int)ph->layMua()),
                    to_string((int)ph->layTrangThai()), so(ph->layTienCoc()), ph->layPhuongThucCoc(),
                    ph->layDaHoanCoc() ? "1" : "0", ph->layMaHoaDon(), ph->layLyDoHuy(), ph->layGhiChu()});
            for (auto &sd : ph->layDichVuSuDung())
            {
                vector<string> t = {ph->layMaPhieu(), to_string((long long)sd.thoiGian), to_string(sd.soLuong)};
                for (auto &x : dichVuThanhTruong(*sd.dichVu))
                    t.push_back(x);
                g.dong(t);
            }
        }
    }
    {
        FileGhi f(dd("hoadon.csv"), "ma|maPhieu|maNV|tgLap|tgThanhToan|tongTheoDecorator|tyLeGiam|tienCoc|"
                                    "daThanhToan|phuongThuc");
        FileGhi g(dd("chitiethoadon.csv"), "maHoaDon|loai|maThamChieu|noiDung|soLuong|donGia|thanhTien");
        for (auto &hd : danhSachHoaDon)
        {
            f.dong({hd->layMa(), hd->layPhieu()->layMaPhieu(),
                    hd->layNhanVienLap() ? hd->layNhanVienLap()->layMa() : "",
                    to_string((long long)hd->layThoiGianLap()), to_string((long long)hd->layThoiGianThanhToan()),
                    so(hd->layTongTheoDecorator()), so(hd->layTyLeGiamGia()), so(hd->layTienCoc()),
                    hd->layDaThanhToan() ? "1" : "0", hd->layPhuongThuc()});
            for (auto &ct : hd->layChiTiet())
                g.dong({hd->layMa(), ct.layLoai(), ct.layMaThamChieu(), ct.layNoiDung(),
                        to_string(ct.laySoLuong()), so(ct.layDonGia()), so(ct.layThanhTien())});
        }
    }
}

// =====================================================================
//                                DOC FILE
// =====================================================================
bool QuanLyKhachSan::docFile(const string &thuMuc, int &loi)
{
    loi = 0;
    auto dd = [&](const string &ten)
    { return (fs::path(thuMuc) / ten).string(); };
    auto ddDoc = [&](const string &ten)
    {
        const string csv = dd(ten + ".csv");
        return fs::exists(csv) ? csv : dd(ten + ".txt");
    };
    nhatKyDoc.clear();
    gNhatKy = &nhatKyDoc;
    struct DonNhatKy
    {
        ~DonNhatKy() { gNhatKy = nullptr; }
    } donNhatKy;
    if (!fs::exists(ddDoc("chinhanh")))
    {
        ghiNhatKy("THIEU FILE: " + dd("chinhanh.csv") + " (khong tim thay)");
        return false;
    }

    danhSachChiNhanh.clear();
    danhSachKhachHang.clear();
    danhSachPhieu.clear();
    danhSachHoaDon.clear();
    danhSachDichVu.clear();
    danhSachPhongDaXoa.clear();

    docTungDong(ddDoc("chinhanh"), 4, loi, [&](const vector<string> &t)
                { danhSachChiNhanh.push_back(make_shared<ChiNhanhResort>(t[0], t[1], t[2], t[3])); });

    docTungDong(ddDoc("nhanvien"), 8, loi, [&](const vector<string> &t)
                {
        auto cn = timChiNhanh(t[6]);
        if (!cn) throw runtime_error("chi nhanh");
        auto nv = make_shared<NhanVien>(t[0], t[1], t[2], t[4], docSo(t[5]), t[6], t[3]);
        nv->datDangLam(t[7] == "1");
        if (t.size() > 8) nv->datCCCD(t[8]);
        if (t.size() > 9) nv->datGioiTinh(t[9]);
        if (t.size() > 10) nv->datNgaySinh(t[10]);
        if (t.size() > 11) nv->datTamNghi(t[11] == "1");
        if (t.size() > 12) nv->datLyDoNghi(t[12]);
        cn->themNhanVien(nv); });

    // Cot rieng cua tung loai phong (huong nhin, so phong ngu, ...) co the de trong trong CSV hien co.
    // Khi trong: khoi phuc theo thu tu phong cung loai trong cung chi nhanh (cung quy tac du lieu goc), KHONG loai phong.
    map<string, int> demTheoNhom;
    docTungDong(ddDoc("phong"), 9, loi, [&](const vector<string> &t0)
                {
        vector<string> t = t0;
        if (t.size() < 16) t.resize(16);
        const int viTri = demTheoNhom[t[1] + "|" + t[2]]++;
        static const char *huongMau[] = {"Thanh pho", "Vuon", "Bien", "Ho boi"};
        auto hoacMau = [&](size_t k, const string &mau) { if (t[k].empty()) t[k] = mau; };
        if (t[2] == "Standard")
            hoacMau(11, huongMau[viTri % 4]);
        else if (t[2] == "Deluxe")
        {
            hoacMau(11, viTri % 5 != 4 ? "1" : "0");
            hoacMau(12, huongMau[(viTri + 2) % 4]);
        }
        else if (t[2] == "Suite")
        {
            hoacMau(11, to_string(2 + (viTri % 3 == 2 ? 1 : 0)));
            hoacMau(12, "1");
            hoacMau(13, to_string(300000 + 10000 * (viTri % 3)));
        }
        else if (t[2] == "Villa")
        {
            hoacMau(11, "1");
            hoacMau(12, to_string(90 + 30 * (viTri % 4)));
            hoacMau(13, to_string(2 + (viTri % 3)));
            hoacMau(14, to_string(400000 + 50000 * (viTri % 3)));
        }
        auto cn = timChiNhanh(t[1]);
        if (!cn) throw runtime_error("chi nhanh");
        LoaiPhong loai = docLoaiPhong(t[2]);
        int tang = max(1, min(5, (int)docNguyen(t[4]))), sc = (int)docNguyen(t[6]);
        double gia = docSo(t[5]), dt = docSo(t[7]);
        shared_ptr<Phong> p;
        switch (loai)
        {
        case LoaiPhong::STANDARD: p = make_shared<StandardRoom>(t[0], t[3], tang, gia, sc, dt, t[11]); break;
        case LoaiPhong::DELUXE: p = make_shared<DeluxeRoom>(t[0], t[3], tang, gia, sc, dt, t[11] == "1", t[12]); break;
        case LoaiPhong::SUITE:
            p = make_shared<Suite>(t[0], t[3], tang, gia, sc, dt, (int)docNguyen(t[11]), t[12] == "1", docSo(t[13]));
            break;
        case LoaiPhong::VILLA:
            p = make_shared<Villa>(t[0], t[3], tang, gia, sc, dt, t[11] == "1", docSo(t[12]), (int)docNguyen(t[13]),
                                   docSo(t[14]));
            break;
        }
        // Khoi phuc trang thai (State Pattern)
        switch ((MaTrangThai)docNguyen(t[8]))
        {
        case MaTrangThai::TRONG: break;
        case MaTrangThai::GIU_CHO: p->datTrangThai(make_shared<PhongGiuCho>((time_t)docNguyen(t[9]))); break;
        case MaTrangThai::DANG_O: p->datTrangThai(make_shared<PhongDangO>()); break;
        case MaTrangThai::DANG_DON: p->datTrangThai(make_shared<PhongDangDon>()); break;
        case MaTrangThai::BAO_TRI: p->datTrangThai(make_shared<PhongBaoTri>(t[10])); break;
        }
        if (t.size() > 15 && t[15] == "1")
        {
            p->datChiNhanh(cn->layTen());
            danhSachPhongDaXoa.push_back({cn->layMa(), p});
        }
        else
            cn->themPhong(p); });

    docTungDong(ddDoc("dichvu"), 6, loi, [&](const vector<string> &t)
                { danhSachDichVu.push_back(truongThanhDichVu(t, 0)); });

    docTungDong(ddDoc("khachhang"), 8, loi, [&](const vector<string> &t)
                {
        // t[0..9] cu, t[10] = maChiNhanh (tuy chon, tuong thich file cu)
        auto kh = make_shared<KhachHang>(t[0], t[1], t[2], t[3], t[4], t[5],
                         t.size() > 9 ? t[9] : "", t.size() > 8 ? t[8] : "",
                         t.size() > 10 ? t[10] : "");
        kh->datDiem((int)docNguyen(t[6]));
        int h = (int)docNguyen(t[7]);
        if (h < 0 || h > 3) throw runtime_error("hang");
        kh->datHang((HangThanhVien)h);
        danhSachKhachHang.push_back(kh); });

    docTungDong(ddDoc("phieudatphong"), 19, loi, [&](const vector<string> &t)
                {
        auto kh = timKhachHang(t[1]);
        auto p = timPhongKeCaDaXoa(t[2]);
        auto nv = timNhanVien(t[3]);
        Ngay nn;
        if (!kh || !p || !nv || !Ngay::docChuoi(t[8], nn)) throw runtime_error("lien ket");
        int mua = (int)docNguyen(t[11]), tt = (int)docNguyen(t[12]);
        if (mua < 1 || mua > 4 || tt < 0 || tt > 3) throw runtime_error("gia tri");
        auto ph = make_shared<PhieuDatPhong>(t[0], kh, p, nv, nn, (int)docNguyen(t[9]), (int)docNguyen(t[10]),
                                             (MuaDuLich)mua, t[18], false);
        ph->khoiPhuc((time_t)docNguyen(t[4]), (time_t)docNguyen(t[5]), (time_t)docNguyen(t[6]),
                     (time_t)docNguyen(t[7]), (TrangThaiPhieu)tt, t[17], t[16], docSo(t[13]), t[14], t[15] == "1");
        danhSachPhieu.push_back(ph);
        kh->themLichSu(ph); });

    // Phong dang GIU_CHO nhung cot hanGiuCho trong CSV bi trong (=0): neu bo mac thi bi coi la het han ngay
    // va nha phong -> mat viec giu cho. Khoi phuc han = het ngay nhan phong cua phieu DA_DAT som nhat cua phong do
    // (cung quy tac luc dat phong: giuCho(ngayNhan 23:59:59)).
    for (auto &p : tatCaPhong())
    {
        auto gc = dynamic_pointer_cast<PhongGiuCho>(p->layTrangThai());
        if (!gc || gc->layHanGiuCho() != 0)
            continue;
        shared_ptr<PhieuDatPhong> som;
        for (auto &ph : danhSachPhieu)
            if (ph->layTrangThai() == TrangThaiPhieu::DA_DAT && ph->layPhong() == p &&
                (!som || ph->layNgayNhan() < som->layNgayNhan()))
                som = ph;
        if (som)
            p->datTrangThai(make_shared<PhongGiuCho>(som->layNgayNhan().toTimeT(23, 59, 59)));
    }

    docTungDong(ddDoc("dichvusudung"), 9, loi, [&](const vector<string> &t)
                {
        auto ph = timPhieu(t[0]);
        if (!ph) throw runtime_error("phieu");
        ph->khoiPhucDichVu(truongThanhDichVu(t, 3), (int)docNguyen(t[2]), (time_t)docNguyen(t[1])); });

    docTungDong(ddDoc("hoadon"), 10, loi, [&](const vector<string> &t)
                {
        auto ph = timPhieu(t[1]);
        if (!ph) throw runtime_error("phieu");
        auto hd = make_shared<HoaDon>(t[0], ph, timNhanVien(t[2]), docSo(t[5]), docSo(t[6]), docSo(t[7]));
        hd->khoiPhuc((time_t)docNguyen(t[3]), (time_t)docNguyen(t[4]), t[8] == "1", t[9]);
        for (auto &sd : ph->layDichVuSuDung()) // HoaDon dung DichVu (nhieu - nhieu)
            hd->themDichVu(sd.dichVu);
        danhSachHoaDon.push_back(hd); });

    docTungDong(ddDoc("chitiethoadon"), 7, loi, [&](const vector<string> &t)
                {
        auto hd = timHoaDon(t[0]);
        if (!hd) throw runtime_error("hoa don");
        hd->themChiTiet(ChiTietHoaDon(t[1], t[2], t[3], (int)docNguyen(t[4]), docSo(t[5]), docSo(t[6]))); });

    capNhatBoDem();
    return true;
}
