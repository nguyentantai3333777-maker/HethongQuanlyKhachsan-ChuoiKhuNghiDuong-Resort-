// TienIch.cpp
#include "TienIch.h"
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <sstream>

using namespace std;

namespace
{
    // Thuat toan Howard Hinnant: doi ngay duong lich <-> so ngay tu 1/1/1970
    long long ngayTuLich(int y, int m, int d)
    {
        y -= (m <= 2) ? 1 : 0;
        const long long era = (y >= 0 ? y : y - 399) / 400;
        const long long yoe = y - era * 400;
        const long long doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
        const long long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
        return era * 146097 + doe - 719468;
    }

    void lichTuNgay(long long z, int &y, int &m, int &d)
    {
        z += 719468;
        const long long era = (z >= 0 ? z : z - 146096) / 146097;
        const long long doe = z - era * 146097;
        const long long yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
        const long long yy = yoe + era * 400;
        const long long doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        const long long mp = (5 * doy + 2) / 153;
        d = (int)(doy - (153 * mp + 2) / 5 + 1);
        m = (int)(mp < 10 ? mp + 3 : mp - 9);
        y = (int)(yy + (m <= 2 ? 1 : 0));
    }
}

Ngay::Ngay(int d, int m, int y) : ngay(d), thang(m), nam(y)
{
    if (!hopLe(d, m, y))
    {
        ngay = 1;
        thang = 1;
        nam = 2000;
    }
}

int Ngay::soNgayTrongThang(int m, int y)
{
    static const int ds[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m < 1 || m > 12)
        return 0;
    if (m == 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0))
        return 29;
    return ds[m - 1];
}

bool Ngay::hopLe(int d, int m, int y)
{
    return y >= 1900 && y <= 2100 && m >= 1 && m <= 12 && d >= 1 && d <= soNgayTrongThang(m, y);
}

tm layTm(time_t t)
{
    tm kq{};
    tm *p = localtime(&t);
    if (p)
        kq = *p;
    return kq;
}

Ngay Ngay::tuTimeT(time_t t)
{
    tm x = layTm(t);
    return Ngay(x.tm_mday, x.tm_mon + 1, x.tm_year + 1900);
}

Ngay Ngay::homNay() { return tuTimeT(time(nullptr)); }

bool Ngay::docChuoi(const string &s, Ngay &kq)
{
    int d = 0, m = 0, y = 0;
    char c1 = 0, c2 = 0, du = 0;
    // Chap nhan dd/mm/yyyy hoac dd-mm-yyyy
    if (sscanf(s.c_str(), "%d%c%d%c%d%c", &d, &c1, &m, &c2, &y, &du) != 5)
        return false;
    if ((c1 != '/' && c1 != '-') || c1 != c2)
        return false;
    if (!hopLe(d, m, y))
        return false;
    kq = Ngay(d, m, y);
    return true;
}

long long Ngay::soNgayTuyetDoi() const { return ngayTuLich(nam, thang, ngay); }

Ngay Ngay::congNgay(int n) const
{
    int y, m, d;
    lichTuNgay(soNgayTuyetDoi() + n, y, m, d);
    return Ngay(d, m, y);
}

int Ngay::khoangCachDen(const Ngay &khac) const
{
    return (int)(khac.soNgayTuyetDoi() - soNgayTuyetDoi());
}

time_t Ngay::toTimeT(int gio, int phut, int giay) const
{
    tm x{};
    x.tm_mday = ngay;
    x.tm_mon = thang - 1;
    x.tm_year = nam - 1900;
    x.tm_hour = gio;
    x.tm_min = phut;
    x.tm_sec = giay;
    x.tm_isdst = -1;
    return mktime(&x);
}

string Ngay::toString() const
{
    char buf[48];
    snprintf(buf, sizeof(buf), "%02d/%02d/%04d", ngay, thang, nam);
    return buf;
}

string dinhDangThoiGian(time_t t)
{
    if (t == 0)
        return "--";
    tm x = layTm(t);
    char buf[64];
    snprintf(buf, sizeof(buf), "%02d/%02d/%04d %02d:%02d", x.tm_mday, x.tm_mon + 1,
             x.tm_year + 1900, x.tm_hour, x.tm_min);
    return buf;
}

string dinhDangThoiGianNgan(time_t t)
{
    if (t == 0)
        return "--";
    tm x = layTm(t);
    char buf[64];
    snprintf(buf, sizeof(buf), "%02d/%02d/%02d %02d:%02d", x.tm_mday, x.tm_mon + 1,
             (x.tm_year + 1900) % 100, x.tm_hour, x.tm_min);
    return buf;
}

string dinhDangTien(double soTien)
{
    long long v = llround(soTien);
    bool am = v < 0;
    string s = to_string(am ? -v : v);
    for (int i = (int)s.size() - 3; i > 0; i -= 3)
        s.insert((size_t)i, ",");
    return am ? "-" + s : s;
}

string dinhDangSoThuc(double v, int chuSoThapPhan)
{
    ostringstream os;
    os << fixed << setprecision(chuSoThapPhan) << v;
    return os.str();
}

MuaDuLich xacDinhMua(const Ngay &n)
{
    int d = n.layNgay(), m = n.layThang();
    // Le/Tet (gan dung): cuoi T1 - giua T2 (Tet Am), 30/4-1/5, 2/9, Giang sinh - Tet Duong
    // (Khong gan ca thang 1-2 de tranh gia le keo dai 2 thang)
    if ((m == 1 && d >= 20) || (m == 2 && d <= 15) || (m == 4 && d >= 28) || (m == 5 && d <= 3) ||
        (m == 9 && d <= 3) || (m == 12 && d >= 20) || (m == 1 && d <= 2))
        return MuaDuLich::LE_TET;
    if (m >= 6 && m <= 8)
        return MuaDuLich::CAO_DIEM;
    if (m >= 9 && m <= 11)
        return MuaDuLich::THAP_DIEM;
    return MuaDuLich::BINH_THUONG;
}

std::string canhBaoNhieuMua(const Ngay &nhan, int soDem)
{
    if (soDem <= 1)
        return "";
    MuaDuLich dau = xacDinhMua(nhan);
    bool khac = false;
    int demMua[5] = {0, 0, 0, 0, 0};
    for (int i = 0; i < soDem; ++i)
    {
        MuaDuLich m = xacDinhMua(nhan.congNgay(i));
        ++demMua[(int)m];
        if (m != dau)
            khac = true;
    }
    if (!khac)
        return "";
    string kq = "Ky nghi di qua nhieu mua (";
    bool dauTien = true;
    for (int m = 1; m <= 4; ++m)
        if (demMua[m] > 0)
        {
            if (!dauTien)
                kq += ", ";
            kq += muaToString((MuaDuLich)m) + " " + to_string(demMua[m]) + " dem";
            dauTien = false;
        }
    kq += "). Gia phong ap he so mua cua ngay nhan phong (" + muaToString(dau) + " x" +
          dinhDangSoThuc(heSoMua(dau), 1) + ") cho toan bo ky o.";
    return kq;
}

string dinhDangTienVN(double soTien) { return dinhDangTien(soTien) + "d"; }
