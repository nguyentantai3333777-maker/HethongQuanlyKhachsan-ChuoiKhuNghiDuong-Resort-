// WebApi.cpp - Cac API JSON cho frontend. Moi ham deu goi lai logic C++ co san.
#include "WebApi.h"
#include "InputHelper.h"
#include "Json.h"
#include "PhongCuThe.h"
#include "TienIch.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>

using namespace std;

// =====================================================================
//                          TIEN ICH CHUNG
// =====================================================================
namespace
{
    string chuoi(const HttpRequest &r, const string &k, bool batBuoc = false)
    {
        string v = InputHelper::trim(r.param(k));
        if (batBuoc && v.empty())
            throw invalid_argument("Thieu thong tin: " + k + ".");
        return v;
    }

    // Backend kiem tra lai KIEU DU LIEU (khong tin frontend): truong so chi nhan chu so, khong chu, khong dau +/-
    double soThuc(const HttpRequest &r, const string &k)
    {
        string s = chuoi(r, k, true);
        if (s[0] == '-' || s[0] == '+')
            throw invalid_argument("Gia tri '" + k + "' khong duoc co dau +/-.");
        double v = 0;
        if (!InputHelper::docSoThuc(s, v))
            throw invalid_argument("Gia tri '" + k + "' chi duoc chua chu so.");
        if (v > 1e12)
            throw invalid_argument("Gia tri '" + k + "' qua lon.");
        return v;
    }

    int soNguyen(const HttpRequest &r, const string &k)
    {
        string s = chuoi(r, k, true);
        if (!InputHelper::laSoThuan(s, 1, 9))
            throw invalid_argument("Gia tri '" + k + "' phai la so nguyen khong am (chi gom chu so).");
        return atoi(s.c_str());
    }

    // So nguyen trong khoang [tu, den]
    int soNguyenTrong(const HttpRequest &r, const string &k, int tu, int den, const string &ten)
    {
        int v = soNguyen(r, k);
        if (v < tu || v > den)
            throw invalid_argument(ten + " phai tu " + to_string(tu) + " den " + to_string(den) + ".");
        return v;
    }

    bool coKhong(const HttpRequest &r, const string &k)
    {
        string v = r.param(k);
        return v == "1" || v == "true" || v == "on";
    }

    // Nhan ca "yyyy-mm-dd" (input type=date) va "dd/mm/yyyy"
    Ngay docNgay(const string &s)
    {
        Ngay n;
        int y, m, d;
        char c1, c2;
        if (s.size() == 10 && s[4] == '-' && sscanf(s.c_str(), "%d%c%d%c%d", &y, &c1, &m, &c2, &d) == 5 &&
            Ngay::hopLe(d, m, y))
            return Ngay(d, m, y);
        if (Ngay::docChuoi(s, n))
            return n;
        throw invalid_argument("Ngay khong hop le: " + s);
    }

    string isoNgay(const Ngay &n)
    {
        char buf[16];
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d", n.layNam(), n.layThang(), n.layNgay());
        return buf;
    }

    string taoToken()
    {
        static mt19937_64 rng((unsigned long long)chrono::steady_clock::now().time_since_epoch().count() ^
                              random_device{}());
        ostringstream os;
        os << hex << rng() << rng();
        return os.str();
    }

    MuaDuLich docMua(const HttpRequest &r, const Ngay &ngayNhan)
    {
        // Bo qua gia tri mua do client gui: mua luon duoc tinh tu dong theo ngay nhan phong.
        (void)r;
        return xacDinhMua(ngayNhan);
    }

    // "DV01:2,DV03:1" -> [(DV01,2),(DV03,1)]
    vector<pair<string, int>> docDanhSachDichVu(const string &s)
    {
        vector<pair<string, int>> kq;
        stringstream ss(s);
        string muc;
        while (getline(ss, muc, ','))
        {
            size_t h = muc.find(':');
            if (h == string::npos)
                continue;
            int sl = atoi(muc.c_str() + h + 1);
            if (sl > 0)
                kq.push_back({InputHelper::trim(muc.substr(0, h)), sl});
        }
        return kq;
    }

    bool batDau(const string &s, const string &tienTo) { return s.compare(0, tienTo.size(), tienTo) == 0; }
}

// =====================================================================
//                     KHOI TAO & DIEM VAO
// =====================================================================
WebApi::WebApi(QuanLyKhachSan &k, QuanLyTaiKhoan &tk, function<void()> l, const string &web)
    : ks(k), qlTaiKhoan(tk), luu(std::move(l)), thuMucWeb(web) {}

HttpResponse WebApi::thanhCong(const string &dataJson, const string &thongBao)
{
    HttpResponse res;
    JsonObject o;
    o.set("ok", true).setRaw("data", dataJson.empty() ? "null" : dataJson);
    if (!thongBao.empty())
        o.set("thongBao", thongBao);
    res.body = o.str();
    return res;
}

HttpResponse WebApi::loi(int status, const string &thongDiep)
{
    HttpResponse res;
    res.status = status;
    res.body = JsonObject().set("ok", false).set("loi", thongDiep).str();
    return res;
}

const WebApi::Phien *WebApi::layPhien(const HttpRequest &req) const
{
    auto it = phien.find(req.header("x-token"));
    return it == phien.end() ? nullptr : &it->second;
}

shared_ptr<NhanVien> WebApi::nhanVienCua(const Phien &p) const { return ks.timNhanVien(p.maNhanVien); }

HttpResponse WebApi::fileTinh(const string &duongDan) const
{
    string p = duongDan == "/" ? "/index.html" : duongDan;
    if (p.find("..") != string::npos) // chan truy cap ra ngoai thu muc frontend
        return loi(403, "Duong dan khong hop le.");
    ifstream f(thuMucWeb + p, ios::binary);
    if (!f)
        return loi(404, "Khong tim thay " + p);
    ostringstream os;
    os << f.rdbuf();
    HttpResponse res;
    res.body = os.str();
    string ext = p.substr(p.find_last_of('.') + 1);
    if (ext == "html")
        res.contentType = "text/html; charset=utf-8";
    else if (ext == "css")
        res.contentType = "text/css; charset=utf-8";
    else if (ext == "js")
        res.contentType = "application/javascript; charset=utf-8";
    else if (ext == "svg")
        res.contentType = "image/svg+xml";
    else if (ext == "png")
        res.contentType = "image/png";
    else if (ext == "jpg" || ext == "jpeg")
        res.contentType = "image/jpeg";
    else if (ext == "ico")
        res.contentType = "image/x-icon";
    else
        res.contentType = "application/octet-stream";
    return res;
}

HttpResponse WebApi::xuLy(const HttpRequest &r)
{
    if (!batDau(r.path, "/api/"))
        return fileTinh(r.path);
    try
    {
        // ---- Dang nhap: khong can token ----
        if (r.path == "/api/dangnhap")
        {
            const TaiKhoan &tk = qlTaiKhoan.dangNhap(r.param("username"), r.param("password"));
            auto nv = ks.timNhanVien(tk.layMaNhanVien());
            if (!nv || !nv->layDangLam())
                return loi(403, "Tai khoan khong gan voi nhan vien dang lam viec.");
            if (!chucVuKhopVaiTro(nv->layChucVu(), tk.layRole()))
                return loi(403, "Tai khoan khong phu hop voi chuc vu nhan vien.");
            string token = taoToken();
            phien[token] = {tk.layUsername(), tk.layRole(), tk.layMaNhanVien()};
            return thanhCong(JsonObject()
                                 .set("token", token)
                                 .set("username", tk.layUsername())
                                 .set("admin", tk.layRole() == Role::ADMIN)
                                 .setRaw("nhanVien", nhanVienJson(*nv))
                                 .str(),
                             "Xin chao " + nv->layHoTen() + "!");
        }
        if (batDau(r.path, "/api/khach/"))
        {
            ks.dongBoTheoNgay();
            HttpResponse res = apiKhach(r);
            if (r.method == "POST" && res.status == 200 && r.path == "/api/khach/datphong")
                luu();
            return res;
        }
        const Phien *p = layPhien(r);
        if (!p)
            return loi(401, "Phien dang nhap da het han. Vui long dang nhap lai.");
        ks.dongBoTheoNgay(); // giong console: moi lan thao tac deu dong bo trang thai theo ngay

        // Xac minh Single Source of Truth: dem ban ghi trong cung doi tuong ks ma Console dang dung
        if (r.path == "/api/dongbo" || r.path == "/api/sync")
        {
            int soPhong = 0;
            for (auto &cn : ks.layDanhSachChiNhanh())
                soPhong += (int)cn->layDanhSachPhong().size();
            return thanhCong(JsonObject()
                                 .set("nguonDuLieu", "CSV dung chung Console + Web")
                                 .set("soChiNhanh", (int)ks.layDanhSachChiNhanh().size())
                                 .set("soPhong", soPhong)
                                 .set("soKhachHang", (int)ks.layDanhSachKhachHang().size())
                                 .set("soNhanVien", (int)ks.tatCaNhanVien(true).size())
                                 .set("soPhieu", (int)ks.layDanhSachPhieu().size())
                                 .set("soHoaDon", (int)ks.layDanhSachHoaDon().size())
                                 .set("soDichVu", (int)ks.layDanhSachDichVu().size())
                                 .set("soTaiKhoan", (int)qlTaiKhoan.layDanhSach().size())
                                 .str(),
                             "Du lieu dang dung chung voi Console (cung doi tuong QuanLyKhachSan).");
        }

        HttpResponse res;
        if (batDau(r.path, "/api/taikhoan"))
            res = apiTaiKhoan(r, *p);
        else if (batDau(r.path, "/api/tongquan") || batDau(r.path, "/api/doanhthu"))
            res = apiTongQuan(r, *p);
        else if (batDau(r.path, "/api/phong") || batDau(r.path, "/api/chinhanh"))
            res = apiPhong(r, *p);
        else if (batDau(r.path, "/api/khachhang"))
            res = apiKhachHang(r, *p);
        else if (batDau(r.path, "/api/phieu") || r.path == "/api/baogia" || r.path == "/api/datphong" ||
                 r.path == "/api/checkin" || r.path == "/api/huyphieu" || r.path == "/api/ghidichvu" ||
                 r.path == "/api/giahan")
            res = apiDatPhong(r, *p);
        else if (batDau(r.path, "/api/dichvu"))
            res = apiDichVu(r, *p);
        else if (batDau(r.path, "/api/hoadon") || r.path == "/api/tamtinh" || r.path == "/api/checkout" ||
                 r.path == "/api/thanhtoan")
            res = apiThanhToan(r, *p);
        else if (batDau(r.path, "/api/nhanvien"))
            res = apiNhanVien(r, *p);
        else if (batDau(r.path, "/api/baocao"))
            res = apiBaoCao(r, *p);
        else
            return loi(404, "API khong ton tai: " + r.path);

        if (r.method == "POST" && res.status == 200)
            luu(); // moi thay doi deu ghi xuong file ngay (dung chung thu muc data/ voi console)
        return res;
    }
    catch (const exception &e)
    {
        return loi(400, e.what());
    }
}

// =====================================================================
//                    CHUYEN DOI DOI TUONG -> JSON
// =====================================================================
string WebApi::seriesJson(const vector<pair<string, double>> &ds)
{
    JsonArray a;
    for (auto &x : ds)
        a.push(JsonObject().set("nhan", x.first).set("giaTri", x.second));
    return a.str();
}

