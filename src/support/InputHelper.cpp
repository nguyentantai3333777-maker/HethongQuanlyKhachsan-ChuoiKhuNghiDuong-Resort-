// InputHelper.cpp
#include <algorithm>
#include "InputHelper.h"
#include "ConsoleUI.h"
#include "TienIch.h"
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <cstdio>
#ifdef _WIN32
#include <conio.h>
#include <io.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

using namespace std;

namespace InputHelper
{
    static bool escHuyDangBat = false;
    // Lan xoa man hinh luc dong "ESC: Quay lai" duoc in gan nhat (-1 = chua in).
    // Dong huong dan chi in 1 LAN cho moi man hinh / moi form, nam duoi bang va phia tren o nhap.
    static int lanXoaKhiInHuongDan = -1;

    PhamViHuyBangEsc::PhamViHuyBangEsc() : trangThaiTruoc(escHuyDangBat)
    {
        escHuyDangBat = true;
        lanXoaKhiInHuongDan = -1; // form moi -> in lai huong dan 1 lan
    }

    PhamViHuyBangEsc::~PhamViHuyBangEsc()
    {
        escHuyDangBat = trangThaiTruoc;
    }

    // =================================================================
    //  Xu ly chuoi
    // =================================================================
    string trim(const string &s)
    {
        size_t dau = s.find_first_not_of(" \t\r\n");
        if (dau == string::npos)
            return "";
        size_t cuoi = s.find_last_not_of(" \t\r\n");
        return s.substr(dau, cuoi - dau + 1);
    }

    string chuHoa(string s)
    {
        for (size_t i = 0; i < s.size(); ++i)
            s[i] = (char)toupper((unsigned char)s[i]);
        return s;
    }

    string chuanHoaHoTen(const string &s)
    {
        string kq;
        bool dauTu = true;
        for (char c : trim(s))
        {
            if (c == ' ' || c == '\t')
            {
                if (!dauTu)
                    kq += ' ';
                dauTu = true;
                continue;
            }
            unsigned char u = (unsigned char)c;
            if (u < 128)
                kq += (char)(dauTu ? toupper(u) : tolower(u));
            else
                kq += c;
            dauTu = false;
        }
        return trim(kq);
    }

    string chuanHoaMa(const string &s)
    {
        // Cho phep nhap ma co khoang trang vo tinh (vi du "DP 01" -> "DP01").
        string dauVao = trim(s);
        dauVao.erase(remove_if(dauVao.begin(), dauVao.end(), [](unsigned char c) { return isspace(c); }), dauVao.end());
        string t = chuHoa(dauVao);
        size_t i = t.find_first_of("0123456789");
        if (i == string::npos || i == 0 || t.size() - i > 6)
            return t;
        for (size_t j = 0; j < i; ++j)
            if (!isalpha((unsigned char)t[j]))
                return t;
        for (size_t j = i; j < t.size(); ++j)
            if (!isdigit((unsigned char)t[j]))
                return t;
        long long so = atoll(t.c_str() + i);
        string chuSo = to_string(so);
        if (chuSo.size() < 2)
            chuSo = "0" + chuSo;
        return t.substr(0, i) + chuSo;
    }

    bool laMa(const string &s, const string &tienTo)
    {
        if (s.size() <= tienTo.size() || s.compare(0, tienTo.size(), tienTo) != 0)
            return false;
        for (size_t i = tienTo.size(); i < s.size(); ++i)
            if (!isdigit((unsigned char)s[i]))
                return false;
        return true;
    }

    // =================================================================
    //  Kiem tra dinh dang
    // =================================================================
    bool laSoDuong(const string &s)
    {
        if (s.empty() || s.size() > 9)
            return false;
        for (size_t i = 0; i < s.size(); ++i)
            if (!isdigit((unsigned char)s[i]))
                return false;
        return s.find_first_not_of('0') != string::npos;
    }

    string chuanHoaSoDuong(const string &s)
    {
        size_t d = s.find_first_not_of('0');
        return d == string::npos ? "0" : s.substr(d);
    }

    static bool docCodepointUTF8(const string &s, size_t &i, unsigned int &cp)
    {
        const unsigned char c = (unsigned char)s[i++];
        if (c < 0x80)
        {
            cp = c;
            return true;
        }
        int extra = 0;
        if ((c & 0xE0) == 0xC0) { cp = c & 0x1F; extra = 1; }
        else if ((c & 0xF0) == 0xE0) { cp = c & 0x0F; extra = 2; }
        else if ((c & 0xF8) == 0xF0) { cp = c & 0x07; extra = 3; }
        else return false;
        if (i + (size_t)extra > s.size())
            return false;
        for (int n = 0; n < extra; ++n)
        {
            const unsigned char tail = (unsigned char)s[i++];
            if ((tail & 0xC0) != 0x80)
                return false;
            cp = (cp << 6) | (tail & 0x3F);
        }
        return true;
    }

