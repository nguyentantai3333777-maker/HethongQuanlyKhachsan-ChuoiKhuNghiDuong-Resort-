// TrangThaiCuThe.cpp
#include "TrangThaiCuThe.h"
#include "Phong.h"
#include "TienIch.h"
#include <memory>
#include <stdexcept>

using namespace std;

// ===== PhongTrong =====
void PhongTrong::giuCho(Phong *p, time_t han)
{
    if (han <= time(nullptr))
        throw invalid_argument("Han giu cho phai o tuong lai.");
    p->datTrangThai(make_shared<PhongGiuCho>(han));
}
void PhongTrong::nhanPhong(Phong *p) { p->datTrangThai(make_shared<PhongDangO>()); }
void PhongTrong::baoTri(Phong *p, const string &lyDo) { p->datTrangThai(make_shared<PhongBaoTri>(lyDo)); }

// ===== PhongGiuCho =====
PhongGiuCho::PhongGiuCho(time_t han) : hanGiuCho(han) {}

bool PhongGiuCho::daHetHan() const { return time(nullptr) > hanGiuCho; }

void PhongGiuCho::nhanPhong(Phong *p)
{
    if (daHetHan())
    {
        p->datTrangThai(make_shared<PhongTrong>());
        throw runtime_error("Giu cho da het han, phong da tro ve trang thai Trong.");
    }
    p->datTrangThai(make_shared<PhongDangO>());
}
void PhongGiuCho::huyGiuCho(Phong *p) { p->datTrangThai(make_shared<PhongTrong>()); }
string PhongGiuCho::ghiChu() const { return "Giu den " + dinhDangThoiGian(hanGiuCho); }

// ===== PhongDangO =====
void PhongDangO::traPhong(Phong *p) { p->datTrangThai(make_shared<PhongDangDon>()); }

// ===== PhongDangDon =====
void PhongDangDon::donPhong(Phong *p) { p->datTrangThai(make_shared<PhongTrong>()); }
void PhongDangDon::baoTri(Phong *p, const string &lyDo) { p->datTrangThai(make_shared<PhongBaoTri>(lyDo)); }

// ===== PhongBaoTri =====
PhongBaoTri::PhongBaoTri(const string &lyDo) : lyDoBaoTri(lyDo.empty() ? "Bao tri dinh ky" : lyDo) {}
void PhongBaoTri::hoanTat(Phong *p) { p->datTrangThai(make_shared<PhongTrong>()); }
