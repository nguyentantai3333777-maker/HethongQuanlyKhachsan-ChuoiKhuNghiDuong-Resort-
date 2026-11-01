# OceanPearl Resort — Nhóm lớp Thực thể & Nghiệp vụ (C++)

## 1. Giới thiệu

Đây là phần **lõi dữ liệu và nghiệp vụ** được tách ra từ dự án **OceanPearl Resort — Hệ thống Quản lý Khách sạn / Chuỗi Khu nghỉ dưỡng** viết bằng C++17, phục vụ học tập và thực hành Lập trình hướng đối tượng (OOP) cùng các Design Pattern.

Gói này **chỉ gồm** nhóm lớp thực thể và nhóm lớp nghiệp vụ. Giao diện Console, Web API/HTTP server và frontend của dự án gốc **không** nằm trong gói; có thể tích hợp lại hoặc dùng độc lập như một thư viện nghiệp vụ.

Hệ thống mô phỏng vận hành của chuỗi resort gồm 2 chi nhánh (**CN01 – Nha Trang**, **CN02 – Quy Nhơn**): quản lý chi nhánh, phòng, nhân sự, khách hàng, đặt phòng, nhận/trả phòng, dịch vụ đi kèm, hóa đơn thanh toán và thống kê doanh thu.

## 2. Mục tiêu

- Vận dụng OOP: kế thừa, đa hình, đóng gói, trừu tượng hóa, `shared_ptr` / `weak_ptr`.
- Áp dụng **State Pattern** cho vòng đời trạng thái phòng.
- Áp dụng **Decorator Pattern** để cộng dồn chi phí dịch vụ vào giá phòng.
- Tách rõ lớp thực thể (dữ liệu + quy tắc của chính nó) và lớp nghiệp vụ (điều phối nhiều thực thể).
- Lưu trữ dữ liệu bằng tệp CSV (UTF-8 BOM), có kiểm tra lỗi từng dòng khi đọc.

## 3. Cấu trúc thư mục

```text
OceanPearl-Entity-Business/
├── README.md
├── build.sh / build.bat          # Biên dịch + chạy kiểm thử
├── data/                         # Dữ liệu CSV mẫu (9 bảng + tài khoản)
├── tests/
│   └── test_nghiep_vu.cpp        # Kiểm thử nhanh nhóm thực thể + nghiệp vụ
└── src/
    ├── entity/                   # NHÓM LỚP THỰC THỂ
    │   ├── Nguoi.h/.cpp
    │   ├── Phong.h/.cpp
    │   ├── PhongCuThe.h/.cpp
    │   ├── LoaiPhong.h
    │   ├── MuaDuLich.h
    │   ├── TrangThaiPhong.h
    │   ├── TrangThaiCuThe.h/.cpp
    │   ├── PhongDecorator.h
    │   ├── DichVu.h/.cpp
    │   ├── PhieuDatPhong.h/.cpp
    │   ├── HoaDon.h/.cpp
    │   ├── ChiNhanhResort.h/.cpp
    │   └── TaiKhoan.h/.cpp       # TaiKhoan (thực thể) + QuanLyTaiKhoan (nghiệp vụ)
    ├── business/                 # NHÓM LỚP NGHIỆP VỤ
    │   ├── QuanLyKhachSan.h/.cpp
    │   └── LuuTruFile.cpp        # QuanLyKhachSan::ghiFile / docFile (CSV)
    └── support/                  # PHỤ THUỘC BẮT BUỘC (không thuộc 2 nhóm trên)
        ├── TienIch.h/.cpp        # Lớp Ngay, định dạng tiền / thời gian, xác định mùa
        ├── InputHelper.h/.cpp    # Hàm kiểm tra định dạng (SĐT, CCCD, email, mã...)
        ├── ConsoleUI.h/.cpp      # InputHelper gọi UI::ghiTrucTiep khi nhập liệu
        └── Utf8Text.h/.cpp       # ConsoleUI dùng để căn lề chuỗi UTF-8
```

