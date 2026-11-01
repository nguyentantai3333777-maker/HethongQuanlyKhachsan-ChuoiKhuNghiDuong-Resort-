// ConsoleUI.cpp - Giao dien console: mau ANSI theo chu de, khung ASCII, CAN GIUA CA KHOI theo cua so that.
// Moi phep do / can le chuoi deu di qua module Utf8Text (do rong hien thi + std::setw).
#include "ConsoleUI.h"
#include "InputHelper.h"
#include "TienIch.h"
#include "Utf8Text.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <cstdlib>
#include <iostream>
#include <streambuf>
#include <functional>
#include <string>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <fcntl.h>
#endif

#ifdef _WIN32
// GetCurrentConsoleFontEx / CONSOLE_FONT_INFOEX can Windows Vista tro len (MinGW cu mac dinh muc thap hon)
#if !defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0600
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#else
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>
#endif

using namespace std;

namespace UI
{
    int RONG = 76;
    int RONG_MENU = 76;
    int RONG_KHOI = 76;
    int RONG_BANG_TOI_DA = 116;
    const string KHOI = "\x02";          // dau dong: dong thuoc khung/bang -> can giua trong khoi
    static const char O_NHAP = '\x03';    // vi tri con tro cua o "Nhap lua chon" trong khung menu

    // ------------------------------------------------------------------
    //  Bang mau ANSI 16 mau chuan (hien thi duoc tren moi console ho tro ANSI, ke ca Windows 10+)
    //    \033[1;33m VANG DAM : moi thong tin / nhan / danh muc lien quan KHACH HANG
    //    \033[1;36m XANH NGOC: tieu de, vien khung chinh       \033[1;31m DO  : loi
    //    \033[1;32m XANH LA  : thanh cong                      \033[1;35m TIM : canh bao
    // ------------------------------------------------------------------
    string RESET = "\033[0m";
    string DAM = "\033[1m";
    string CHU = "\033[37m";        // chu thuong (trang xam)
    string MO = "\033[90m";         // chu phu (xam)
    string XANH_TROI = "\033[1;34m";
    string VANG = "\033[33m";       // vang thuong
    string KHACH = "\033[37m";      // TRANG: thong tin khach hang dung CHUNG mau chu voi toan menu (khong con vang rieng)
    string HONG = "\033[1;35m";     // hong - nhan su
    string XANH_LA = "\033[1;32m";  // thanh cong
    string CAM = "\033[38;5;208m";      // cam - thanh toan
    string NGOC = "\033[1;36m";     // xanh ngoc - tieu de
    string DO = "\033[1;31m";       // do - thong bao loi
    string TIM = "\033[35m";
    string NAU = "\033[34m";
    string XAM = "\033[37m";
    string CARAMEL = "\033[36m";
    string XAM_XANH = "\033[94m";

    static vector<string> &bangChuDe()
    {
        static vector<string> ds = {
            XANH_TROI, // CHINH      - Xanh bien
            VANG,      // PHONG      - Vang
            HONG,      // DICH_VU    - Hong
            XANH_LA,   // DAT_PHONG  - Xanh la
            CAM,       // THANH_TOAN - Cam
            VANG,      // KHACH_HANG - Vang nhat
            HONG,      // NHAN_SU    - Hong
            "\033[31m", // THONG_KE  - Do
            XAM_XANH}; // TAI_KHOAN  - Xam xanh
        return ds;
    }

    // Mau tung cot bieu do (vang danh cho khach hang -> khong dung o day)
    static const vector<string> &sacCot()
    {
        static vector<string> ds = {"\033[1;36m", "\033[1;34m", "\033[1;32m", "\033[1;35m", "\033[36m", "\033[34m"};
        return ds;
    }

    // Nhan / muc / cot co lien quan den khach hang ("Khach hang", "Khach", "Quan ly khach hang"...) -> MAU VANG
    static bool lienQuanKhach(const string &s)
    {
        string t;
        for (char c : s)
            t += (char)tolower((unsigned char)c);
        (void)t;
        return false; // khong tu dong to mau theo tu khoa "khach": cung mot menu phai dong nhat mau chu
    }

    static ChuDe chuDeHienTai = ChuDe::CHINH;
    static bool epMauKhach = false; // dang in khoi thong tin cua 1 khach hang -> moi nhan / gia tri VANG
    static bool coAnsi = true;

    void datChuDe(ChuDe c) { chuDeHienTai = c; }
    const string &mau() { return bangChuDe()[(size_t)chuDeHienTai]; }
    string mauCua(ChuDe c) { return bangChuDe()[(size_t)c]; }

    // ------------------------------------------------------------------
    //  Ky tu khung: ASCII (mac dinh, chac chan 1 cot tren MOI font console) hoac Unicode (HRM_UNICODE=1)
    // ------------------------------------------------------------------
    struct BoKhung
    {
        string TL, TR, BL, BR, H, V, LT, RT, TT, BT, X;
    };
    static const BoKhung DOI_UNI = {"╔", "╗", "╚", "╝", "═", "║", "╠", "╣", "╦", "╩", "╬"};
    static const BoKhung DON_UNI = {"┌", "┐", "└", "┘", "─", "│", "├", "┤", "┬", "┴", "┼"};
    static const BoKhung DOI_ASCII = {"+", "+", "+", "+", "=", "|", "+", "+", "+", "+", "+"};
    static const BoKhung DON_ASCII = {"+", "+", "+", "+", "-", "|", "+", "+", "+", "+", "+"};
    static bool dungUnicode = false;
    static const BoKhung &K2() { return dungUnicode ? DOI_UNI : DOI_ASCII; } // khung chinh (net doi)
    static const BoKhung &K1() { return dungUnicode ? DON_UNI : DON_ASCII; } // bang du lieu (net don)
    static string kyTuCot() { return "#"; } // thanh bieu do / tien trinh: "####...." luon 1 cot / ky tu
    static string kyTuNen() { return "."; }
    static const string OK = "[OK]", SAI = "[!]", CHU_Y = "[!]", INFO = "*";

    // ------------------------------------------------------------------
    //  Ghi truc tiep ra console (bo qua bo dem)
    // ------------------------------------------------------------------
    static streambuf *bufGoc = nullptr; // streambuf goc cua std::cout
    static bool dangCanGiua = false;

    static bool laTTY()
    {
#ifdef _WIN32
        DWORD m = 0;
        return GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &m) != 0;
#else
        return isatty(STDOUT_FILENO) != 0;
#endif
    }

    static void ghiRa(const string &s)
    {
        if (s.empty())
            return;
#ifdef _WIN32
        // Console Windows: doi UTF-8 -> UTF-16 roi WriteConsoleW (khong phu thuoc code page, khong vo ky tu nhieu byte)
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD m = 0;
        if (GetConsoleMode(h, &m))
        {
            int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
            if (n > 0)
            {
                wstring w((size_t)n, L'\0');
                MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &w[0], n);
                DWORD daGhi = 0;
                WriteConsoleW(h, w.data(), (DWORD)w.size(), &daGhi, nullptr);
                return;
            }
        }
#endif
        if (bufGoc)
        {
            bufGoc->sputn(s.data(), (streamsize)s.size());
            bufGoc->pubsync();
        }
        else
            cout << s << flush;
    }

    // Public: echo ky tu nhap lieu thang ra man hinh, khong di qua bo dem can giua
    void ghiTrucTiep(const string &s) { ghiRa(s); }

    enum MenuFrameStyle
    {
        MENU_FRAME_NORMAL,
        MENU_FRAME_SELECTED,
        MENU_FRAME_FOOTER
    };

    static bool doubleBufferInitialized = false;
    static int menuFrameWidth = 0;
    static int menuFrameHeight = 0;
    static vector<string> menuFrameText;
    static vector<int> menuFrameStyle;
#ifdef _WIN32
    static vector<CHAR_INFO> menuFrameCells;
    static WORD menuFrameNormalAttributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
#endif

    void initDoubleBuffer()
    {
        if (doubleBufferInitialized)
            return;
        doubleBufferInitialized = true;
        const size_t capacity = (size_t)max(1, getConsoleWidth()) * (size_t)max(1, getConsoleHeight());
        menuFrameText.reserve((size_t)max(1, getConsoleHeight()));
        menuFrameStyle.reserve((size_t)max(1, getConsoleHeight()));
#ifdef _WIN32
        menuFrameCells.reserve(capacity);
#else
        (void)capacity;
#endif
    }

    void cleanupDoubleBuffer()
    {
        vector<string>().swap(menuFrameText);
        vector<int>().swap(menuFrameStyle);
#ifdef _WIN32
        vector<CHAR_INFO>().swap(menuFrameCells);
#endif
        menuFrameWidth = menuFrameHeight = 0;
        doubleBufferInitialized = false;
    }

    static void beginFrame(int width, int height
#ifdef _WIN32
                           , WORD normalAttributes
#endif
    )
    {
        if (!doubleBufferInitialized)
            initDoubleBuffer();
        menuFrameWidth = max(1, width);
        menuFrameHeight = max(1, height);
        menuFrameText.assign((size_t)menuFrameHeight, string((size_t)menuFrameWidth, ' '));
        menuFrameStyle.assign((size_t)menuFrameHeight, MENU_FRAME_NORMAL);
#ifdef _WIN32
        menuFrameNormalAttributes = normalAttributes;
        menuFrameCells.assign((size_t)menuFrameWidth * (size_t)menuFrameHeight, CHAR_INFO{});
#endif
    }

    static void writeText(int x, int y, const string &text, int style = MENU_FRAME_NORMAL)
    {
        if (y < 0 || y >= menuFrameHeight || x < 0 || x >= menuFrameWidth)
            return;
        string line = canTrai(catNgan(text, menuFrameWidth - x), menuFrameWidth - x);
        menuFrameText[(size_t)y].replace((size_t)x, line.size(), line);
        menuFrameStyle[(size_t)y] = style;
    }

