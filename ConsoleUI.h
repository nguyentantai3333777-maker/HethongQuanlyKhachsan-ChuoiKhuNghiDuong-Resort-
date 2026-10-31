// ConsoleUI.h - Ham ho tro giao dien console: mau theo tung menu, khung, bang, bieu do, xoa man hinh
//
// CAN GIUA THEO KHOI (block):
//   Moi man hinh (menu, danh sach, form...) duoc xem la MOT khoi giao dien. Moi thu in ra std::cout duoc giu
//   trong bo dem (BoDemCanGiua) cho den luc can nhap lieu / dung man hinh; khi do khoi duoc do kich thuoc
//   (rong x cao) roi dat vao giua cua so console THAT (doc lai kich thuoc moi lan ve -> doi kich thuoc cua so
//   thi man hinh sau tu can lai). Khong hard-code so khoang trang.
//     - Dong thuoc khung / bang (bat dau bang UI::KHOI) duoc can giua trong khoi.
//     - Dong chu tu do (thong bao, o nhap) thang le trai cua khoi.
//     - Menu co o "Nhap lua chon" NAM TRONG khung; con tro duoc dua vao dung o do.
//   Khung mac dinh dung ASCII (+=|) de khong bao gio vo khung tren moi font; HRM_UNICODE=1 -> khung Unicode.
//   Thong tin / nhan / danh muc lien quan KHACH HANG luon in mau vang UI::KHACH (\033[1;33m).
//   Mau ANSI chi la lop trang tri: do rong tinh bo qua ma mau.
#pragma once
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace UI
{
    // Do rong mac dinh; tu thu nho khi cua so hep (capNhatKichThuoc goi moi lan xoa man hinh)
    extern int RONG;             // khung tieu de / thong tin (gom ca 2 vien)
    extern int RONG_MENU;        // do rong toi da cua khung menu
    extern int RONG_KHOI;        // khung thong tin / hoa don / bao gia
    extern int RONG_BANG_TOI_DA; // bang rong hon muc nay se tu thu hep cot

    // Ky tu danh dau dau dong: "dong nay thuoc mot khung/bang -> can giua trong khoi"
    extern const std::string KHOI;

    // Moi menu mot mau chu de (mau nhat, de nhin tren nen toi)
    enum class ChuDe
    {
        // Dong bo voi giao dien Web Ocean Pearl Resort (frontend/css/oceanpearl.css)
        CHINH,      // xanh bien      (Dashboard)
        PHONG,      // nau caramel    (Quan ly phong)
        DICH_VU,    // hong           (Dich vu)
        DAT_PHONG,  // xanh la        (Dat phong)
        THANH_TOAN, // vang           (Thanh toan)
        KHACH_HANG, // xam ghi        (Khach hang)
        NHAN_SU,    // xanh ngoc      (Nhan vien)
        THONG_KE,   // nau dam        (Bao cao & thong ke)
        TAI_KHOAN   // xam xanh       (Cai dat / tai khoan)
    };

    // ===== Mau co ban (khong phai const: console khong ho tro ANSI se tat mau) =====
    extern std::string RESET, DAM, CHU, MO;
    extern std::string XANH_TROI, VANG, HONG, XANH_LA, CAM, NGOC, DO, TIM, NAU, XAM, CARAMEL, XAM_XANH;
    extern std::string KHACH; // \033[1;33m VANG: moi thong tin / nhan / danh muc lien quan den khach hang

    void datChuDe(ChuDe c);
    const std::string &mau();     // mau chu cua chu de hien tai
    std::string mauCua(ChuDe c);  // mau chu cua mot chu de bat ky

    void khoiTaoConsole();
    void initDoubleBuffer();
    void cleanupDoubleBuffer();
    void xoaManHinh();
    void pauseScreen();

    // ===== Kich thuoc cua so & can giua khoi =====
    int getConsoleWidth();                     // so cot dang hien thi cua cua so console
    int getConsoleHeight();                    // so dong dang hien thi
    int calculateCenterX(int blockWidth);      // (rong cua so - rong khoi) / 2, khong am
    int calculateCenterY(int blockHeight);     // (cao cua so - cao khoi) / 2, khong am
    void setCursorPosition(int x, int y);      // toa do tuyet doi trong cua so (0,0 = goc tren trai)
    void capNhatKichThuoc();                   // tinh lai RONG... theo cua so hien tai
    void batDauCanGiua();                      // gan bo dem can giua vao std::cout (che do console)
    void ketThucCanGiua();                     // ve not phan con lai, tra std::cout ve nhu cu
    void hienThi();                            // ve khoi dang cho ra man hinh (goi truoc moi lan nhap)
    // Ghi thang ra console (bo qua bo dem can giua) — dung khi echo ky tu nhap lieu
    void ghiTrucTiep(const std::string &s);
    int demLanXoaManHinh(); // so lan xoaManHinh() da goi (dung de in huong dan phim 1 lan / man hinh)
    bool coONhapMenu();                        // vua in menu co o nhap trong khung va chua in gi them
    std::string docTaiONhapMenu();             // dua con tro vao o nhap trong khung menu roi doc 1 dong

    // ===== Xu ly chuoi theo do rong hien thi (UTF-8) =====
    int doRong(const std::string &s);
    std::string lap(const std::string &s, int n);
    std::string catNgan(const std::string &s, int w);
    std::string canTrai(const std::string &s, int w);
    std::string canPhai(const std::string &s, int w);
    std::string canGiuaChuoi(const std::string &s, int w);

    // ===== Thanh phan giao dien =====
    std::vector<std::string> ngatDong(const std::string &s, int w); // ngat theo tu, giu nguyen dau xuong dong

    // ===== Khung dung chung (vien theo mau chu de; noi dung luon canh le dung do rong) =====
    void khungTren(int w, const std::string &tieu = "");
    void khungNgang(int w);
    void khungDuoi(int w);
    void khungDong(const std::string &chu, int w, const std::string &mauChu = "");
    void khungDongGiua(const std::string &chu, int w, const std::string &mauChu = "");
    void khungDong2(const std::string &trai, const std::string &phai, int w, const std::string &mauTrai = "",
                    const std::string &mauPhai = "");
    void khungNhan(const std::string &nhan, const std::string &giaTri, int w, int rongNhan = 14,
                   const std::string &mauGiaTri = "");
    void khungTien(const std::string &nhan, double soTien, int w, const std::string &mauDong = "",
                   const std::string &dau = "");

    void inBanner();
    void tieuDe(const std::string &s, const std::string &phu = "");
    void tieuDeNho(const std::string &s);
    void duongKe(int rong = RONG, bool dam = false);
    // muc[i] hien thi voi so (i+1); mauSo (tuy chon) = mau rieng cho tung so. O "Nhap lua chon" nam trong khung.
    void inMenu(const std::vector<std::string> &muc, const std::string &mucThoat = "Quay lai",
                const std::vector<std::string> &mauSo = {});
    // Menu co tieu de nam CHUNG mot khung voi cac muc (dung cho cac man hinh menu chinh / menu con)
    void inMenuKhung(const std::string &tieuDe, const std::string &phu, const std::vector<std::string> &muc,
                     const std::string &mucThoat = "Quay lai", const std::vector<std::string> &mauSo = {});

    // ===== Menu tuong tac (mui ten ↑↓ + ENTER + ESC) =====
    enum class Phim { LEN, XUONG, ENTER, ESC, SO, KHAC };
    // docPhim: tra ve phim; neu la SO thi soPhim = '1'..'9' (hoac '0')
    Phim docPhim(char *soPhim = nullptr); // doc 1 phim (mui ten / Enter / ESC / so) - khong can nhan Enter them
    // Ve menu + cho phep di chuyen bang ↑↓, Enter chon, ESC = 0 (quay lai). Khong dung phim so de chon nhanh.
    // Tra ve 1..n = chon muc, 0 = ESC/quay lai. Muc dang chon co ▶ ◀ va mau chu de.
    // veLaiHeader: goi truoc moi lan ve menu (vd inThanhTrangThai + bang du lieu)
    int chonMenuTuongTac(const std::string &tieuDe, const std::string &phu,
                         const std::vector<std::string> &muc,
                         const std::string &mucThoat = "Quay lai",
                         const std::vector<std::string> &mauSo = {},
                         int chonBanDau = 0,
                         const std::function<void()> &veLaiHeader = nullptr);
    void thanhCong(const std::string &s);
    void loi(const std::string &s);
    void canhBao(const std::string &s);
    void thongTin(const std::string &s);
    void dongThongTin(const std::string &nhan, const std::string &giaTri, const std::string &mauGiaTri = "",
                      int rongNhan = 20);
    void inKhoi(const std::string &tieuDe, const std::vector<std::pair<std::string, std::string>> &dong,
                int rongNhan = 20, bool khachHang = false); // khachHang = true: toan khoi MAU VANG
    void dongTien(const std::string &nhan, double soTien, const std::string &mauDong = "", int rong = 70,
                  const std::string &dau = "");
    void inChiSo(const std::vector<std::pair<std::string, std::string>> &chiSo);
    void veBieuDoCot(const std::string &tieuDe, const std::vector<std::pair<std::string, double>> &duLieu,
                     bool laTien = true, const std::string &donVi = "");

    // ===== Bang du lieu co khung =====
    class Bang
    {
    public:
        struct Cot
        {
            std::string ten;
            int rong;
            bool canPhai;
        };
        explicit Bang(const std::vector<Cot> &cot, bool canGiuaTatCaO = true);
        void themDong(const std::vector<std::string> &o, const std::vector<std::string> &mau = {});
        void in(const std::string &khiRong = "(Khong co du lieu)", bool hienSoDong = false) const;
        size_t soDong() const { return dong.size(); }
        // Mau rieng cho dong tieu de cot (vd KHACH cho bang khach hang); rong = mau chu de
        std::string mauTieuDe;

    private:
        std::vector<Cot> cot;
        bool canGiuaTatCaO;
        std::vector<std::vector<std::string>> dong;
        std::vector<std::vector<std::string>> mauDong;
        int tongRong() const;
    };
}