string WebApi::phongJson(const Phong &p) const
{
    JsonObject o;
    auto cn = ks.chiNhanhCuaPhong(p.layMaPhong());
    o.set("ma", p.layMaPhong())
        .set("so", p.laySoPhong())
        .set("maCN", cn ? cn->layMa() : "")
        .set("chiNhanh", p.layChiNhanh())
        .set("loai", loaiPhongToString(p.layLoaiPhong()))
        .set("tang", p.layTang())
        .set("gia", p.layGiaPhong())
        .set("sucChua", p.laySucChua())
        .set("dienTich", p.layDienTich())
        .set("maTrangThai", (int)p.layMaTrangThai())
        .set("trangThai", p.layTenTrangThai())
        .set("ghiChuTrangThai", p.ghiChuTrangThai())
        .set("moTa", p.moTa());
    auto hienTai = ks.phieuHienTaiCuaPhong(p.layMaPhong());
    auto sapToi = ks.phieuSapToiCuaPhong(p.layMaPhong());
    o.set("khachDangO", hienTai && hienTai->layTrangThai() == TrangThaiPhieu::DA_NHAN_PHONG
                            ? hienTai->layKhachHang()->layHoTen()
                            : "");
    o.set("phieuHienTai", hienTai ? hienTai->layMaPhieu() : "");
    o.set("quaHan", hienTai && hienTai->dangOverstay());
    o.set("soNgayQuaHan", hienTai ? hienTai->soNgayOverstay() : 0);
    o.set("khachHienTai", hienTai ? hienTai->layKhachHang()->layHoTen() : "");
    o.set("ngayTraHienTai", hienTai ? hienTai->layNgayTra().toString() : "");
    o.set("datSapToi", sapToi ? sapToi->layNgayNhan().toString() + " - " + sapToi->layKhachHang()->layHoTen() : "");
    o.set("soPhieuHoatDong", (int)ks.lichDatCuaPhong(p.layMaPhong()).size());

    JsonArray rieng, tienNghi;
    for (auto &x : p.layThongTinRieng())
        rieng.push(JsonObject().set("k", x.first).set("v", x.second));
    for (auto &x : p.layDichVuDiKem())
        tienNghi.push(x);
    o.setRaw("thongTinRieng", rieng.str()).setRaw("tienNghi", tienNghi.str());

    // Thuoc tinh rieng dang tho (cho form sua)
    JsonObject f;
    if (auto s = dynamic_cast<const StandardRoom *>(&p))
        f.set("huongNhin", s->layHuongNhin());
    else if (auto d = dynamic_cast<const DeluxeRoom *>(&p))
        f.set("huongNhin", d->layHuongNhin()).set("banCong", d->layCoBanCong());
    else if (auto su = dynamic_cast<const Suite *>(&p))
        f.set("soPhongNgu", su->laySoPhongNgu())
            .set("phongKhach", su->layCoPhongKhach())
            .set("phiBuffet", su->layPhiBuffetSang());
    else if (auto v = dynamic_cast<const Villa *>(&p))
        f.set("hoBoi", v->layCoHoBoi())
            .set("sanVuon", v->layDienTichSanVuon())
            .set("soPhongNgu", v->laySoPhongNgu())
            .set("phiQuanGia", v->layPhiQuanGia());
    o.setRaw("rieng", f.str());
    return o.str();
}

string WebApi::khachJson(const KhachHang &kh) const
{
    auto ls = kh.layLichSuDatPhong();
    double tongChi = 0;
    string trangThai = "Binh thuong";
    JsonArray dsCN; // chi nhanh gan cho khach + cac chi nhanh tung dat / luu tru
    vector<string> daCo;
    // Uu tien maChiNhanh da gan
    if (!kh.layMaChiNhanh().empty())
    {
        daCo.push_back(kh.layMaChiNhanh());
        dsCN.push(kh.layMaChiNhanh());
    }
    for (auto &ph : ls)
    {
        string m = ks.maChiNhanhCuaPhong(ph->layPhong());
        if (!m.empty() && find(daCo.begin(), daCo.end(), m) == daCo.end())
        {
            daCo.push_back(m);
            dsCN.push(m);
        }
        if (auto hd = ks.timHoaDon(ph->layMaHoaDon()))
            if (hd->layDaThanhToan())
                tongChi += hd->tongThanhToan();
        if (ph->layTrangThai() == TrangThaiPhieu::DA_NHAN_PHONG)
            trangThai = "Dang luu tru";
        else if (ph->layTrangThai() == TrangThaiPhieu::DA_DAT && trangThai != "Dang luu tru")
            trangThai = "Sap den";
    }
    return JsonObject()
        .set("ma", kh.layMa())
        .set("hoTen", kh.layHoTen())
        .set("sdt", kh.laySoDienThoai())
        .set("email", kh.layEmail())
        .set("cccd", kh.layCCCD())
        .set("gioiTinh", kh.layGioiTinh())
        .set("diaChiThuongTru", kh.layDiaChiThuongTru())
        .set("quocTich", kh.layQuocTich())
        .set("diem", kh.layDiemTichLuy())
        .set("maHang", (int)kh.layHang())
        .set("hang", kh.hangThanhVien())
        .set("giamGia", kh.tyLeGiamGia())
        .set("soLanDat", (int)ls.size())
        .set("soLanLuuTru", kh.soLanLuuTru())
        .set("tongChiTieu", tongChi)
        .set("trangThai", trangThai)
        .set("maChiNhanh", kh.layMaChiNhanh())
        .setRaw("dsChiNhanh", dsCN.str())
        .str();
}

string WebApi::phieuJson(const PhieuDatPhong &ph) const
{
    auto kh = ph.layKhachHang();
    auto p = ph.layPhong();
    // Phieu "Cho nhan phong" ma phong dang bi khach khac (chua check-out) chiem -> can cho check-out / chuyen phong
    // (giong QuanLyKhachSan::phieuBiChanBoiKhachDangO: chi tinh phieu da den ngay nhan)
    auto dangO = ph.layTrangThai() == TrangThaiPhieu::DA_DAT && ph.layNgayNhan() <= Ngay::homNay()
                     ? ks.phieuDangOCuaPhong(p->layMaPhong())
                     : nullptr;
    bool biChan = dangO && dangO.get() != &ph;
    JsonArray dv;
    for (auto &sd : ph.layDichVuSuDung())
        dv.push(JsonObject()
                    .set("ma", sd.dichVu->layMa())
                    .set("ten", sd.dichVu->layTen())
                    .set("loai", sd.dichVu->layLoai())
                    .set("soLuong", sd.soLuong)
                    .set("donGia", sd.dichVu->layDonGia())
                    .set("thanhTien", sd.thanhTien())
                    .set("thoiGian", dinhDangThoiGian(sd.thoiGian)));
    return JsonObject()
        .set("ma", ph.layMaPhieu())
        .set("maKH", kh->layMa())
        .set("khach", kh->layHoTen())
        .set("sdt", kh->laySoDienThoai())
        .set("hang", kh->hangThanhVien())
        .set("maPhong", p->layMaPhong())
        .set("soPhong", p->laySoPhong())
        .set("loaiPhong", loaiPhongToString(p->layLoaiPhong()))
        .set("chiNhanh", p->layChiNhanh())
        .set("nhanVien", ph.layNhanVien()->layHoTen())
        .set("tgDat", dinhDangThoiGian(ph.layThoiGianDat()))
        .set("tgDatSo", (long long)ph.layThoiGianDat())
        .set("tgCheckIn", ph.layThoiGianCheckIn() ? dinhDangThoiGian(ph.layThoiGianCheckIn()) : "")
        .set("tgCheckOut", ph.layThoiGianCheckOut() ? dinhDangThoiGian(ph.layThoiGianCheckOut()) : "")
        .set("tgHuy", ph.layThoiGianHuy() ? dinhDangThoiGian(ph.layThoiGianHuy()) : "")
        .set("ngayNhan", ph.layNgayNhan().toString())
        .set("ngayTra", ph.layNgayTra().toString())
        .set("ngayNhanIso", isoNgay(ph.layNgayNhan()))
        .set("ngayTraIso", isoNgay(ph.layNgayTra()))
        .set("soDem", ph.laySoDem())
        .set("soNguoi", ph.laySoNguoi())
        .set("mua", muaToString(ph.layMua()))
        .set("heSoMua", heSoMua(ph.layMua()))
        .set("maTrangThai", (int)ph.layTrangThai())
        .set("trangThai", trangThaiPhieuToString(ph.layTrangThai()))
        .set("tienCoc", ph.layTienCoc())
        .set("ptCoc", ph.layPhuongThucCoc())
        .set("daHoanCoc", ph.layDaHoanCoc())
        .set("lyDoHuy", ph.layLyDoHuy())
        .set("ghiChu", ph.layGhiChu())
        .set("maHoaDon", ph.layMaHoaDon())
        .set("quaHan", ph.dangOverstay())
        .set("soNgayQuaHan", ph.soNgayOverstay())
        .set("maCN", ks.maChiNhanhCuaPhong(p))
        .set("biChan", biChan)
        .set("khachChiemPhong", dangO && biChan ? dangO->layKhachHang()->layHoTen() : "")
        .set("tienPhong", ph.tinhTienPhong())
        .set("tienDichVu", ph.tinhTienDichVu())
        .setRaw("dichVu", dv.str())
        .str();
}

string WebApi::hoaDonJson(const HoaDon &hd) const
{
    auto ph = hd.layPhieu();
    JsonArray ct;
    for (auto &c : hd.layChiTiet())
        ct.push(JsonObject()
                    .set("loai", c.layLoai())
                    .set("ma", c.layMaThamChieu())
                    .set("noiDung", c.layNoiDung())
                    .set("soLuong", c.laySoLuong())
                    .set("donGia", c.layDonGia())
                    .set("thanhTien", c.layThanhTien()));
    return JsonObject()
        .set("ma", hd.layMa())
        .set("maPhieu", ph->layMaPhieu())
        .set("maKH", ph->layKhachHang()->layMa())
        .set("khach", ph->layKhachHang()->layHoTen())
        .set("hang", ph->layKhachHang()->hangThanhVien())
        .set("maPhong", ph->layPhong()->layMaPhong())
        .set("soPhong", ph->layPhong()->laySoPhong())
        .set("loaiPhong", loaiPhongToString(ph->layPhong()->layLoaiPhong()))
        .set("chiNhanh", ph->layPhong()->layChiNhanh())
        .set("maCN", ks.maChiNhanhCuaPhong(ph->layPhong()))
        .set("ngayNhan", ph->layNgayNhan().toString())
        .set("ngayTra", ph->layNgayTra().toString())
        .set("soDem", ph->laySoDem())
        .set("tgLap", dinhDangThoiGian(hd.layThoiGianLap()))
        .set("tgLapSo", (long long)hd.layThoiGianLap())
        .set("tgThanhToan", hd.layThoiGianThanhToan() ? dinhDangThoiGian(hd.layThoiGianThanhToan()) : "")
        .set("nhanVien", hd.layNhanVienLap() ? hd.layNhanVienLap()->layHoTen() : "")
        .set("tienPhong", hd.tienPhong())
        .set("tienDichVu", hd.tienDichVu())
        .set("tamTinh", hd.tamTinh())
        .set("tyLeGiam", hd.layTyLeGiamGia())
        .set("tienGiam", hd.tienGiam())
        .set("thue", hd.tienThue())
        .set("tong", hd.tongThanhToan())
        .set("coc", hd.layTienCoc())
        .set("conLai", hd.conLaiPhaiTra())
        .set("cocDu", hd.tienCocDu())
        .set("cocDaTru", hd.tienCocDaTru())
        .set("daThanhToan", hd.layDaThanhToan())
        .set("phuongThuc", hd.layPhuongThuc())
        .set("khopDecorator", hd.khopVoiDecorator())
        .setRaw("chiTiet", ct.str())
        .str();
}

string WebApi::dichVuJson(const DichVu &dv) const
{
    JsonObject o;
    o.set("ma", dv.layMa())
        .set("ten", dv.layTen())
        .set("loai", dv.layLoai())
        .set("donGia", dv.layDonGia())
        .set("donVi", dv.layDonViTinh())
        .set("moTa", dv.moTaChiTiet());
    if (auto a = dynamic_cast<const AmThuc *>(&dv))
        o.set("loaiBuaAn", a->layLoaiBuaAn());
    else if (auto s = dynamic_cast<const Spa *>(&dv))
        o.set("thoiLuong", s->layThoiLuong());
    else if (auto t = dynamic_cast<const TourTuyen *>(&dv))
        o.set("diemDen", t->layDiemDen()).set("soNgay", t->laySoNgay());
    return o.str();
}