#ifdef _WIN32
    // Dung cung mau chu de cho vien menu va toan bo muc dang chon (ca hai mui ten).
    static WORD mauMenuWindows()
    {
        switch (chuDeHienTai)
        {
        case ChuDe::PHONG: return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; // vang
        case ChuDe::DICH_VU: return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY; // hong/tim
        case ChuDe::DAT_PHONG: return FOREGROUND_GREEN | FOREGROUND_INTENSITY; // xanh la
        case ChuDe::THANH_TOAN: return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; // vang
        case ChuDe::KHACH_HANG: return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; // vang
        case ChuDe::NHAN_SU: return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; // xanh ngoc
        case ChuDe::THONG_KE: return FOREGROUND_RED | FOREGROUND_INTENSITY; // do
        case ChuDe::TAI_KHOAN: return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; // xanh xam
        default: return FOREGROUND_BLUE | FOREGROUND_INTENSITY; // menu chinh: xanh duong
        }
    }
#endif

    static void endFrame(int x, int y, bool hasAbsolutePosition, bool firstFrame)
    {
#ifdef _WIN32
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        // Chi to mau ky tu danh dau va noi dung footer; nen va vien khung giu nguyen.
        const WORD markerAttributes = mauMenuWindows();
        const WORD footerTextAttributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        for (int row = 0; row < menuFrameHeight; ++row)
        {
            const string &utf8 = menuFrameText[(size_t)row];
            const int count = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), (int)utf8.size(), nullptr, 0);
            wstring wide((size_t)max(0, count), L' ');
            if (count > 0)
                MultiByteToWideChar(CP_UTF8, 0, utf8.data(), (int)utf8.size(), &wide[0], count);
            size_t footerStart = wide.find(L"ENTER:");
            for (int col = 0; col < menuFrameWidth; ++col)
            {
                CHAR_INFO &cell = menuFrameCells[(size_t)row * (size_t)menuFrameWidth + (size_t)col];
                cell.Char.UnicodeChar = (size_t)col < wide.size() ? wide[(size_t)col] : L' ';
                WORD attr = menuFrameNormalAttributes;
                const wchar_t glyph = cell.Char.UnicodeChar;
                const bool borderGlyph = glyph == L'+' || glyph == L'=' || glyph == L'|' ||
                                         glyph == L'╔' || glyph == L'╗' || glyph == L'╚' || glyph == L'╝' ||
                                         glyph == L'═' || glyph == L'║' || glyph == L'╠' || glyph == L'╣' ||
                                         glyph == L'╦' || glyph == L'╩' || glyph == L'╬';
                if (menuFrameStyle[(size_t)row] == MENU_FRAME_SELECTED || borderGlyph)
                    attr = markerAttributes;
                if (menuFrameStyle[(size_t)row] == MENU_FRAME_FOOTER && footerStart != wstring::npos && (size_t)col >= footerStart && (size_t)col < footerStart + wcslen(L"ENTER: chon | ESC: quay lai | ↑/↓: di chuyen"))
                    attr = footerTextAttributes;
                cell.Attributes = attr;
            }
        }
        COORD size = {(SHORT)menuFrameWidth, (SHORT)menuFrameHeight};
        COORD origin = {0, 0};
        SMALL_RECT rect = {(SHORT)x, (SHORT)y, (SHORT)(x + menuFrameWidth - 1),
                           (SHORT)(y + menuFrameHeight - 1)};
        WriteConsoleOutputW(h, menuFrameCells.data(), size, origin, &rect);
        COORD below = {0, (SHORT)(y + menuFrameHeight)};
        SetConsoleCursorPosition(h, below);
#else
        string frame;
        if (hasAbsolutePosition)
            frame += "\033[" + to_string(y + 1) + ";" + to_string(x + 1) + "H";
        else if (!firstFrame)
            frame += "\033[" + to_string(menuFrameHeight) + "A\r";
        for (int row = 0; row < menuFrameHeight; ++row)
        {
            const int style = menuFrameStyle[(size_t)row];
            string rendered = menuFrameText[(size_t)row];
            if (coAnsi && style == MENU_FRAME_SELECTED)
            {
                // To mau toan bo dong dang chon, bao gom so thu tu, noi dung va ca hai mui ten.
                rendered = mau() + rendered + RESET;
            }
            else if (coAnsi && style == MENU_FRAME_FOOTER)
            {
                const size_t footer = rendered.find("ENTER:");
                if (footer != string::npos)
                    rendered.insert(footer, VANG), rendered.insert(footer + VANG.size() + strlen("ENTER: chon | ESC: quay lai | ↑/↓: di chuyen"), RESET);
            }
            else if (coAnsi)
            {
                // Vien menu luon theo mau chu de; noi dung dong khong duoc chon van giu mau thuong.
                string colored;
                colored.reserve(rendered.size() + 32);
                for (char ch : rendered)
                {
                    if (ch == '+' || ch == '=' || ch == '|')
                        colored += mau(), colored.push_back(ch), colored += RESET;
                    else
                        colored.push_back(ch);
                }
                rendered.swap(colored);
            }
            if (!hasAbsolutePosition)
                frame += string((size_t)x, ' ');
            else if (row > 0 && x > 0)
                frame += "\033[" + to_string(x + 1) + "G"; // dong sau cung phai thut le nhu dong dau
            frame += rendered;
            frame += "\033[K";
            if (row + 1 < menuFrameHeight || !hasAbsolutePosition)
                frame += "\r\n";
        }
        if (hasAbsolutePosition)
            frame += "\033[" + to_string(y + menuFrameHeight + 1) + ";1H";
        ghiRa(frame);
#endif
    }

    // ------------------------------------------------------------------
    //  Kich thuoc cua so & vi tri con tro
    // ------------------------------------------------------------------
    static int soTuBien(const char *ten, int macDinh)
    {
        const char *v = getenv(ten);
        int n = v ? atoi(v) : 0;
        return n > 10 ? n : macDinh;
    }

    int getConsoleWidth()
    {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
            return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
        winsize ws{};
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
            return ws.ws_col;
#endif
        return soTuBien("COLUMNS", 120); // khong phai console (chuyen huong ra file) -> mac dinh
    }

    int getConsoleHeight()
    {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
            return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
        winsize ws{};
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0)
            return ws.ws_row;
#endif
        return soTuBien("LINES", 40);
    }

    static int centerPadding(int consoleWidth, int blockWidth)
    {
        return max(0, (consoleWidth - blockWidth) / 2);
    }

    int calculateCenterX(int blockWidth) { return centerPadding(getConsoleWidth(), blockWidth); }
    int calculateCenterY(int blockHeight) { return max(0, (getConsoleHeight() - blockHeight) / 2); }

    void setCursorPosition(int x, int y)
    {
#ifdef _WIN32
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(h, &csbi))
        {
            COORD c = {(SHORT)max(0, x), (SHORT)(csbi.srWindow.Top + max(0, y))};
            SetConsoleCursorPosition(h, c);
            return;
        }
#endif
        ghiRa("\x1b[" + to_string(max(0, y) + 1) + ";" + to_string(max(0, x) + 1) + "H");
    }

    // Di chuyen con tro tuong doi: len 'len' dong (am = xuong) va dat cot 'cot' (-1 = giu cot 0)
    static void diChuyenTuongDoi(int len, int cot)
    {
#ifdef _WIN32
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(h, &csbi))
        {
            COORD c = {(SHORT)max(0, cot), (SHORT)max(0, csbi.dwCursorPosition.Y - len)};
            SetConsoleCursorPosition(h, c);
            return;
        }
