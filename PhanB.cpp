#include <iostream>
#include <string>
#include <iomanip>

using namespace std;

// ==========================================
// 1. LỚP CƠ SỞ TRỪU TƯỢNG: DichVu
// ==========================================
class DichVu
{
private:
    string maDichVu;

protected:
    string tenDichVu;
    double donGia;
    string moTaDichVu;

public:
    DichVu() : maDichVu(""), tenDichVu(""), donGia(0.0), moTaDichVu("") {}

    DichVu(string ma, string ten, double gia, string moTa)
        : maDichVu(ma), tenDichVu(ten), donGia(gia), moTaDichVu(moTa) {}

    virtual ~DichVu() {}

    // Phương thức ảo tính chi phí
    virtual double tinhChiPhi(int soLuong) const
    {
        return donGia * soLuong;
    }

    // Phương thức ảo lấy loại dịch vụ
    virtual string layLoaiDichVu() const = 0;

    // Getters & Setters
    string layMaDichVu() const { return maDichVu; }
    string layTenDichVu() const { return tenDichVu; }
    double layDonGia() const { return donGia; }
    void datDonGia(double gia) { donGia = gia; }

    // Operator xuất stream
    friend ostream &operator<<(ostream &os, const DichVu &dv)
    {
        os << "[" << dv.maDichVu << "] " << dv.tenDichVu
           << " | Đơn gia: " << fixed << setprecision(0) << dv.donGia << " VNĐ"
           << " | Mô ta: " << dv.moTaDichVu;
        return os;
    }
};

// ==========================================
// 2. LỚP DỊCH VỤ CỤ THỂ: AmThuc
// ==========================================
class AmThuc : public DichVu
{
protected:
    string loaiBuaAn;
    int soSuatAn;
    double phiPhucVu;

public:
    AmThuc() : DichVu(), loaiBuaAn(""), soSuatAn(0), phiPhucVu(0.0) {}

    AmThuc(string ma, string ten, double gia, string moTa, string loaiBua, int soSuat, double phiDichVu)
        : DichVu(ma, ten, gia, moTa), loaiBuaAn(loaiBua), soSuatAn(soSuat), phiPhucVu(phiDichVu) {}

    // Ghi đè phương thức tính chi phí: donGia * soLuong + phiPhucVu
    double tinhChiPhi(int soLuong) const override
    {
        return (donGia * soLuong) + phiPhucVu;
    }

    string layLoaiDichVu() const override
    {
        return "Am thuc (" + loaiBuaAn + ")";
    }
};

// ==========================================
// 3. LỚP DỊCH VỤ CỤ THỂ: Spa
// ==========================================
class Spa : public DichVu
{
protected:
    int thoiLuongPhut;
    string loaiLieuTrinh;
    string tenKyThuatVien;

public:
    Spa() : DichVu(), thoiLuongPhut(0), loaiLieuTrinh(""), tenKyThuatVien("") {}

    Spa(string ma, string ten, double gia, string moTa, int thoiLuong, string lieuTrinh, string ktv)
        : DichVu(ma, ten, gia, moTa), thoiLuongPhut(thoiLuong), loaiLieuTrinh(lieuTrinh), tenKyThuatVien(ktv) {}

    double tinhChiPhi(int soLuong) const override
    {
        return donGia * soLuong;
    }

    string layLoaiDichVu() const override
    {
        return "Spa & Tri lieu";
    }
};

// ==========================================
// 4. LỚP DỊCH VỤ CỤ THỂ: TourTuyen
// ==========================================
class TourTuyen : public DichVu
{
protected:
    string diaDiem;
    int soNguoiThamGia;
    string ngayKhoiHanh;
    double phiHuongDanVien;

public:
    TourTuyen() : DichVu(), diaDiem(""), soNguoiThamGia(0), ngayKhoiHanh(""), phiHuongDanVien(0.0) {}

    TourTuyen(string ma, string ten, double gia, string moTa, string dd, int soNguoi, string ngay, double phiHDV)
        : DichVu(ma, ten, gia, moTa), diaDiem(dd), soNguoiThamGia(soNguoi), ngayKhoiHanh(ngay), phiHuongDanVien(phiHDV) {}

    // Ghi đè phương thức tính chi phí: donGia * soLuong + phiHuongDanVien
    double tinhChiPhi(int soLuong) const override
    {
        return (donGia * soLuong) + phiHuongDanVien;
    }

    string layLoaiDichVu() const override
    {
        return "Tour du lich";
    }
};

// ==========================================
// HÀM MAIN MINH HỌA (DEMO)
// ==========================================
int main()
{
    // Khởi tạo các dịch vụ
    AmThuc buffet("DV01", "Buffet Hai San Cao Cap", 500000, "Buffet toi", "Toi", 2, 50000);
    Spa massage("DV02", "Massage Đa Nong", 800000, "Thư gian toan than", 60, "Toan than", "Nguyen Van A");
    TourTuyen tour("DV03", "Tour Tham Quan Đao", 1200000, "Kham pha đao ngoc", "Phu Quoc", 4, "15/10/2026", 300000);

    // Mảng con trỏ lớp cơ sở để minh họa tính Đa hình
    DichVu *danhSachDV[] = {&buffet, &massage, &tour};

    cout << "================ DANH SACH DICH VU KHACH SAN/RESORT ================\n\n";

    for (int i = 0; i < 3; ++i)
    {
        int soLuongSuDung = 2; // Ví dụ khách dùng 2 suất/lần
        cout << "Dich vu " << i + 1 << ": " << *danhSachDV[i] << endl;
        cout << " -> Loai dich vu: " << danhSachDV[i]->layLoaiDichVu() << endl;
        cout << " -> Tong chi phi (" << soLuongSuDung << " luot): "
             << fixed << setprecision(0) << danhSachDV[i]->tinhChiPhi(soLuongSuDung) << " VNĐ" << endl;
        cout << "----------------------------------------------------------------------\n";
    }

    return 0;
}