string WebApi::nhanVienJson(const NhanVien &nv) const
{
    auto cn = ks.timChiNhanh(nv.layMaChiNhanh());
    auto tk = qlTaiKhoan.timTheoNhanVien(nv.layMa());
    return JsonObject()
        .set("ma", nv.layMa())
        .set("hoTen", nv.layHoTen())
        .set("sdt", nv.laySoDienThoai())
        .set("email", nv.layEmail())
        .set("chucVu", nv.layChucVu())
        .set("cccd", nv.layCCCD())
        .set("gioiTinh", nv.layGioiTinh())
        .set("ngaySinh", nv.layNgaySinh())
        .set("tamNghi", nv.layTamNghi())
        .set("luong", nv.layLuongCoBan())
        .set("heSoLuong", nv.heSoLuong())
        .set("luongThucNhan", nv.luongThucNhan())
        .set("maCN", nv.layMaChiNhanh())
        .set("chiNhanh", cn ? cn->layTen() : "")
        .set("dangLam", nv.layDangLam())
        .set("taiKhoan", tk ? tk->layUsername() : "")
        .set("quyen", tk ? (tk->layRole() == Role::ADMIN ? "ADMIN" : "USER") : "")
        .str();
}

// =====================================================================
//                             TAI KHOAN
// =====================================================================
HttpResponse WebApi::apiTaiKhoan(const HttpRequest &r, const Phien &p)
{
    if (r.path == "/api/toi")
    {
        auto nv = nhanVienCua(p);
        return thanhCong(JsonObject()
                             .set("username", p.username)
                             .set("admin", p.role == Role::ADMIN)
                             .setRaw("nhanVien", nv ? nhanVienJson(*nv) : "null")
                             .str());
    }
    if (r.path == "/api/dangxuat")
    {
        phien.erase(r.header("x-token"));
        return thanhCong("", "Da dang xuat.");
    }
    if (p.role != Role::ADMIN)
        return loi(403, "Chuc nang danh rieng cho ADMIN.");
    if (r.path == "/api/taikhoan")
    {
        JsonArray a;
        for (auto &tk : qlTaiKhoan.layDanhSach())
        {
            auto nv = ks.timNhanVien(tk.layMaNhanVien());
            a.push(JsonObject()
                       .set("username", tk.layUsername())
                       .set("quyen", tk.layRole() == Role::ADMIN ? "ADMIN" : "USER")
                       .set("maNV", tk.layMaNhanVien())
                       .set("nhanVien", nv ? nv->layHoTen() : "")
                       .set("chucVu", nv ? nv->layChucVu() : "")
                       .set("laToi", tk.layUsername() == p.username));
        }
        return thanhCong(a.str());
    }
    if (r.path == "/api/taikhoan/them")
    {
        auto nv = ks.timNhanVien(chuoi(r, "maNV", true));
        if (!nv || !nv->layDangLam())
            throw runtime_error("Nhan vien khong hop le.");
        const Role role = laChucVuQuanLy(nv->layChucVu()) ? Role::ADMIN : Role::USER;
        if (!chucVuKhopVaiTro(nv->layChucVu(), role))
            throw invalid_argument("Chi quan ly va le tan moi duoc cap tai khoan.");
        qlTaiKhoan.themTaiKhoan(chuoi(r, "username", true), r.param("password"),
                                role, nv->layMa());
        return thanhCong("", "Tao tai khoan thanh cong!");
    }
    if (r.path == "/api/taikhoan/xoa")
    {
        string u = chuoi(r, "username", true);
        if (u == p.username)
            throw runtime_error("Khong the xoa tai khoan dang dang nhap.");
        qlTaiKhoan.xoaTaiKhoan(u);
        return thanhCong("", "Da xoa tai khoan " + u + ".");
    }
    if (r.path == "/api/taikhoan/doimatkhau")
    {
        if (r.param("matKhauMoi") != r.param("nhapLai"))
            throw runtime_error("Mat khau nhap lai khong khop.");
        qlTaiKhoan.doiMatKhau(p.username, r.param("matKhauCu"), r.param("matKhauMoi"));
        return thanhCong("", "Doi mat khau thanh cong!");
    }
    return loi(404, "API khong ton tai.");
}

// =====================================================================
//                       TONG QUAN (DASHBOARD)
// =====================================================================
HttpResponse WebApi::apiTongQuan(const HttpRequest &r, const Phien &p)
{
    // maCN (tuy chon): doanh thu cua mot chi nhanh; rong = toan he thong
    string maCN = InputHelper::chuanHoaMa(r.param("maCN"));
    if (!maCN.empty() && !ks.timChiNhanh(maCN))
        throw invalid_argument("Khong tim thay chi nhanh " + maCN + ".");
    Ngay hn = Ngay::homNay();
    time_t dauNgay = hn.toTimeT(), cuoiNgay = hn.congNgay(1).toTimeT();
    Ngay dauThang(1, hn.layThang(), hn.layNam());
    Ngay dauThangSau = hn.layThang() == 12 ? Ngay(1, 1, hn.layNam() + 1) : Ngay(1, hn.layThang() + 1, hn.layNam());
    Ngay dauThangTruoc = hn.layThang() == 1 ? Ngay(1, 12, hn.layNam() - 1) : Ngay(1, hn.layThang() - 1, hn.layNam());
    int thu = layTm(hn.toTimeT(12)).tm_wday;
    Ngay dauTuan = hn.congNgay(-((thu + 6) % 7));

    JsonObject dt;
    dt.set("homNay", ks.doanhThuTrongKhoang(dauNgay, cuoiNgay, maCN))
        .set("tuan", ks.doanhThuTrongKhoang(dauTuan.toTimeT(), dauTuan.congNgay(7).toTimeT(), maCN))
        .set("thang", ks.doanhThuTrongKhoang(dauThang.toTimeT(), dauThangSau.toTimeT(), maCN))
        .set("thangTruoc", ks.doanhThuTrongKhoang(dauThangTruoc.toTimeT(), dauThang.toTimeT(), maCN))
        .set("nam", ks.doanhThuTrongKhoang(Ngay(1, 1, hn.layNam()).toTimeT(), Ngay(1, 1, hn.layNam() + 1).toTimeT(), maCN))
        .set("tatCa", ks.tongDoanhThu(maCN))
        .set("conNo", (int)ks.soHoaDonChuaThanhToan(maCN))
        .set("maCN", maCN);
    if (p.role == Role::ADMIN)
    {
        const double chiPhiLuong = ks.chiPhiLuongThang(maCN);
        dt.set("chiPhiLuongThang", chiPhiLuong)
            .set("mucTieuDoanhThu3x", chiPhiLuong * 3.0)
            .set("datMucTieu3x", ks.doanhThuThangHienTai(maCN) >= chiPhiLuong * 3.0);
    }
    if (r.path == "/api/doanhthu")
        return thanhCong(dt.str());

    auto phong = ks.tatCaPhong();
    int dem[5] = {0, 0, 0, 0, 0};
    for (auto &x : phong)
        ++dem[(int)x->layMaTrangThai()];
    int datHomNay = 0, denHomNay = 0;
    for (auto &ph : ks.layDanhSachPhieu())
    {
        if (ph->layThoiGianDat() >= dauNgay && ph->layThoiGianDat() < cuoiNgay)
            ++datHomNay;
        if (ph->layTrangThai() == TrangThaiPhieu::DA_DAT && ph->layNgayNhan() == hn)
            ++denHomNay;
    }
    vector<pair<string, double>> trangThai;
    for (int i = 0; i < 5; ++i)
        trangThai.push_back({tenTrangThai((MaTrangThai)i), (double)dem[i]});

    auto ds = ks.layDanhSachPhieu();
    sort(ds.begin(), ds.end(), [](const shared_ptr<PhieuDatPhong> &a, const shared_ptr<PhieuDatPhong> &b)
         { return a->layThoiGianDat() > b->layThoiGianDat(); });
    JsonArray ganDay, hoatDong;
    for (size_t i = 0; i < ds.size() && i < 6; ++i)
        ganDay.pushRaw(phieuJson(*ds[i]));
    auto sk = ks.layLichSuSuKien();
    for (size_t i = 0; i < sk.size() && i < 8; ++i)
        hoatDong.push(JsonObject()
                          .set("thoiGian", dinhDangThoiGian(sk[i].thoiGian))
                          .set("loai", sk[i].loai)
                          .set("maPhieu", sk[i].phieu->layMaPhieu())
                          .set("khach", sk[i].phieu->layKhachHang()->layHoTen())
                          .set("phong", sk[i].phieu->layPhong()->layMaPhong() + " - " +
                                            sk[i].phieu->layPhong()->laySoPhong())
                          .set("chiNhanh", sk[i].phieu->layPhong()->layChiNhanh()));

    // Thong ke theo chi nhanh (so phong theo trang thai + doanh thu)
    JsonArray theoCN;
    auto dtCN = ks.doanhThuTheoChiNhanh();
    for (auto &cn : ks.layDanhSachChiNhanh())
    {
        double dtc = 0;
        // dtCN = (ten, doanh thu) theo thu tu danhSachChiNhanh
        for (auto &x : dtCN)
            if (x.first == cn->layTen())
                dtc = x.second;
        if (dtc == 0)
            dtc = ks.tongDoanhThu(cn->layMa()); // dam bao khop Console
        int dangKhach = 0, quaHan = 0;
        for (auto &ph : ks.layDanhSachPhieu())
            if (ph->dangHoatDong() && ks.chiNhanhCuaPhong(ph->layPhong()->layMaPhong()) == cn)
            {
                if (ph->layTrangThai() == TrangThaiPhieu::DA_NHAN_PHONG)
                    ++dangKhach;
                if (ph->dangOverstay())
                    ++quaHan;
            }
        theoCN.push(JsonObject().set("ma", cn->layMa()).set("ten", cn->layTen())
                        .set("tongPhong", (int)cn->layDanhSachPhong().size())
                        .set("trong", cn->demPhong(MaTrangThai::TRONG)).set("giuCho", cn->demPhong(MaTrangThai::GIU_CHO))
                        .set("dangO", cn->demPhong(MaTrangThai::DANG_O)).set("dangDon", cn->demPhong(MaTrangThai::DANG_DON))
                        .set("baoTri", cn->demPhong(MaTrangThai::BAO_TRI)).set("doanhThu", dtc)
                        .set("khachDangO", dangKhach).set("quaHan", quaHan));
    }
    // Doanh thu theo loai phong
    return thanhCong(JsonObject()
                         .setRaw("theoChiNhanh", theoCN.str())
                         .setRaw("doanhThuLoaiPhong", seriesJson(ks.doanhThuTheoLoaiPhong()))
                         .set("tongPhong", (int)phong.size())
                         .set("phongTrong", dem[(int)MaTrangThai::TRONG])
                         .set("phongGiuCho", dem[(int)MaTrangThai::GIU_CHO])
                         .set("phongDangO", dem[(int)MaTrangThai::DANG_O])
                         .set("phongDangDon", dem[(int)MaTrangThai::DANG_DON])
                         .set("phongBaoTri", dem[(int)MaTrangThai::BAO_TRI])
                         .set("tongKhach", (int)ks.layDanhSachKhachHang().size())
                         .set("khachQuaHan", (int)ks.phieuQuaHan().size())
                         .set("phieuBiChan", (int)ks.phieuBiChanBoiKhachDangO().size())
                         .set("cocGiuLai", ks.tongCocGiuLai())
                         .set("cocDangGiu", ks.tongCocDangGiu())
                         .set("soLuotDat", (int)ks.layDanhSachPhieu().size())
                         .set("datHomNay", datHomNay)
                         .set("khachDenHomNay", denHomNay)
                         .set("congSuat", ks.congSuatPhong())
                         .setRaw("doanhThu", dt.str())
                         .setRaw("bieuDoDoanhThu", seriesJson(ks.doanhThuTheoKy("thang", 6)))
                         .setRaw("phongTheoTrangThai", seriesJson(trangThai))
                         .setRaw("datPhongGanDay", ganDay.str())
                         .setRaw("hoatDong", hoatDong.str())
                         .str());
}