#endif
        string s;
        if (len > 0)
            s += "\x1b[" + to_string(len) + "A";
        else if (len < 0)
            s += "\x1b[" + to_string(-len) + "B";
        s += "\x1b[" + to_string(max(0, cot) + 1) + "G";
        ghiRa(s);
    }

    void capNhatKichThuoc()
    {
        int cw = getConsoleWidth();
        // Mo rong khung chinh khi terminal rong de bang / menu can doi hon
        RONG = max(44, min(100, cw - 4));
        RONG_MENU = RONG;
        RONG_KHOI = RONG;
        // Bang duoc phep dung gan het chieu rong console (tru le 2 ben)
        RONG_BANG_TOI_DA = max(40, min(cw - 4, 140));
    }

    // ------------------------------------------------------------------
    //  Chuoi & do rong: uy quyen cho Utf8Text (do rong hien thi UTF-8 + std::setw)
    int doRong(const string &s) { return Utf8::doRong(s); }

    string lap(const string &s, int n)
    {
        string kq;
        for (int i = 0; i < n; ++i)
            kq += s;
        return kq;
    }

    string catNgan(const string &s, int w) { return Utf8::catVua(s, w); }
    string canTrai(const string &s, int w) { return Utf8::canTrai(s, w); }
    string canPhai(const string &s, int w) { return Utf8::canPhai(s, w); }
    string canGiuaChuoi(const string &s, int w) { return Utf8::canGiua(s, w); }

    vector<string> ngatDong(const string &s, int w)
    {
        auto kq = Utf8::ngatDong(s, max(4, w));
        if (kq.empty())
            kq.push_back("");
        return kq;
    }

    // ==================================================================
    //  BO DEM CAN GIUA: gom moi thu in ra cout thanh mot khoi roi ve vao giua cua so
    // ==================================================================
    class BoDemCanGiua : public streambuf
    {
    public:
        string cho; // noi dung chua ve

    protected:
        int_type overflow(int_type c) override
        {
            if (c != traits_type::eof())
                cho.push_back((char)c);
            return traits_type::not_eof(c);
        }
        streamsize xsputn(const char *s, streamsize n) override
        {
            cho.append(s, (size_t)n);
            return n;
        }
        int sync() override { return 0; } // khong ve o day: chi ve khi can nhap lieu / xoa man hinh
    };

    static BoDemCanGiua boDem;
    static bool manHinhMoi = true; // vua xoa man hinh -> khoi dau tien can giua ca chieu doc
    static int xManHinh = 0;       // le trai cua man hinh hien tai (cac khoi sau thang hang khoi dau)
    static int wManHinh = 0;       // do rong khoi cua man hinh hien tai (khung sau duoc can giua trong do rong nay)
    static bool menuCho = false;   // vua in menu co o nhap trong khung
    static size_t viTriSauMenu = 0;
    static int oDongDuoi = -1, oCot = -1; // vi tri o nhap trong khoi vua ve (so dong ben duoi, cot)

    void batDauCanGiua()
    {
        if (dangCanGiua)
            return;
        bufGoc = cout.rdbuf(&boDem);
        dangCanGiua = true;
        capNhatKichThuoc();
    }

    void ketThucCanGiua()
    {
        if (!dangCanGiua)
            return;
        hienThi();
        cout.rdbuf(bufGoc);
        dangCanGiua = false;
        bufGoc = nullptr;
    }

    void hienThi()
    {
        oDongDuoi = oCot = -1;
        if (!dangCanGiua || boDem.cho.empty())
            return;
        string tatCa;
        tatCa.swap(boDem.cho);
        menuCho = false;

        // Tach dong; dong cuoi khong co '\n' la dong dang nhap (prompt)
        vector<string> dong;
        size_t bd = 0;
        while (bd < tatCa.size())
        {
            size_t kt = tatCa.find('\n', bd);
            if (kt == string::npos)
            {
                dong.push_back(tatCa.substr(bd));
                bd = tatCa.size();
                break;
            }
            dong.push_back(tatCa.substr(bd, kt - bd));
            bd = kt + 1;
        }
        const bool coDongDo = !tatCa.empty() && tatCa.back() != '\n';

        const int cw = getConsoleWidth();
        int W = 0; // do rong khoi = dong rong nhat
        for (auto &d : dong)
            W = max(W, doRong(d));
        const bool quaHep = W > cw - 1;
        if (quaHep)
            W = cw - 1;

        string out;
        int x, wKhoi = W; // wKhoi: do rong dung de can giua cac dong khung/bang
        if (manHinhMoi)
        {
            manHinhMoi = false;
            x = calculateCenterX(W);
            xManHinh = x;
            wManHinh = W;
            int H = (int)dong.size() + (quaHep ? 1 : 0);
            int y = calculateCenterY(H);
            if (laTTY())
            {
                ghiRa(out);
                setCursorPosition(0, y);
            }
            else
                out += string((size_t)y, '\n');
            if (quaHep) // noi dung rong hon cua so: bao nguoi dung phong to (thong bao cung tu cat vua cua so)
                out += MO + catNgan("(!) Cua so hep (" + to_string(cw) + " cot) - hay phong to cua so console.", cw - 1) +
                       RESET + "\n";
        }
        else if (W <= wManHinh && xManHinh + wManHinh <= cw - 1)
        {
            // Phan in them tren cung man hinh: nam trong khoi da ve -> cung le trai, khung can giua trong khoi
            x = xManHinh;
            wKhoi = wManHinh;
        }
        else
        {
            x = calculateCenterX(W); // rong hon khoi cu (hoac cua so vua thu nho) -> can giua lai
            xManHinh = x;
            wManHinh = W;
        }

        int dongO = -1, cotO = -1;
        for (size_t i = 0; i < dong.size(); ++i)
        {
            string d = dong[i];
            bool laKhoi = !d.empty() && d[0] == KHOI[0];
            if (laKhoi)
                d.erase(0, 1);
            int lech = laKhoi ? max(0, (wKhoi - doRong(d)) / 2) : 0;
            size_t viTriO = d.find(O_NHAP);
            if (viTriO != string::npos)
            {
                dongO = (int)i;
                cotO = x + lech + doRong(d.substr(0, viTriO));
                d.erase(viTriO, 1);
            }
            int conLai = cw - 1 - x - lech;
            if (doRong(d) > conLai)
                d = catNgan(d, max(1, conLai));
            out += string((size_t)(x + lech), ' ') + d;
            bool cuoi = i + 1 == dong.size();
            if (!(cuoi && coDongDo))
                out += "\n";
        }
        ghiRa(out);
        if (dongO >= 0 && !coDongDo)
        {
            oDongDuoi = (int)dong.size() - dongO; // so dong tu o nhap den dong ngay sau khoi
            oCot = cotO;
        }
    }

    bool coONhapMenu() { return dangCanGiua && menuCho && boDem.cho.size() == viTriSauMenu; }

    string docTaiONhapMenu()
    {
        hienThi();
        const bool diChuyen = laTTY() && oDongDuoi > 0;
        const int duoi = oDongDuoi;
        if (diChuyen)
            diChuyenTuongDoi(duoi, oCot);
        string s;
        bool ok = (bool)getline(cin, s);
        if (diChuyen)
            diChuyenTuongDoi(-(duoi - 1), 0); // sau Enter con tro o dong ngay duoi o nhap -> ve sau khoi
        if (!ok)
            throw HetDuLieuNhap();
        return s;
    }

    // ------------------------------------------------------------------
    //  Khoi tao / dieu khien man hinh
    // ------------------------------------------------------------------
    void khoiTaoConsole()
    {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleTitleA("Ocean Pearl Resort - Quan ly Khach san & Resort");
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        if (h == INVALID_HANDLE_VALUE || !GetConsoleMode(h, &mode) ||
            !SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
            coAnsi = false;
        // Console cu mac dinh chi 80 cot -> noi rong bo dem de bang rong khong bi xuong dong giua chung
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (h != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(h, &csbi) && csbi.dwSize.X < 122)
        {
            COORD size = {122, csbi.dwSize.Y < 300 ? (SHORT)300 : csbi.dwSize.Y};
            if (SetConsoleScreenBufferSize(h, size))
            {
                CONSOLE_SCREEN_BUFFER_INFO c2;
                if (GetConsoleScreenBufferInfo(h, &c2))
                {
                    SMALL_RECT win = {0, 0, (SHORT)(min<int>(121, c2.dwMaximumWindowSize.X - 1)), (SHORT)(c2.srWindow.Bottom - c2.srWindow.Top)};
                    SetConsoleWindowInfo(h, TRUE, &win);
                }
            }
        }
#endif
        if (getenv("HRM_UNICODE")) // tuy chon: khung net doi Unicode (chi dung khi font console ve dung 1 cot)
            dungUnicode = true;
        if (getenv("NO_COLOR"))
            coAnsi = false;
        if (!coAnsi)
        {
            RESET = DAM = CHU = MO = "";
            XANH_TROI = VANG = KHACH = HONG = XANH_LA = CAM = NGOC = DO = TIM = NAU = XAM = CARAMEL = XAM_XANH = "";
            for (auto &c : bangChuDe())
                c = "";
            for (auto &s : const_cast<vector<string> &>(sacCot()))
                s = "";
        }
        capNhatKichThuoc();
        initDoubleBuffer();
    }

    static int soLanXoaManHinh = 0;
    int demLanXoaManHinh() { return soLanXoaManHinh; }

    void xoaManHinh()
    {
        ++soLanXoaManHinh;
        hienThi(); // ve not phan dang cho (neu co) truoc khi xoa
        if (coAnsi)
            ghiRa("\x1b[2J\x1b[3J\x1b[H");
        else
        {
#ifdef _WIN32
            system("cls");
#else
            ghiRa(string(3, '\n'));
#endif
        }
        manHinhMoi = true;
        capNhatKichThuoc(); // cua so vua doi kich thuoc -> khung tu thu nho / mo rong
    }

    void pauseScreen()
    {
        cout << "\n  " << MO << "Nhan Enter de tiep tuc..." << RESET << flush;
        hienThi();
        string s;
        if (!getline(cin, s))
            throw HetDuLieuNhap();
    }

    // ------------------------------------------------------------------
    //  Khung net doi: "╔══╗", "║ noi dung ║" - luon dung do rong w (tinh ca 2 vien)
    // ------------------------------------------------------------------
    static string vien(const string &s) { return mau() + s + RESET; }

    void khungTren(int w, const string &tieu)
    {
        const BoKhung &k = K2();
        string giua;
        if (tieu.empty())
            giua = lap(k.H, w - 2);
        else
        {
            string t = " " + catNgan(tieu, w - 8) + " ";
            giua = k.H + k.H + DAM + CHU + t + RESET + mau() + lap(k.H, w - 4 - doRong(t));
        }
        cout << KHOI << vien(k.TL) << mau() << giua << RESET << vien(k.TR) << "\n";
    }

    void khungNgang(int w) { cout << KHOI << vien(K2().LT + lap(K2().H, w - 2) + K2().RT) << "\n"; }
    void khungDuoi(int w) { cout << KHOI << vien(K2().BL + lap(K2().H, w - 2) + K2().BR) << "\n"; }

    void khungDong(const string &chu, int w, const string &mauChu)
    {
        cout << KHOI << vien(K2().V) << " " << mauChu << canTrai(chu, w - 4) << RESET << " " << vien(K2().V) << "\n";
    }

    void khungDongGiua(const string &chu, int w, const string &mauChu)
    {
        cout << KHOI << vien(K2().V) << " " << mauChu << canGiuaChuoi(chu, w - 4) << RESET << " " << vien(K2().V)
             << "\n";
    }

    void khungDong2(const string &trai, const string &phai, int w, const string &mauTrai, const string &mauPhai)
    {
        int rongPhai = doRong(phai);
        int rongTrai = max(1, w - 4 - rongPhai - 1);
        string t = canTrai(trai, rongTrai);
        cout << KHOI << vien(K2().V) << " " << mauTrai << t << RESET
             << string((size_t)max(1, w - 4 - doRong(t) - rongPhai), ' ') << mauPhai << phai << RESET << " "
             << vien(K2().V) << "\n";
    }

    void khungNhan(const string &nhan, const string &giaTri, int w, int rongNhan, const string &mauGT)
    {
        const int rongGT = max(8, w - 4 - rongNhan - 1);
        auto dongs = ngatDong(giaTri, rongGT);
        const bool khach = epMauKhach || lienQuanKhach(nhan);
        for (size_t i = 0; i < dongs.size(); ++i)
            cout << KHOI << vien(K2().V) << " " << (khach ? KHACH : MO) << canGiuaChuoi(i == 0 ? nhan : "", rongNhan) << RESET
                 << " " << (khach ? KHACH : mauGT.empty() ? CHU : mauGT) << canGiuaChuoi(dongs[i], rongGT) << RESET << " "
                 << vien(K2().V) << "\n";
    }

    void khungTien(const string &nhan, double soTien, int w, const string &mauDong, const string &dau)
    {
        khungDong2(nhan, dau + dinhDangTienVN(soTien), w, mauDong.empty() ? CHU : mauDong,
                   mauDong.empty() ? CHU : mauDong);
    }

    // ------------------------------------------------------------------
    //  Thanh phan giao dien
    // ------------------------------------------------------------------
    void inBanner()
    {
        const int w = RONG;
        const BoKhung &k = K2();
        const string vienB = XANH_TROI;
        auto dong = [&](const string &noiDung, const string &mauChu)
        {
            cout << KHOI << vienB << k.V << RESET << mauChu << canGiuaChuoi(noiDung, w - 2) << RESET << vienB << k.V
                 << RESET << "\n";
        };
        string song = "~ "; // ASCII: chac chan 1 o tren moi font console
        cout << KHOI << vienB << k.TL << lap(k.H, w - 2) << k.TR << RESET << "\n";
        dong(lap(song, min(18, (w - 6) / 2)), XANH_TROI);
        dong("O C E A N   P E A R L   R E S O R T", DAM + VANG);
        dong("Hotel & Resort  -  He thong quan ly khach san", CHU);
        dong(lap(song, min(18, (w - 6) / 2)), XANH_TROI);
        cout << KHOI << vienB << k.BL << lap(k.H, w - 2) << k.BR << RESET << "\n";
    }

    void tieuDe(const string &s, const string &phu)
    {
        cout << "\n";
        khungTren(RONG);
        khungDongGiua(s, RONG, DAM + CHU);
        if (!phu.empty())
            for (auto &d : ngatDong(phu, RONG - 6))
                khungDongGiua(d, RONG, MO);
        khungDuoi(RONG);
        cout << "\n";
    }

    void tieuDeNho(const string &s)
    {
        const string &H = K1().H;
        string t = " " + catNgan(s, RONG - 8) + " ";
        cout << "\n" << KHOI << mau() << H << H << RESET << DAM << CHU << t << RESET << mau()
             << lap(H, max(2, RONG - 2 - doRong(t))) << RESET << "\n";
    }

    void duongKe(int rong, bool dam)
    {
        cout << KHOI << (dam ? mau() : MO) << lap(dam ? K2().H : K1().H, rong) << RESET << "\n";
    }

    // Menu trong MOT khung: [tieu de] / cac muc / 0. thoat / o "Nhap lua chon" (con tro dat trong khung)
    void inMenuKhung(const string &tieu, const string &phu, const vector<string> &muc, const string &mucThoat,
                     const vector<string> &mauSo)
    {
        const string soCach = "   ";
        const int rongSo = muc.size() >= 10 ? 3 : 2; // "1." / "10."
        int noiDung = max(doRong(tieu), doRong("Nhap lua chon: ") + 4);
        for (auto &m : muc)
            noiDung = max(noiDung, rongSo + 1 + doRong(m));
        noiDung = max(noiDung, rongSo + 1 + doRong(mucThoat));
        const int w = max(46, min(RONG_MENU, noiDung + 2 * (int)soCach.size() + 4));
        const int rongChu = w - 4 - (int)soCach.size() * 2 - rongSo - 1;
        auto dongMuc = [&](const string &so, const string &chu, const string &mauSoNay, const string &mauChu)
        {
            auto dongs = ngatDong(chu, rongChu);
            for (size_t i = 0; i < dongs.size(); ++i)
            {
                string trai = soCach + DAM + mauSoNay + canPhai(i == 0 ? so + "." : "", rongSo) + RESET + " " +
                              mauChu + canTrai(dongs[i], rongChu) + RESET + soCach;
                cout << KHOI << vien(K2().V) << " " << trai << " " << vien(K2().V) << "\n";
            }
        };
        khungTren(w);
        if (!tieu.empty())
        {
            khungDongGiua(tieu, w, DAM + CHU);
            if (!phu.empty())
                for (auto &d : ngatDong(phu, w - 8))
                    khungDongGiua(d, w, MO);
            khungNgang(w);
        }
        khungDong("", w);
        for (size_t i = 0; i < muc.size(); ++i)
        {
            bool riengMau = i < mauSo.size() && !mauSo[i].empty();
            const bool khach = lienQuanKhach(muc[i]) || chuDeHienTai == ChuDe::KHACH_HANG; // danh muc khach hang: ca so lan chu deu VANG
            (void)riengMau;
            (void)khach;
            dongMuc(to_string(i + 1), muc[i], CHU, CHU); // so + chu cung mot mau trong toan menu
        }
        dongMuc("0", mucThoat, MO, MO);
        khungDong("", w);
        // O nhap: chu "Nhap lua chon:" + ky tu danh dau vi tri con tro (khong chiem cho tren man hinh)
        string nhap = soCach + CHU + "Nhap lua chon: " + RESET;
        cout << KHOI << vien(K2().V) << " " << nhap << O_NHAP << string((size_t)max(0, w - 4 - doRong(nhap)), ' ')
             << " " << vien(K2().V) << "\n";
        khungDuoi(w);
        menuCho = dangCanGiua;
        viTriSauMenu = boDem.cho.size();
    }

    void inMenu(const vector<string> &muc, const string &mucThoat, const vector<string> &mauSo)
    {
        inMenuKhung("", "", muc, mucThoat, mauSo);
    }

    // ------------------------------------------------------------------
    //  Doc phim mui ten / Enter / ESC (thoi gian thuc, khong can Enter)
    // ------------------------------------------------------------------
    Phim docPhim(char *soPhim)
    {
        if (soPhim) *soPhim = 0;
#ifdef _WIN32
        int c = _getch();
        if (c == 0 || c == 224) // phim mo rong (mui ten)
        {
            c = _getch();
            if (c == 72) return Phim::LEN;
            if (c == 80) return Phim::XUONG;
            return Phim::KHAC;
        }
        if (c == 13) return Phim::ENTER;
        if (c == 27) return Phim::ESC;
        if (c >= '0' && c <= '9')
        {
            if (soPhim) *soPhim = (char)c;
            return Phim::SO;
        }
        return Phim::KHAC;
#else
        termios oldt{}, newt{};
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        unsigned char c = 0;
        if (read(STDIN_FILENO, &c, 1) != 1) { tcsetattr(STDIN_FILENO, TCSANOW, &oldt); return Phim::KHAC; }
        if (c == 27)
        {
            int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
            fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
            unsigned char c2 = 0, c3 = 0;
            int n2 = read(STDIN_FILENO, &c2, 1);
            int n3 = (n2 == 1) ? read(STDIN_FILENO, &c3, 1) : 0;
            fcntl(STDIN_FILENO, F_SETFL, flags);
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            if (n2 == 1 && c2 == '[' && n3 == 1)
            {
                if (c3 == 'A') return Phim::LEN;
                if (c3 == 'B') return Phim::XUONG;
            }
            return Phim::ESC;
        }
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        if (c == 10 || c == 13) return Phim::ENTER;
        if (c >= '0' && c <= '9')
        {
            if (soPhim) *soPhim = (char)c;
            return Phim::SO;
        }
        return Phim::KHAC;
#endif
    }

    // Ve menu tuong tac: ↑↓ di chuyen, Enter chon, ESC quay lai (tra 0)
    // Muc dang chon: ▶ ... ◀ + mau chu de hien tai. Dong huong dan mau vang.
    // ------------------------------------------------------------------
    //  Menu tuong tac: VE 1 LAN, ↑↓ chi doi selectedIndex + ve lai 2 dong
    //  Khong clear man hinh, khong reload du lieu, khong in menu moi.
    // ------------------------------------------------------------------
    static void datConTro(int x, int y)
    {
#ifdef _WIN32
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD c;
        c.X = (SHORT)max(0, x);
        c.Y = (SHORT)max(0, y);
        SetConsoleCursorPosition(h, c);
#else
        cout << "\033[" << (y + 1) << ";" << (x + 1) << "H";
#endif
    }

    static void anConTro(bool an)
    {
#ifdef _WIN32
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        if (GetConsoleCursorInfo(h, &info))
        {
            info.bVisible = an ? FALSE : TRUE;
            SetConsoleCursorInfo(h, &info);
        }
#else
    ghiRa(an ? "\033[?25l" : "\033[?25h");
#endif
    }

    static int doRongConsole()
    {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi{};
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
            return max(20, (int)(csbi.srWindow.Right - csbi.srWindow.Left + 1));
#else
        // Lay chieu rong cua terminal that. Khong gia dinh RONG_MENU la so cot
        // terminal, vi viec in dong dai hon cua so se wrap va lam lech cursor.
        struct winsize ws{};
        if (isatty(STDOUT_FILENO) && ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
            return max(20, (int)ws.ws_col);
#endif
        return max(20, RONG_MENU);
    }

    // Can le khung menu/form theo cung truc voi noi dung man hinh (bang, tieu de, inKhoi).
    // Dung xManHinh/wManHinh tu BoDemCanGiua khi co, tranh can rieng gay lech.
    static void layCanLeKhung(int rongMongMuon, int &outPad, int &outW)
    {
        const int consoleW = max(20, doRongConsole());
        int w = min(max(20, rongMongMuon), max(20, consoleW - 2));
        int pad;
        if (wManHinh > 0 && xManHinh >= 0)
        {
            w = min(w, wManHinh);
            pad = xManHinh + max(0, (wManHinh - w) / 2);
            if (pad + w > consoleW - 1)
                pad = max(0, (consoleW - w) / 2);
        }
        else
            pad = centerPadding(consoleW, w);
        outPad = pad;
        outW = w;
    }

    static int doCaoConsole()
    {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
            return max(15, (int)(csbi.srWindow.Bottom - csbi.srWindow.Top + 1));
#endif
        return 30;
    }

#ifndef _WIN32
    // Query vi tri cursor theo ANSI DSR (ESC[6n). Neu terminal khong ho tro,
    // tra false de dung fallback tuong doi ma khong lam thay doi man hinh.
    static bool layViTriConTroAnsi(int &row, int &col)
    {
        if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))
            return false;
        termios oldt{};
        if (tcgetattr(STDIN_FILENO, &oldt) != 0)
            return false;
        termios raw = oldt;
        raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 1;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0)
            return false;
        ghiRa("\033[6n");
        string reply;
        char ch = 0;
        // Response format: ESC [ row ; col R. Keep timeout short to avoid hanging
        // in terminals/IDE consoles that do not implement cursor-position reports.
        for (int i = 0; i < 32; ++i)
        {
            ssize_t n = ::read(STDIN_FILENO, &ch, 1);
            if (n != 1)
                break;
            reply.push_back(ch);
            if (ch == 'R')
                break;
        }
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        size_t b = reply.find('['), semi = reply.find(';', b == string::npos ? 0 : b + 1), end = reply.find('R');
        if (b == string::npos || semi == string::npos || end == string::npos || semi >= end)
            return false;
        try
        {
            row = stoi(reply.substr(b + 1, semi - b - 1)) - 1;
            col = stoi(reply.substr(semi + 1, end - semi - 1)) - 1;
            return row >= 0 && col >= 0;
        }
        catch (...) { return false; }
    }