> Thư mục `support/` được giữ lại vì các lớp thực thể dùng `InputHelper` để kiểm tra dữ liệu đầu vào, và `InputHelper` lại gọi `ConsoleUI`. Nếu muốn thư viện thuần (không phụ thuộc console), có thể tách phần kiểm tra định dạng ra khỏi `InputHelper` và bỏ `ConsoleUI`/`Utf8Text`.

## 4. Nhóm lớp Thực thể (`src/entity/`)

Mỗi lớp thực thể tự bảo vệ tính hợp lệ của dữ liệu (setter có kiểm tra, ném `std::invalid_argument` khi sai).

### 4.1. Người: `Nguoi`, `KhachHang`, `NhanVien`

- `Nguoi`: lớp trừu tượng (mã, họ tên, SĐT, email, giới tính), hàm ảo thuần `layVaiTro()`, `layThongTin()` — **đa hình**.
- `KhachHang`: CCCD/hộ chiếu, quốc tịch, địa chỉ, chi nhánh, điểm tích lũy, hạng thành viên, lịch sử đặt phòng (`weak_ptr<PhieuDatPhong>` để tránh vòng tham chiếu).
  - Hạng thành viên (`HangThanhVien`): Thành viên 0% (0 điểm), Bạc 5% (100), Vàng 10% (400), Kim cương 15% (1000). Mỗi **100.000 VND = 1 điểm**; tự nâng hạng, không tự hạ hạng.
- `NhanVien`: chức vụ, lương cơ bản (tối thiểu **5.500.000 VND**), hệ số lương (Quản lý ×3.0, còn lại ×2.0), trạng thái đang làm / tạm nghỉ / nghỉ việc. Xóa nhân viên = đánh dấu nghỉ việc để giữ lịch sử phiếu và hóa đơn.

### 4.2. Phòng: State Pattern + Decorator Pattern

- `Phong`: lớp cơ sở, vừa là **Context** của State Pattern vừa là **Component** của Decorator Pattern. Có `tinhGiaPhong(soDem, mua)` ảo; ưu đãi lưu trú dài: từ **7 đêm** trở lên giảm **10%** tiền phòng.
- `LoaiPhong`: `STANDARD`, `DELUXE`, `SUITE`, `VILLA`.
- `PhongCuThe.h`: `StandardRoom`, `DeluxeRoom`, `Suite` (cộng phí buffet sáng mỗi đêm), `Villa` (cộng phí quản gia mỗi đêm) — override giá, mô tả, dịch vụ đi kèm.
- `MuaDuLich`: hệ số giá theo mùa — Thấp điểm ×0.8, Bình thường ×1.0, Cao điểm ×1.3, Lễ/Tết ×1.6.

**State Pattern** (`TrangThaiPhong.h`, `TrangThaiCuThe.h`): mỗi trạng thái chỉ cho phép một số chuyển đổi hợp lệ, hành động sai sẽ ném ngoại lệ.

```text
Trống --giữ chỗ--> Giữ chỗ --nhận phòng--> Đang ở --trả phòng--> Đang dọn --dọn xong--> Trống
  |                    \--hủy giữ chỗ / hết hạn--> Trống              |
  |--nhận phòng (khách vãng lai)--> Đang ở                            |--bảo trì--> Bảo trì
  \--bảo trì--> Bảo trì --hoàn tất--> Trống
```

Các lớp: `PhongTrong`, `PhongGiuCho`, `PhongDangO`, `PhongDangDon`, `PhongBaoTri`.

**Decorator Pattern** (`PhongDecorator.h`): `DecoratorAmThuc`, `DecoratorSpa`, `DecoratorTour`, `DecoratorTienIch` bọc một `Phong` và cộng chi phí dịch vụ vào `tinhGiaPhong()`. Có thể bọc lồng nhiều lớp, ví dụ `Tour(Spa(AmThuc(Suite)))`.

### 4.3. Dịch vụ: `DichVu` và các lớp con

`DichVu` (trừu tượng) → `AmThuc`, `Spa`, `TourTuyen` (đoàn từ **5 người** giảm 10%), `DichVuTienIch`. Có `clone()` để **chốt giá** tại thời điểm sử dụng, đổi giá sau này không ảnh hưởng hóa đơn cũ.