// =====================================================================
//                           PHONG & CHI NHANH
// =====================================================================
HttpResponse WebApi::apiPhong(const HttpRequest &r, const Phien &p)
{
    bool admin = p.role == Role::ADMIN;
    if (r.path == "/api/chinhanh")
    {
        JsonArray a;
        for (auto &cn : ks.layDanhSachChiNhanh())
        {
            int nv = 0;
            for (auto &x : cn->layDanhSachNhanVien())
                if (x->layDangLam())
                    ++nv;
            a.push(JsonObject()
                       .set("ma", cn->layMa())
                       .set("ten", cn->layTen())
                       .set("diaChi", cn->layDiaChi())
                       .set("hotline", cn->layHotline())
                       .set("soPhong", (int)cn->layDanhSachPhong().size())
                       .set("phongTrong", cn->demPhong(MaTrangThai::TRONG))
                       .set("phongDangO", cn->demPhong(MaTrangThai::DANG_O))
                       .set("soNhanVien", nv)
                       .set("soHoaDon", (int)ks.hoaDonCuaChiNhanh(cn->layMa()).size())
                       .set("xoaDuoc", ks.lyDoKhongXoaDuocChiNhanh(cn->layMa()).empty()));
        }
        return thanhCong(a.str());
    }
    if (r.path == "/api/chinhanh/them")
    {
        if (!admin)
            return loi(403, "Chuc nang danh rieng cho ADMIN.");
        const string ten = chuoi(r, "ten", true);
        const string diaChi = chuoi(r, "diaChi", true);
        if (!InputHelper::laChuThuan(ten) || !InputHelper::laDiaChi(diaChi))
            throw invalid_argument("Ten chi duoc chua chu cai; dia chi cho phep chu, so va , . / - ( ) # (5-120 ky tu).");
        auto cn = ks.themChiNhanh(ten, diaChi, chuoi(r, "hotline", true));
        ks.damBaoNhanSuMacDinh();
        qlTaiKhoan.damBaoTaiKhoanNhanSu(ks.tatCaNhanVien());
        return thanhCong(JsonObject().set("ma", cn->layMa()).set("ten", cn->layTen()).str(),
                         "Da them chi nhanh thanh cong! (" + cn->layMa() + " - " + cn->layTen() + ")");
    }
    // Kiem tra quan he du lieu truoc khi xoa (web hien ly do trong modal xac nhan)
    if (r.path == "/api/chinhanh/kiemtraxoa")
    {
        string ma = InputHelper::chuanHoaMa(chuoi(r, "ma", true));
        if (!InputHelper::laMa(ma, "CN"))
            throw invalid_argument("Ma chi nhanh sai dinh dang.");
        if (!ks.timChiNhanh(ma))
            throw runtime_error("Ma chi nhanh " + ma + " khong ton tai.");
        JsonArray ly;
        for (auto &x : ks.lyDoKhongXoaDuocChiNhanh(ma))
            ly.push(x);
        return thanhCong(JsonObject().set("ma", ma).set("xoaDuoc", ly.str() == "[]").setRaw("lyDo", ly.str()).str());
    }
    if (r.path == "/api/chinhanh/xoa")
    {
        if (!admin)
            return loi(403, "Chuc nang danh rieng cho ADMIN.");
        if (r.method != "POST")
            return loi(405, "Phai dung POST.");
        string ma = InputHelper::chuanHoaMa(chuoi(r, "ma", true));
        auto cn = ks.timChiNhanh(ma);
        if (!cn)
            throw runtime_error("Ma chi nhanh " + ma + " khong ton tai.");
        string ten = cn->layTen();
        ks.xoaChiNhanh(ma); // tu choi kem ly do neu con phong / nhan vien / lich su
        return thanhCong("", "Da xoa chi nhanh thanh cong! (" + ma + " - " + ten + ")");
    }
    if (r.path == "/api/phong")
    {
        string maCN = InputHelper::chuanHoaMa(r.param("maCN"));
        JsonArray a;
        for (auto &x : ks.tatCaPhong())
        {
            if (!maCN.empty() && ks.maChiNhanhCuaPhong(x) != maCN)
                continue;
            a.pushRaw(phongJson(*x));
        }
        return thanhCong(a.str());
    }
    if (r.path == "/api/phong/chitiet")
    {
        auto x = ks.timPhong(chuoi(r, "ma", true));
        if (!x)
            throw runtime_error("Khong tim thay phong.");
        JsonArray gia, lich;
        for (int m = 1; m <= 4; ++m)
            gia.push(JsonObject()
                         .set("mua", muaToString((MuaDuLich)m))
                         .set("heSo", heSoMua((MuaDuLich)m))
                         .set("motDem", x->tinhGiaPhong(1, (MuaDuLich)m))
                         .set("bayDem", x->tinhGiaPhong(7, (MuaDuLich)m)));
        for (auto &ph : ks.lichDatCuaPhong(x->layMaPhong()))
            lich.pushRaw(phieuJson(*ph));
        return thanhCong(JsonObject().setRaw("phong", phongJson(*x)).setRaw("giaTheoMua", gia.str())
                             .setRaw("lichDat", lich.str()).str());
    }
    // Kiem tra lich trong cua cac phong cho mot khoang ngay (dung cho form dat phong)
    if (r.path == "/api/phong/kiemtra")
    {
        Ngay nn = docNgay(chuoi(r, "ngayNhan", true));
        int soDem = soNguyen(r, "soDem"), soNguoi = soNguyen(r, "soNguoi");
        if (soNguoi < 1 || soNguoi > 20)
            throw invalid_argument("So khach phai tu 1 den 20.");
        MuaDuLich mua = docMua(r, nn);
        string maCN = r.param("maCN");
        JsonArray a;
        for (auto &x : ks.tatCaPhong())
        {
            auto cn = ks.chiNhanhCuaPhong(x->layMaPhong());
            if (!maCN.empty() && (!cn || cn->layMa() != InputHelper::chuanHoaMa(maCN)))
                continue;
            string lyDo = ks.lyDoKhongDatDuoc(*x, nn, soDem, soNguoi);
            a.pushRaw(JsonObject()
                          .setRaw("phong", phongJson(*x))
                          .set("datDuoc", lyDo.empty())
                          .set("lyDo", lyDo)
                          .set("tienPhong", x->tinhGiaPhong(soDem, mua))
                          .str());
        }
        return thanhCong(a.str());
    }
    if (r.path == "/api/phong/trangthai")
    {
        auto x = ks.timPhong(chuoi(r, "ma", true));
        if (!x)
            throw runtime_error("Khong tim thay phong.");
        string hd = r.param("hanhDong"), truoc = x->layTenTrangThai();
        if (hd == "dondep")
            x->donPhong();
        else if (hd == "baotri")
            ks.chuyenBaoTri(x->layMaPhong(), chuoi(r, "lyDo", true)); // tu choi neu con phieu dat/dang o
        else if (hd == "hoantat")
            x->hoanTat();
        else
            throw invalid_argument("Hanh dong khong hop le.");
        ks.dongBoTheoNgay();
        return thanhCong(phongJson(*x), "Phong " + x->layMaPhong() + ": " + truoc + " -> " + x->layTenTrangThai());
    }

    if (!admin)
        return loi(403, "Chuc nang danh rieng cho ADMIN.");
    if (r.path == "/api/phong/them")
    {
        auto cn = ks.timChiNhanh(chuoi(r, "maCN", true));
        if (!cn)
            throw runtime_error("Khong tim thay chi nhanh.");
        string so = chuoi(r, "so", true);
        if (!InputHelper::laSoDuong(so))
            throw invalid_argument("So phong phai la so duong.");
        if (cn->coSoPhong(InputHelper::chuanHoaSoDuong(so)))
            throw invalid_argument("So phong " + so + " da co trong chi nhanh " + cn->layTen() + ".");
        string ma = ks.taoMaPhong(), loai = r.param("loai");
        int tang = soNguyen(r, "tang"), sc = soNguyen(r, "sucChua");
        double gia = soThuc(r, "gia"), dt = soThuc(r, "dienTich");
        if (gia < 500000 || gia > 2000000)
            throw invalid_argument("Gia phong phai tu 500.000 den 2.000.000 VND.");
        // Gioi han suc chua theo loai phong
        auto kiemTraSC = [](const string &loai, int sc) {
            if (loai == "Standard" && (sc < 1 || sc > 2))
                throw invalid_argument("Standard chi nhan suc chua 1-2 khach.");
            if ((loai == "Deluxe" || loai == "Suite") && (sc < 2 || sc > 6))
                throw invalid_argument(loai + " chi nhan suc chua 2-6 khach.");
            if (loai == "Villa" && (sc < 8 || sc > 20))
                throw invalid_argument("Villa chi nhan suc chua 8-20 khach.");
        };
        kiemTraSC(loai, sc);
        shared_ptr<Phong> x;
        if (loai == "Standard")
            x = make_shared<StandardRoom>(ma, so, tang, gia, sc, dt, chuoi(r, "huongNhin", true));
        else if (loai == "Deluxe")
            x = make_shared<DeluxeRoom>(ma, so, tang, gia, sc, dt, coKhong(r, "banCong"), chuoi(r, "huongNhin", true));
        else if (loai == "Suite")
            x = make_shared<Suite>(ma, so, tang, gia, sc, dt, soNguyen(r, "soPhongNgu"), coKhong(r, "phongKhach"),
                                   soThuc(r, "phiBuffet"));
        else if (loai == "Villa")
            x = make_shared<Villa>(ma, so, tang, gia, sc, dt, coKhong(r, "hoBoi"), soThuc(r, "sanVuon"),
                                   soNguyen(r, "soPhongNgu"), soThuc(r, "phiQuanGia"));
        else
            throw invalid_argument("Loai phong khong hop le.");
        ks.themPhong(cn->layMa(), x);
        return thanhCong(phongJson(*x), "Them phong " + x->layMaPhong() + " thanh cong!");
    }
    if (r.path == "/api/phong/sua")
    {
        auto x = ks.timPhong(chuoi(r, "ma", true));
        if (!x)
            throw runtime_error("Khong tim thay phong.");
        // Cac truong anh huong gia: chi doi khi phong khong con phieu (giong console)
        auto doiGia = [&](double cu, double moi)
        {
            if (fabs(cu - moi) > 0.5)
                ks.kiemTraCoTheDoiGia(x->layMaPhong());
        };
        double gia = soThuc(r, "gia");
        // Dong bo quy tac gia voi Console: 500.000 - 2.000.000 VND
        if (gia < 500000 || gia > 2000000)
            throw invalid_argument("Gia phong phai tu 500.000 den 2.000.000 VND.");
        int sc = soNguyen(r, "sucChua");
        {
            string loai = loaiPhongToString(x->layLoaiPhong());
            if (loai == "Standard" && (sc < 1 || sc > 2))
                throw invalid_argument("Standard chi nhan suc chua 1-2 khach.");
            if ((loai == "Deluxe" || loai == "Suite") && (sc < 2 || sc > 6))
                throw invalid_argument(loai + " chi nhan suc chua 2-6 khach.");
            if (loai == "Villa" && (sc < 8 || sc > 20))
                throw invalid_argument("Villa chi nhan suc chua 8-20 khach.");
        }
        doiGia(x->layGiaPhong(), gia);
        doiGia(x->laySucChua(), sc);
        auto su = dynamic_pointer_cast<Suite>(x);
        auto vi = dynamic_pointer_cast<Villa>(x);
        if (su)
            doiGia(su->layPhiBuffetSang(), soThuc(r, "phiBuffet"));
        if (vi)
            doiGia(vi->layPhiQuanGia(), soThuc(r, "phiQuanGia"));

        string so = chuoi(r, "so", true);
        if (InputHelper::chuanHoaSoDuong(so) != x->laySoPhong())
            ks.doiSoPhong(x->layMaPhong(), so);
        x->datGiaCoBan(gia);
        x->datSucChua(sc);
        x->datDienTich(soThuc(r, "dienTich"));
        x->datTang(soNguyen(r, "tang"));
        if (auto st = dynamic_pointer_cast<StandardRoom>(x))
            st->datHuongNhin(chuoi(r, "huongNhin", true));
        else if (auto dl = dynamic_pointer_cast<DeluxeRoom>(x))
        {
            dl->datHuongNhin(chuoi(r, "huongNhin", true));
            dl->datCoBanCong(coKhong(r, "banCong"));
        }
        else if (su)
        {
            su->datSoPhongNgu(soNguyen(r, "soPhongNgu"));
            su->datCoPhongKhach(coKhong(r, "phongKhach"));
            su->datPhiBuffetSang(soThuc(r, "phiBuffet"));
        }
        else if (vi)
        {
            vi->datCoHoBoi(coKhong(r, "hoBoi"));
            vi->datDienTichSanVuon(soThuc(r, "sanVuon"));
            vi->datSoPhongNgu(soNguyen(r, "soPhongNgu"));
            vi->datPhiQuanGia(soThuc(r, "phiQuanGia"));
        }
        return thanhCong(phongJson(*x), "Cap nhat phong " + x->layMaPhong() + " thanh cong!");
    }
    if (r.path == "/api/phong/xoa")
    {
        string ma = InputHelper::chuanHoaMa(chuoi(r, "ma", true));
        ks.xoaPhong(ma);
        return thanhCong("", "Da xoa phong " + ma + ".");
    }
    return loi(404, "API khong ton tai.");
}

