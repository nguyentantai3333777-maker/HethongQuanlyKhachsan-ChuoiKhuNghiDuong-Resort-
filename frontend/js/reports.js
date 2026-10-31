/* =====================================================================
   reports.js - Bao cao & thong ke (mau nau, bieu do 6 sac nau dam nhat)
   API: /api/baocao?ky=ngay|tuan|thang|quy|nam  (QuanLyKhachSan::doanhThuTheoKy,
        doanhThuTheoChiNhanh, doanhThuTheoLoaiPhong, thongKeDichVu, topKhachHang, thongKeNhanVien)
   ===================================================================== */
App.register('reports', {
  sub: 'Doanh thu tính trên hóa đơn đã thanh toán (sau giảm giá, gồm VAT)',
  ky: 'thang',
  scope: '', // pham vi: '' = tat ca chi nhanh, hoac ma chi nhanh

  async render(el) {
    if (this.scope && !App.branchInfo(this.scope)) this.scope = '';
    // Chi phi luong / doanh thu lay tu Backend (giong Console: chiPhiLuongThang, doanhThuThang)
    // Khong nhan he so 2x/3x them o Frontend (luongThucNhan da gom he so).
    const d = await Api.get('baocao', { ky: this.ky, maCN: this.scope });
    const doanhThuKy = Math.max(0, Number(d.tongKy) || 0);
    const doanhThuThang = Math.max(0, Number(d.doanhThuThang) || 0);
    const tongDoanhThu = Math.max(0, Number(d.tongDoanhThu) || 0);
    const chiPhiLuong = Math.max(0, Number(d.chiPhiLuongThang) || 0);
    // Dong bo Console: thue 10% va loi nhuan tinh tren DOANH THU THANG HIEN TAI
    const thue = doanhThuThang * 0.10;
    const loiNhuan = doanhThuThang - chiPhiLuong - thue;
    const pv = this.scope ? branchName(App.branchInfo(this.scope).ten) : 'Tất cả chi nhánh';
    document.getElementById('page-title').textContent = `Thống kê – ${pv}`;
    const B = Charts.BROWNS, P = Charts.PALETTE; // P: xanh la, vang, xanh duong, nau, do, tim
    const kyText = { ngay: '14 ngày', tuan: '8 tuần', thang: '12 tháng', quy: '8 quý', nam: '5 năm' }[this.ky];
    el.innerHTML = `<div class="page">
      ${App.branchTabs(this.scope, { all: true, label: 'Phạm vi thống kê' })}
      <div class="report-head">
        <div class="segmented" role="group" aria-label="Kỳ báo cáo">
          ${[['ngay', 'Ngày'], ['tuan', 'Tuần'], ['thang', 'Tháng'], ['quy', 'Quý'], ['nam', 'Năm']].map(([k, t]) => `<button type="button" data-ky="${k}" class="${k === this.ky ? 'is-active' : ''}">${t}</button>`).join('')}
        </div>
        <span class="muted">Đang xem ${kyText} gần nhất</span>
      </div>
      <section class="stats">
        ${statCard({ label: `Doanh thu ${kyText}`, value: num(doanhThuKy), unit: 'đ', ic: 'reports', accent: true, tone: 'brown', foot: `Tổng đã thu ${money(tongDoanhThu)}` })}
        ${statCard({ label: 'Doanh thu tháng này', value: num(doanhThuThang), unit: 'đ', ic: 'wallet', tone: 'gold', foot: 'Cùng cách tính Console' })}
        ${statCard({ label: 'Chi phí lương tháng', value: num(chiPhiLuong), unit: 'đ', ic: 'staff', tone: 'gold', foot: 'Theo lương thực nhận (Backend)' })}
        ${statCard({ label: 'Thuế doanh thu 10%', value: num(thue), unit: 'đ', ic: 'money', tone: 'red', foot: '10% doanh thu tháng này' })}
        ${statCard({ label: 'Lợi nhuận tháng', value: num(loiNhuan), unit: 'đ', ic: 'money', tone: loiNhuan >= 0 ? 'green' : 'red', foot: 'DT tháng − lương − thuế 10%' })}
        ${statCard({ label: 'Công suất hiện tại', value: d.congSuatHienTai.toFixed(1), unit: '%', ic: 'key', tone: 'blue', foot: 'Phòng đang có khách' })}
      </section>

      <div class="panel" style="margin-bottom:18px">
        <div class="panel__head"><h2>Doanh thu theo ${kyText.split(' ')[1]}</h2></div>
        <div class="panel__body"><div id="rp-line"></div></div>
      </div>

      <section class="grid grid--1-1" style="margin-bottom:18px">
        <div class="panel"><div class="panel__head"><h2>Doanh thu theo chi nhánh</h2>${this.scope ? '<span class="muted">So sánh toàn hệ thống</span>' : ''}</div><div class="panel__body"><div id="rp-branch"></div></div></div>
        <div class="panel"><div class="panel__head"><h2>Doanh thu theo loại phòng</h2></div><div class="panel__body donut-wrap"><div id="rp-type"></div><div id="rp-type-legend"></div></div></div>
      </section>

      <section class="grid grid--1-1" style="margin-bottom:18px">
        <div class="panel"><div class="panel__head"><h2>Công suất phòng</h2></div><div class="panel__body" id="rp-occ"></div></div>
        <div class="panel"><div class="panel__head"><h2>Phiếu đặt phòng</h2></div><div class="panel__body donut-wrap"><div id="rp-book"></div><div id="rp-book-legend"></div></div></div>
      </section>

      <section class="grid grid--1-1" style="margin-bottom:18px">
        <div class="panel"><div class="panel__head"><h2>Dịch vụ</h2></div><div class="panel__body" id="rp-svc"></div></div>
        <div class="panel"><div class="panel__head"><h2>Khách hàng chi tiêu nhiều nhất</h2></div><div class="panel__body" style="padding:12px 0 0"><div id="rp-cus"></div></div></div>
      </section>

      <div class="panel" hidden><div class="panel__head"><h2>Nhân viên</h2><span class="muted">Danh sách nhân viên đang hoạt động</span></div><div class="panel__body" style="padding:12px 0 0"><div id="rp-staff"></div></div></div>
      <p class="muted" style="margin-top:14px">Báo cáo kho hàng chưa có vì hệ thống C++ hiện tại không quản lý kho.</p></div>`;
    const page = el.firstElementChild;

    page.querySelectorAll('[data-ky]').forEach((b) => b.addEventListener('click', () => { this.ky = b.dataset.ky; App.refresh(); }));
    App.bindBranchTabs(page, (ma) => { this.scope = ma; App.refresh(); }, { remember: false });

    const nonNeg = (v) => Math.max(0, Number(v) || 0);
    Charts.line(page.querySelector('#rp-line'), { labels: d.doanhThu.map((x) => x.nhan), values: d.doanhThu.map((x) => nonNeg(x.giaTri)), color: P[1] });
    Charts.bar(page.querySelector('#rp-branch'), { labels: d.theoChiNhanh.map((x) => branchName(x.nhan)), values: d.theoChiNhanh.map((x) => nonNeg(x.giaTri)), colors: [P[3], P[2], P[0], P[1]], height: 230 });
    const tv = d.theoLoaiPhong.map((x) => nonNeg(x.giaTri)), tl = d.theoLoaiPhong.map((x) => x.nhan), tc = [P[0], P[1], P[2], P[3]];
    Charts.doughnut(page.querySelector('#rp-type'), { labels: tl, values: tv, colors: tc, center: '' });
    page.querySelector('#rp-type-legend').innerHTML = Charts.legend(tl, tv, tc, money);
    const bl = BOOKING_STATUS.map((s) => s.t); // cung thu tu TrangThaiPhieu trong C++
    const bv = d.phieuTheoTrangThai.map((x) => x.giaTri), bc = [P[1], P[2], P[0], P[4]]; // cho nhan: vang, dang o: xanh duong, da tra: xanh la, huy: do
    Charts.doughnut(page.querySelector('#rp-book'), { labels: bl, values: bv, colors: bc, center: String(bv.reduce((a, b) => a + b, 0)) });
    page.querySelector('#rp-book-legend').innerHTML = Charts.legend(bl, bv, bc);

    // Cong suat: thanh ngang theo chi nhanh
    page.querySelector('#rp-occ').innerHTML = `<div class="bar-list">${d.congSuat.map((c, i) => `
      <div class="bar-row"><span>${esc(branchName(c.chiNhanh))}<span class="muted" style="display:block;font-size:12px">${c.dangO}/${c.tong} phòng có khách</span></span>
      <div class="bar-row__track"><div class="bar-row__fill" style="width:${c.phanTram}%;background:${P[i % P.length]}"></div></div>
      <b class="num">${c.phanTram.toFixed(1)}%</b></div>`).join('')}</div>
      <p class="muted" style="margin:14px 0 0;font-size:12.5px">${d.congSuat.map((c) => `${branchName(c.chiNhanh)}: ${c.trong} trống, ${c.giuCho} giữ chỗ, ${c.dangDon} đang dọn, ${c.baoTri} bảo trì`).join('<br>')}</p>`;

    const maxSvc = Math.max(1, ...d.dichVu.map((x) => x.doanhThu));
    page.querySelector('#rp-svc').innerHTML = d.dichVu.length ? `<div class="bar-list">${d.dichVu.map((s, i) => `
      <div class="bar-row"><span>${esc(s.ten)}<span class="muted" style="display:block;font-size:12px">${s.soLuong} lượt – ${esc(vi(s.loai))}</span></span>
      <div class="bar-row__track"><div class="bar-row__fill" style="width:${(s.doanhThu / maxSvc) * 100}%;background:${P[i % P.length]}"></div></div>
      <b class="num">${money(s.doanhThu)}</b></div>`).join('')}</div>` : '<p class="muted">Chưa có dịch vụ nào được thanh toán.</p>';

    new DataTable(page.querySelector('#rp-cus'), {
      rows: d.khachHang, pageSize: 6, empty: 'Chưa có khách thanh toán.',
      columns: [
        { key: 'hoTen', label: 'Khách hàng', render: (r) => `${esc(r.hoTen)}<span class="sub">${esc(r.ma)} – ${esc(vi(r.hang))}</span>` },
        { key: 'soLan', label: 'Số lần', align: 'num' },
        { key: 'tongChi', label: 'Tổng chi tiêu', align: 'num', render: (r) => `<b>${money(r.tongChi)}</b>` },
      ],
    });
    new DataTable(page.querySelector('#rp-staff'), {
      rows: d.nhanVien, pageSize: 8,
      columns: [
        { key: 'hoTen', label: 'Nhân viên', render: (r) => `${esc(r.hoTen)}<span class="sub">${esc(r.ma)} – ${esc(vi(r.chucVu))}${r.dangLam ? '' : ' (đã nghỉ)'}</span>` },
      ],
    });
  },
});
