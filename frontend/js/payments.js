/* =====================================================================
   payments.js - Thanh toan & doanh thu (mau vang)
   API: /api/doanhthu, /api/phieu?trangThai=1, /api/tamtinh, /api/checkout,
        /api/hoadon, /api/hoadon/chitiet, /api/thanhtoan
   Hoa don = tien phong + tien dich vu - giam gia hang thanh vien + VAT 10% - tien coc
   ===================================================================== */
const Invoice = {
  // Hoa don gon: noi bat 2 khoan tien phong va tien dich vu
  html(h, title) {
    const rooms = h.chiTiet.filter((c) => c.loai === 'Phong'), svcs = h.chiTiet.filter((c) => c.loai !== 'Phong');
    return `<div class="invoice">
      <div class="invoice__head">
        <div><h3>${esc(title || 'Hóa đơn ' + h.ma)}</h3><span class="muted">${esc(h.tgLap)}${h.nhanVien ? ' – ' + esc(h.nhanVien) : ''}</span></div>
        <div style="text-align:right">${h.ma === 'TAM TINH' ? badge('Tạm tính', 'neutral') : h.daThanhToan ? badge('Đã thanh toán', 'ok') : badge('Chưa thanh toán', 'warn')}
          ${h.daThanhToan ? `<div class="muted" style="margin-top:4px;font-size:12.5px">${esc(vi(h.phuongThuc))}, ${esc(h.tgThanhToan)}</div>` : ''}</div>
      </div>
      <div class="invoice__block">
        <dl class="detail-list" style="grid-template-columns:110px 1fr">
          <dt>Khách hàng</dt><dd>${esc(h.khach)} (${esc(h.maKH)}), hạng ${esc(vi(h.hang))}</dd>
          <dt>Phòng</dt><dd>${esc(h.maPhong)} – ${esc(h.soPhong)} ${esc(h.loaiPhong)}, ${esc(branchName(h.chiNhanh))}</dd>
          <dt>Lưu trú</dt><dd>${esc(h.ngayNhan)} → ${esc(h.ngayTra)} (${h.soDem} đêm), phiếu ${esc(h.maPhieu)}</dd>
        </dl>
      </div>
      <div class="invoice__block">
        <div class="invoice__line invoice__line--big"><span class="chip-room">Tiền phòng</span><span>${money(h.tienPhong)}</span></div>
        ${rooms.map((c) => `<div class="invoice__line invoice__line--sub"><span>${c.soLuong} đêm × ${money(c.donGia)} (${esc(c.noiDung)})</span></div>`).join('')}
      </div>
      <div class="invoice__block">
        <div class="invoice__line invoice__line--big"><span class="chip-service">Tiền dịch vụ</span><span>${money(h.tienDichVu)}</span></div>
        ${svcs.length ? svcs.map((c) => `<div class="invoice__line invoice__line--sub"><span>${esc(c.noiDung)} – ${c.soLuong} × ${money(c.donGia)}</span><span>${money(c.thanhTien)}</span></div>`).join('') : '<div class="invoice__line invoice__line--sub"><span>Không sử dụng dịch vụ</span></div>'}
      </div>
      <div class="invoice__block">
        <div class="invoice__line"><span>Tạm tính</span><span>${money(h.tamTinh)}</span></div>
        ${h.tienGiam ? `<div class="invoice__line"><span>Giảm giá thành viên ${Math.round(h.tyLeGiam * 100)}%</span><span class="trend-up">−${money(h.tienGiam)}</span></div>` : ''}
        <div class="invoice__line"><span>Thuế VAT 10%</span><span>+${money(h.thue)}</span></div>
        <div class="invoice__line strong"><span>Tổng cộng</span><span>${money(h.tong)}</span></div>
        ${h.coc ? `<div class="invoice__line"><span>Đã đặt cọc${h.cocDu ? ' (trừ tối đa bằng tổng hóa đơn)' : ''}</span><span class="trend-up">−${money(h.cocDaTru)}</span></div>` : ''}
        ${h.cocDu ? `<div class="invoice__line strong" style="color:var(--info)"><span>Cọc dư – hoàn lại cho khách khi trả phòng</span><span>${money(h.cocDu)}</span></div>` : ''}
      </div>
      <div class="invoice__total"><span>${h.daThanhToan ? 'Đã thanh toán' : 'Còn phải thanh toán'}</span><span>${money(h.conLai)}</span></div>
    </div>`;
  },
};