// =====================================================================
//                              KHACH HANG
// =====================================================================
HttpResponse WebApi::apiKhachHang(const HttpRequest &r, const Phien &p)
{
    if (r.path == "/api/khachhang")
    {
        // maCN (tuy chon): chi tra khach thuoc chi nhanh do (theo maChiNhanh / lich su)
        string maCN = InputHelper::chuanHoaMa(r.param("maCN"));
        JsonArray a;
        for (auto &kh : ks.layDanhSachKhachHang())
        {
            if (!maCN.empty())
            {
                const string &gan = kh->layMaChiNhanh();
                if (!gan.empty())
                {
                    if (gan != maCN)
                        continue;
                }
                else
                {
                    // Tuong thich du lieu cu: loc theo lich su dat phong
                    bool thuoc = false;
                    for (auto &ph : kh->layLichSuDatPhong())
                        if (ks.maChiNhanhCuaPhong(ph->layPhong()) == maCN)
                        {
                            thuoc = true;
                            break;
                        }
                    if (!thuoc)
                        continue;
                }
            }
            a.pushRaw(khachJson(*kh));
        }
        return thanhCong(a.str());
    }
    if (r.path == "/api/khachhang/chitiet")
    {
        auto kh = ks.timKhachHang(chuoi(r, "ma", true));
        if (!kh)
            throw runtime_error("Khong tim thay khach hang.");
        auto ls = kh->layLichSuDatPhong();
        sort(ls.begin(), ls.end(), [](const shared_ptr<PhieuDatPhong> &a, const shared_ptr<PhieuDatPhong> &b)
             { return a->layThoiGianDat() > b->layThoiGianDat(); });
        JsonArray a;
        for (auto &ph : ls)
            a.pushRaw(phieuJson(*ph));
        return thanhCong(JsonObject().setRaw("khach", khachJson(*kh)).setRaw("lichSu", a.str()).str());
    }
    if (r.path == "/api/khachhang/them")
    {
        const string diaChi = chuoi(r, "diaChiThuongTru");
        if (!diaChi.empty() && !InputHelper::laDiaChi(diaChi))
            throw invalid_argument("Dia chi khong hop le! Cho phep chu, so va , . / - ( ) #, dai 5-120 ky tu.");
        auto kh = ks.themKhachHang(chuoi(r, "hoTen", true), chuoi(r, "sdt", true), chuoi(r, "cccd", true),
                                   chuoi(r, "email"), chuoi(r, "quocTich").empty() ? "Viet Nam" : chuoi(r, "quocTich"),
                                   diaChi, chuoi(r, "gioiTinh"));
        // Gan chi nhanh that vao ho so: maCN request hoac chi nhanh NV dang nhap
        string maCN = chuoi(r, "maCN");
        if (maCN.empty())
        {
            if (auto nv = nhanVienCua(p))
                maCN = nv->layMaChiNhanh();
        }
        if (!maCN.empty())
        {
            if (!ks.timChiNhanh(maCN))
                throw invalid_argument("Ma chi nhanh khong ton tai: " + maCN);
            kh->datMaChiNhanh(maCN);
        }
        return thanhCong(khachJson(*kh), "Them khach hang " + kh->layMa() + " thanh cong!");
    }
    if (r.path == "/api/khachhang/sua")
    {
        auto kh = ks.timKhachHang(chuoi(r, "ma", true));
        if (!kh)
            throw runtime_error("Khong tim thay khach hang.");
        string sdt = chuoi(r, "sdt", true), cccd = chuoi(r, "cccd", true);
        ks.kiemTraTrungKhach(sdt, cccd, kh->layMa());
        int h = soNguyen(r, "maHang");
        if (h < 0 || h > 3)
            throw invalid_argument("Hang thanh vien khong hop le.");
        kh->datHoTen(chuoi(r, "hoTen", true));
        kh->datSoDienThoai(sdt);
        kh->datCCCD(cccd);
        kh->datEmail(chuoi(r, "email"));
        kh->datQuocTich(chuoi(r, "quocTich"));
        if (!r.param("diaChiThuongTru").empty())
        {
            const string diaChi = chuoi(r, "diaChiThuongTru");
            if (!InputHelper::laDiaChi(diaChi))
                throw invalid_argument("Dia chi khong hop le! Cho phep chu, so va , . / - ( ) #, dai 5-120 ky tu.");
            kh->datDiaChiThuongTru(diaChi);
        }
        if (!r.param("gioiTinh").empty())
            kh->datGioiTinh(chuoi(r, "gioiTinh"));
        kh->datHang((HangThanhVien)h);
        return thanhCong(khachJson(*kh), "Cap nhat khach hang " + kh->layMa() + " thanh cong!");
    }
    return loi(404, "API khong ton tai.");
}

// =====================================================================
//                       DAT PHONG & CHECK-IN
// =====================================================================
HttpResponse WebApi::apiDatPhong(const HttpRequest &r, const Phien &p)
{
    if (r.path == "/api/phieu")
    {
        string tt = r.param("trangThai"), maCN = InputHelper::chuanHoaMa(r.param("maCN"));
        JsonArray a;
        auto ds = ks.layDanhSachPhieu();
        sort(ds.begin(), ds.end(), [](const shared_ptr<PhieuDatPhong> &x, const shared_ptr<PhieuDatPhong> &y)
             { return x->layThoiGianDat() > y->layThoiGianDat(); });
        for (auto &ph : ds)
        {
            if (!tt.empty() && to_string((int)ph->layTrangThai()) != tt)
                continue;
            auto cn = ks.chiNhanhCuaPhong(ph->layPhong()->layMaPhong());
            if (!maCN.empty() && (!cn || cn->layMa() != maCN))
                continue;
            a.pushRaw(phieuJson(*ph));
        }
        return thanhCong(a.str());
    }
    if (r.path == "/api/phieu/chitiet")
    {
        auto ph = ks.timPhieu(chuoi(r, "ma", true));
        if (!ph)
            throw runtime_error("Khong tim thay phieu.");
        return thanhCong(phieuJson(*ph));
    }
    // Bao gia: tien phong + dich vu di kem, tinh qua chuoi Decorator (giong console)
    if (r.path == "/api/baogia")
    {
        auto x = ks.timPhong(chuoi(r, "maPhong", true));
        if (!x)
            throw runtime_error("Khong tim thay phong.");
        Ngay nn = docNgay(chuoi(r, "ngayNhan", true));
        int soDem = soNguyen(r, "soDem");
        MuaDuLich mua = docMua(r, nn);
        shared_ptr<Phong> boc = x;
        JsonArray dsDV;
        for (auto &d : docDanhSachDichVu(r.param("dichVu")))
        {
            auto dv = ks.timDichVu(d.first);
            if (!dv)
                throw runtime_error("Khong tim thay dich vu " + d.first + ".");
            boc = QuanLyKhachSan::bocDichVu(boc, dv, d.second);
            dsDV.push(JsonObject()
                          .set("ma", dv->layMa())
                          .set("ten", dv->layTen())
                          .set("soLuong", d.second)
                          .set("thanhTien", dv->tinhChiPhi(d.second)));
        }
        double tienPhong = x->tinhGiaPhong(soDem, mua), tamTinh = boc->tinhGiaPhong(soDem, mua);
        double tyGiam = 0;
        if (auto kh = ks.timKhachHang(r.param("maKH")))
            tyGiam = kh->tyLeGiamGia();
        KetQuaTinhTien kq = HoaDon::tinhTien(tamTinh, tyGiam); // CUNG cong thuc voi hoa don that
        double giam = kq.tienGiam, thue = kq.tienThue;
        double tranCoc = ks.tranCocToiDa(*x, soDem, mua, tyGiam);
        return thanhCong(JsonObject()
                             .set("mua", muaToString(mua))
                             .set("maMua", (int)mua)
                             .set("heSo", heSoMua(mua))
                             .set("ngayTra", nn.congNgay(soDem).toString())
                             .set("tienPhong", tienPhong)
                             .set("tienDichVu", tamTinh - tienPhong)
                             .set("tamTinh", tamTinh)
                             .set("giam", giam)
                             .set("thue", thue)
                             .set("tyLeGiam", tyGiam)
                             .set("tong", kq.tong)
                             .set("tranCoc", tranCoc)
                             .set("canhBaoMua", canhBaoNhieuMua(nn, soDem))
                             .set("cocGoiY", min(tranCoc, round(kq.tong * 0.3 / 1000) * 1000))
                             .set("chuoiDecorator", boc->moTa())
                             .set("lichTrong", ks.lyDoKhongDatDuoc(*x, nn, soDem, 1))
                             .setRaw("dichVu", dsDV.str())
                             .str());
    }
    if (r.path == "/api/datphong")
    {
        auto nv = nhanVienCua(p);
        string maKH = r.param("maKH");
        if (maKH.empty()) // khach moi
        {
            const string diaChi = chuoi(r, "diaChiThuongTru");
            if (!diaChi.empty() && !InputHelper::laDiaChi(diaChi))
                throw invalid_argument("Dia chi khong hop le! Cho phep chu, so va , . / - ( ) #, dai 5-120 ky tu.");
            maKH = ks.themKhachHang(chuoi(r, "hoTen", true), chuoi(r, "sdt", true), chuoi(r, "cccd", true),
                                    chuoi(r, "email"), chuoi(r, "quocTich").empty() ? "Viet Nam" : chuoi(r, "quocTich"),
                                    diaChi, chuoi(r, "gioiTinh"))
                       ->layMa();
        }
        Ngay nn = docNgay(chuoi(r, "ngayNhan", true));
        double coc = r.param("tienCoc").empty() ? 0 : soThuc(r, "tienCoc");
        string ptCoc = coc > 0 ? chuoi(r, "ptCoc", true) : "";
        int soDemDat = soNguyen(r, "soDem"), soNguoiDat = soNguyen(r, "soNguoi");
        if (soNguoiDat < 1 || soNguoiDat > 20)
            throw invalid_argument("So khach phai tu 1 den 20.");
        auto ph = ks.datPhong(maKH, chuoi(r, "maPhong", true), nv, nn, soDemDat, soNguoiDat,
                              docMua(r, nn), chuoi(r, "ghiChu"), coc, ptCoc);
        for (auto &d : docDanhSachDichVu(r.param("dichVu"))) // dich vu dat truoc
            ks.ghiDichVu(ph->layMaPhieu(), d.first, d.second);
        string tb = "Dat phong thanh cong! Ma phieu " + ph->layMaPhieu();
        if (coKhong(r, "nhanPhongNgay") && nn == Ngay::homNay())
        {
            ks.checkIn(ph->layMaPhieu());
            tb += " - da check-in";
        }
        return thanhCong(phieuJson(*ph), tb);
    }
    // Chuyen phieu sang phong khac: danh sach phong phu hop cho TOAN BO khoang ngay cua phieu
    if (r.path == "/api/phieu/phongchuyen")
    {
        auto ph = ks.timPhieu(chuoi(r, "ma", true));
        if (!ph)
            throw runtime_error("Khong tim thay phieu.");
        bool cungLoai = r.param("cungLoai", "1") != "0";
        JsonArray a;
        for (auto &x : ks.phongCoTheChuyen(ph->layMaPhieu(), cungLoai))
            a.pushRaw(JsonObject().setRaw("phong", phongJson(*x))
                          .set("tienPhongMoi", x->tinhGiaPhong(ph->laySoDem(), ph->layMua())).str());
        return thanhCong(JsonObject().setRaw("phieu", phieuJson(*ph)).set("loi", ks.kiemTraChuyenPhong(ph->layMaPhieu()))
                             .set("cungLoai", cungLoai).setRaw("phong", a.str()).str());
    }
    if (r.path == "/api/phieu/chuyenphong")
    {
        if (r.method != "POST")
            return loi(405, "Phai dung POST.");
        string ma = chuoi(r, "ma", true);
        auto cu = ks.chuyenPhongPhieu(ma, chuoi(r, "maPhong", true)); // cap nhat chinh phieu cu, khong tao phieu moi
        auto ph = ks.timPhieu(ma);
        return thanhCong(phieuJson(*ph), "Chuyen phong thanh cong! Phieu " + ph->layMaPhieu() + ": phong cu " +
                                             cu->laySoPhong() + " -> phong moi " + ph->layPhong()->laySoPhong() + ".");
    }
    if (r.path == "/api/checkin")
    {
        string ma = chuoi(r, "ma", true);
        ks.checkIn(ma);
        return thanhCong(phieuJson(*ks.timPhieu(ma)), "Check-in thanh cong!");
    }
    if (r.path == "/api/huyphieu")
    {
        string ma = chuoi(r, "ma", true);
        ks.huyDatPhong(ma, chuoi(r, "lyDo"), coKhong(r, "hoanCoc"));
        return thanhCong(phieuJson(*ks.timPhieu(ma)), "Da huy phieu " + InputHelper::chuanHoaMa(ma) + ".");
    }
    // Gia han cho khach DANG O: cung phieu, cung phong; bao xung dot neu da co phieu khac. xemTruoc=1 chi kiem tra.
    if (r.path == "/api/giahan")
    {
        string ma = chuoi(r, "ma", true);
        int them = soNguyenTrong(r, "themDem", 1, 30, "So dem gia han");
        auto ph = ks.timPhieu(ma);
        if (!ph)
            throw runtime_error("Khong tim thay phieu.");
        double truoc = ph->tinhTienPhong(), sau = ph->tinhTienPhong(them);
        string ngayTraMoi = ph->layNgayTra().congNgay(them).toString();
        if (coKhong(r, "xemTruoc"))
        {
            string ly = ks.kiemTraGiaHan(ma, them);
            return thanhCong(JsonObject().set("hopLe", ly.empty()).set("lyDo", ly).set("ngayTraMoi", ngayTraMoi)
                                 .set("tienPhongCu", truoc).set("tienPhongMoi", sau).set("chenhLech", sau - truoc).str());
        }
        ks.giaHan(ma, them);
        return thanhCong(phieuJson(*ks.timPhieu(ma)), "Da gia han " + to_string(them) + " dem, ngay tra moi " + ngayTraMoi + ".");
    }
    if (r.path == "/api/ghidichvu")
    {
        string ma = chuoi(r, "ma", true);
        ks.ghiDichVu(ma, chuoi(r, "maDV", true), soNguyen(r, "soLuong"));
        return thanhCong(phieuJson(*ks.timPhieu(ma)), "Da ghi dich vu vao phieu.");
    }
    return loi(404, "API khong ton tai.");
}

