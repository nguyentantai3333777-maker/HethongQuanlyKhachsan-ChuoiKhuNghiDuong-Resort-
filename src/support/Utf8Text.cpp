// Utf8Text.cpp - Do rong hien thi chuoi UTF-8 + can le bang std::setw (xem Utf8Text.h)
#include "Utf8Text.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

using namespace std;

namespace Utf8
{
    size_t docKyTu(const string &s, size_t i, uint32_t &cp)
    {
        const unsigned char c = (unsigned char)s[i];
        size_t n = c < 0x80 ? 1 : (c >> 5) == 0x6 ? 2 : (c >> 4) == 0xE ? 3 : (c >> 3) == 0x1E ? 4 : 0;
        if (n == 0 || i + n > s.size())
        {
            cp = 0xFFFD; // byte loi: coi la 1 ky tu
            return 1;
        }
        if (n == 1)
        {
            cp = c;
            return 1;
        }
        cp = c & (0xFF >> (n + 1));
        for (size_t k = 1; k < n; ++k)
        {
            unsigned char t = (unsigned char)s[i + k];
            if ((t & 0xC0) != 0x80)
            {
                cp = 0xFFFD;
                return 1;
            }
            cp = (cp << 6) | (t & 0x3F);
        }
        return n;
    }

    size_t doDaiMaAnsi(const string &s, size_t i)
    {
        if (i + 1 >= s.size() || s[i] != '\x1b' || s[i + 1] != '[')
            return 0;
        size_t j = i + 2;
        while (j < s.size() && !(s[j] >= '@' && s[j] <= '~')) // tham so, ket thuc bang 1 ky tu 0x40-0x7E
            ++j;
        return j < s.size() ? j - i + 1 : s.size() - i;
    }

    int doRongKyTu(uint32_t cp)
    {
        if (cp == 0x02 || cp == 0x03) // ky tu danh dau noi bo cua ConsoleUI
            return 0;
        if (cp < 0x20 || (cp >= 0x7F && cp < 0xA0))
            return 0;
        // Dau to hop (tieng Viet dang NFD: "e" + dau mu + dau sac) va ky tu khong rong
        if ((cp >= 0x0300 && cp <= 0x036F) || (cp >= 0x1AB0 && cp <= 0x1AFF) || (cp >= 0x1DC0 && cp <= 0x1DFF) ||
            (cp >= 0x20D0 && cp <= 0x20FF) || (cp >= 0xFE20 && cp <= 0xFE2F) || (cp >= 0x200B && cp <= 0x200F) ||
            cp == 0xFEFF)
            return 0;
        // Ky tu rong 2 cot (Dong A, emoji)
        if ((cp >= 0x1100 && cp <= 0x115F) || (cp >= 0x2E80 && cp <= 0xA4CF) || (cp >= 0xAC00 && cp <= 0xD7A3) ||
            (cp >= 0xF900 && cp <= 0xFAFF) || (cp >= 0xFE30 && cp <= 0xFE4F) || (cp >= 0xFF00 && cp <= 0xFF60) ||
            (cp >= 0xFFE0 && cp <= 0xFFE6) || (cp >= 0x1F300 && cp <= 0x1FAFF) || (cp >= 0x20000 && cp <= 0x3FFFD))
            return 2;
        return 1;
    }

    int doRong(const string &s)
    {
        int w = 0;
        for (size_t i = 0; i < s.size();)
        {
            if (size_t a = doDaiMaAnsi(s, i))
            {
                i += a;
                continue;
            }
            uint32_t cp;
            i += docKyTu(s, i, cp);
            w += doRongKyTu(cp);
        }
        return w;
    }

    string catVua(const string &s, int w, const string &dauCat)
    {
        if (w <= 0)
            return "";
        if (doRong(s) <= w)
            return s;
        const int giu = max(0, w - doRong(dauCat));
        string kq;
        int dem = 0;
        bool coMau = false;
        for (size_t i = 0; i < s.size();)
        {
            if (size_t a = doDaiMaAnsi(s, i))
            {
                coMau = true;
                kq += s.substr(i, a);
                i += a;
                continue;
            }
            uint32_t cp;
            size_t n = docKyTu(s, i, cp);
            int r = doRongKyTu(cp);
            if (dem + r > giu)
                break;
            kq += s.substr(i, n);
            dem += r;
            i += n;
        }
        kq += dauCat.substr(0, (size_t)min<int>((int)dauCat.size(), w));
        if (coMau)
            kq += "\x1b[0m";
        return kq;
    }

    // setw dem theo byte -> bu them (soByte - doRong) de dung w COT hien thi
    static string datBangSetw(const string &s, int w, bool trai)
    {
        string t = catVua(s, w);
        ostringstream os;
        int bu = (int)t.size() - doRong(t);
        os << (trai ? left : right) << setw(w + bu) << t;
        return os.str();
    }

    string canTrai(const string &s, int w) { return datBangSetw(s, w, true); }
    string canPhai(const string &s, int w) { return datBangSetw(s, w, false); }

    string canGiua(const string &s, int w)
    {
        string t = catVua(s, w);
        int thieu = max(0, w - doRong(t));
        return canTrai(string((size_t)(thieu / 2), ' ') + t, w);
    }

    vector<string> ngatDong(const string &s, int w)
    {
        vector<string> kq;
        if (w < 1)
            w = 1;
        size_t bd = 0;
        while (true)
        {
            size_t kt = s.find('\n', bd);
            string doan = s.substr(bd, kt == string::npos ? string::npos : kt - bd);
            string hienTai, tu;
            auto day = [&]()
            {
                kq.push_back(hienTai);
                hienTai.clear();
            };
            auto themTu = [&]()
            {
                while (doRong(tu) > w) // tu dai hon ca dong -> cat cung theo do rong
                {
                    if (!hienTai.empty())
                        day();
                    string dau;
                    int dem = 0;
                    size_t i = 0;
                    while (i < tu.size())
                    {
                        uint32_t cp;
                        size_t n = docKyTu(tu, i, cp);
                        int r = doRongKyTu(cp);
                        if (dem + r > w && dem > 0)
                            break;
                        dau += tu.substr(i, n);
                        dem += r;
                        i += n;
                    }
                    kq.push_back(dau);
                    tu = tu.substr(i);
                }
                if (tu.empty())
                    return;
                if (!hienTai.empty() && doRong(hienTai) + 1 + doRong(tu) > w)
                    day();
                hienTai += (hienTai.empty() ? "" : " ") + tu;
                tu.clear();
            };
            for (char ch : doan)
            {
                if (ch == ' ' || ch == '\t')
                    themTu();
                else
                    tu += ch;
            }
            themTu();
            kq.push_back(hienTai);
            if (kt == string::npos)
                break;
            bd = kt + 1;
        }
        return kq;
    }
}