#endif

    static int layDongHienTai()
    {
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
            return (int)csbi.dwCursorPosition.Y;
#endif
        return 0;
    }

    static int chonMenuTuongTacLegacy(const string &tieu, const string &phu, const vector<string> &muc,
                                      const string & /*mucThoat*/, const vector<string> & /*mauSo*/, int chonBanDau,
                                      const function<void()> &veLaiHeader)
    {
        // ============================================================
        // MENU TINH:
        // - Ve menu dung 1 lan.
        // - Moi item co 1 toa do Y co dinh.
        // - ↑/↓ CHI ve lai 2 dong: oldIndex va newIndex.
        // - Tuyet doi khong system("cls"), khong append menu moi.
        // - Windows dung SetConsoleCursorPosition() de ghi dung dong.
        // ============================================================
        const int soMuc = (int)muc.size();
        if (soMuc <= 0)
            return 0;

        int activeIndex = chonBanDau;
        if (activeIndex < 0 || activeIndex >= soMuc)
            activeIndex = 0;

        // Header/banner ve trong che do can giua truoc, de dong bo truc ngang.
        if (dangCanGiua)
        {
            if (veLaiHeader)
            {
                veLaiHeader();
                hienThi();
            }
            ketThucCanGiua();
        }
        else if (veLaiHeader)
            veLaiHeader();

        int padding = 0, w = RONG;
        layCanLeKhung(RONG, padding, w);
        const int startX = padding;
        const string pad((size_t)padding, ' ');

        auto noiMuc = [&](int i, bool dang) -> string
        {
            if (dang)
                return "▶ " + to_string(i + 1) + ". " + muc[(size_t)i] + " ◀";
            return "  " + to_string(i + 1) + ". " + muc[(size_t)i];
        };

        auto inDongMenu = [&](int i, bool dang)
        {
            const string mauDong = dang ? (DAM + mau()) : CHU;
            cout << pad << vien(K2().V) << " "
                 << mauDong << canTrai(noiMuc(i, dang), w - 4) << RESET
                 << " " << vien(K2().V);
#ifdef _WIN32
            cout.flush();
#else
            cout << "\n";
#endif
        };

        // ----- VE KHUNG + MENU DUNG 1 LAN -----
        if (!tieu.empty())
        {
            cout << pad << vien(K2().TL + lap(K2().H, w - 2) + K2().TR) << "\n";
            cout << pad << vien(K2().V) << " " << DAM << mau()
                 << canGiuaChuoi(tieu, w - 4) << RESET
                 << " " << vien(K2().V) << "\n";

            if (!phu.empty())
            {
                for (const auto &d : ngatDong(phu, w - 8))
                {
                    cout << pad << vien(K2().V) << " " << MO
                         << canGiuaChuoi(d, w - 4) << RESET
                         << " " << vien(K2().V) << "\n";
                }
            }

            cout << pad << vien(K2().LT + lap(K2().H, w - 2) + K2().RT) << "\n";
        }
        else
        {
            cout << pad << vien(K2().TL + lap(K2().H, w - 2) + K2().TR) << "\n";
        }

        cout << pad << vien(K2().V) << " " << canTrai("", w - 4)
             << " " << vien(K2().V) << "\n";

#ifdef _WIN32
        // Luu toa do Y THUC TE cua tung item ngay luc ve lan dau.
    vector<SHORT> itemY((size_t)soMuc, 0);
    int startY = 0;
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        WORD normalAttributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    CONSOLE_SCREEN_BUFFER_INFO firstItem{};
        if (GetConsoleScreenBufferInfo(hConsole, &firstItem))
        {
        startY = firstItem.dwCursorPosition.Y;
            normalAttributes = firstItem.wAttributes;
        }
#else
        vector<int> itemY((size_t)soMuc);
        int firstItemRow = -1, firstItemCol = 0;
        const bool hasAbsoluteAnsiCursor = layViTriConTroAnsi(firstItemRow, firstItemCol);
        int safeAnsiRow = -1, safeAnsiCol = 0;
    int startY = 0;
#endif

        for (int i = 0; i < soMuc; ++i)
        {
#ifdef _WIN32
        itemY[(size_t)i] = (SHORT)(startY + i);
        COORD itemPos{};
        itemPos.X = (SHORT)startX;
        itemPos.Y = itemY[(size_t)i];
        SetConsoleCursorPosition(hConsole, itemPos);
        SetConsoleTextAttribute(hConsole, activeIndex == i
                          ? mauMenuWindows()
                          : normalAttributes);
#endif
            inDongMenu(i, activeIndex == i);
#ifdef _WIN32
        SetConsoleTextAttribute(hConsole, normalAttributes);
#endif
#ifndef _WIN32
            // Neu terminal ho tro DSR, luu toa do tuyet doi cho tung item.
            // Fallback se dung cursor-relative, nhung khong dung itemY gia lap.
            itemY[(size_t)i] = hasAbsoluteAnsiCursor ? firstItemRow + i : i;
#endif
        }
#ifdef _WIN32
        COORD afterItems{};
        afterItems.X = 0;
        afterItems.Y = (SHORT)(startY + soMuc);
        SetConsoleCursorPosition(hConsole, afterItems);
#else
            // Luu toa do Y cua dong dau tien cua menu (muc thu 0) de tinh toan tuyet doi
            startY = itemY[0];
#endif

        cout << pad << vien(K2().V) << " " << canTrai("", w - 4)
             << " " << vien(K2().V) << "\n";

        {
            // Footer tu rut gon tren terminal hep de khong bao gio tu wrap.
            const int footerW = max(1, w - 4);
            string hd = "ENTER: chon | ESC: quay lai | ↑/↓: di chuyen";
            if (doRong(hd) > footerW)
                hd = "ENTER | ESC | ↑/↓";
            if (doRong(hd) > footerW)
                hd = "ENTER | ESC";
            if (doRong(hd) > footerW)
                hd = "ENTER | ESC";
            cout << pad << vien(K2().V) << " " << VANG
                 << canGiuaChuoi(hd, footerW) << RESET
                 << " " << vien(K2().V) << "\n";
        }

        cout << pad << vien(K2().BL + lap(K2().H, w - 2) + K2().BR) << "\n";
        cout.flush();
#ifndef _WIN32
        if (hasAbsoluteAnsiCursor)
            (void)layViTriConTroAnsi(safeAnsiRow, safeAnsiCol);
#endif

        anConTro(true);

#ifdef _WIN32
        // Luu vi tri an toan sau menu. Moi lan redraw xong se tra cursor ve day.
        COORD safeCursor{};
        {
            CONSOLE_SCREEN_BUFFER_INFO csbi{};
            if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
                safeCursor = csbi.dwCursorPosition;
        }

        auto veLaiDongWindows = [&](int index, bool isActive)
        {
            if (index < 0 || index >= soMuc)
                return;

            // Tuyet doi khong xoa man hinh.
            // Chi chon dung dong cua item can thay doi.
            // Tinh toa do Y tuyet doi: startY + index
            // Khong goi ham ve dong co '\n' de tranh cursor tu nhay xuong dong va sinh menu lap.
            const SHORT offsetInnerX = 0;
            const SHORT targetX = (SHORT)(startX + offsetInnerX);
            const SHORT targetY = (SHORT)(startY + index);
            COORD pos{};
            pos.X = targetX;
            pos.Y = targetY;
            SetConsoleCursorPosition(hConsole, pos);

            CONSOLE_SCREEN_BUFFER_INFO csbi{};
            GetConsoleScreenBufferInfo(hConsole, &csbi);
            const DWORD lineWidth = (DWORD)max(1, w);
            DWORD written = 0;
            FillConsoleOutputCharacterA(hConsole, ' ', lineWidth, pos, &written);
            FillConsoleOutputAttribute(hConsole, normalAttributes, lineWidth, pos, &written);
            SetConsoleCursorPosition(hConsole, pos);

              SetConsoleTextAttribute(hConsole, isActive
                                            ? mauMenuWindows()
                                            : normalAttributes);
              const string mauDong = isActive ? (DAM + mau()) : CHU;
            cout << vien(K2().V) << " " << mauDong
                  << canTrai(noiMuc(index, isActive), w - 4) << RESET
                 << " " << vien(K2().V);
            cout.flush();
              SetConsoleTextAttribute(hConsole, normalAttributes);
            SetConsoleCursorPosition(hConsole, safeCursor);
        };
#else
        auto veLaiDongAnsi = [&](int i, bool dangChon)
        {
            if (i < 0 || i >= soMuc)
                return;

            if (hasAbsoluteAnsiCursor && safeAnsiRow >= 0)
            {
                // Dung toa do tuyet doi: startY + i
                // Khong phu thuoc so dong da cuon, ky tu Unicode hay vi tri cursor sau khi terminal wrap dong.
                cout << "\033[" << ((startY + i) + 1) << ";1H\r\033[2K";
                cout << pad << vien(K2().V) << " "
                     << (dangChon ? (DAM + mau()) : CHU)
                     << canTrai(noiMuc(i, dangChon), w - 4) << RESET
                     << " " << vien(K2().V);
                cout << "\033[K";
                cout << "\033[" << (safeAnsiRow + 1) << ";" << (safeAnsiCol + 1) << "H";
                cout.flush();
                return;
            }

            // Fallback terminal khong ho tro DSR: cursor dang o dong ngay sau
            // vien duoi. Vi chieu rong da duoc gioi han de khong wrap, so dong
            // vat ly co dinh: item i cach cursor an toan (soMuc - i + 3) dong.
            // Xoa dong truoc khi ve de khong sot ky tu ACTIVE cu.
            const int khoangCach = soMuc - i + 3;
            cout << "\033[" << khoangCach << "A\r\033[2K";
            cout << pad << vien(K2().V) << " "
                 << (dangChon ? (DAM + mau()) : CHU)
                 << canTrai(noiMuc(i, dangChon), w - 4) << RESET
                 << " " << vien(K2().V) << "\033[K";
            cout.flush();
            cout << "\033[" << khoangCach << "B\r";
            cout.flush();
        };
#endif

        while (true)
        {
            char soPhim = 0;
            Phim p = docPhim(&soPhim);

            if (p == Phim::LEN || p == Phim::XUONG)
            {
                const int oldIndex = activeIndex;

                if (p == Phim::LEN)
                    activeIndex = (activeIndex - 1 + soMuc) % soMuc;
                else
                    activeIndex = (activeIndex + 1) % soMuc;

                if (oldIndex == activeIndex)
                    continue;

#ifdef _WIN32
                // CHI 2 DONG: Chi xoa/mau phu 2 dong can thay doi (oldIndex va activeIndex).
                // Khi oldIndex == activeIndex (ko xay ra vi check tren), khong lam gi.
                if (oldIndex != activeIndex) {
                    veLaiDongWindows(oldIndex, false);
                    veLaiDongWindows(activeIndex, true);
                }
#else
                // ANSI terminal fallback.
                // Windows (muc tieu chinh) luon dung SetConsoleCursorPosition().
                if (oldIndex != activeIndex) {
                    veLaiDongAnsi(oldIndex, false);
                    veLaiDongAnsi(activeIndex, true);
                }
#endif
            }
            else if (p == Phim::ENTER)
            {
                anConTro(false);
#ifdef _WIN32
                SetConsoleCursorPosition(hConsole, safeCursor);
#endif
                cout << "\n";
                if (!dangCanGiua)
                    batDauCanGiua();
                return activeIndex + 1;
            }
            else if (p == Phim::ESC)
            {
                anConTro(false);
#ifdef _WIN32
                SetConsoleCursorPosition(hConsole, safeCursor);
#endif
                cout << "\n";
                if (!dangCanGiua)
                    batDauCanGiua();
                return 0;
            }
        }
    }

    int chonMenuTuongTac(const string &tieu, const string &phu, const vector<string> &muc,
                         const string & /*mucThoat*/, const vector<string> & /*mauSo*/, int chonBanDau,
                         const function<void()> &veLaiHeader)
    {
        const int soMuc = (int)muc.size();
        if (soMuc <= 0)
            return 0;

        int selected = chonBanDau;
        if (selected < 0 || selected >= soMuc)
            selected = 0;

        // Khi co noi dung phia tren (bang phong, danh sach...), dung ve tuan tu (sequential)
        // de menu LUON xuat hien ngay ben duoi, khong bi day ra ngoai man hinh.
        // Khi khong co noi dung (menu chinh / menu don), van dung absolute frame de dep.
        const bool coNoiDungTren = (veLaiHeader != nullptr);

        // Helper: khoi phuc che do can giua sau khi menu tu ve (khong de tat can giua vinh vien)
        auto khoiPhucCanGiua = [&]() {
            if (!dangCanGiua)
                batDauCanGiua();
        };

        auto inMenuSequential = [&]() {
            int pad = 0, w = RONG;
            layCanLeKhung(RONG, pad, w);
            const string padS((size_t)pad, ' ');

            auto dongKhung = [&](const string &noi, bool center, bool highlight = false) {
                string body = center ? canGiuaChuoi(noi, w - 4) : canTrai(noi, w - 4);
                cout << padS << K2().V << " ";
                if (highlight)
                    cout << DAM << mau() << body << RESET;
                else
                    cout << CHU << body << RESET;
                cout << " " << K2().V << "\n";
            };

            cout << "\n";
            cout << padS << K2().TL << lap(K2().H, w - 2) << K2().TR << "\n";
            if (!tieu.empty())
            {
                dongKhung(tieu, true);
                if (!phu.empty())
                    for (const auto &line : ngatDong(phu, w - 8))
                        dongKhung(line, true);
                cout << padS << K2().LT << lap(K2().H, w - 2) << K2().RT << "\n";
            }
            dongKhung("", false);
            for (int i = 0; i < soMuc; ++i)
            {
                const bool active = (i == selected);
                string label = active ? ("▶ " + to_string(i + 1) + ". " + muc[(size_t)i] + " ◀")
                                      : ("  " + to_string(i + 1) + ". " + muc[(size_t)i]);
                dongKhung(label, false, active);
            }
            dongKhung("", false);
            {
                string hd = "ENTER: chon | ESC: quay lai | ↑/↓: di chuyen";
                if (doRong(hd) > w - 4)
                    hd = "ENTER | ESC | ↑/↓";
                cout << padS << K2().V << " " << VANG << canGiuaChuoi(hd, w - 4) << RESET
                     << " " << K2().V << "\n";
            }
            cout << padS << K2().BL << lap(K2().H, w - 2) << K2().BR << "\n";
            cout << flush;
        };

        if (coNoiDungTren)
        {
            // Menu ben duoi danh sach. Lan dau ve header+bang (van trong che do can giua),
            // roi moi ve khung menu can theo cung truc xManHinh/wManHinh.
            const string tieuMenu = "CHON CHUC NANG";
            const string phuMenu = "ESC de quay lai";
            int soDongMenu = 0;

            auto inMenuOnly = [&]() {
                int pad = 0, w = RONG;
                layCanLeKhung(RONG, pad, w);
                const string padS((size_t)pad, ' ');
                auto dongKhung = [&](const string &noi, bool center, bool highlight = false) {
                    string body = center ? canGiuaChuoi(noi, w - 4) : canTrai(noi, w - 4);
                    cout << padS << K2().V << " ";
                    if (highlight)
                        cout << DAM << mau() << body << RESET;
                    else
                        cout << CHU << body << RESET;
                    cout << " " << K2().V << "\n";
                };
                int dem = 0;
                auto line = [&]() { ++dem; };
                cout << "\n"; line();
                cout << padS << K2().TL << lap(K2().H, w - 2) << K2().TR << "\n"; line();
                dongKhung(tieuMenu, true); line();
                dongKhung(phuMenu, true); line();
                cout << padS << K2().LT << lap(K2().H, w - 2) << K2().RT << "\n"; line();
                dongKhung("", false); line();
                for (int i = 0; i < soMuc; ++i)
                {
                    const bool active = (i == selected);
                    string label = active ? ("▶ " + to_string(i + 1) + ". " + muc[(size_t)i] + " ◀")
                                          : ("  " + to_string(i + 1) + ". " + muc[(size_t)i]);
                    dongKhung(label, false, active); line();
                }
                dongKhung("", false); line();
                {
                    string hd = "ENTER: chon | ESC: quay lai | ↑/↓: di chuyen";
                    if (doRong(hd) > w - 4)
                        hd = "ENTER | ESC | ↑/↓";
                    cout << padS << K2().V << " " << VANG << canGiuaChuoi(hd, w - 4) << RESET
                         << " " << K2().V << "\n";
                    line();
                }
                cout << padS << K2().BL << lap(K2().H, w - 2) << K2().BR << "\n"; line();
                cout << flush;
                soDongMenu = dem;
            };

            anConTro(true);
            // Ve header/bang KHI VAN dang can giua de noi dung can dung truc
            if (!dangCanGiua)
                batDauCanGiua();
            veLaiHeader();
            hienThi(); // flush bang/tieu de, cap nhat xManHinh/wManHinh
            ketThucCanGiua(); // menu ve truc tiep (khong qua bo dem) nhung dung cung pad
            inMenuOnly();
            while (true)
            {
                char soPhim = 0;
                const Phim key = docPhim(&soPhim);
                if (key == Phim::LEN || key == Phim::XUONG)
                {
                    selected = key == Phim::LEN ? (selected - 1 + soMuc) % soMuc
                                                : (selected + 1) % soMuc;
                    // Chi ve lai menu: dua cursor len soDongMenu dong
                    if (soDongMenu > 0)
                        cout << "\033[" << soDongMenu << "A" << flush;
                    inMenuOnly();
                }
                else if (key == Phim::ENTER)
                {
                    anConTro(false);
                    xoaManHinh();
                    khoiPhucCanGiua();
                    return selected + 1;
                }
                else if (key == Phim::ESC)
                {
                    anConTro(false);
                    xoaManHinh();
                    khoiPhucCanGiua();
                    return 0;
                }
            }
        }

        // ===== Che do absolute (menu chinh / khong co noi dung tren) =====
        // Flush noi dung dang cho (neu co) roi tat can giua tam thoi de ve menu tuyet doi
        if (dangCanGiua)
            ketThucCanGiua();
        if (veLaiHeader)
            veLaiHeader();

        int padding = 0, w = RONG;
        layCanLeKhung(RONG, padding, w);
        int startX = padding;
        int startY = -1;
        bool hasAbsolutePosition = false;
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        WORD normalAttributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        CONSOLE_SCREEN_BUFFER_INFO csbi{};
        if (GetConsoleScreenBufferInfo(hConsole, &csbi))
        {
            startX += csbi.srWindow.Left;
            startY = csbi.dwCursorPosition.Y;
            normalAttributes = csbi.wAttributes;
            hasAbsolutePosition = true;
        }
#else
        int cursorColumn = 0;
        hasAbsolutePosition = layViTriConTroAnsi(startY, cursorColumn);
#endif

        auto bodyRow = [&](const string &text, bool center)
        {
            const string content = center ? canGiuaChuoi(text, w - 4) : canTrai(text, w - 4);
            return K2().V + " " + content + " " + K2().V;
        };
        vector<pair<string, int>> rows;
        auto addRow = [&](const string &text, int style = MENU_FRAME_NORMAL)
        {
            rows.push_back({canTrai(catNgan(text, w), w), style});
        };
        const string top = K2().TL + lap(K2().H, w - 2) + K2().TR;

        // Cuon danh sach khi nhieu muc (vd 50 khach) de khong bi cat man hinh
        const int MAX_VISIBLE = 12;
        int scrollTop = 0;
        auto ensureVisible = [&]() {
            if (soMuc <= MAX_VISIBLE) { scrollTop = 0; return; }
            if (selected < scrollTop) scrollTop = selected;
            if (selected >= scrollTop + MAX_VISIBLE) scrollTop = selected - MAX_VISIBLE + 1;
            if (scrollTop < 0) scrollTop = 0;
            if (scrollTop > soMuc - MAX_VISIBLE) scrollTop = soMuc - MAX_VISIBLE;
        };
        ensureVisible();

        auto rebuildRows = [&]() {
            rows.clear();
            if (!tieu.empty())
            {
                addRow(top);
                addRow(bodyRow(tieu, true));
                if (!phu.empty())
                    for (const auto &line : ngatDong(phu, w - 8))
                        addRow(bodyRow(line, true));
                addRow(K2().LT + lap(K2().H, w - 2) + K2().RT);
            }
            else
                addRow(top);
            addRow(bodyRow("", false));
            const int from = scrollTop;
            const int to = (soMuc <= MAX_VISIBLE) ? soMuc : min(soMuc, scrollTop + MAX_VISIBLE);
            if (from > 0)
                addRow(bodyRow("  ... (" + to_string(from) + " muc tren)", false), MENU_FRAME_FOOTER);
            for (int i = from; i < to; ++i)
            {
                const bool active = i == selected;
                const string label = active ? "▶ " + to_string(i + 1) + ". " + muc[(size_t)i] + " ◀"
                                            : "  " + to_string(i + 1) + ". " + muc[(size_t)i];
                addRow(bodyRow(label, false), active ? MENU_FRAME_SELECTED : MENU_FRAME_NORMAL);
            }
            if (to < soMuc)
                addRow(bodyRow("  ... (" + to_string(soMuc - to) + " muc duoi)", false), MENU_FRAME_FOOTER);
            addRow(bodyRow("", false));
            string hd = (soMuc > MAX_VISIBLE)
                ? "ENTER: chon | ESC: quay lai | ↑/↓: cuon danh sach"
                : "ENTER: chon | ESC: quay lai | ↑/↓: di chuyen";
            addRow(bodyRow(hd, true), MENU_FRAME_FOOTER);
            addRow(K2().BL + lap(K2().H, w - 2) + K2().BR);
        };

        rebuildRows();

        // Chua san cho cho ca khung menu: khi noi dung phia tren (bang phong, bang dich vu...) da
        // day den cuoi man hinh, khung ve tuyet doi se bi cat mat (khong thay muc menu). In dung
        // so dong can thiet de man hinh cuon len, roi lay lai toa do dong dau cua khung.
        {
            const int soDongKhung = (int)rows.size();
            if (soDongKhung > 0)
            {
                cout << string((size_t)soDongKhung, '\n') << flush;
#ifdef _WIN32
                CONSOLE_SCREEN_BUFFER_INFO sau{};
                if (GetConsoleScreenBufferInfo(hConsole, &sau))
                {
                    startY = sau.dwCursorPosition.Y - soDongKhung;
                    if (startY < 0)
                        startY = 0;
                }
#else
                int rowSau = -1, colSau = 0;
                if (hasAbsolutePosition && layViTriConTroAnsi(rowSau, colSau))
                    startY = max(0, rowSau - soDongKhung);
                else
                    cout << "\033[" << soDongKhung << "A\r" << flush; // khong co toa do tuyet doi: lui lai tuong doi
#endif
            }
        }

        bool firstFrame = true;
        auto renderFrame = [&]()
        {
#ifdef _WIN32
            beginFrame(w, (int)rows.size(), normalAttributes);
#else
            beginFrame(w, (int)rows.size());
#endif
            for (size_t row = 0; row < rows.size(); ++row)
                writeText(0, (int)row, rows[row].first, rows[row].second);
            endFrame(startX, startY, hasAbsolutePosition, firstFrame);
            firstFrame = false;
        };

        anConTro(true);
        renderFrame();
        while (true)
        {
            char soPhim = 0;
            const Phim key = docPhim(&soPhim);
            if (key == Phim::LEN || key == Phim::XUONG)
            {
                selected = key == Phim::LEN ? (selected - 1 + soMuc) % soMuc : (selected + 1) % soMuc;
                ensureVisible();
                rebuildRows();
                renderFrame();
            }
            else if (key == Phim::ENTER)
            {
                anConTro(false);
                xoaManHinh();
                khoiPhucCanGiua();
                return selected + 1;
            }
            else if (key == Phim::ESC)
            {
                anConTro(false);
                xoaManHinh();
                khoiPhucCanGiua();
                return 0;
            }
        }
    }

    // Thong bao: tien to co dinh, tu dong ngat dong, dong tiep theo thut le thang hang
    static void inThongBao(const string &tienTo, const string &mauTT, const string &mauChu, const string &s)
    {
        const int le = doRong(tienTo) + 1;
        auto dongs = ngatDong(s, RONG - le);
        for (size_t i = 0; i < dongs.size(); ++i)
            cout << "  " << (i == 0 ? mauTT + tienTo + RESET : string((size_t)doRong(tienTo), ' ')) << " " << mauChu
                 << dongs[i] << RESET << "\n";
    }

    void thanhCong(const string &s) { inThongBao(OK, XANH_LA, XANH_LA, s); }
    void loi(const string &s) { inThongBao(SAI, DO, DO, s); }
    void canhBao(const string &s) { inThongBao(CHU_Y, CAM, CAM, s); }
    void thongTin(const string &s) { inThongBao(INFO, VANG, VANG, s); }

    void dongThongTin(const string &nhan, const string &giaTri, const string &mauGT, int rongNhan)
    {
        const int rongGT = max(10, RONG - 4 - rongNhan);
        auto dongs = ngatDong(giaTri, rongGT);
        const bool khach = lienQuanKhach(nhan);
        for (size_t i = 0; i < dongs.size(); ++i)
            cout << "    " << (khach ? KHACH : MO) << canTrai(i == 0 ? nhan : "", rongNhan) << RESET
                 << (khach ? KHACH : mauGT.empty() ? CHU : mauGT) << dongs[i] << RESET << "\n";
    }

    void inKhoi(const string &tieu, const vector<pair<string, string>> &dong, int rongNhan, bool khachHang)
    {
        epMauKhach = khachHang || lienQuanKhach(tieu);
        khungTren(RONG_KHOI, tieu);
        for (auto &d : dong)
            khungNhan(d.first, d.second, RONG_KHOI, rongNhan);
        khungDuoi(RONG_KHOI);
        epMauKhach = false;
    }

    void dongTien(const string &nhan, double soTien, const string &mauDong, int rong, const string &dau)
    {
        string tien = dau + dinhDangTienVN(soTien);
        int cach = max(1, rong - 4 - doRong(nhan) - doRong(tien));
        const string &m = mauDong.empty() ? CHU : mauDong;
        cout << "    " << m << nhan << string((size_t)cach, ' ') << tien << RESET << "\n";
    }

    // Chi so 2 cot: nhan rong hon de khong cat chu (Ty le DT/luong, Luong thuc nhan...)
    void inChiSo(const vector<pair<string, string>> &cs)
    {
        const int le = 2, cach = 3;
        const int o = (RONG - le - cach) / 2; // do rong moi cap (nhan + gia tri)
        // Uu tien nhan ~55% de tieu de dai van doc duoc; gia tri can phai
        int rongNhan = min(28, max(18, o * 55 / 100));
        int rongGT = o - rongNhan;
        if (rongGT < 12)
        {
            rongGT = 12;
            rongNhan = o - rongGT;
        }
        for (size_t i = 0; i < cs.size(); i += 2)
        {
            cout << string(le, ' ') << MO << canTrai(cs[i].first, rongNhan) << RESET << DAM << CHU
                 << canPhai(cs[i].second, rongGT) << RESET;
            if (i + 1 < cs.size())
                cout << string(cach, ' ') << MO << canTrai(cs[i + 1].first, rongNhan) << RESET << DAM << CHU
                     << canPhai(cs[i + 1].second, rongGT) << RESET;
            cout << "\n";
        }
    }

    // Bieu do cot ngang: [nhan][ ####.... ][gia tri][ %] - moi dong DUNG RONG cot -> thanh # thang hang tuyet doi
    void veBieuDoCot(const string &tieu, const vector<pair<string, double>> &dl, bool laTien, const string &donVi)
    {
        tieuDeNho(tieu);
        if (dl.empty())
        {
            cout << "    " << MO << "(Chua co du lieu)" << RESET << "\n";
            return;
        }
        // Bo cuc CO DINH cho moi bieu do -> cac thanh ##### cua moi bieu do deu thang hang, cung chieu dai:
        //   [le 4][nhan 18] [thanh ####....] [gia tri 14 can phai][phan tram 5]
        // Cot gia tri du cho "999,999,999d"; neu so dai hon thi cot tu noi rong (khong bao gio rot chu "d").
        const bool laPhanTram = donVi == "%"; // du lieu da la % (cong suat): thanh do theo thang 100%, khong cot %
        double maxV = 0, tong = 0;
        int rongGT = 14;
        vector<string> giaTri;
        for (auto &x : dl)
        {
            maxV = max(maxV, x.second);
            tong += x.second;
            string g = laTien ? dinhDangTienVN(x.second)
                       : laPhanTram ? dinhDangSoThuc(x.second, 1) + "%"
                                    : dinhDangSoThuc(x.second, 0) + (donVi.empty() ? "" : " " + donVi);
            giaTri.push_back(g);
            rongGT = max(rongGT, doRong(g));
        }
        if (laPhanTram)
            maxV = 100;
        const int le = 4, rongNhan = 18, rongPT = 5; // " 100%"
        const int rongCot = max(8, RONG - le - rongNhan - 1 - 1 - rongGT - rongPT); // phan con lai cho thanh ####
        auto &sac = sacCot();
        for (size_t i = 0; i < dl.size(); ++i)
        {
            int dai = maxV > 0 ? (int)lround(min(1.0, dl[i].second / maxV) * rongCot) : 0;
            if (dl[i].second > 0 && dai == 0)
                dai = 1;
            string phanTram = !laPhanTram && tong > 0 ? dinhDangSoThuc(100.0 * dl[i].second / tong, 0) + "%" : "";
            const bool khach = lienQuanKhach(tieu);
            cout << string(le, ' ') << (khach ? KHACH : CHU) << canTrai(dl[i].first, rongNhan) << RESET << " "
                 << sac[i % sac.size()] << lap(kyTuCot(), dai) << RESET << MO << lap(kyTuNen(), rongCot - dai) << RESET
                 << " " << DAM << CHU << canPhai(giaTri[i], rongGT) << RESET << MO << canPhai(phanTram, rongPT) << RESET
                 << "\n";
        }
    }

    // ------------------------------------------------------------------
    //  Bang co khung: +----+----+ / | o | o |
    //   - Do rong cot = max(tieu de, noi dung dai nhat) -> khong cat chu khi du cho.
    //   - Bang rong hon cho phep: chi thu hep cot CHU (can trai, rong nhat truoc); cot SO (can phai: tien,
    //     so luong) giu nguyen de khong bao gio mat chu so / chu "d".
    //   - O bi thu hep: noi dung XUONG DONG BEN TRONG O (dong bang cao hon), khung van thang hang.
    // ------------------------------------------------------------------
    Bang::Bang(const vector<Cot> &c, bool giuaTatCaO) : cot(c), canGiuaTatCaO(giuaTatCaO) {}

    int Bang::tongRong() const
    {
        int t = 1;
        for (auto &c : cot)
            t += c.rong + 3;
        return t;
    }

    void Bang::themDong(const vector<string> &o, const vector<string> &mauO)
    {
        vector<string> d = o;
        d.resize(cot.size());
        vector<string> m = mauO;
        m.resize(cot.size());
        dong.push_back(d);
        mauDong.push_back(m);
    }

    void Bang::in(const string &khiRong, bool hienSoDong) const
    {
        const BoKhung &k = K1();
        const size_t n = cot.size();
        if (n == 0)
            return;

        // 1) Do rong cot = max(tieu de, noi dung) — uu tien 1 dong, khong xuong dong
        vector<int> w(n);
        for (size_t i = 0; i < n; ++i)
        {
            int mx = max(cot[i].rong, doRong(cot[i].ten));
            for (auto &d : dong)
                mx = max(mx, doRong(d[i]));
            w[i] = mx;
        }

        auto tongW = [&]() {
            int t = 1;
            for (int x : w)
                t += x + 3; // " | " + content
            return t;
        };

        // 2) Neu bang hep hon RONG_BANG_TOI_DA: gian deu cot chu de bang rong can doi
        int mucTieu = RONG_BANG_TOI_DA;
        int hienTai = tongW();
        if (hienTai < mucTieu && !dong.empty())
        {
            int thua = mucTieu - hienTai;
            // Chi gian cot khong canPhai (cot chu)
            vector<size_t> chu;
            for (size_t i = 0; i < n; ++i)
                if (!cot[i].canPhai)
                    chu.push_back(i);
            if (chu.empty())
            {
                for (size_t i = 0; i < n; ++i)
                    chu.push_back(i);
            }
            for (size_t k = 0; thua > 0 && !chu.empty(); ++k)
            {
                w[chu[k % chu.size()]]++;
                --thua;
            }
        }

        // 3) Neu bang van rong qua: thu hep cot chu, cat "..." (khong xuong dong)
        while (tongW() > mucTieu)
        {
            int lon = -1;
            for (size_t i = 0; i < n; ++i)
            {
                int minW = cot[i].canPhai ? max(4, doRong(cot[i].ten)) : 4;
                if (w[i] > minW && (lon < 0 || w[i] > w[(size_t)lon]))
                    lon = (int)i;
            }
            if (lon < 0)
                break;
            --w[(size_t)lon];
        }

        const int bangRong = tongW();

        // Can giua bang theo cung truc voi cac khung khac (BoDemCanGiua + KHOI).
        // Khong pad them khoang trang noi bo: tranh lech trai/phai so voi tieu de va inKhoi.
        // Neu bang rong hon khung form, bo dem se can lai theo chieu rong bang (tam chung).
        auto inDongCanGiua = [&](const string &noiDung) {
            cout << KHOI << noiDung << "\n";
        };

        auto ngang = [&](const string &trai, const string &giua, const string &phai) {
            string s = trai;
            for (size_t i = 0; i < n; ++i)
                s += lap(k.H, w[i] + 2) + (i + 1 < n ? giua : phai);
            inDongCanGiua(MO + s + RESET);
        };

        // 1 dong duy nhat: catNgan neu dai, luon can GIUA o
        auto inHang = [&](const vector<string> &o, const vector<string> &mauO, bool laTieuDe) {
            string dongHienThi = MO + k.V + RESET;
            for (size_t i = 0; i < n; ++i)
            {
                string chu = catNgan(o[i], w[i]);
                string oChu = canGiuaChuoi(chu, w[i]); // luon can giua
                const bool khach = lienQuanKhach(cot[i].ten);
                const string m = laTieuDe
                    ? (!mauTieuDe.empty() ? mauTieuDe : (khach ? KHACH : DAM + CHU))
                    : (mauO[i].empty() ? (khach ? KHACH : CHU) : mauO[i]);
                dongHienThi += " " + m + oChu + RESET + " " + MO + k.V + RESET;
            }
            inDongCanGiua(dongHienThi);
        };

        ngang(k.TL, k.TT, k.TR);
        vector<string> tieuDeHang, mauRong(n);
        for (auto &c : cot)
            tieuDeHang.push_back(c.ten);
        inHang(tieuDeHang, mauRong, true);
        ngang(k.LT, k.X, k.RT);
        if (dong.empty())
            inDongCanGiua(MO + k.V + RESET + " " + MO + canGiuaChuoi(khiRong, max(1, bangRong - 4)) + RESET +
                          " " + MO + k.V + RESET);
        for (size_t r = 0; r < dong.size(); ++r)
            inHang(dong[r], mauDong[r], false);
        ngang(k.BL, k.BT, k.BR);
        if (hienSoDong && !dong.empty())
            inDongCanGiua(MO + canGiuaChuoi(to_string(dong.size()) + " dong", bangRong) + RESET);
    }
}
