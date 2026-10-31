/* =====================================================================
   dashboard.js - Trang tong quan (mau xanh bien)
   Du lieu: GET /api/tongquan  (QuanLyKhachSan: congSuatPhong, doanhThuTrongKhoang,
   doanhThuTheoKy, layLichSuSuKien...)
   ===================================================================== */
/* ---------- 2 canh bao dau trang: moi canh bao dung doc lap, tinh lai tu du lieu that (GET /api/tongquan) ---------- */
function dashAlertQuaHan(n) {
  return n ? `<div class="alert alert--danger" role="alert"><b>${n} khách ở quá hạn, chưa check-out.</b>
        <span>Phòng của họ vẫn bị khóa, không nhận thêm phiếu mới. <a href="#/bookings?tt=1">Xử lý: gia hạn hoặc trả phòng →</a></span></div>` : '';
}
function dashAlertBiChan(n) {
  return n ? `<div class="alert alert--danger" role="alert"><b>${n} phiếu đặt trước đã đến ngày nhận nhưng phòng vẫn có khách chưa check-out.</b>
        <span><a href="#/bookings?tt=0">Xử lý: cho khách cũ check-out hoặc chuyển phiếu sang phòng khác →</a></span></div>` : '';
}
// Dong bo lai 2 canh bao khi quay lai tab / cua so hoac moi 30 giay (chi khi dang o Dashboard); loi mang -> giu nguyen
const dashAlertSync = (() => {
  let bound = false, busy = false;
  async function run() {
    if (busy || App.current !== 'dashboard' || !Api.hasToken()) return;
    const a = document.getElementById('dash-alert-quahan'), b = document.getElementById('dash-alert-bichan');
    if (!a || !b) return;
    busy = true;
    try {
      const d = await Api.get('tongquan');
      if (App.current !== 'dashboard') return;
      a.innerHTML = dashAlertQuaHan(d.khachQuaHan);
      b.innerHTML = dashAlertBiChan(d.phieuBiChan);
    } catch (e) { /* giu nguyen canh bao hien tai */ } finally { busy = false; }
  }
  return {
    start() {
      if (bound) return; bound = true;
      document.addEventListener('visibilitychange', () => { if (!document.hidden) run(); });
      window.addEventListener('focus', run);
      setInterval(run, 30000);
    },
  };
})();