// =====================================================================
//                                DICH VU
// =====================================================================
HttpResponse WebApi::apiDichVu(const HttpRequest &r, const Phien &p)
{
    if (r.path == "/api/dichvu")
    {
        JsonArray a;
        for (auto &dv : ks.layDanhSachDichVu())
            a.pushRaw(dichVuJson(*dv));
        return thanhCong(a.str());
    }
    if (p.role != Role::ADMIN)
        return loi(403, "Chuc nang danh rieng cho ADMIN.");
    if (r.path == "/api/dichvu/them")
    {
        string loai = r.param("loai"), ten = chuoi(r, "ten", true), ma = ks.taoMaDichVu();
        double gia = soThuc(r, "donGia");
        shared_ptr<DichVu> dv;
        if (loai == "Am thuc")
            dv = make_shared<AmThuc>(ma, ten, gia, chuoi(r, "loaiBuaAn", true));
        else if (loai == "Spa")
            dv = make_shared<Spa>(ma, ten, gia, soNguyen(r, "thoiLuong"));
        else if (loai == "Tour")
            dv = make_shared<TourTuyen>(ma, ten, gia, chuoi(r, "diemDen", true), soNguyen(r, "soNgay"));
        else
            throw invalid_argument("Loai dich vu khong hop le.");
        ks.themDichVu(dv);
        return thanhCong(dichVuJson(*dv), "Them dich vu " + dv->layMa() + " thanh cong!");
    }
    auto dv = ks.timDichVu(chuoi(r, "ma", true));
    if (!dv)
        throw runtime_error("Khong tim thay dich vu.");
    if (r.path == "/api/dichvu/sua")
    {
        string ten = chuoi(r, "ten", true);
        for (auto &d : ks.layDanhSachDichVu())
            if (d != dv && InputHelper::chuHoa(d->layTen()) == InputHelper::chuHoa(ten))
                throw runtime_error("Ten dich vu da ton tai.");
        dv->datTen(ten);
        dv->datDonGia(soThuc(r, "donGia"));
        return thanhCong(dichVuJson(*dv), "Cap nhat dich vu " + dv->layMa() + " thanh cong!");
    }
    if (r.path == "/api/dichvu/xoa")
    {
        string ma = dv->layMa();
        ks.xoaDichVu(ma);
        return thanhCong("", "Da xoa dich vu " + ma + ".");
    }
    return loi(404, "API khong ton tai.");
}

// =====================================================================
//                         THANH TOAN & HOA DON
// =====================================================================
HttpResponse WebApi::apiThanhToan(const HttpRequest &r, const Phien &p)
{
    if (r.path == "/api/hoadon")
    {
        string maCN = InputHelper::chuanHoaMa(r.param("maCN")); // rong = toan he thong (tim kiem quan tri)
        auto ds = ks.hoaDonCuaChiNhanh(maCN);
        sort(ds.begin(), ds.end(), [](const shared_ptr<HoaDon> &a, const shared_ptr<HoaDon> &b)
             { return a->layThoiGianLap() > b->layThoiGianLap(); });
        JsonArray a;
        for (auto &hd : ds)
            a.pushRaw(hoaDonJson(*hd));
        return thanhCong(a.str());
    }
    if (r.path == "/api/hoadon/chitiet")
    {
        auto hd = ks.timHoaDon(chuoi(r, "ma", true));
        if (!hd)
            throw runtime_error("Khong tim thay hoa don.");
        return thanhCong(hoaDonJson(*hd));
    }
    if (r.path == "/api/tamtinh")
        return thanhCong(hoaDonJson(*ks.xemTamTinh(chuoi(r, "ma", true), nhanVienCua(p))));
    if (r.path == "/api/checkout")
    {
        auto hd = ks.checkOut(chuoi(r, "ma", true), nhanVienCua(p));
        if (hd->conLaiPhaiTra() < 0.5) // coc da du
            ks.thanhToanHoaDon(hd->layMa(), "Tien coc");
        return thanhCong(hoaDonJson(*hd), "Check-out thanh cong, da lap hoa don " + hd->layMa() + ".");
    }
    if (r.path == "/api/thanhtoan")
    {
        string ma = chuoi(r, "ma", true), pt = chuoi(r, "phuongThuc", true);
        ks.thanhToanHoaDon(ma, pt);
        auto hd = ks.timHoaDon(ma);
        return thanhCong(hoaDonJson(*hd), "Thanh toan hoa don " + hd->layMa() + " thanh cong!");
    }
    return loi(404, "API khong ton tai.");
}

// =====================================================================
//                              NHAN VIEN
// =====================================================================
HttpResponse WebApi::apiNhanVien(const HttpRequest &r, const Phien &p)
{
    // USER: chi duoc xem danh sach nhan vien (cung chi nhanh), khong luong / khong thao tac quan tri
    if (r.path == "/api/nhanvien")
    {
        JsonArray a;
        const bool admin = p.role == Role::ADMIN;
        string maCNUser;
        if (!admin)
        {
            if (p.maNhanVien.empty())
                return thanhCong("[]"); // USER khong gan nhan vien -> khong xem danh sach
            auto me = ks.timNhanVien(p.maNhanVien);
            if (!me)
                return thanhCong("[]");
            maCNUser = me->layMaChiNhanh();
        }
        for (auto &nv : ks.tatCaNhanVien(admin && coKhong(r, "caNguoiDaNghi")))
        {
            if (!admin)
            {
                if (!nv->layDangLam())
                    continue;
                if (nv->layMaChiNhanh() != maCNUser)
                    continue;
            }
            if (admin)
                a.pushRaw(nhanVienJson(*nv));
            else
            {
                // Ban cong khai cho USER: khong luong, khong CCCD, khong SĐT, khong email ca nhan
                auto cn = ks.timChiNhanh(nv->layMaChiNhanh());
                a.push(JsonObject()
                           .set("ma", nv->layMa())
                           .set("hoTen", nv->layHoTen())
                           .set("chucVu", nv->layChucVu())
                           .set("maCN", nv->layMaChiNhanh())
                           .set("chiNhanh", cn ? cn->layTen() : "")
                           .set("dangLam", nv->layDangLam())
                           .set("tamNghi", nv->layTamNghi()));
            }
        }
        return thanhCong(a.str());
    }
    if (p.role != Role::ADMIN)
        return loi(403, "Chuc nang danh rieng cho ADMIN.");
    if (r.path == "/api/nhanvien/them")
    {
        const string chucVu = chuoi(r, "chucVu", true);
        const bool duocCapTaiKhoan = laChucVuQuanLy(chucVu) || laChucVuLeTan(chucVu);
        const bool taoTaiKhoan = coKhong(r, "taoTaiKhoan");
        if (taoTaiKhoan && !duocCapTaiKhoan)
            throw invalid_argument("Chi quan ly va le tan moi duoc cap tai khoan.");
        const Role role = laChucVuQuanLy(chucVu) ? Role::ADMIN : Role::USER;
        if (taoTaiKhoan)
        {
            QuanLyTaiKhoan::kiemTraUsername(chuoi(r, "username", true));
            QuanLyTaiKhoan::kiemTraMatKhau(r.param("password"));
            if (qlTaiKhoan.timTheoUsername(chuoi(r, "username", true)))
                throw invalid_argument("Username '" + chuoi(r, "username", true) + "' da ton tai.");
        }
        auto nv = ks.themNhanVien(chuoi(r, "maCN", true), chuoi(r, "hoTen", true), chuoi(r, "sdt", true),
                                  chucVu, soThuc(r, "luong"), chuoi(r, "email"), chuoi(r, "cccd"),
                                  chuoi(r, "gioiTinh"), chuoi(r, "ngaySinh"));
        string tb = "Them nhan vien " + nv->layMa() + " thanh cong!";
        if (taoTaiKhoan)
        {
            qlTaiKhoan.themTaiKhoan(chuoi(r, "username", true), r.param("password"),
                                    role, nv->layMa());
            tb += " Da tao tai khoan.";
        }
        return thanhCong(nhanVienJson(*nv), tb);
    }
    auto nv = ks.timNhanVien(chuoi(r, "ma", true));
    if (!nv || !nv->layDangLam())
        throw runtime_error("Khong tim thay nhan vien dang lam viec.");
    if (r.path == "/api/nhanvien/sua")
    {
        const string chucVu = chuoi(r, "chucVu", true);
        if (const auto tk = qlTaiKhoan.timTheoNhanVien(nv->layMa());
            tk && !chucVuKhopVaiTro(chucVu, tk->layRole()))
            throw invalid_argument("Khong the doi chuc vu khi tai khoan khong con phu hop vai tro.");
        string sdt = chuoi(r, "sdt", true);
        for (auto &x : ks.tatCaNhanVien())
            if (x != nv && x->laySoDienThoai() == sdt)
                throw invalid_argument("So dien thoai da duoc nhan vien " + x->layMa() + " su dung.");
        nv->datHoTen(chuoi(r, "hoTen", true));
        nv->datSoDienThoai(sdt);
        nv->datEmail(chuoi(r, "email"));
        nv->datChucVu(chucVu);
        nv->datLuongCoBan(soThuc(r, "luong"));
        if (!r.param("cccd").empty())
            nv->datCCCD(chuoi(r, "cccd"));
        if (!r.param("gioiTinh").empty())
            nv->datGioiTinh(chuoi(r, "gioiTinh"));
        if (!r.param("ngaySinh").empty())
            nv->datNgaySinh(chuoi(r, "ngaySinh"));
        if (!r.param("tamNghi").empty())
            nv->datTamNghi(coKhong(r, "tamNghi"));
        return thanhCong(nhanVienJson(*nv), "Cap nhat nhan vien " + nv->layMa() + " thanh cong!");
    }
    if (r.path == "/api/nhanvien/nghiviec")
    {
        if (nv->layMa() == p.maNhanVien)
            throw runtime_error("Khong the cho chinh minh nghi viec.");
        if (auto tk = qlTaiKhoan.timTheoNhanVien(nv->layMa()))
            qlTaiKhoan.xoaTaiKhoan(tk->layUsername());
        ks.xoaNhanVien(nv->layMa());
        return thanhCong("", nv->layHoTen() + " da nghi viec.");
    }
    return loi(404, "API khong ton tai.");
}

