// WebApi.h - Lop giao tiep giua Frontend (HTML/CSS/JS) va logic C++
// Moi API chi CHUYEN DOI du lieu (HTTP <-> JSON) roi goi dung cac ham san co cua QuanLyKhachSan /
// QuanLyTaiKhoan - giong het cach ChuongTrinh (console) dang goi. Khong co logic nghiep vu rieng cho web.
//
// Quy uoc:  request  = GET query hoac POST form (a=1&b=2), header X-Token = phien dang nhap
//           response = {"ok":true,"data":...}  hoac  {"ok":false,"loi":"..."}
#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>
#include "HttpServer.h"
#include "QuanLyKhachSan.h"
#include "TaiKhoan.h"

class WebApi
{
public:
    WebApi(QuanLyKhachSan &ks, QuanLyTaiKhoan &tk, std::function<void()> luuDuLieu, const std::string &thuMucWeb);
    HttpResponse xuLy(const HttpRequest &req); // diem vao duy nhat cho moi request

private:
    struct Phien
    {
        std::string username;
        Role role;
        std::string maNhanVien;
    };

    QuanLyKhachSan &ks;
    QuanLyTaiKhoan &qlTaiKhoan;
    std::function<void()> luu;
    std::string thuMucWeb;
    std::map<std::string, Phien> phien; // token -> phien

    // ---- Tien ich ----
    HttpResponse fileTinh(const std::string &duongDan) const;
    static HttpResponse thanhCong(const std::string &dataJson, const std::string &thongBao = "");
    static HttpResponse loi(int status, const std::string &thongDiep);
    const Phien *layPhien(const HttpRequest &req) const;
    std::shared_ptr<NhanVien> nhanVienCua(const Phien &p) const;

    // ---- Chuyen doi doi tuong C++ -> JSON ----
    std::string phongJson(const Phong &p) const;
    std::string khachJson(const KhachHang &kh) const;
    std::string phieuJson(const PhieuDatPhong &ph) const;
    std::string hoaDonJson(const HoaDon &hd) const;
    std::string dichVuJson(const DichVu &dv) const;
    std::string nhanVienJson(const NhanVien &nv) const;
    static std::string seriesJson(const std::vector<std::pair<std::string, double>> &ds);

    // ---- Nhom API ----
    HttpResponse apiTaiKhoan(const HttpRequest &r, const Phien &p);
    HttpResponse apiTongQuan(const HttpRequest &r, const Phien &p);
    HttpResponse apiPhong(const HttpRequest &r, const Phien &p);
    HttpResponse apiKhachHang(const HttpRequest &r, const Phien &p);
    HttpResponse apiDatPhong(const HttpRequest &r, const Phien &p);
    HttpResponse apiDichVu(const HttpRequest &r, const Phien &p);
    HttpResponse apiThanhToan(const HttpRequest &r, const Phien &p);
    HttpResponse apiNhanVien(const HttpRequest &r, const Phien &p);
    HttpResponse apiBaoCao(const HttpRequest &r, const Phien &p);

    // ---- Website cho khach (khong can dang nhap, khong lo thong tin khach khac) ----
    HttpResponse apiKhach(const HttpRequest &r);
    std::shared_ptr<NhanVien> nhanVienKenhOnline(); // "nhan vien" dai dien kenh dat phong online
};
