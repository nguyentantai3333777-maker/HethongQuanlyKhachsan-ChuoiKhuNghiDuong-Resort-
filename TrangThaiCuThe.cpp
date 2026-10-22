// TrangThaiCuThe.cpp
#include "TrangThaiCuThe.h"
#include "Phong.h"
#include <iostream>
#include <memory>
#include <stdexcept>

using namespace std;

// ===== PhongTrong =====
void PhongTrong::giuCho(Phong* p)   { p->datTrangThai(make_shared<PhongGiuCho>()); }
void PhongTrong::nhanPhong(Phong* p){ p->datTrangThai(make_shared<PhongDangO>()); }
void PhongTrong::baoTri(Phong* p)   { p->datTrangThai(make_shared<PhongBaoTri>()); }
string PhongTrong::layTenTrangThai() const { return "Trong"; }

// ===== PhongGiuCho =====
PhongGiuCho::PhongGiuCho(int soGio) : hanGiuCho(time(nullptr) + (time_t)soGio * 3600) {}

bool PhongGiuCho::kiemTraHetHan() const { return time(nullptr) > hanGiuCho; }

void PhongGiuCho::nhanPhong(Phong* p)
{
    if (kiemTraHetHan())
    {
        p->datTrangThai(make_shared<PhongTrong>());
        throw runtime_error("Giu cho da het han, phong da tro ve trang thai Trong.");
    }
    p->datTrangThai(make_shared<PhongDangO>());
}
void PhongGiuCho::hoanTat(Phong* p) { p->datTrangThai(make_shared<PhongTrong>()); }
string PhongGiuCho::layTenTrangThai() const { return "Dang giu cho"; }

// ===== PhongDangO =====
void PhongDangO::traPhong(Phong* p) { p->datTrangThai(make_shared<PhongDangDon>()); }
string PhongDangO::layTenTrangThai() const { return "Dang o"; }

// ===== PhongDangDon =====
void PhongDangDon::donPhong(Phong*)
{
    cout << "Phong dang duoc don dep. Khi don xong hay chon 'Hoan tat'.\n";
}
void PhongDangDon::hoanTat(Phong* p) { p->datTrangThai(make_shared<PhongTrong>()); }
string PhongDangDon::layTenTrangThai() const { return "Dang don dep"; }

// ===== PhongBaoTri =====
PhongBaoTri::PhongBaoTri(const string& lyDo) : lyDoBaoTri(lyDo) {}
void PhongBaoTri::hoanTat(Phong* p) { p->datTrangThai(make_shared<PhongTrong>()); }
string PhongBaoTri::layTenTrangThai() const { return "Dang bao tri"; }