App.register('dashboard', {
  // Loi chao theo gio trong ngay cho nguoi dang dang nhap
  sub() {
    const h = new Date().getHours();
    const buoi = h < 11 ? 'buổi sáng' : h < 14 ? 'buổi trưa' : h < 18 ? 'buổi chiều' : 'buổi tối';
    const ten = (App.user?.nhanVien?.hoTen || '').trim().split(/\s+/).slice(-2).join(' ');
    return `Chào ${buoi}${ten ? ', ' + ten : ''}! Đây là tổng quan hôm nay của hệ thống Ocean Pearl Resort.`;
  },
  async render(el) {
    const d = await Api.get('tongquan');
    const dt = d.doanhThu;
    const trend = dt.thangTruoc > 0 ? ((dt.thang - dt.thangTruoc) / dt.thangTruoc) * 100 : 0;
    const trendHtml = dt.thangTruoc > 0
      ? `<span class="${trend >= 0 ? 'trend-up' : 'trend-down'}">${trend >= 0 ? '+' : ''}${trend.toFixed(1)}%</span> so với tháng trước`
      : 'Tháng trước chưa có doanh thu';

    el.innerHTML = `
      <div id="dash-alert-quahan" style="display:contents">${dashAlertQuaHan(d.khachQuaHan)}</div>
      <div id="dash-alert-bichan" style="display:contents">${dashAlertBiChan(d.phieuBiChan)}</div>
      ${App.branchTabs(App.branch, { label: 'Chi nhánh' })}
      ${checkBar(App.branch || App.user.nhanVien?.maCN)}
      <section class="stats">
        ${statCard({ label: 'Tổng số phòng', value: d.tongPhong, ic: 'door', tone: 'brown', foot: `${App.cache.branches.length} chi nhánh` })}
        ${statCard({ label: 'Phòng đang trống', value: d.phongTrong, ic: 'bed', tone: 'green', foot: `${d.phongGiuCho} phòng đang giữ chỗ` })}
        ${statCard({ label: 'Phòng đang sử dụng', value: d.phongDangO, ic: 'key', tone: 'blue', foot: `Công suất ${d.congSuat.toFixed(1)}%` })}
        ${statCard({ label: 'Phòng bảo trì', value: d.phongBaoTri, ic: 'tool', tone: 'red', foot: `${d.phongDangDon} phòng đang dọn` })}
        ${statCard({ label: 'Tổng số khách', value: d.tongKhach, ic: 'customers', tone: 'teal', foot: 'Đã lưu trong hệ thống' })}
        ${statCard({ label: 'Số lượt đặt phòng', value: d.soLuotDat, ic: 'calendarCheck', tone: 'gold', foot: `${d.datHomNay} đặt hôm nay · ${d.khachDenHomNay} khách đến nhận phòng` })}
        ${statCard({ label: 'Doanh thu hôm nay', value: num(Math.max(0, dt.homNay || 0)), unit: 'đ', ic: 'wallet', tone: 'gold', foot: `Tuần này ${money(Math.max(0, dt.tuan || 0))}` })}
        ${statCard({ label: 'Doanh thu tháng', value: num(Math.max(0, dt.thang || 0)), unit: 'đ', ic: 'reports', tone: 'brown', foot: trendHtml, accent: true })}
      </section>

      <section class="branch-grid">
        ${d.theoChiNhanh.map((c, i) => {
          const seg = [['Trống', c.trong, '#4f8a62'], ['Giữ chỗ', c.giuCho, '#c08b2c'], ['Đang ở', c.dangO, '#3f7ba6'], ['Dọn phòng', c.dangDon, '#8d6240'], ['Bảo trì', c.baoTri, '#b5483f']];
          return `<article class="branch-card" style="--bc:${['#0e3557', '#1f6fa3', '#c9a24a'][i % 3]}">
            <h3>${esc(branchName(c.ten))}</h3>
            <div class="muted">${c.tongPhong} phòng · ${c.khachDangO} khách đang ở${c.quaHan ? ` · <b style="color:var(--danger)">${c.quaHan} quá hạn</b>` : ''}</div>
            <div class="branch-card__rev">${money(Math.max(0, c.doanhThu || 0))}</div>
            <div class="branch-bar" title="Tình trạng phòng">${seg.map(([n, v, col]) => `<i style="width:${c.tongPhong ? (v / c.tongPhong) * 100 : 0}%;background:${col}" title="${n}: ${v}"></i>`).join('')}</div>
            <div class="branch-legend">${seg.map(([n, v, col]) => `<span style="--dot:${col}">${n} ${v}</span>`).join('')}</div>
          </article>`;
        }).join('')}
      </section>

      <section class="grid grid--2-1" style="margin-bottom:18px">
        <div class="panel">
          <div class="panel__head"><h2>Doanh thu 6 tháng gần nhất</h2><span class="muted">Theo ngày thanh toán</span></div>
          <div class="panel__body"><div id="dash-revenue"></div></div>
        </div>
        <div class="panel">
          <div class="panel__head"><h2>Tình trạng phòng</h2><a href="#/rooms" class="btn btn--sm btn--ghost">Xem phòng</a></div>
          <div class="panel__body">
            <div class="progress-list">
              <div class="progress progress--main">
                <div class="progress__head"><span>Công suất phòng</span><b>${d.congSuat.toFixed(1)}%</b></div>
                <div class="progress__track"><div class="progress__fill" style="width:${Math.min(100, d.congSuat)}%"></div></div>
              </div>
              <div id="dash-rooms-bars" class="progress-list"></div>
            </div>
          </div>
        </div>
      </section>

      <section class="grid grid--2-1">
        <div class="panel">
          <div class="panel__head"><h2>Đặt phòng gần đây</h2><a href="#/bookings" class="btn btn--sm btn--ghost">Xem tất cả</a></div>
          <div class="panel__body" style="padding:12px 0 0"><div id="dash-recent"></div></div>
        </div>
        <div class="panel">
          <div class="panel__head"><h2>Hoạt động gần đây</h2></div>
          <div class="panel__body"><ul class="timeline" id="dash-activity"></ul></div>
        </div>
      </section>`;

    Charts.bar(el.querySelector('#dash-revenue'), {
      labels: d.bieuDoDoanhThu.map((x) => x.nhan.replace('Thang ', 'T')),
      values: d.bieuDoDoanhThu.map((x) => Math.max(0, Number(x.giaTri) || 0)),
      colors: ['#0e3557', '#1f6fa3', '#5fa8d8', '#c9a24a', '#2f8a5e', '#8a7bc2'],
    });

    // Thanh tien do theo 5 trang thai phong (mau trang thai co dinh de de doc)
    const colors = ['#4f8a62', '#c08b2c', '#3f7ba6', '#8d6240', '#b5483f'];
    const vals = d.phongTheoTrangThai.map((x) => x.giaTri);
    el.querySelector('#dash-rooms-bars').innerHTML = ROOM_STATUS.map((s, i) => `
      <a class="progress" href="#/rooms?st=${i}" style="color:inherit;text-decoration:none">
        <div class="progress__head"><span>${esc(s.t)}</span><b>${vals[i]} / ${d.tongPhong}</b></div>
        <div class="progress__track"><div class="progress__fill" style="width:${d.tongPhong ? (vals[i] / d.tongPhong) * 100 : 0}%;background:${colors[i]}"></div></div>
      </a>`).join('');
    bindCheckBar(el);
    dashAlertSync.start();

    new DataTable(el.querySelector('#dash-recent'), {
      rows: d.datPhongGanDay, pageSize: 6,
      columns: [
        { key: 'ma', label: 'Mã phiếu', render: (r) => `<b>${esc(r.ma)}</b>` },
        { key: 'khach', label: 'Khách hàng', render: (r) => `${esc(r.khach)}<span class="sub">${esc(r.sdt)}</span>` },
        { key: 'maPhong', label: 'Phòng', render: (r) => `${esc(r.maPhong)} – ${esc(r.soPhong)}<span class="sub">${esc(branchName(r.chiNhanh))}</span>` },
        { key: 'ngayNhan', label: 'Ngày ở', sort: false, render: (r) => `${esc(r.ngayNhan)}<span class="sub">${r.soDem} đêm</span>` },
        { key: 'maTrangThai', label: 'Trạng thái', render: (r) => bookingBadge(r.maTrangThai) },
      ],
    });

    const evColor = { 'DAT PHONG': 'var(--warn)', 'CHECK-IN': 'var(--info)', 'CHECK-OUT': 'var(--ok)', HUY: 'var(--danger)' };
    const evText = { 'DAT PHONG': 'đặt phòng', 'CHECK-IN': 'nhận phòng', 'CHECK-OUT': 'trả phòng', HUY: 'hủy đặt phòng' };
    el.querySelector('#dash-activity').innerHTML = d.hoatDong.length
      ? d.hoatDong.map((a) => `<li><i style="background:${evColor[a.loai]}"></i><div>
          <p><b>${esc(a.khach)}</b> ${evText[a.loai]} ${esc(a.phong)} (${esc(branchName(a.chiNhanh))})</p>
          <small>${esc(a.thoiGian)} – ${esc(a.maPhieu)}</small></div></li>`).join('')
      : '<li class="muted">Chưa có hoạt động.</li>';
  },
});