### 4.4. Đặt phòng và hóa đơn

- `PhieuDatPhong`: liên kết `KhachHang`, `Phong`, `NhanVien` (nhiều–1). Trạng thái phiếu: Đã đặt → Đã nhận phòng → Đã trả phòng / Đã hủy. Hỗ trợ đặt cọc, gia hạn, đổi phòng, check-out sớm, phát hiện quá hạn (overstay), kiểm tra trùng lịch.
- `HoaDon` + `ChiTietHoaDon` (Composition 1–nhiều): **một công thức duy nhất** cho mọi báo giá và hóa đơn:

  ```text
  giảm = tạm tính × tỷ lệ giảm hạng;  VAT = (tạm tính − giảm) × 10%;  tổng = tạm tính − giảm + VAT
  ```

  Có đối chiếu `khopVoiDecorator()` để kiểm tra tổng tiền nhất quán, xử lý trừ cọc / hoàn cọc dư.

### 4.5. Chi nhánh và tài khoản

- `ChiNhanhResort`: quản lý danh sách `Phong` và `NhanVien` của một chi nhánh.
- `TaiKhoan`: tài khoản gắn với một nhân viên, vai trò `Role::ADMIN` (quản lý, toàn quyền) hoặc `Role::USER` (lễ tân).

## 5. Nhóm lớp Nghiệp vụ (`src/business/`, `TaiKhoan`)

### 5.1. `QuanLyKhachSan` — lớp điều phối trung tâm

Quản lý chi nhánh, khách hàng, phiếu, hóa đơn, dịch vụ. **Mọi nghiệp vụ đều đi qua lớp này** để trạng thái *phòng* (State) và trạng thái *phiếu* luôn đồng bộ.

| Nhóm chức năng | Hàm tiêu biểu |
|---|---|
| Chi nhánh | `themChiNhanh`, `xoaChiNhanh` (từ chối kèm lý do nếu còn dữ liệu liên quan) |
| Phòng | `themPhong`, `locPhong`, `doiSoPhong`, `chuyenBaoTri`, `xoaPhong` |
| Lịch đặt | `timPhieuTrungLich`, `lyDoKhongDatDuoc`, `phieuQuaHan`, `phongCoTheChuyen`, `chuyenPhongPhieu` |
| Nhân viên | `themNhanVien`, `xoaNhanVien` (đánh dấu nghỉ việc) |
| Khách hàng | `themKhachHang`, `kiemTraTrungKhach`, `timKhachTheoSDT/CCCD/TuKhoa` |
| Nghiệp vụ lưu trú | `datPhong`, `checkIn`, `huyDatPhong`, `giaHan`, `ghiDichVu`, `xemTamTinh`, `checkOut`, `thanhToanHoaDon`, `dongBoTheoNgay` |
| Thống kê | `tongDoanhThu`, `congSuatPhong`, `doanhThuTheoChiNhanh/LoaiPhong/Thang/Ky`, `thongKeDichVu`, `topKhachHang`, `thongKeNhanVien`, `chiPhiLuongThang` |

Điểm đáng chú ý:

- **Chống trùng lịch**: phiếu `Đã đặt` chiếm `[ngày nhận, ngày trả)`; phiếu `Đã nhận phòng` chiếm **vô thời hạn** đến khi check-out thật.
- **Doanh thu** chỉ tính hóa đơn đã thanh toán; có thể lọc theo từng chi nhánh.
- Hỗ trợ lọc dữ liệu theo chi nhánh kể cả phòng đã xóa nhưng còn lịch sử phiếu.

### 5.2. `QuanLyTaiKhoan` (trong `TaiKhoan.h/.cpp`)

Đăng nhập, thêm/xóa tài khoản, đổi mật khẩu, kiểm tra username/mật khẩu hợp lệ, đảm bảo luôn còn ít nhất một ADMIN, tự gắn tài khoản cho nhân sự.

### 5.3. Lưu trữ tệp (`LuuTruFile.cpp`)