App.register('payments', {
  sub: 'Chọn chi nhánh để xem trả phòng, hóa đơn và doanh thu',
  tab: 'checkout',

  async render(el, params) {
    if (params.ma) { // mo hoa don tu tim kiem nhanh -> dung chi nhanh cua hoa don
      const h = await Api.get('hoadon/chitiet', { ma: params.ma }).catch(() => null);
      if (h?.maCN) App.setBranch(h.maCN);
    }
    const maCN = App.branch;
    const cn = App.branchInfo(maCN);
    // Chi tai du lieu cua chi nhanh dang chon (khong hien hoa don tat ca chi nhanh cung luc)
    const [dt, staying, invoices] = maCN
      ? await Promise.all([Api.get('doanhthu', { maCN }), Api.get('phieu', { trangThai: 1, maCN }), Api.get('hoadon', { maCN })])
      : [{ homNay: 0, tuan: 0, thang: 0, thangTruoc: 0, nam: 0, conNo: 0 }, [], []];
    document.getElementById('page-title').textContent = cn ? `Hóa đơn – ${branchName(cn.ten)}` : 'Hóa đơn & thanh toán';
    if (params.tab === 'invoices' || params.ma) this.tab = 'invoices';
    el.innerHTML = `<div class="page">
      ${App.branchTabs(maCN, { label: 'Chi nhánh', extra: App.isAdmin() ? `<button class="btn btn--sm" type="button" id="p-global">${icon('search', 15)} Tìm kiếm toàn hệ thống</button>` : '' })}
      <section class="stats">
        ${statCard({ label: 'Doanh thu hôm nay', value: num(Math.max(0, dt.homNay || 0)), unit: 'đ', ic: 'wallet', tone: 'gold' })}
        ${statCard({ label: 'Doanh thu tuần', value: num(Math.max(0, dt.tuan || 0)), unit: 'đ', ic: 'wallet', tone: 'green' })}
        ${statCard({ label: 'Doanh thu tháng', value: num(Math.max(0, dt.thang || 0)), unit: 'đ', ic: 'reports', accent: true, tone: 'brown', foot: `Tháng trước ${money(Math.max(0, dt.thangTruoc || 0))}` })}
        ${statCard({ label: 'Doanh thu năm', value: num(Math.max(0, dt.nam || 0)), unit: 'đ', ic: 'reports', tone: 'blue', foot: `${dt.conNo || 0} hóa đơn chưa thanh toán` })}
      </section>
      <div class="tabs">
        <button type="button" data-tab="checkout">Khách đang ở (${staying.length})</button>
        <button type="button" data-tab="invoices">Hóa đơn (${invoices.length})</button>
      </div>
      <section id="p-checkout"><div class="panel" id="p-staying"></div></section>
      <section id="p-invoices" hidden>
        <div class="toolbar">
          <input class="input input--search" id="p-q" type="search" placeholder="Tìm mã hóa đơn, khách, phòng…" value="${esc(params.ma || '')}">
          <select class="select" id="p-tt"><option value="">Mọi trạng thái</option><option value="0">Chưa thanh toán</option><option value="1">Đã thanh toán</option></select>
          <select class="select" id="p-pt"><option value="">Mọi phương thức</option>${options(PAY_METHODS.concat({ v: 'Tien coc', t: 'Tiền cọc' }))}</select>
        </div>
        <div class="panel" id="p-table"></div>
      </section></div>`;
    const page = el.firstElementChild;

    new DataTable(page.querySelector('#p-staying'), {
      rows: staying, empty: 'Hiện không có khách đang ở.',
      columns: [
        { key: 'ma', label: 'Phiếu', render: (r) => `<b>${esc(r.ma)}</b>` },
        { key: 'khach', label: 'Khách hàng', render: (r) => `${esc(r.khach)}<span class="sub">${esc(r.sdt)}</span>` },
        { key: 'maPhong', label: 'Phòng', render: (r) => `${esc(r.soPhong)} – ${esc(r.loaiPhong)}<span class="sub">${esc(r.maPhong)}</span>` },
        { key: 'ngayTraIso', label: 'Ngày ở', render: (r) => `${esc(r.ngayNhan)} → ${esc(r.ngayTra)}<span class="sub">Nhận phòng ${esc(r.tgCheckIn)}</span>` },
        { key: 'tienPhong', label: 'Tiền phòng', align: 'num', render: (r) => money(r.tienPhong) },
        { key: 'tienDichVu', label: 'Tiền dịch vụ', align: 'num', render: (r) => money(r.tienDichVu) },
        { key: 'act', label: 'Thao tác', sort: false, align: 'num', render: (r) => `<div class="actions"><button class="btn btn--sm" data-preview="${r.ma}">Tạm tính</button><button class="btn btn--sm btn--primary" data-checkout="${r.ma}">Check-out</button></div>` },
      ],
    });

    const table = new DataTable(page.querySelector('#p-table'), {
      rows: invoices,
      searchText: (r) => `${r.ma} ${r.khach} ${r.maPhong} ${r.soPhong} ${r.maPhieu}`,
      columns: [
        { key: 'ma', label: 'Mã HĐ', value: (r) => r.tgLapSo, render: (r) => `<b>${esc(r.ma)}</b><span class="sub">${esc(r.tgLap)}</span>` },
        { key: 'khach', label: 'Khách hàng' },
        { key: 'maPhong', label: 'Phòng', render: (r) => `${esc(r.soPhong)} – ${esc(r.loaiPhong)}<span class="sub">Phiếu ${esc(r.maPhieu)}</span>` },
        { key: 'tienPhong', label: 'Tiền phòng', align: 'num', render: (r) => money(r.tienPhong) },
        { key: 'tienDichVu', label: 'Tiền dịch vụ', align: 'num', render: (r) => money(r.tienDichVu) },
        { key: 'tienGiam', label: 'Giảm giá', align: 'num', render: (r) => (r.tienGiam ? '−' + money(r.tienGiam) : '<span class="muted">—</span>') },
        { key: 'thue', label: 'Thuế', align: 'num', render: (r) => money(r.thue) },
        { key: 'tong', label: 'Tổng tiền', align: 'num', render: (r) => `<b>${money(r.tong)}</b>` },
        { key: 'phuongThuc', label: 'Phương thức', render: (r) => esc(vi(r.phuongThuc) || '—') },
        { key: 'daThanhToan', label: 'Trạng thái', value: (r) => (r.daThanhToan ? 1 : 0), render: (r) => (r.daThanhToan ? badge('Đã thanh toán', 'ok') : badge('Chưa thanh toán', 'warn')) },
        { key: 'act', label: '', sort: false, align: 'num', render: (r) => `<div class="actions"><button class="btn btn--sm" data-view="${r.ma}" title="Xem hóa đơn">${icon('eye', 15)}</button>${r.daThanhToan ? '' : `<button class="btn btn--sm btn--primary" data-pay="${r.ma}">Thanh toán</button>`}</div>` },
      ],
    });
    const apply = () => {
      const tt = page.querySelector('#p-tt').value, pt = page.querySelector('#p-pt').value;
      table.setFilter((r) => (tt === '' || r.daThanhToan === (tt === '1')) && (!pt || r.phuongThuc === pt));
      table.setQuery(page.querySelector('#p-q').value);
    };
    page.querySelector('#p-q').addEventListener('input', debounce(apply, 200));
    page.querySelector('#p-tt').addEventListener('change', apply);
    page.querySelector('#p-pt').addEventListener('change', apply);
    apply();

    const setTab = (t) => {
      this.tab = t;
      page.querySelectorAll('[data-tab]').forEach((b) => b.classList.toggle('is-active', b.dataset.tab === t));
      page.querySelector('#p-checkout').hidden = t !== 'checkout';
      page.querySelector('#p-invoices').hidden = t !== 'invoices';
    };
    page.querySelectorAll('[data-tab]').forEach((b) => b.addEventListener('click', () => setTab(b.dataset.tab)));
    setTab(this.tab);
    App.bindBranchTabs(page, () => App.refresh());
    page.querySelector('#p-global')?.addEventListener('click', () => this.globalSearch());

    page.addEventListener('click', (e) => {
      const t = e.target.closest('[data-preview],[data-checkout],[data-view],[data-pay]');
      if (!t) return;
      if (t.dataset.preview) this.preview(t.dataset.preview);
      if (t.dataset.checkout) this.checkout(t.dataset.checkout);
      if (t.dataset.view) this.view(invoices.find((h) => h.ma === t.dataset.view));
      if (t.dataset.pay) this.pay(invoices.find((h) => h.ma === t.dataset.pay));
    });
    if (params.ma) { const h = invoices.find((x) => x.ma === params.ma.toUpperCase()); if (h) this.view(h); }
  },

  // Tim kiem TOAN HE THONG: chuc nang rieng cho quan tri, tach khoi man hinh hoa don cua tung chi nhanh
  globalSearch() {
    App.modal({
      title: 'Tìm kiếm hóa đơn toàn hệ thống', size: 'xl',
      body: `<div class="toolbar"><input class="input input--search" id="gs-q" type="search" placeholder="Mã hóa đơn, mã phiếu, tên hoặc SĐT khách…" autofocus></div><div class="panel" id="gs-table"><div class="loading">Đang tải…</div></div>`,
      onOpen: async (body) => {
        const all = await App.run(() => Api.get('hoadon'));
        if (!all) return;
        const t = new DataTable(body.querySelector('#gs-table'), {
          rows: all, pageSize: 8, empty: 'Không tìm thấy hóa đơn.',
          searchText: (r) => `${r.ma} ${r.maPhieu} ${r.khach} ${r.maKH}`,
          columns: [
            { key: 'ma', label: 'Mã HĐ', render: (r) => `<b>${esc(r.ma)}</b>` },
            { key: 'chiNhanh', label: 'Chi nhánh', render: (r) => esc(branchName(r.chiNhanh)) },
            { key: 'khach', label: 'Khách hàng' },
            { key: 'tgLapSo', label: 'Ngày lập', render: (r) => esc(r.tgLap) },
            { key: 'tong', label: 'Tổng tiền', align: 'num', render: (r) => `<b>${money(r.tong)}</b>` },
            { key: 'daThanhToan', label: 'Trạng thái', render: (r) => (r.daThanhToan ? badge('Đã thanh toán', 'ok') : badge('Chưa thanh toán', 'warn')) },
            { key: 'act', label: '', sort: false, align: 'num', render: (r) => `<button class="btn btn--sm" data-gs="${r.ma}">Mở</button>` },
          ],
        });
        body.querySelector('#gs-q').addEventListener('input', debounce((e) => t.setQuery(e.target.value), 200));
        body.addEventListener('click', (e) => {
          const b = e.target.closest('[data-gs]');
          if (b) { const h = all.find((x) => x.ma === b.dataset.gs); App.closeModal(); App.setBranch(h.maCN); location.hash = `#/payments?ma=${h.ma}`; }
        });
      },
      footer: [{ text: 'Đóng' }],
    });
  },

  async preview(ma) {
    const h = await App.run(() => Api.post('tamtinh', { ma }));
    if (h) App.modal({ title: `Tạm tính phiếu ${ma}`, size: 'lg', body: Invoice.html(h, 'Tạm tính'), footer: [{ text: 'Đóng' }, { text: 'Check-out', cls: 'btn--primary', onClick: () => this.checkout(ma) }] });
  },

  async checkout(ma) {
    if (!(await App.confirm(`Xác nhận check-out phiếu ${ma} và lập hóa đơn? Khách trả sớm chỉ tính số đêm thực tế đã ở; nếu khách ở quá hạn, hóa đơn tự tính thêm các đêm quá hạn.`, { ok: 'Check-out', title: 'Xác nhận check-out' }))) return;
    const h = await App.run(() => Api.post('checkout', { ma }), (x) => `Check-out thành công, đã lập hóa đơn ${x.ma}.`);
    if (!h) return;
    if (h.daThanhToan) { this.view(h); App.refresh(); } else this.pay(h, true);
  },

  view(h) {
    App.modal({
      title: `Hóa đơn ${h.ma}`, size: 'lg', body: Invoice.html(h),
      footer: [{ text: 'In hóa đơn', onClick: () => window.print() }, ...(h.daThanhToan ? [] : [{ text: 'Thanh toán', cls: 'btn--primary', onClick: () => this.pay(h) }]), { text: 'Đóng' }],
    });
  },

  pay(h, refreshAfter = false) {
    const conLai = Math.round(h.conLai);
    App.modal({
      title: `Thanh toán ${h.ma}`, size: 'lg',
      body: `${Invoice.html(h)}
        <form class="form-grid" id="pay-form" style="margin-top:18px" novalidate>
          <label class="field"><span>Phương thức</span><select name="phuongThuc">${options(PAY_METHODS, 'Tien mat')}</select></label>
          <label class="field" id="cash-field"><span>Khách đưa (đ)</span><input name="khachDua" inputmode="numeric" value="${conLai}" autocomplete="off"></label>
          <p class="full muted" id="change" style="margin:0">Tiền thối lại: ${money(0)}</p>
        </form>`,
      onOpen: (b) => {
        const f = b.querySelector('#pay-form');
        const sync = () => {
          const cash = f.phuongThuc.value === 'Tien mat';
          b.querySelector('#cash-field').hidden = !cash;
          b.querySelector('#change').hidden = !cash;
          b.querySelector('#change').textContent = `Tiền thối lại: ${money(Math.max(0, Number(f.khachDua.value) - conLai))}`;
        };
        f.addEventListener('input', sync); f.addEventListener('change', sync); sync();
      },
      footer: [
        { text: 'Thanh toán sau', onClick: () => { App.closeModal(); if (refreshAfter) App.refresh(); } },
        {
          text: `Thu ${money(conLai)}`, cls: 'btn--primary',
          onClick: async (b) => {
            const f = b.querySelector('#pay-form');
            const v = App.formValues(f);
            if (v.phuongThuc === 'Tien mat' && Number(v.khachDua) < conLai) return App.showErrors(f, { khachDua: `Khách cần đưa ít nhất ${money(conLai)}.` });
            if (!(await App.confirm({ html: `<p>Xác nhận thu <b>${money(conLai)}</b> cho hóa đơn <b>${esc(h.ma)}</b> bằng <b>${esc(vi(v.phuongThuc))}</b>?</p>` }, { title: 'Xác nhận thanh toán', ok: 'Thanh toán' }))) return;
            const d = await App.run(() => Api.post('thanhtoan', { ma: h.ma, phuongThuc: v.phuongThuc }), `Thanh toán hóa đơn ${h.ma} thành công!`);
            if (d) { App.closeModal(); App.refresh(); }
          },
        },
      ],
    });
  },
});
