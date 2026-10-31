// TaiKhoan.cpp
#include "TaiKhoan.h"
#include "InputHelper.h"
#include "Nguoi.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdexcept>

using namespace std;

static string maHoaCsv(const string &value)
{
    string clean = value;
    for (char &c : clean)
        if (c == '\r' || c == '\n')
            c = ' ';
    if (clean.find_first_of(",\"") == string::npos)
        return clean;
    string encoded = "\"";
    for (char c : clean)
    {
        if (c == '"')
            encoded += "\"\"";
        else
            encoded += c;
    }
    return encoded + "\"";
}

static bool tachCsv(const string &line, vector<string> &fields)
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
        else if (c == ',')
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

bool laChucVuQuanLy(const string &chucVu)
{
    return chucVu.find("Quan ly") != string::npos || chucVu.find("Quản lý") != string::npos ||
            chucVu.find("QUAN LY") != string::npos || chucVu.find("QUẢN LÝ") != string::npos ||
            chucVu.find("quản lý") != string::npos;
}

bool laChucVuLeTan(const string &chucVu)
{
    return chucVu.find("Le tan") != string::npos || chucVu.find("Lễ tân") != string::npos ||
            chucVu.find("LE TAN") != string::npos || chucVu.find("LỄ TÂN") != string::npos ||
            chucVu.find("lễ tân") != string::npos;
}

bool chucVuKhopVaiTro(const string &chucVu, Role role)
{
    return role == Role::ADMIN ? laChucVuQuanLy(chucVu) : laChucVuLeTan(chucVu);
}

void QuanLyTaiKhoan::damBaoTaiKhoanNhanSu(const vector<shared_ptr<NhanVien>> &nhanVien)
{
    danhSach.erase(remove_if(danhSach.begin(), danhSach.end(), [&](const TaiKhoan &tk)
                             {
        const auto nv = find_if(nhanVien.begin(), nhanVien.end(), [&](const shared_ptr<NhanVien> &x)
                                { return x && x->layMa() == tk.layMaNhanVien(); });
        return nv == nhanVien.end() || !(*nv)->layDangLam() ||
               !chucVuKhopVaiTro((*nv)->layChucVu(), tk.layRole()); }),
                   danhSach.end());

    for (const auto &nv : nhanVien)
    {
        if (!nv || !nv->layDangLam() || timTheoNhanVien(nv->layMa()))
            continue;
        Role role;
        string tienTo;
        if (laChucVuQuanLy(nv->layChucVu()))
        {
            role = Role::ADMIN;
            tienTo = "ql";
        }
        else if (laChucVuLeTan(nv->layChucVu()))
        {
            role = Role::USER;
            tienTo = "lt";
        }
        else
            continue;

        string user = tienTo + nv->layMa();
        const string base = user;
        for (int suffix = 2; timTheoUsername(user); ++suffix)
            user = base + to_string(suffix);
        themTaiKhoan(user, "welcome123", role, nv->layMa());
    }
}

TaiKhoan::TaiKhoan(const string &user, const string &pass, Role r, const string &maNV)
    : username(user), password(pass), role(r), maNhanVien(maNV) {}

void QuanLyTaiKhoan::kiemTraUsername(const string &user)
{
    if (user.size() < 3 || user.size() > 20)
        throw invalid_argument("Username phai dai 3-20 ky tu.");
    if (!InputHelper::laUsername(user))
        throw invalid_argument("Username chi gom chu, so, dau '_' hoac '.'.");
}

void QuanLyTaiKhoan::kiemTraMatKhau(const string &pass)
{
    if (pass.size() < 6 || pass.size() > 32)
        throw invalid_argument("Mat khau phai dai 6-32 ky tu.");
    for (char c : pass)
        if (isspace((unsigned char)c))
            throw invalid_argument("Mat khau khong duoc chua khoang trang.");
}

void QuanLyTaiKhoan::themTaiKhoan(const string &user, const string &pass, Role role, const string &maNV)
{
    kiemTraUsername(user);
    kiemTraMatKhau(pass);
    if (timTheoUsername(user))
        throw invalid_argument("Username '" + user + "' da ton tai.");
    if (timTheoNhanVien(maNV))
        throw invalid_argument("Nhan vien " + maNV + " da co tai khoan.");
    danhSach.push_back(TaiKhoan(user, pass, role, maNV));
}