`QuanLyKhachSan::ghiFile(thuMuc)` và `docFile(thuMuc, soDongLoi)` đọc/ghi các bảng CSV. Khi đọc, mọi dòng lỗi được ghi vào `nhatKyDoc` (tên file, số dòng, lý do) thay vì bỏ qua âm thầm.

| File | Nội dung |
|---|---|
| `chinhanh.csv` | Chi nhánh |
| `nhanvien.csv` | Nhân viên |
| `phong.csv` | Phòng (kèm trạng thái, thuộc tính riêng từng loại) |
| `dichvu.csv` | Danh mục dịch vụ |
| `khachhang.csv` | Khách hàng |
| `phieudatphong.csv` | Phiếu đặt phòng |
| `dichvusudung.csv` | Dịch vụ đã dùng theo phiếu |
| `hoadon.csv`, `chitiethoadon.csv` | Hóa đơn và chi tiết |
| `taikhoan.csv` | Tài khoản đăng nhập |

## 6. Biên dịch và kiểm thử

Yêu cầu: `g++` hỗ trợ C++17.

```bash
./build.sh                  # Linux / macOS
./test_nghiep_vu data       # chạy kiểm thử với dữ liệu mẫu
```

Windows (MinGW): chạy `build.bat`, sau đó `test_nghiep_vu.exe data`.

Bộ kiểm thử `tests/test_nghiep_vu.cpp` kiểm tra:

- Đọc đủ 9 file CSV, không lỗi (2 chi nhánh, 52 phòng, 50 khách, 86 phiếu, 70 hóa đơn).
- Công thức tính tiền (giảm hạng + VAT).
- Hạng thành viên và ngưỡng điểm.
- State Pattern: chuyển trạng thái hợp lệ và từ chối chuyển không hợp lệ.
- Decorator Pattern: cộng đúng chi phí dịch vụ.
- Đăng nhập sai bị từ chối.

## 7. Tài khoản mẫu (trong `data/taikhoan.csv`)

| Username | Mật khẩu | Vai trò |
|---|---|---|
| `admin` | `admin123` | ADMIN (Quản lý) |
| `user` | `user123` | USER (Lễ tân) |

## 8. Ví dụ sử dụng

```cpp
#include "QuanLyKhachSan.h"

QuanLyKhachSan ks;
int soLoi = 0;
if (!ks.docFile("data", soLoi) || soLoi > 0) { /* xem ks.nhatKyDoc */ }

auto kh = ks.timKhachTheoSDT("0920000137");   // KH01 trong data/khachhang.csv
auto nv = ks.timNhanVien("NV02");
// Chọn một phòng còn bán được cho khoảng ngày này (P01 trong dữ liệu mẫu đang có khách ở)
auto ngayNhan = Ngay::homNay();   // check-in chỉ được khi đã đến ngày nhận
auto ds = ks.locPhong([&](const Phong &p) { return ks.lyDoKhongDatDuoc(p, ngayNhan, 3, 2).empty(); });
auto phieu = ks.datPhong(kh->layMa(), ds.front()->layMaPhong(), nv, ngayNhan,
                         /*soDem*/ 3, /*soNguoi*/ 2, xacDinhMua(ngayNhan));
ks.checkIn(phieu->layMaPhieu());
ks.ghiDichVu(phieu->layMaPhieu(), "DV02", 1);   // Set menu hải sản
auto hd = ks.checkOut(phieu->layMaPhieu(), nv);
ks.thanhToanHoaDon(hd->layMa(), "Tien mat");
ks.ghiFile("data");
```

## 9. Lưu ý

- Chuỗi trong mã nguồn dùng tiếng Việt không dấu; tệp CSV dùng UTF-8 có BOM.
- Hệ số mùa của **ngày nhận phòng** được áp cho toàn bộ kỳ nghỉ (`canhBaoNhieuMua` cảnh báo khi kỳ nghỉ đi qua nhiều mùa).
- Khi `docFile()` thất bại, không nên ghi đè dữ liệu cũ (dự án gốc khóa ghi và sao lưu CSV trước lần ghi đầu tiên).