// =====================================================================
//                          BAO CAO & THONG KE
// =====================================================================
HttpResponse WebApi::apiBaoCao(const HttpRequest &r, const Phien &p)
{
    if (p.role != Role::ADMIN)
        return loi(403, "Chuc nang danh rieng cho ADMIN.");
    string ky = r.param("ky", "thang");
    int soKy = ky == "ngay" ? 14 : ky == "tuan" ? 8 : ky == "quy" ? 8 : ky == "nam" ? 5 : 12;
    // Pham vi thong ke: mot chi nhanh (maCN) hoac toan he thong (rong)
    string maCN = InputHelper::chuanHoaMa(r.param("maCN"));
    auto cnLoc = maCN.empty() ? nullptr : ks.timChiNhanh(maCN);
    if (!maCN.empty() && !cnLoc)
        throw invalid_argument("Khong tim thay chi nhanh " + maCN + ".");
    auto doanhThu = ks.doanhThuTheoKy(ky, soKy, maCN);

    JsonArray congSuat, dichVu, khach, nhanVien, phieu;
    for (auto &cn : ks.layDanhSachChiNhanh())
    {
        if (cnLoc && cn != cnLoc)
            continue;
        int tong = (int)cn->layDanhSachPhong().size(), o = cn->demPhong(MaTrangThai::DANG_O);
        congSuat.push(JsonObject()
                          .set("chiNhanh", cn->layTen())
                          .set("tong", tong)
                          .set("trong", cn->demPhong(MaTrangThai::TRONG))
                          .set("giuCho", cn->demPhong(MaTrangThai::GIU_CHO))
                          .set("dangO", o)
                          .set("dangDon", cn->demPhong(MaTrangThai::DANG_DON))
                          .set("baoTri", cn->demPhong(MaTrangThai::BAO_TRI))
                          .set("phanTram", tong ? 100.0 * o / tong : 0.0));
    }
    for (auto &x : ks.thongKeDichVu(maCN))
        dichVu.push(JsonObject().set("ma", x.ma).set("ten", x.ten).set("loai", x.loai).set("soLuong", x.soLuong)
                        .set("doanhThu", x.doanhThu));
    for (auto &x : ks.topKhachHang(10, maCN))
        khach.push(JsonObject()
                       .set("ma", x.khach->layMa())
                       .set("hoTen", x.khach->layHoTen())
                       .set("hang", x.khach->hangThanhVien())
                       .set("soLan", x.soHoaDon)
                       .set("tongChi", x.tongChi));
    for (auto &x : ks.thongKeNhanVien())
        if (!cnLoc || x.nhanVien->layMaChiNhanh() == maCN)
        nhanVien.push(JsonObject()
                          .set("ma", x.nhanVien->layMa())
                          .set("hoTen", x.nhanVien->layHoTen())
                          .set("chucVu", x.nhanVien->layChucVu())
                          .set("dangLam", x.nhanVien->layDangLam())
                          .set("soPhieu", x.soPhieuLap)
                          .set("soHoaDon", x.soHoaDonLap)
                          .set("doanhThu", x.doanhThu));
    vector<pair<string, double>> pt;
    auto dsPhieu = ks.phieuCuaChiNhanh(maCN);
    for (int i = 0; i <= (int)TrangThaiPhieu::DA_HUY; ++i)
    {
        int d = 0;
        for (auto &ph : dsPhieu)
            d += (int)ph->layTrangThai() == i ? 1 : 0;
        pt.push_back({trangThaiPhieuToString((TrangThaiPhieu)i), (double)d});
    }

    double tongKy = 0;
    for (auto &x : doanhThu)
        tongKy += x.second;
    JsonArray taiChinhChiNhanh;
    for (const auto &cn : ks.layDanhSachChiNhanh())
    {
        if (cnLoc && cn != cnLoc)
            continue;
        const double doanhThuThang = ks.doanhThuThangHienTai(cn->layMa());
        const double chiPhiLuong = ks.chiPhiLuongThang(cn->layMa());
        taiChinhChiNhanh.push(JsonObject()
                                  .set("maCN", cn->layMa())
                                  .set("chiNhanh", cn->layTen())
                                  .set("doanhThuThang", doanhThuThang)
                                  .set("chiPhiLuongThang", chiPhiLuong)
                                  .set("mucTieuDoanhThu3x", chiPhiLuong * 3.0)
                                  .set("datMucTieu3x", doanhThuThang >= chiPhiLuong * 3.0));
    }
    const double doanhThuThang = ks.doanhThuThangHienTai(maCN);
    const double chiPhiLuongThang = ks.chiPhiLuongThang(maCN);
    return thanhCong(JsonObject()
                         .set("ky", ky)
                         .set("tongKy", tongKy)
                         .set("maCN", maCN)
                         .set("phamVi", cnLoc ? cnLoc->layTen() : "")
                         .set("tongDoanhThu", ks.tongDoanhThu(maCN))
                         .set("doanhThuPhong", ks.doanhThuPhong(maCN))
                         .set("doanhThuDichVu", ks.doanhThuDichVu(maCN))
                         .set("doanhThuThang", doanhThuThang)
                         .set("chiPhiLuongThang", chiPhiLuongThang)
                         .set("mucTieuDoanhThu3x", chiPhiLuongThang * 3.0)
                         .set("datMucTieu3x", doanhThuThang >= chiPhiLuongThang * 3.0)
                         .set("congSuatHienTai", ks.congSuatPhong(maCN))
                         .setRaw("doanhThu", seriesJson(doanhThu))
                         .setRaw("theoChiNhanh", seriesJson(ks.doanhThuTheoChiNhanh()))
                         .setRaw("theoLoaiPhong", seriesJson(ks.doanhThuTheoLoaiPhong(maCN)))
                         .setRaw("congSuat", congSuat.str())
                         .setRaw("dichVu", dichVu.str())
                         .setRaw("khachHang", khach.str())
                         .setRaw("nhanVien", nhanVien.str())
                         .setRaw("phieuTheoTrangThai", seriesJson(pt))
                         .setRaw("taiChinhChiNhanh", taiChinhChiNhanh.str())
                         .str());
}

// =====================================================================
//                    WEBSITE CHO KHACH (CONG KHAI)
// =====================================================================
// Phieu dat phong trong C++ bat buoc co NhanVien lap phieu. Dat phong online duoc ghi nhan
// cho mot nhan vien dai dien "Kênh Đặt Phòng Online" (tao 1 lan bang ham themNhanVien co san) ->
// bao cao "Nhan vien" se cho biet so phieu den tu kenh online.
shared_ptr<NhanVien> WebApi::nhanVienKenhOnline()
{
    for (auto &nv : ks.tatCaNhanVien())
        if (nv->layChucVu() == "Kênh Đặt Phòng Online")
            return nv;
    auto &dsCN = ks.layDanhSachChiNhanh();
    if (dsCN.empty())
        throw runtime_error("He thong chua co chi nhanh.");
    // So dien thoai phai duy nhat: thu hotline chi nhanh, sau do cac so du phong
    vector<string> sdt = {dsCN[0]->layHotline(), "0999000001", "0999000002", "0999000003"};
    for (auto &s : sdt)
    {
        try
        {
                    return ks.themNhanVien(dsCN[0]->layMa(), "Kênh Đặt Phòng Online", s, "Kênh Đặt Phòng Online",
                                           NhanVien::LUONG_CO_BAN_TOI_THIEU);
        }
        catch (const exception &)
        {
        }
    }
    throw runtime_error("Khong tao duoc kenh dat phong online.");
}