void QuanLyTaiKhoan::xoaTaiKhoan(const string &user)
{
    for (size_t i = 0; i < danhSach.size(); ++i)
    {
        if (danhSach[i].layUsername() == user)
        {
            if (danhSach[i].layRole() == Role::ADMIN && demAdmin() <= 1)
                throw runtime_error("Khong the xoa ADMIN cuoi cung.");
            danhSach.erase(danhSach.begin() + (long)i);
            return;
        }
    }
    throw runtime_error("Khong tim thay tai khoan '" + user + "'.");
}

void QuanLyTaiKhoan::doiMatKhau(const string &user, const string &passCu, const string &passMoi)
{
    for (auto &tk : danhSach)
        if (tk.layUsername() == user)
        {
            if (!tk.kiemTraMatKhau(passCu))
                throw runtime_error("Mat khau cu khong dung.");
            kiemTraMatKhau(passMoi);
            if (passCu == passMoi)
                throw invalid_argument("Mat khau moi phai khac mat khau cu.");
            tk.datMatKhau(passMoi);
            return;
        }
    throw runtime_error("Khong tim thay tai khoan.");
}

const TaiKhoan &QuanLyTaiKhoan::dangNhap(const string &user, const string &pass) const
{
    for (auto &tk : danhSach)
        if (tk.layUsername() == user && tk.kiemTraMatKhau(pass))
            return tk;
    throw runtime_error("Sai ten dang nhap hoac mat khau.");
}

const TaiKhoan *QuanLyTaiKhoan::timTheoUsername(const string &user) const
{
    for (auto &tk : danhSach)
        if (tk.layUsername() == user)
            return &tk;
    return nullptr;
}

const TaiKhoan *QuanLyTaiKhoan::timTheoNhanVien(const string &maNV) const
{
    for (auto &tk : danhSach)
        if (tk.layMaNhanVien() == maNV)
            return &tk;
    return nullptr;
}

int QuanLyTaiKhoan::demAdmin() const
{
    int dem = 0;
    for (auto &tk : danhSach)
        if (tk.layRole() == Role::ADMIN)
            ++dem;
    return dem;
}

// File taikhoan.txt: username|password|role(0=ADMIN,1=USER)|maNhanVien
void QuanLyTaiKhoan::ghiFile(const string &duongDan) const
{
    ofstream f(duongDan, ios::binary | ios::trunc);
    if (!f)
        throw runtime_error("Khong ghi duoc file " + duongDan);
    static const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
    f.write(reinterpret_cast<const char *>(bom), 3);
    f << "username,password,role,maNhanVien\r\n";
    for (auto &tk : danhSach)
        f << maHoaCsv(tk.layUsername()) << ',' << maHoaCsv(tk.matKhauLuuFile()) << ','
          << (tk.layRole() == Role::ADMIN ? 0 : 1) << ',' << maHoaCsv(tk.layMaNhanVien()) << "\r\n";
}

bool QuanLyTaiKhoan::docFile(const string &duongDan)
{
    ifstream f(duongDan);
    if (!f)
        return false;
    danhSach.clear();
    string dong;
    const bool csv = duongDan.size() >= 4 && duongDan.substr(duongDan.size() - 4) == ".csv";
    bool daXuLyHeaderCSV = false;
    bool daBoBOM = false;
    while (getline(f, dong))
    {
        if (!dong.empty() && dong.back() == '\r')
            dong.pop_back();
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
        vector<string> fields;
        if (csv)
        {
            if (!daXuLyHeaderCSV)
            {
                daXuLyHeaderCSV = true;
                continue;
            }
            if (!tachCsv(dong, fields))
                continue;
        }
        else
        {
            size_t a = dong.find('|'), b = dong.find('|', a + 1), c = dong.find('|', b + 1);
            if (a == string::npos || b == string::npos || c == string::npos)
                continue;
            fields = {dong.substr(0, a), dong.substr(a + 1, b - a - 1),
                      dong.substr(b + 1, c - b - 1), dong.substr(c + 1)};
        }
        if (fields.size() < 4)
            continue;
        danhSach.push_back(TaiKhoan(fields[0], fields[1], fields[2] == "0" ? Role::ADMIN : Role::USER, fields[3]));
    }
    return !danhSach.empty();
}
