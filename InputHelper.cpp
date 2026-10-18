// InputHelper.cpp
#include "InputHelper.h"
#include <iostream>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>

using namespace std;

namespace InputHelper
{
    string trim(const string& s)
    {
        size_t dau = s.find_first_not_of(" \t\r\n");
        if (dau == string::npos) return "";
        size_t cuoi = s.find_last_not_of(" \t\r\n");
        return s.substr(dau, cuoi - dau + 1);
    }

    string chuHoa(string s)
    {
        for (size_t i = 0; i < s.size(); ++i)
            s[i] = (char)toupper((unsigned char)s[i]);
        return s;
    }

    // Luon doc nguyen 1 dong -> khong bi loi bo dem (khong tron cin >> va getline)
    static string docDong()
    {
        string s;
        if (!getline(cin, s)) throw HetDuLieuNhap();
        return s;
    }

    string nhapDong(const string& nhan)
    {
        cout << nhan;
        return trim(docDong());
    }

    string nhapChuoi(const string& nhan)
    {
        while (true)
        {
            string s = nhapDong(nhan);
            if (!s.empty()) return s;
            cout << "Khong duoc de trong hoac chi toan khoang trang, vui long nhap lai.\n";
        }
    }

    // Chi chap nhan: [+/-] cac chu so, khong du ky tu nao khac
    static bool docSoNguyen(const string& s, long long& kq)
    {
        if (s.empty()) return false;
        size_t i = (s[0] == '-' || s[0] == '+') ? 1 : 0;
        if (i == s.size()) return false;
        for (size_t j = i; j < s.size(); ++j)
            if (!isdigit((unsigned char)s[j])) return false;
        errno = 0;
        char* end = nullptr;
        kq = strtoll(s.c_str(), &end, 10);
        return errno != ERANGE && *end == '\0';   // ERANGE = so qua lon
    }

    // Chi chap nhan: [+/-] chu so, toi da 1 dau cham
    static bool docSoThuc(const string& s, double& kq)
    {
        if (s.empty()) return false;
        size_t i = (s[0] == '-' || s[0] == '+') ? 1 : 0;
        int soCham = 0, soChuSo = 0;
        for (size_t j = i; j < s.size(); ++j)
        {
            if (s[j] == '.') ++soCham;
            else if (isdigit((unsigned char)s[j])) ++soChuSo;
            else return false;
        }
        if (soCham > 1 || soChuSo == 0) return false;
        errno = 0;
        char* end = nullptr;
        kq = strtod(s.c_str(), &end);
        return errno != ERANGE && *end == '\0' && isfinite(kq);
    }

    int nhapSoNguyen(const string& nhan, int nhoNhat, int lonNhat)
    {
        while (true)
        {
            string s = nhapDong(nhan);
            long long v = 0;
            if (!docSoNguyen(s, v))
            {
                cout << "Gia tri khong hop le (phai la so nguyen), vui long nhap lai.\n";
                continue;
            }
            if (v < nhoNhat || v > lonNhat)
            {
                cout << "Gia tri phai nam trong khoang tu " << nhoNhat << " den " << lonNhat
                     << ", vui long nhap lai.\n";
                continue;
            }
            return (int)v;
        }
    }

    double nhapSoThucDuong(const string& nhan, double lonNhat)
    {
        while (true)
        {
            string s = nhapDong(nhan);
            double v = 0;
            if (!docSoThuc(s, v))
            {
                cout << "Gia tri khong hop le (phai la so), vui long nhap lai.\n";
                continue;
            }
            if (v <= 0)
            {
                cout << "Gia tri phai lon hon 0, vui long nhap lai.\n";
                continue;
            }
            if (v > lonNhat)
            {
                cout << "Gia tri qua lon (toi da " << (long long)lonNhat << "), vui long nhap lai.\n";
                continue;
            }
            return v;
        }
    }