    static bool laChuLatin(unsigned int cp)
    {
        return (cp >= 'A' && cp <= 'Z') || (cp >= 'a' && cp <= 'z') ||
               (cp >= 0x00C0 && cp <= 0x024F) || (cp >= 0x1E00 && cp <= 0x1EFF);
    }

    bool laChuThuan(const string &s)
    {
        const string t = trim(s);
        bool coChu = false;
        size_t i = 0;
        while (i < t.size())
        {
            unsigned int cp = 0;
            if (!docCodepointUTF8(t, i, cp))
                return false;
            if (cp == ' ')
                continue;
            if (!laChuLatin(cp))
                return false;
            coChu = true;
        }
        return coChu;
    }

    bool laSoDienThoai(const string &s)
    {
        if (s.size() != 10 || s[0] != '0')
            return false;
        for (char c : s)
            if (!isdigit((unsigned char)c))
                return false;
        return true;
    }

    bool laCCCD(const string &s)
    {
        if (s.size() == 9 || s.size() == 12)
        {
            for (char c : s)
                if (!isdigit((unsigned char)c))
                    return false;
            return true;
        }
        // Ho chieu: 8-9 ky tu, ky tu dau la chu cai, con lai la chu/so
        if ((s.size() == 8 || s.size() == 9) && isalpha((unsigned char)s[0]))
        {
            for (char c : s)
                if (!isalnum((unsigned char)c))
                    return false;
            return true;
        }
        return false;
    }

    bool laEmail(const string &s)
    {
        size_t a = s.find('@');
        if (a == string::npos || a == 0 || s.find('@', a + 1) != string::npos)
            return false;
        size_t cham = s.find('.', a);
        if (cham == string::npos || cham == a + 1 || cham + 1 >= s.size())
            return false;
        for (char c : s)
            if (isspace((unsigned char)c))
                return false;
        return true;
    }

    bool coKyTuDieuKhien(const string &s)
    {
        for (unsigned char ch : s)
            if (ch < 32 || ch == 127 || ch == '|' || ch == '<' || ch == '>')
                return true;
        return false;
    }

    bool laSoThuan(const string &s, size_t toiThieu, size_t toiDa)
    {
        if (s.size() < toiThieu || s.size() > toiDa || s.empty())
            return false;
        for (unsigned char ch : s)
            if (!isdigit(ch))
                return false;
        return true;
    }

    // Ky tu >= 128 la byte cua chu cai co dau (UTF-8) nen duoc xem la chu
    static bool laChuCai(unsigned char ch) { return ch >= 128 || isalpha(ch); }

    bool laTenRieng(const string &s0, size_t toiDa)
    {
        string s = trim(s0);
        if (s.size() < 2 || s.size() > toiDa || coKyTuDieuKhien(s))
            return false;
        bool coChu = false;
        for (unsigned char ch : s)
        {
            if (laChuCai(ch))
                coChu = true;
            else if (!(isdigit(ch) || ch == ' ' || ch == '-' || ch == '\'' || ch == '.'))
                return false;
        }
        return coChu;
    }

    bool laDiaChi(const string &s0)
    {
        string s = trim(s0);
        if (s.size() < 5 || s.size() > 120 || coKyTuDieuKhien(s))
            return false;
        bool coChu = false;
        for (unsigned char ch : s)
        {
            if (laChuCai(ch))
                coChu = true;
            else if (!(isdigit(ch) || ch == ' ' || ch == ',' || ch == '.' || ch == '/' || ch == '-' ||
                       ch == '(' || ch == ')' || ch == '#' || ch == '\''))
                return false;
        }
        return coChu;
    }

    bool laUsername(const string &s)
    {
        if (s.size() < 3 || s.size() > 20)
            return false;
        for (unsigned char ch : s)
            if (!(isalnum(ch) || ch == '_' || ch == '.'))
                return false;
        return true;
    }

    // =================================================================
    //  Doc so
    // =================================================================
    static bool docSoNguyen(const string &s, long long &kq)
    {
        if (!laSoThuan(s, 1, 18))
            return false;
        errno = 0;
        char *end = nullptr;
        kq = strtoll(s.c_str(), &end, 10);
        return errno != ERANGE && *end == '\0';
    }

    bool docSoThuc(string s, double &kq)
    {
        if (s.empty())
            return false;
        int soCham = 0, soChuSo = 0;
        for (size_t j = 0; j < s.size(); ++j)
        {
            if (s[j] == '.')
                ++soCham;
            else if (isdigit((unsigned char)s[j]))
                ++soChuSo;
            else
                return false;
        }
        if (soCham > 1 || soChuSo == 0)
            return false;
        errno = 0;
        char *end = nullptr;
        kq = strtod(s.c_str(), &end);
        return errno != ERANGE && *end == '\0' && isfinite(kq);
    }