HttpResponse WebApi::apiKhach(const HttpRequest &r)
{
    // Thong tin gioi thieu: chi nhanh, loai phong (gia tu), dich vu
    if (r.path == "/api/khach/thongtin")
    {
        JsonArray cn, loai, dv;
        for (auto &c : ks.layDanhSachChiNhanh())
            cn.push(JsonObject().set("ma", c->layMa()).set("ten", c->layTen()).set("diaChi", c->layDiaChi())
                        .set("hotline", c->layHotline()).set("soPhong", (int)c->layDanhSachPhong().size()));
        const char *tenLoai[] = {"Standard", "Deluxe", "Suite", "Villa"};
        for (int i = 0; i < 4; ++i)
        {
            auto ds = ks.locPhong([i](const Phong &p) { return (int)p.layLoaiPhong() == i; });
            if (ds.empty())
                continue;
            double giaTu = ds[0]->layGiaPhong(), dtTu = ds[0]->layDienTich(), dtDen = dtTu;
            int scMax = 0;
            for (auto &p : ds)
            {
                giaTu = min(giaTu, p->layGiaPhong());
                dtTu = min(dtTu, p->layDienTich());
                dtDen = max(dtDen, p->layDienTich());
                scMax = max(scMax, p->laySucChua());
            }
            JsonArray tn;
            for (auto &t : ds[0]->layDichVuDiKem())
                tn.push(t);
            loai.push(JsonObject().set("loai", tenLoai[i]).set("giaTu", giaTu).set("sucChua", scMax)
                          .set("dienTichTu", dtTu).set("dienTichDen", dtDen).set("soPhong", (int)ds.size())
                          .setRaw("tienNghi", tn.str()));
        }
        for (auto &d : ks.layDanhSachDichVu())
            dv.pushRaw(dichVuJson(*d));
        return thanhCong(JsonObject().setRaw("chiNhanh", cn.str()).setRaw("loaiPhong", loai.str())
                             .setRaw("dichVu", dv.str()).set("homNay", isoNgay(Ngay::homNay())).str());
    }
    // Phong con trong theo ngay - chi tra ve phong dat duoc (khong lo ten khach da dat)
    if (r.path == "/api/khach/phongtrong")
    {
        Ngay nn = docNgay(chuoi(r, "ngayNhan", true));
        int soDem = soNguyen(r, "soDem"), soNguoi = soNguyen(r, "soNguoi");
        if (soNguoi < 1 || soNguoi > 20)
            throw invalid_argument("So khach phai tu 1 den 20.");
        if (nn < Ngay::homNay())
            throw invalid_argument("Ngay nhan phong khong duoc o qua khu.");
        if (soDem < 1 || soDem > 60)
            throw invalid_argument("So dem phai tu 1 den 60.");
        MuaDuLich mua = xacDinhMua(nn);
        string maCN = InputHelper::chuanHoaMa(r.param("maCN"));
        JsonArray a;
        for (auto &x : ks.tatCaPhong())
        {
            auto cn = ks.chiNhanhCuaPhong(x->layMaPhong());
            if (!maCN.empty() && (!cn || cn->layMa() != maCN))
                continue;
            if (!ks.lyDoKhongDatDuoc(*x, nn, soDem, soNguoi).empty())
                continue;
            JsonArray tn, rieng;
            for (auto &t : x->layDichVuDiKem())
                tn.push(t);
            for (auto &t : x->layThongTinRieng())
                rieng.push(JsonObject().set("k", t.first).set("v", t.second));
            a.push(JsonObject().set("ma", x->layMaPhong()).set("so", x->laySoPhong())
                       .set("loai", loaiPhongToString(x->layLoaiPhong())).set("chiNhanh", x->layChiNhanh())
                       .set("maCN", cn ? cn->layMa() : "").set("gia", x->layGiaPhong()).set("sucChua", x->laySucChua())
                       .set("dienTich", x->layDienTich()).set("tang", x->layTang())
                       .set("tienPhong", x->tinhGiaPhong(soDem, mua)).setRaw("tienNghi", tn.str())
                       .setRaw("thongTinRieng", rieng.str()));
        }
        return thanhCong(JsonObject().set("mua", muaToString(mua)).set("heSo", heSoMua(mua))
                             .set("canhBaoMua", canhBaoNhieuMua(nn, soDem))
                             .set("ngayTra", nn.congNgay(soDem).toString()).setRaw("phong", a.str()).str());
    }
    // Danh sach TAT CA phong theo loai (cho trang danh sach phong) + trang thai + co the dat theo ngay
    if (r.path == "/api/khach/danhsachphong")
    {
        string loaiStr = chuoi(r, "loai", true);
        Ngay nn = docNgay(chuoi(r, "ngayNhan", true));
        int soDem = soNguyen(r, "soDem"), soNguoi = soNguyen(r, "soNguoi");
        if (soNguoi < 1 || soNguoi > 20)
            throw invalid_argument("So khach phai tu 1 den 20.");
        if (nn < Ngay::homNay())
            throw invalid_argument("Ngay nhan phong khong duoc o qua khu.");
        if (soDem < 1 || soDem > 60)
            throw invalid_argument("So dem phai tu 1 den 60.");
        MuaDuLich mua = xacDinhMua(nn);
        string maCN = InputHelper::chuanHoaMa(r.param("maCN"));
        JsonArray a;
        for (auto &x : ks.tatCaPhong())
        {
            if (loaiPhongToString(x->layLoaiPhong()) != loaiStr)
                continue;
            auto cn = ks.chiNhanhCuaPhong(x->layMaPhong());
            if (!maCN.empty() && (!cn || cn->layMa() != maCN))
                continue;
            string lyDo = ks.lyDoKhongDatDuoc(*x, nn, soDem, soNguoi);
            bool coTheDat = lyDo.empty();
            JsonArray tn, rieng;
            for (auto &t : x->layDichVuDiKem())
                tn.push(t);
            for (auto &t : x->layThongTinRieng())
                rieng.push(JsonObject().set("k", t.first).set("v", t.second));
            a.push(JsonObject()
                       .set("ma", x->layMaPhong())
                       .set("so", x->laySoPhong())
                       .set("loai", loaiPhongToString(x->layLoaiPhong()))
                       .set("chiNhanh", x->layChiNhanh())
                       .set("maCN", cn ? cn->layMa() : "")
                       .set("gia", x->layGiaPhong())
                       .set("sucChua", x->laySucChua())
                       .set("dienTich", x->layDienTich())
                       .set("tang", x->layTang())
                       .set("maTrangThai", (int)x->layMaTrangThai())
                       .set("trangThai", x->layTenTrangThai())
                       .set("ghiChuTrangThai", x->ghiChuTrangThai())
                       .set("coTheDat", coTheDat)
                       .set("lyDoKhongDat", lyDo)
                       .set("tienPhong", x->tinhGiaPhong(soDem, mua))
                       .setRaw("tienNghi", tn.str())
                       .setRaw("thongTinRieng", rieng.str()));
        }
        return thanhCong(JsonObject()
                             .set("loai", loaiStr)
                             .set("mua", muaToString(mua))
                             .set("heSo", heSoMua(mua))
                             .set("canhBaoMua", canhBaoNhieuMua(nn, soDem))
                             .set("ngayTra", nn.congNgay(soDem).toString())
                             .setRaw("phong", a.str())
                             .str());
    }
    // Bao gia (Decorator) - ap giam gia thanh vien neu khach nhap SDT da co trong he thong
    if (r.path == "/api/khach/baogia")
    {
        auto x = ks.timPhong(chuoi(r, "maPhong", true));
        if (!x)
            throw runtime_error("Khong tim thay phong.");
        Ngay nn = docNgay(chuoi(r, "ngayNhan", true));
        int soDem = soNguyen(r, "soDem");
        MuaDuLich mua = xacDinhMua(nn);
        shared_ptr<Phong> boc = x;
        for (auto &d : docDanhSachDichVu(r.param("dichVu")))
            if (auto dv = ks.timDichVu(d.first))
                boc = QuanLyKhachSan::bocDichVu(boc, dv, d.second);
        double tienPhong = x->tinhGiaPhong(soDem, mua), tamTinh = boc->tinhGiaPhong(soDem, mua);
        // Khach da la thanh vien (nhap dung SDT) -> ap CUNG muc giam nhu hoa don that
        double tyGiam = 0;
        string sdt = chuoi(r, "sdt");
        if (!sdt.empty())
            if (auto kh = ks.timKhachTheoSDT(sdt))
                tyGiam = kh->tyLeGiamGia();
        KetQuaTinhTien kq = HoaDon::tinhTien(tamTinh, tyGiam);
        return thanhCong(JsonObject().set("tienPhong", tienPhong).set("tienDichVu", tamTinh - tienPhong)
                             .set("tamTinh", kq.tamTinh).set("tyLeGiam", tyGiam).set("giam", kq.tienGiam)
                             .set("thue", kq.tienThue).set("tong", kq.tong)
                             .set("canhBaoMua", canhBaoNhieuMua(nn, soDem))
                             .set("mua", muaToString(mua)).set("ngayTra", nn.congNgay(soDem).toString()).str());
    }
    // Dat phong online: tim/tao khach hang roi goi QuanLyKhachSan::datPhong nhu le tan
    if (r.path == "/api/khach/datphong" && r.method == "POST")
    {
        string sdt = chuoi(r, "sdt", true), cccd = InputHelper::chuHoa(chuoi(r, "cccd", true));
        const string gioiTinh = chuoi(r, "gioiTinh");
        const string diaChi = chuoi(r, "diaChiThuongTru");
        if (!diaChi.empty() && (diaChi.size() < 5 || diaChi.size() > 120 || !InputHelper::laChuThuan(diaChi)))
            throw invalid_argument("Dia chi chi duoc chua chu cai va khoang trang.");
        // Kiem tra phong, ngay, dich vu TRUOC khi tao khach (khong de lai khach "mo coi" khi dat that bai)
        // va khong lo ten khach khac trong thong bao trung lich
        auto phongDat = ks.timPhong(chuoi(r, "maPhong", true));
        if (!phongDat)
            throw runtime_error("Khong tim thay phong.");
        Ngay nn = docNgay(chuoi(r, "ngayNhan", true));
        int soDem = soNguyen(r, "soDem"), soNguoi = soNguyen(r, "soNguoi");
        if (nn < Ngay::homNay())
            throw invalid_argument("Ngay nhan phong khong duoc o qua khu.");
        if (soDem < 1 || soDem > 60)
            throw invalid_argument("So dem phai tu 1 den 60.");
        if (soNguoi < 1 || soNguoi > 20)
            throw invalid_argument("So khach phai tu 1 den 20.");
        string lyDo = ks.lyDoKhongDatDuoc(*phongDat, nn, soDem, soNguoi);
        if (batDau(lyDo, "Trung "))
            throw runtime_error("Phong " + phongDat->laySoPhong() + " vua co khach dat trong thoi gian nay. Vui long chon phong khac.");
        if (!lyDo.empty())
            throw runtime_error("Khong dat duoc phong " + phongDat->laySoPhong() + ": " + lyDo + ".");
        auto dsDV = docDanhSachDichVu(r.param("dichVu"));
        for (auto &d : dsDV)
            if (!ks.timDichVu(d.first) || d.second > 100)
                throw invalid_argument("Dich vu khong hop le: " + d.first);

        auto theoSdt = ks.timKhachTheoSDT(sdt), theoCccd = ks.timKhachTheoCCCD(cccd);
        shared_ptr<KhachHang> kh;
        if (theoSdt || theoCccd)
        {
            // Khach cu: SDT va CCCD phai cung thuoc mot nguoi
            if (theoSdt != theoCccd)
                throw runtime_error("So dien thoai hoac CCCD da duoc dang ky voi thong tin khac. Vui long lien he le tan.");
            kh = theoSdt;
        }
        else
            kh = ks.themKhachHang(chuoi(r, "hoTen", true), sdt, cccd, chuoi(r, "email"),
                                  chuoi(r, "quocTich").empty() ? "Viet Nam" : chuoi(r, "quocTich"),
                                  diaChi, gioiTinh);
        auto ph = ks.datPhong(kh->layMa(), phongDat->layMaPhong(), nhanVienKenhOnline(), nn, soDem, soNguoi,
                              xacDinhMua(nn), "Đặt online. " + chuoi(r, "ghiChu"));
        for (auto &d : dsDV)
            ks.ghiDichVu(ph->layMaPhieu(), d.first, d.second);
        double tong = HoaDon::tinhTien(ks.taoPhongCoDichVu(*ph)->tinhGiaPhong(ph->laySoDem(), ph->layMua()),
                                       kh->tyLeGiamGia()).tong; // cung cong thuc & muc giam voi hoa don
        return thanhCong(JsonObject().set("maPhieu", ph->layMaPhieu()).set("khach", kh->layHoTen())
                             .set("chiNhanh", ph->layPhong()->layChiNhanh()).set("phong", ph->layPhong()->laySoPhong())
                             .set("loai", loaiPhongToString(ph->layPhong()->layLoaiPhong()))
                             .set("ngayNhan", ph->layNgayNhan().toString()).set("ngayTra", ph->layNgayTra().toString())
                             .set("tong", tong).set("giamGia", kh->tyLeGiamGia()).str());
    }
    // Tra cuu phieu / hoa don cong khai: bat buoc ma phieu + so dien thoai dung
    if (r.path == "/api/khach/tracuuphieu")
    {
        string ma = InputHelper::chuanHoaMa(chuoi(r, "ma", true));
        string sdt = InputHelper::trim(chuoi(r, "sdt", true));
        auto ph = ks.timPhieu(ma);
        if (!ph || ph->layKhachHang()->laySoDienThoai() != sdt)
            throw runtime_error("Khong tim thay phieu. Kiem tra lai ma phieu va so dien thoai.");
        JsonObject o;
        o.setRaw("phieu", phieuJson(*ph));
        if (!ph->layMaHoaDon().empty())
        {
            if (auto hd = ks.timHoaDon(ph->layMaHoaDon()))
                o.setRaw("hoaDon", hoaDonJson(*hd));
        }
        return thanhCong(o.str());
    }
    if (r.path == "/api/khach/tracuhoadon")
    {
        string ma = InputHelper::chuanHoaMa(chuoi(r, "ma", true));
        string sdt = InputHelper::trim(chuoi(r, "sdt", true));
        auto hd = ks.timHoaDon(ma);
        if (!hd)
        {
            // Cho phep nhap ma phieu thay vi ma hoa don
            if (auto ph = ks.timPhieu(ma))
            {
                if (ph->layKhachHang()->laySoDienThoai() != sdt)
                    throw runtime_error("Khong tim thay hoa don. Kiem tra lai ma va so dien thoai.");
                if (ph->layMaHoaDon().empty())
                    throw runtime_error("Phieu " + ph->layMaPhieu() + " chua co hoa don (chua check-out).");
                hd = ks.timHoaDon(ph->layMaHoaDon());
            }
        }
        if (!hd || hd->layPhieu()->layKhachHang()->laySoDienThoai() != sdt)
            throw runtime_error("Khong tim thay hoa don. Kiem tra lai ma hoa don/phieu va so dien thoai.");
        return thanhCong(hoaDonJson(*hd));
    }
    return loi(404, "API khong ton tai.");
}