/* ---------- Thanh "Kiem tra phong trong" (kieu anh mau) - dung o Dashboard va Dat phong ----------
   Gui -> mo form dat phong voi chi nhanh / ngay / so khach da chon; C++ kiem tra trung lich trong form. */
function checkBar(maCN) {
  const branches = App.cache.branches || [];
  const selected = maCN || App.branch || branches[0]?.ma || '';
  return `<form class="check-bar check-bar--pills dash-check" data-check-bar novalidate>
    <div class="check-bar__label">Kiểm tra phòng trống</div>
    <input type="hidden" name="maCN" value="${esc(selected)}">
    <label class="check-bar__field"><span>Ngày nhận</span><input type="date" name="ngayNhan" min="${todayIso()}" value="${todayIso()}"></label>
    <label class="check-bar__field"><span>Ngày trả</span><input type="date" name="ngayTra" min="${todayIso(1)}" value="${todayIso(1)}"></label>
    <label class="check-bar__field"><span>Số khách</span><input type="number" name="soNguoi" min="1" max="20" value="2"></label>
    <button class="btn btn--primary check-bar__submit" type="submit">Tìm phòng</button>
  </form>`;
}
function syncDashBranchUI(root, maCN) {
  const input = root.querySelector('[data-check-bar] [name=maCN]');
  if (input) input.value = maCN || '';
  root.querySelectorAll('[data-dash-set-cn]').forEach((btn) => {
    const on = btn.dataset.dashSetCn === maCN;
    btn.classList.toggle('is-active', on);
    btn.setAttribute('aria-pressed', on ? 'true' : 'false');
  });
  root.querySelectorAll('[data-branch-tab]').forEach((btn) => {
    const on = btn.dataset.branchTab === maCN;
    btn.classList.toggle('is-active', on);
    btn.setAttribute('aria-selected', on ? 'true' : 'false');
  });
}

function bindCheckBar(root) {
  const f = root.querySelector('[data-check-bar]');
  if (!f) return;
  f.ngayNhan.addEventListener('change', () => {
    f.ngayTra.min = addDays(f.ngayNhan.value, 1);
    if (f.ngayTra.value <= f.ngayNhan.value) f.ngayTra.value = addDays(f.ngayNhan.value, 1);
  });
  f.addEventListener('submit', (e) => {
    e.preventDefault();
    const v = App.formValues(f);
    if (v.ngayNhan < todayIso()) return toast('Ngày nhận không được ở quá khứ.', 'error');
    if (v.ngayTra <= v.ngayNhan) return toast('Ngày trả phải sau ngày nhận.', 'error');
    if (v.maCN) App.setBranch(v.maCN);
    App.pages.bookings.form({ maCN: v.maCN, ngayNhan: v.ngayNhan, ngayTra: v.ngayTra, soNguoi: v.soNguoi });
  });
  // Pill trong form <-> tab phía trên (App.branch)
  root.addEventListener('click', (e) => {
    const pill = e.target.closest('[data-dash-set-cn]');
    if (pill) {
      e.preventDefault();
      const ma = pill.dataset.dashSetCn || '';
      App.setBranch(ma);
      syncDashBranchUI(root, ma);
      return;
    }
  });
  App.bindBranchTabs(root, (ma) => {
    if (ma) App.setBranch(ma);
    syncDashBranchUI(root, ma || App.branch);
  });
}