    static void xoaDongNhapSai()
    {
#ifdef _WIN32
        if (!_isatty(_fileno(stdin)))
            return;
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi{};
        if (!GetConsoleScreenBufferInfo(h, &csbi) || csbi.dwCursorPosition.Y <= csbi.srWindow.Top)
            return;
        COORD pos = {csbi.srWindow.Left, (SHORT)(csbi.dwCursorPosition.Y - 1)};
        DWORD written = 0;
        const DWORD width = (DWORD)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
        FillConsoleOutputCharacterW(h, L' ', width, pos, &written);
        FillConsoleOutputAttribute(h, csbi.wAttributes, width, pos, &written);
        SetConsoleCursorPosition(h, pos);
#else
        if (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO))
            cout << "\033[1A\r\033[2K" << flush;
#endif
    }

    static void baoLoi(const string &s)
    {
        // In loi ngay BEN DUOI truong dang nhap (mau do), khong qua buffer can-giua
        // de tranh nhay man hinh / mat bo cuc form dang nhap.
        // Khong goi UI::loi + hienThi (se ve lai toan man hinh va day con tro len giua).
        UI::ghiTrucTiep(string("  ") + UI::DO + "[!] " + s + UI::RESET + "\n");
    }

    // =================================================================
    //  Nhap lieu
    // =================================================================
    static string docDong()
    {
        string s;
        if (!getline(cin, s))
            throw HetDuLieuNhap();
        return s;
    }

    // Doc ma co the huy bang ESC ngay lap tuc. Tra ve chuoi rong neu huy.
    // Chi dung o cac truong nhap ma; cac truong nhap van ban khac van giu nguyen getline.
    static string docDongCoEsc()
    {
#ifdef _WIN32
        if (!_isatty(_fileno(stdin)))
            return docDong();
        string s;
        while (true)
        {
            int c = _getch();
            if (c == 27)
            {
                UI::ghiTrucTiep("\r\n");
                return "\x1b";
            }
            if (c == 0 || c == 224) { (void)_getch(); continue; }
            if (c == '\r' || c == '\n') { UI::ghiTrucTiep("\r\n"); return s; }
            if (c == 8)
            {
                if (!s.empty()) { s.pop_back(); UI::ghiTrucTiep("\b \b"); }
                continue;
            }
            if (c >= 32 && c != 127) { s.push_back((char)c); UI::ghiTrucTiep(string(1, (char)c)); }
        }
#else
        if (!isatty(STDIN_FILENO))
            return docDong();
        termios oldt{};
        if (tcgetattr(STDIN_FILENO, &oldt) != 0)
            return docDong();
        termios raw = oldt;
        raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0)
            return docDong();
        string s;
        while (true)
        {
            unsigned char c = 0;
            ssize_t n = ::read(STDIN_FILENO, &c, 1);
            if (n <= 0) { tcsetattr(STDIN_FILENO, TCSANOW, &oldt); throw HetDuLieuNhap(); }
            if (c == 27)
            {
                tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                UI::ghiTrucTiep("\r\n");
                return "\x1b";
            }
            if (c == '\r' || c == '\n')
            {
                tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                UI::ghiTrucTiep("\r\n");
                return s;
            }
            if (c == 127 || c == 8)
            {
                if (!s.empty()) { s.pop_back(); UI::ghiTrucTiep("\b \b"); }
                continue;
            }
            if (c >= 32) { s.push_back((char)c); UI::ghiTrucTiep(string(1, (char)c)); }
        }