    // Dung chung cho menu va chon enum
    static int nhapLuaChonVoiNhan(const string& nhan, int nhoNhat, int lonNhat)
    {
        while (true)
        {
            string s = nhapDong(nhan);
            long long v = 0;
            if (docSoNguyen(s, v) && v >= nhoNhat && v <= lonNhat) return (int)v;
            cout << "Lua chon khong hop le, vui long nhap lai.\n";
        }
    }

    int nhapLuaChon(int nhoNhat, int lonNhat)
    {
        return nhapLuaChonVoiNhan("Nhap lua chon: ", nhoNhat, lonNhat);
    }

    LoaiPhong nhapLoaiPhong()
    {
        cout << "Loai phong: 1.Standard  2.Deluxe  3.Suite  4.Villa\n";
        int c = nhapLuaChonVoiNhan("Chon loai phong (1-4): ", 1, 4);
        switch (c)
        {
        case 1: return LoaiPhong::STANDARD;
        case 2: return LoaiPhong::DELUXE;
        case 3: return LoaiPhong::SUITE;
        default: return LoaiPhong::VILLA;
        }
    }

    MuaDuLich nhapMuaDuLich()
    {
        cout << "Mua du lich:\n"                       // khop he so mua trong Phong::tinhHeSoMua
             << "  1. Thap diem   - giam 20% tien phong\n"
             << "  2. Binh thuong - gia goc\n"
             << "  3. Cao diem    - tang 30% tien phong\n"
             << "  4. Le/Tet      - tang 60% tien phong\n";
        int c = nhapLuaChonVoiNhan("Chon mua (1-4): ", 1, 4);
        switch (c)
        {
        case 1: return MuaDuLich::THAP_DIEM;
        case 2: return MuaDuLich::BINH_THUONG;
        case 3: return MuaDuLich::CAO_DIEM;
        default: return MuaDuLich::LE_TET;
        }
    }

    bool nhapCoKhong(const string& nhan)
    {
        while (true)
        {
            string s = chuHoa(nhapDong(nhan + " (c/k): "));
            if (s == "C" || s == "Y") return true;
            if (s == "K" || s == "N") return false;
            cout << "Chi nhap 'c' (co) hoac 'k' (khong).\n";
        }
    }

    bool laSoDuong(const string& s)
    {
        if (s.empty() || s.size() > 9) return false;
        for (size_t i = 0; i < s.size(); ++i)
            if (!isdigit((unsigned char)s[i])) return false;
        return s.find_first_not_of('0') != string::npos;
    }

    string chuanHoaSoDuong(const string& s)
    {
        size_t d = s.find_first_not_of('0');
        return d == string::npos ? "0" : s.substr(d);
    }

    bool laChuThuan(const string& s)
    {
        string t = trim(s);
        bool coChu = false;
        for (size_t i = 0; i < t.size(); ++i)
        {
            unsigned char ch = (unsigned char)t[i];
            if (ch >= 128 || isalpha(ch)) coChu = true;   // >=128: chu co dau (UTF-8)
            else if (ch != ' ') return false;
        }
        return coChu;
    }

    string nhapSoDuongChuoi(const string& nhan)
    {
        while (true)
        {
            string s = nhapDong(nhan);
            if (laSoDuong(s)) return chuanHoaSoDuong(s);
            if (s.empty()) cout << "Khong duoc de trong, vui long nhap lai.\n";
            else if (s[0] == '-') cout << "Khong duoc nhap so am, chi nhap so duong.\n";
            else if (s.find_first_not_of("0123456789") != string::npos)
                cout << "Chi duoc nhap so duong (khong nhap chu hay ky tu dac biet).\n";
            else if (s.size() > 9) cout << "So qua dai (toi da 9 chu so).\n";
            else cout << "Phai la so duong lon hon 0.\n";
        }
    }

    string nhapChuoiChu(const string& nhan)
    {
        while (true)
        {
            string s = nhapDong(nhan);
            if (laChuThuan(s)) return s;
            cout << "Chi duoc nhap chu (khong nhap so hay ky tu dac biet), vui long nhap lai.\n";
        }
    }
}