#endif
    }

    // In (tuy chon) dong huong dan "ESC: Quay lai" 1 lan ngay duoi bang / phia tren o nhap,
    // roi in nhan nhap. Con tro nam NGAY SAU nhan (cung dong) - khong in huong dan duoi cho nhap.
    // Dung ghiTrucTiep de giu vi tri con tro tuan tu, khong can-giua lai tung dong.
    static void inNhanNhapLieu(const string &nhan, bool hienHuongDanEsc, bool anHuongDan = false)
    {
        UI::hienThi(); // flush buffer can-giua neu con noi dung dang cho
        if (hienHuongDanEsc && !anHuongDan && lanXoaKhiInHuongDan != UI::demLanXoaManHinh())
        {
            UI::ghiTrucTiep(string("  ") + UI::VANG + "ESC: Quay lai" + UI::RESET + "\n");
            lanXoaKhiInHuongDan = UI::demLanXoaManHinh();
        }
        // Nhan khong co dau ':' (vd "Ma phong can sua") -> them ": " de chu nhap khong dinh vao nhan
        string nhanIn = nhan;
        if (!nhanIn.empty() && nhanIn.back() != ' ')
            nhanIn += nhanIn.back() == ':' ? " " : ": ";
        UI::ghiTrucTiep(string("  ") + UI::CHU + "> " + UI::RESET + UI::CHU + nhanIn + UI::RESET);
    }

    string nhapDong(const string &nhan)
    {
        inNhanNhapLieu(nhan, escHuyDangBat);
        string s = escHuyDangBat ? docDongCoEsc() : docDong();
        UI::ghiTrucTiep(UI::RESET);
        if (escHuyDangBat && s == "\x1b")
            throw HuyNhap();
        return trim(s);
    }

    // Nhap dong co the huy bang ESC. Tra ve ky tu ESC lam sentinel de
    // phan biet ro rang voi Enter de trong; khong can nhap so 0 de huy.
    string nhapDongCoEsc(const string &nhan, bool hienHuongDan)
    {
        inNhanNhapLieu(nhan, true, !hienHuongDan);
        string s = docDongCoEsc();
        UI::ghiTrucTiep(UI::RESET);
        if (s == "\x1b") return s;
        return trim(s);
    }

    // Doc mat khau ky tu an (hien '*'). Tra ve "\x1b" neu ESC (khi cho phep).
    static string docMatKhauAn(bool choPhepEsc)
    {
#ifdef _WIN32
        if (!_isatty(_fileno(stdin)))
        {
            string s;
            if (!getline(cin, s)) throw HetDuLieuNhap();
            return trim(s);
        }
        string s;
        while (true)
        {
            int c = _getch();
            if (c == 27)
            {
                UI::ghiTrucTiep("\r\n");
                return choPhepEsc ? "\x1b" : s; // neu khong cho ESC thi bo qua
            }
            if (c == 0 || c == 224) { (void)_getch(); continue; }
            if (c == '\r' || c == '\n') { UI::ghiTrucTiep("\r\n"); return s; }
            if (c == 8)
            {
                if (!s.empty()) { s.pop_back(); UI::ghiTrucTiep("\b \b"); }
                continue;
            }
            if (c >= 32 && c != 127) { s.push_back((char)c); UI::ghiTrucTiep("*"); }
        }
#else
        if (!isatty(STDIN_FILENO))
        {
            string s;
            if (!getline(cin, s)) throw HetDuLieuNhap();
            return trim(s);
        }
        termios oldt{};
        if (tcgetattr(STDIN_FILENO, &oldt) != 0)
        {
            string s;
            if (!getline(cin, s)) throw HetDuLieuNhap();
            return trim(s);
        }
        termios raw = oldt;
        raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0)
        {
            string s;
            if (!getline(cin, s)) throw HetDuLieuNhap();
            return trim(s);
        }
        string s;
        while (true)
        {
            unsigned char c = 0;
            ssize_t n = ::read(STDIN_FILENO, &c, 1);
            if (n <= 0) { tcsetattr(STDIN_FILENO, TCSANOW, &oldt); throw HetDuLieuNhap(); }
            if (c == 27)
            {
                tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                UI::ghiTrucTiep("\r\n");
                return choPhepEsc ? "\x1b" : s;
            }
            if (c == '\r' || c == '\n')
            {
                tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                UI::ghiTrucTiep("\r\n");
                return s;
            }
            if (c == 127 || c == 8)
            {
                if (!s.empty()) { s.pop_back(); UI::ghiTrucTiep("\b \b"); }
                continue;
            }
            if (c >= 32) { s.push_back((char)c); UI::ghiTrucTiep("*"); }
        }
#endif
    }

    string nhapMatKhau(const string &nhan, bool choPhepEsc, bool hienHuongDan)
    {
        while (true)
        {
            // Nhan + dau '*' deu ghi truc tiep (cung kenh voi o tai khoan) de hien dung va thang hang
            inNhanNhapLieu(nhan, choPhepEsc, !hienHuongDan);
            string s = docMatKhauAn(choPhepEsc);
            UI::ghiTrucTiep(UI::RESET);
            if (choPhepEsc && s == "\x1b")
                return s;
            s = trim(s);
            if (s.empty())
            {
                baoLoi("Loi: Mat khau khong duoc de trong! Vui long nhap lai.");
                continue;
            }
            if (coKyTuDieuKhien(s))
            {
                baoLoi("Loi: Mat khau khong duoc chua ky tu dieu khien hoac | < >. Vui long nhap lai.");
                continue;
            }
            return s;
        }
    }

    string nhapTaiKhoan(const string &nhan, bool choPhepEsc, bool hienHuongDan)
    {
        while (true)
        {
            string s = choPhepEsc ? nhapDongCoEsc(nhan, hienHuongDan) : nhapDong(nhan);
            if (choPhepEsc && s == "\x1b")
                return s;
            s = trim(s);
            if (s.empty())
            {
                baoLoi("Loi: Tai khoan khong duoc de trong! Vui long nhap lai.");
                continue;
            }
            if (coKyTuDieuKhien(s))
            {
                baoLoi("Loi: Tai khoan khong duoc chua ky tu | < > va ky tu dieu khien. Vui long nhap lai.");
                continue;
            }
            // Chap nhan username hop le (chu, so, _ . ) hoac bat ky chuoi khong rong de tuong thich du lieu cu
            return s;
        }
    }

    string nhapChuoi(const string &nhan)
    {
        while (true)
        {
            string s = nhapDong(nhan);
            if (!s.empty() && !coKyTuDieuKhien(s))
                return s;
            baoLoi(s.empty() ? "Loi: Truong nay bat buoc, khong duoc de trong! Vui long nhap lai."
                             : "Loi: Khong dung cac ky tu | < > va ky tu dieu khien. Vui long nhap lai.");
        }
    }

    int nhapSoNguyenHopLe(const string &nhan, int nhoNhat, int lonNhat)
    {
        while (true)
        {
            string s = nhapDong(nhan);
            long long v = 0;
            double thuc = 0;
            if (!s.empty() && (s[0] == '-' || s[0] == '+'))
            {
                baoLoi("Loi: Khong nhap dau am/duong; chi nhap chu so.");
                continue;
            }
            if (s.empty())
            {
                baoLoi("Loi: Khong duoc de trong! Vui long nhap so.");
                continue;
            }
            if (!laSoThuan(s, 1, 18) || !docSoNguyen(s, v))
            {
                if (s.find('.') != string::npos && docSoThuc(s, thuc))
                    baoLoi("Loi: Vui long nhap so nguyen (khong co phan thap phan)!");
                else
                    baoLoi("Loi: Vui long chi nhap so!");
                continue;
            }
            if (v < nhoNhat || v > lonNhat)
            {
                if (nhoNhat > 0 && v <= 0)
                {
                    baoLoi("Loi: Gia tri phai lon hon 0.");
                    continue;
                }
                baoLoi("Loi: Gia tri phai tu " + to_string(nhoNhat) + " den " + to_string(lonNhat) + ".");
                continue;
            }
            return (int)v;
        }
    }

    // Doc gia dang so nguyen: "10000000" hoac "10,000,000" (dau phay chi ngan nhom 3 chu so)
    static bool docGiaNguyen(const string &s, double &kq)
    {
        if (s.empty())
            return false;
        string so;
        if (s.find(',') == string::npos)
        {
            for (unsigned char ch : s)
                if (!isdigit(ch))
                    return false;
            so = s;
        }
        else
        {
            size_t dau = s.find(',');
            if (dau < 1 || dau > 3)
                return false;
            for (size_t i = 0; i < s.size(); ++i)
            {
                if (s[i] == ',')
                {
                    if (i != dau && (i - dau) % 4 != 0)
                        return false;
                    continue;
                }
                if (!isdigit((unsigned char)s[i]))
                    return false;
            }
            // moi nhom sau dau phay phai du 3 chu so
            if ((s.size() - dau) % 4 != 0 || s.back() == ',')
                return false;
            for (char ch : s)
                if (ch != ',')
                    so += ch;
        }
        if (so.size() > 12)
            return false;
        kq = stod(so);
        return true;
    }

    double nhapGiaTimKiem(const string &nhan, double nhoNhat, double lonNhat, double khongNhoHon)
    {
        const string khoang = "Gia phong phai tu " + dinhDangTien(nhoNhat) + " den " + dinhDangTien(lonNhat) + " VND.";
        while (true)
        {
            string s = nhapDong(nhan);
            double v = 0;
            if (s.empty())
            {
                baoLoi("Loi: Khong duoc de trong! " + khoang);
                continue;
            }
            if (!docGiaNguyen(s, v))
            {
                baoLoi("Loi: Gia khong hop le. Chi nhap so nguyen, vd 500000 hoac 500,000 (khong am, khong chu, khong dau cham). " + khoang);
                continue;
            }
            if (v < nhoNhat || v > lonNhat)
            {
                baoLoi("Loi: " + khoang);
                continue;
            }
            if (khongNhoHon > 0 && v < khongNhoHon)
            {
                baoLoi("Loi: Gia den phai lon hon hoac bang gia tu (" + dinhDangTien(khongNhoHon) + " VND).");
                continue;
            }
            return v;
        }
    }

    double nhapSoThucHopLe(const string &nhan, double nhoNhat, double lonNhat)
    {
        while (true)
        {
            string s = nhapDong(nhan);
            double v = 0;
            if (s.empty())
            {
                baoLoi("Loi: Khong duoc de trong! Vui long nhap so.");
                continue;
            }
            if (s[0] == '-' || s[0] == '+')
            {
                baoLoi("Loi: Gia tri phai duong; khong nhap dau am/duong.");
                continue;
            }
            bool chiSoVaCham = true;
            int soCham = 0;
            for (unsigned char c : s)
            {
                if (c == '.')
                    ++soCham;
                else if (!isdigit(c))
                    chiSoVaCham = false;
            }
            if (!chiSoVaCham || soCham > 1 || !docSoThuc(s, v))
            {
                baoLoi("Loi: Vui long nhap so thuc hop le.");
                continue;
            }
            if (v < nhoNhat || v > lonNhat)
            {
                if (nhoNhat > 0 && v <= 0)
                {
                    baoLoi("Loi: Gia tri phai lon hon 0.");
                    continue;
                }
                baoLoi("Loi: Gia tri phai tu " + dinhDangTien(nhoNhat) + " den " + dinhDangTien(lonNhat) + ".");
                continue;
            }
            if (nhoNhat > 0 && !(v > 0))
            {
                baoLoi("Loi: Gia tri phai lon hon 0.");
                continue;
            }
            return v;
        }
    }

    int nhapLuaChon(int nhoNhat, int lonNhat)
    {
        // Menu vua in co o "Nhap lua chon" nam trong khung -> nhap ngay trong khung
        bool trongKhung = UI::coONhapMenu();
        if (!trongKhung)
            cout << "\n";
        while (true)
        {
            string s;
            if (trongKhung)
            {
                s = trim(UI::docTaiONhapMenu());
                trongKhung = false;
            }
            else
                s = nhapDong("Nhap lua chon [" + to_string(nhoNhat) + "-" + to_string(lonNhat) + "]: ");
            long long v = 0;
            if (docSoNguyen(s, v) && v >= nhoNhat && v <= lonNhat)
                return (int)v;
            string khoang = "tu " + to_string(nhoNhat) + " den " + to_string(lonNhat) + ".";
            if (s.empty())
                baoLoi("Lua chon khong hop le! Ban chua nhap gi. Vui long chon " + khoang);
            else if (!docSoNguyen(s, v))
                baoLoi("Lua chon khong hop le! Vui long chi nhap so " + khoang);
            else
                baoLoi("Lua chon khong hop le! Vui long chon " + khoang);
        }
    }

    // Xac nhan CHAT: chi Y/y = dong y, N/n = khong; nhap khac -> bao loi va hoi lai
    // Ve khung xac nhan thong nhat: cau hoi NOI BAT o giua, khong *, khong >>>.
    // Console khong doi font size an toan; dung DAM + VANG + khung + khoang trang de noi bat.
    static bool veKhungXacNhanEnterEsc(const string &cauHoiGoc)
    {
        // Bo tien to / ky hieu trang tri neu goi truyen vao
        string cau = cauHoiGoc;
        while (!cau.empty() && (cau[0] == '*' || cau[0] == '>' || cau[0] == ' '))
            cau.erase(0, 1);
        // Bo huong dan ENTER/ESC nhung trong chuoi (se in rieng)
        auto catHuongDan = [](string s) {
            for (const char *p : {" ENTER:", "ENTER:", " | ESC", "| ESC", " ESC:"})
            {
                size_t pos = s.find(p);
                if (pos != string::npos)
                    s = s.substr(0, pos);
            }
            while (!s.empty() && (s.back() == ' ' || s.back() == '?' || s.back() == '.'))
            {
                if (s.back() == '?')
                    break;
                s.pop_back();
            }
            if (!s.empty() && s.back() != '?')
                s += '?';
            return s;
        };
        cau = catHuongDan(cau);
        // Viet hoa de noi bat (giu nguyen ky tu da la chu hoa / so)
        string cauIn = cau;
        for (char &ch : cauIn)
            if (ch >= 'a' && ch <= 'z')
                ch = (char)(ch - 'a' + 'A');

        const string hd = "ENTER: Co | ESC: Khong";
        const int pad = 4; // le trong khung moi ben
        int wNoi = max(UI::doRong(cauIn), UI::doRong(hd));
        // Chieu rong CO DINH de moi hop thoai xac nhan cung do rong, thang cot giua man hinh
        // (THEM PHONG NAY / THEM PHONG TIEP, DAT TRUOC DV, THEM DV KHAC, CO CHAC MUON THEM...)
        const int RONG_CO_DINH = 56;
        int trong = RONG_CO_DINH;
        // Neu noi dung dai bat thuong thi mo rong them (van can giua)
        if (wNoi + pad * 2 > trong)
            trong = wNoi + pad * 2;
        // Gioi han theo chieu rong console/khung du an
        int toiDa = max(40, UI::RONG - 6);
        if (trong > toiDa)
            trong = toiDa;

        auto canGiuaDong = [&](const string &s) {
            int rw = UI::doRong(s);
            int left = max(0, (trong - rw) / 2);
            int right = max(0, trong - rw - left);
            return string(left, ' ') + s + string(right, ' ');
        };

        // Can giua khung tren man hinh (thang cot giua cac lan xac nhan)
        int leTraiKhung = max(0, (UI::RONG - (trong + 2)) / 2);
        string le((size_t)leTraiKhung, ' ');

        // Khung ASCII day du 4 canh (tranh ky tu Unicode loi font)
        cout << "\n";
        cout << le << "+" << string(trong, '-') << "+\n";
        cout << le << "|" << string(trong, ' ') << "|\n";
        cout << le << "|" << UI::VANG << UI::DAM << canGiuaDong(cauIn) << UI::RESET << "|\n";
        cout << le << "|" << string(trong, ' ') << "|\n";
        cout << le << "|" << UI::VANG << canGiuaDong(hd) << UI::RESET << "|\n";
        cout << le << "|" << string(trong, ' ') << "|\n";
        cout << le << "+" << string(trong, '-') << "+\n";
        cout << flush;
        UI::hienThi();

        while (true)
        {
            UI::Phim p = UI::docPhim();
            if (p == UI::Phim::ENTER)
                return true;
            if (p == UI::Phim::ESC)
                return false;
        }
    }

    bool nhapCoKhong(const string &nhan)
    {
        // Thong nhat giao dien xac nhan: khung + ENTER/ESC (khong con menu Co/Khong + dau *)
        bool ok = veKhungXacNhanEnterEsc(nhan);
        if (!ok && escHuyDangBat)
            throw HuyNhap();
        return ok;
    }

    // Xac nhan ENTER = dong y, ESC = huy.
    bool xacNhanEnterEsc(const string &nhan)
    {
        return veKhungXacNhanEnterEsc(nhan);
    }

    string nhapGioiTinh()
    {
        int c = UI::chonMenuTuongTac("GIOI TINH", "", {"Nam", "Nữ", "Khác"}, "Quay lai");
        if (c == 0)
        {
            if (escHuyDangBat)
                throw HuyNhap();
            return "";
        }
        return c == 1 ? "Nam" : c == 2 ? "Nữ" : "Khác";
    }

    const char *const CAU_XAC_NHAN = "Bạn có chắc chắn muốn xác nhận thực hiện hay không?";

    bool nhapXacNhanYN() { return nhapCoKhong(CAU_XAC_NHAN); }

    string nhapHoTen(const string &nhan)
    {
        while (true)
        {
            string s = nhapDong(nhan);
            if (laChuThuan(s) && s.size() <= 50)
                return chuanHoaHoTen(s);
            baoLoi("Chi duoc nhap chu cai va khoang trang (toi da 50 ky tu).");
        }
    }

    string nhapTenChiNhanh(const string &nhan)
    {
        while (true)
        {
            string s = trim(nhapDong(nhan));
            string lyDo;
            if (s.empty())
                lyDo = "Ten chi nhanh khong duoc de trong.";
            else if (any_of(s.begin(), s.end(), [](unsigned char c) { return isdigit(c) != 0; }))
                lyDo = "Ten chi nhanh khong duoc chua so.";
            else if (!laChuThuan(s))
                lyDo = "Ten chi nhanh chi duoc chua chu cai va khoang trang (khong dung ky tu dac biet).";
            else if (s.size() < 2 || s.size() > 40)
                lyDo = "Ten chi nhanh phai dai 2-40 ky tu.";
            if (lyDo.empty())
                return chuanHoaHoTen(s);
            UI::loi("Ten chi nhanh khong hop le! " + lyDo);
            UI::thongTin("Vui long nhap lai.");
        }
    }

    string nhapDiaChi(const string &nhan)
    {
        while (true)
        {
            string s = trim(nhapDong(nhan));
            if (laDiaChi(s))
                return s;
            baoLoi("Dia chi khong hop le! Cho phep chu, so va , . / - ( ) #, dai 5-120 ky tu. Hay nhap lai.");
        }
    }

    string nhapChuoiChu(const string &nhan)
    {
        while (true)
        {
            string s = nhapDong(nhan);
            if (laChuThuan(s))
                return s;
            baoLoi("Chi duoc nhap chu cai.");
        }
    }

    string nhapSoDienThoai(const string &nhan)
    {
        while (true)
        {
            string s = nhapDong(nhan);
            if (laSoDienThoai(s))
                return s;
            baoLoi("So dien thoai gom 10 chu so va bat dau bang 0.");
        }
    }

    string nhapCCCD(const string &nhan)
    {
        while (true)
        {
            string s = nhapDong(nhan);
            if (laCCCD(s))
                return chuHoa(s);
            baoLoi("CCCD gom 12 chu so; ho chieu 8-9 ky tu, bat dau bang chu cai.");
        }
    }

    string nhapEmail(const string &nhan)
    {
        while (true)
        {
            string s = nhapDong(nhan);
            if (s.empty() || laEmail(s))
                return s;
            baoLoi("Email khong hop le.");
        }
    }

    string nhapSoDuongChuoi(const string &nhan)
    {
        while (true)
        {
            string s = nhapDong(nhan);
            if (laSoDuong(s))
                return chuanHoaSoDuong(s);
            baoLoi(s.empty() ? "Loi: Khong duoc de trong! Vui long nhap so." : "Loi: Vui long chi nhap so nguyen duong (toi da 9 chu so)!");
        }
    }

    Ngay nhapNgay(const string &nhan, bool enterLaHomNay)
    {
        // Dam bao nhan luon co chu thich dinh dang (dd/mm/yyyy)
        string nhanHien = nhan;
        if (nhanHien.find("dd/mm") == string::npos && nhanHien.find("DD/MM") == string::npos)
        {
            // Chen (dd/mm/yyyy) truoc dau ':' neu co, neu khong thi noi vao cuoi
            size_t colon = nhanHien.rfind(':');
            if (colon != string::npos)
                nhanHien.insert(colon, " (dd/mm/yyyy)");
            else
                nhanHien += " (dd/mm/yyyy): ";
        }
        while (true)
        {
            string s = nhapDong(nhanHien);
            if (s.empty() && enterLaHomNay)
                return Ngay::homNay();
            Ngay n;
            if (Ngay::docChuoi(s, n))
                return n;
            if (s.empty())
                baoLoi("Ngay khong duoc de trong. Nhap theo dinh dang dd/mm/yyyy.");
            else
                baoLoi("Ngay khong hop le (vd 32/13/2026). Hay nhap dd/mm/yyyy hop le.");
        }
    }

    string nhapGhiChu(const string &nhan, size_t toiDa)
    {
        while (true)
        {
            string s = nhapDong(nhan);
            if (coKyTuDieuKhien(s))
                baoLoi("Loi: Khong dung cac ky tu | < > (ky tu phan cach du lieu). Vui long nhap lai.");
            else if (s.size() > toiDa)
                baoLoi("Loi: Toi da " + to_string(toiDa) + " ky tu. Vui long nhap lai.");
            else
                return s;
        }
    }

    bool kiemTraMaNhap(const string &s, const string &tienTo)
    {
        if (s.empty())
        {
            baoLoi("Loi: Khong duoc de trong! Vui long nhap ma.");
            return false;
        }
        if (!laMa(chuanHoaMa(s), tienTo))
        {
            baoLoi("Loi: Ma '" + s + "' sai dinh dang. Ma gom tien to " + tienTo + " va chu so.");
            return false;
        }
        return true;
    }

    string nhapMa(const string &nhan, const string &tienTo, bool choPhepHuy, bool hienHuongDanEsc)
    {
        while (true)
        {
            // Dung inNhanNhapLieu de khong nhay vi tri; co the an dong "ESC: Quay lai" khi hienHuongDanEsc=false
            const bool choHuy = choPhepHuy || escHuyDangBat;
            inNhanNhapLieu(nhan, choHuy, !hienHuongDanEsc);
            string s = trim(docDongCoEsc());
            // ESC quay lai; giu gia tri huy "0" noi bo de tuong thich cac luong nghiep vu.
            if ((choPhepHuy || escHuyDangBat) && s == "\x1b")
            {
                if (escHuyDangBat && !choPhepHuy)
                    throw HuyNhap();
                return "0";
            }
            if (s.empty())
            {
                baoLoi("Truong nay khong duoc de trong! Hay nhap lai.");
                continue;
            }
            string m = chuanHoaMa(s);
            if (!laMa(m, tienTo))
            {
                baoLoi("Ma '" + s + "' sai dinh dang (tien to " + tienTo + " + chu so). Hay nhap lai.");
                continue;
            }
            return m;
        }
    }

    LoaiPhong nhapLoaiPhong()
    {
        UI::thongTin("Loai phong:  1. Standard   2. Deluxe   3. Suite   4. Villa");
        int c = nhapSoNguyenHopLe("Chon loai phong [1-4]: ", 1, 4);
        switch (c)
        {
        case 1:
            return LoaiPhong::STANDARD;
        case 2:
            return LoaiPhong::DELUXE;
        case 3:
            return LoaiPhong::SUITE;
        default:
            return LoaiPhong::VILLA;
        }
    }

    MuaDuLich nhapMuaDuLich()
    {
        UI::thongTin("Mua du lich:");
        cout << "      1. Thap diem   - giam 20% tien phong (x0.8)\n"
             << "      2. Binh thuong - gia goc            (x1.0)\n"
             << "      3. Cao diem    - tang 30%           (x1.3)\n"
             << "      4. Le/Tet      - tang 60%           (x1.6)\n";
        int c = nhapSoNguyenHopLe("Chon mua [1-4]: ", 1, 4);
        return static_cast<MuaDuLich>(c);
    }
}
