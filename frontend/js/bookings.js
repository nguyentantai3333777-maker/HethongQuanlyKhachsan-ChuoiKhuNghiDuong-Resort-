/* =====================================================================
   bookings.js - Dat phong (mau xanh la)
   - Danh sach phieu + loc; lich phong dang Calendar (phong x ngay)
   - Form dat phong: chon phong theo lich trong (C++ kiem tra trung ngay),
     khach cu/moi, dich vu di kem, coc; tong tien tinh tu dong qua /api/baogia
   ===================================================================== */

App.register('bookings', {
  sub: 'Phiếu đặt phòng, lịch phòng và nhận phòng',
  tab: 'list',

  async render(el, params) {
    if (params.ma) { // mo phieu tu tim kiem nhanh -> chuyen sang dung chi nhanh cua phieu
      const ph = await Api.get('phieu/chitiet', { ma: params.ma }).catch(() => null);
      if (ph?.maCN) App.setBranch(ph.maCN);
    }
    const maCN = App.branch;
    const cn = App.branchInfo(maCN);
    // Chi lay phieu / phong cua chi nhanh dang chon
    const [list, rooms] = maCN ? await Promise.all([Api.get('phieu', { maCN }), Api.get('phong', { maCN })]) : [[], []];
    this.rooms = rooms;
    document.getElementById('page-title').textContent = cn ? `Đặt phòng – ${branchName(cn.ten)}` : 'Đặt phòng';
    if (params.tab === 'calendar') this.tab = 'calendar'; else if (params.tab === 'list' || params.tt !== undefined) this.tab = 'list';
    const blocked = list.filter((r) => r.biChan);
    el.innerHTML = `<div class="page">
      ${App.branchTabs(maCN, { label: 'Chi nhánh' })}
      ${blocked.length ? `<div class="alert alert--danger" role="alert"><span><b>${blocked.length} phiếu đã đến ngày nhận nhưng phòng vẫn có khách chưa check-out:</b>
        ${blocked.map((b) => `${esc(b.ma)} (phòng ${esc(b.soPhong)}, khách đang ở: ${esc(b.khachChiemPhong)})`).join('; ')}.
        Hãy cho khách cũ check-out hoặc dùng <b>Chuyển phòng</b> cho phiếu mới.</span></div>` : ''}
      ${checkBar(maCN)}
      <div class="toolbar">
        <div class="tabs" style="margin:0;border:0">
          <button type="button" data-tab="list">Danh sách phiếu</button>
          <button type="button" data-tab="calendar">Lịch phòng</button>
        </div>
        <span class="spacer"></span>
        <button class="btn btn--primary" id="b-new" type="button">${icon('plus', 16)} Đặt phòng mới</button>
      </div>
      <section id="tab-list">
        <div class="toolbar">
          <input class="input input--search" id="b-q" type="search" placeholder="Tìm mã phiếu, khách, số điện thoại, phòng…">
          <select class="select" id="b-tt"><option value="">Mọi trạng thái</option>${BOOKING_STATUS.map((s, i) => `<option value="${i}"${params.tt === String(i) ? ' selected' : ''}>${s.t}</option>`).join('')}</select>
        </div>
        <div class="panel" id="b-table"></div>
      </section>
      <section id="tab-calendar" hidden>
        <div class="toolbar">
          <input class="input" type="date" id="cal-from" value="${todayIso(-1)}">
          <span class="muted">14 ngày, bấm vào thanh để xem phiếu</span>
        </div>
        <div class="panel calendar" id="cal"></div>
      </section></div>`;
    const page = el.firstElementChild;

    const table = new DataTable(page.querySelector('#b-table'), {
      rows: list,
      searchText: (r) => `${r.ma} ${r.khach} ${r.sdt} ${r.maPhong} ${r.soPhong}`,
      empty: 'Không có phiếu phù hợp.',
      columns: [
        { key: 'ma', label: 'Mã phiếu', value: (r) => r.tgDatSo, render: (r) => `<b>${esc(r.ma)}</b>` },
        { key: 'khach', label: 'Khách hàng', render: (r) => `${esc(r.khach)}<span class="sub">${esc(r.sdt)}</span>` },
        { key: 'soPhong', label: 'Phòng', render: (r) => `${esc(r.soPhong)} – ${esc(r.loaiPhong)}<span class="sub">${esc(r.maPhong)}</span>` },
        { key: 'ngayNhanIso', label: 'Ngày ở', render: (r) => `<span style="white-space:nowrap">${esc(r.ngayNhan.slice(0, 5))} → ${esc(r.ngayTra)}</span><span class="sub">${r.soDem} đêm, ${r.soNguoi} khách</span>` },
        { key: 'tienCoc', label: 'Tiền cọc', align: 'num', render: (r) => (r.tienCoc ? money(r.tienCoc) : '<span class="muted">—</span>') },
        { key: 'maTrangThai', label: 'Trạng thái', render: (r) => this.statusBadge(r) },
        { key: 'tgDatSo', label: 'Đặt lúc', render: (r) => `${esc(r.tgDat)}<span class="sub">${esc(vi(r.nhanVien))}</span>` },
        { key: 'act', label: 'Thao tác', sort: false, align: 'num', render: (r) => this.actions(r) },
      ],
    });
    const apply = () => {
      const tt = page.querySelector('#b-tt').value;
      table.setFilter((r) => tt === '' || r.maTrangThai === Number(tt));
      table.setQuery(page.querySelector('#b-q').value);
    };
    page.querySelector('#b-q').addEventListener('input', debounce(apply, 200));
    page.querySelector('#b-tt').addEventListener('change', apply);
    App.bindBranchTabs(page, () => App.refresh());
    apply();

    const setTab = (t) => {
      this.tab = t;
      page.querySelectorAll('[data-tab]').forEach((b) => b.classList.toggle('is-active', b.dataset.tab === t));
      page.querySelector('#tab-list').hidden = t !== 'list';
      page.querySelector('#tab-calendar').hidden = t !== 'calendar';
      if (t === 'calendar') this.calendar(page, list);
    };
    page.querySelectorAll('[data-tab]').forEach((b) => b.addEventListener('click', () => setTab(b.dataset.tab)));
    page.querySelector('#cal-from').addEventListener('change', () => this.calendar(page, list));
    setTab(this.tab);

    page.addEventListener('click', (e) => {
      const b = e.target.closest('[data-act]');
      if (!b) return;
      const r = list.find((x) => x.ma === b.dataset.ma);
      ({ view: () => this.detail(r.ma), checkin: () => this.checkin(r), checkout: () => App.pages.payments.checkout(r.ma), cancel: () => this.cancel(r), service: () => this.addService(r), extend: () => this.extend(r), transfer: () => this.transfer(r) })[b.dataset.act]();
    });
    page.querySelector('#b-new').addEventListener('click', () => this.form());
    bindCheckBar(page);
    if (params.ma) this.detail(params.ma);
    if (params.new) { history.replaceState(null, '', '#/bookings'); this.form(); }
  },

  // Trang thai + canh bao qua han (khach chua check-out du da qua ngay tra)
  statusBadge(r) {
    return bookingBadge(r.maTrangThai) + (r.quaHan ? ` ${badge(`Quá hạn ${r.soNgayQuaHan} ngày`, 'danger')}` : '')
      + (r.biChan ? ` ${badge('Phòng đang có khách', 'danger')}` : '');
  },

  actions(r) {
    const canCheckin = r.maTrangThai === 0 && r.ngayNhanIso === todayIso();
    return `<div class="actions">
      <button class="btn btn--sm" data-act="view" data-ma="${r.ma}" title="Xem chi tiết">${icon('eye', 15)}</button>
      ${canCheckin ? `<button class="btn btn--sm btn--primary" data-act="checkin" data-ma="${r.ma}">Check-in</button>` : ''}
      ${r.maTrangThai === 1 ? `<button class="btn btn--sm btn--primary" data-act="checkout" data-ma="${r.ma}" title="Trả phòng (kể cả trước ngày dự kiến)">Check-out</button>` : ''}
      ${r.maTrangThai === 1 ? `<button class="btn btn--sm${r.quaHan ? ' btn--danger' : ''}" data-act="extend" data-ma="${r.ma}" title="Gia hạn thêm đêm cho khách đang ở">Gia hạn</button>` : ''}
      ${r.maTrangThai === 0 ? `<button class="btn btn--sm${r.biChan ? ' btn--primary' : ''}" data-act="transfer" data-ma="${r.ma}" title="Chuyển phiếu sang phòng khác" aria-label="Chuyển phòng">${icon('swap', 15)}${r.biChan ? ' Chuyển phòng' : ''}</button>` : ''}
      ${r.maTrangThai <= 1 ? `<button class="btn btn--sm" data-act="service" data-ma="${r.ma}" title="Thêm dịch vụ" aria-label="Thêm dịch vụ">${icon('services', 15)}</button>` : ''}
      ${r.maTrangThai === 0 ? `<button class="btn btn--sm btn--danger" data-act="cancel" data-ma="${r.ma}">Hủy</button>` : ''}
    </div>`;
  },

  /* ---------- Lich phong: hang = phong, cot = 14 ngay ---------- */
  calendar(page, list) {
    const cn = App.branch;
    const from = page.querySelector('#cal-from').value || todayIso();
    const days = 14, today = todayIso();
    const branch = App.cache.branches.find((b) => b.ma === cn);
    const rooms = this.rooms.filter((r) => r.maCN === cn);
    const dayHead = Array.from({ length: days }, (_, i) => {
      const iso = addDays(from, i), d = new Date(iso + 'T00:00:00');
      return `<div class="calendar__cell calendar__head${iso === today ? ' is-today' : ''}"><span>${['CN', 'T2', 'T3', 'T4', 'T5', 'T6', 'T7'][d.getDay()]}</span><b>${d.getDate()}/${d.getMonth() + 1}</b></div>`;
    }).join('');
    const rows = rooms.map((room) => {
      const bars = list.filter((b) => b.maPhong === room.ma && b.maTrangThai !== 3).map((b) => {
 // khach qua han: phong van bi khoa -> thanh keo den het hom nay
        const traIso = b.quaHan ? todayIso(1) : b.ngayTraIso;
        const s = Math.max(0, daysBetween(from, b.ngayNhanIso)), e = Math.min(days, daysBetween(from, traIso));
        if (e <= 0 || s >= days || e <= s) return '';
        return `<div class="calendar__bar${b.quaHan ? ' is-overstay' : ''}" data-st="${b.maTrangThai}" data-act="view" data-ma="${b.ma}" title="${esc(b.ma)} – ${esc(b.khach)} (${esc(b.ngayNhan)} → ${esc(b.ngayTra)})"
          style="left:calc(${(s / days) * 100}% + 2px);width:calc(${((e - s) / days) * 100}% - 4px)">${esc(b.khach)}</div>`;
      }).join('');
      return `<div class="calendar__cell calendar__room"><b>${esc(room.so)} – ${esc(room.loai)}</b><small>${esc(room.ma)}</small></div>
        <div class="calendar__cell" style="grid-column:span ${days};background-image:linear-gradient(to right,var(--border-color) 1px,transparent 1px);background-size:calc(100% / ${days}) 100%">${bars}</div>`;
    }).join('');
    page.querySelector('#cal').innerHTML = `
      <div class="calendar__grid" style="grid-template-columns:170px repeat(${days}, minmax(54px, 1fr))">
        <div class="calendar__cell calendar__head calendar__room"><b>${esc(branch ? branchName(branch.ten) : '')}</b></div>${dayHead}${rows}
      </div>
      <div class="pager"><span>${badge('Chờ nhận phòng', 'warn')} ${badge('Đang ở', 'info')} ${badge('Đã trả phòng', 'ok')} ${badge('Quá hạn (phòng bị khóa)', 'danger')}</span></div>`;
  },

  /* ---------- Chi tiet phieu ---------- */
  async detail(ma) {
    const b = await App.run(() => Api.get('phieu/chitiet', { ma }));
    if (!b) return;
    const footer = [{ text: 'Đóng' }];
    if (b.maTrangThai === 0) footer.push({ text: 'Hủy phiếu', cls: 'btn--danger', onClick: () => this.cancel(b) });
    if (b.maTrangThai === 0) footer.push({ text: 'Chuyển phòng', cls: b.biChan ? 'btn--primary' : '', onClick: () => this.transfer(b) });
    if (b.maTrangThai === 1) footer.push({ text: 'Gia hạn', cls: b.quaHan ? 'btn--danger' : '', onClick: () => this.extend(b) });
    if (b.maTrangThai <= 1) footer.push({ text: 'Thêm dịch vụ', onClick: () => this.addService(b) });
    if (b.maTrangThai === 0 && b.ngayNhanIso === todayIso()) footer.push({ text: 'Check-in', cls: 'btn--primary', onClick: () => this.checkin(b) });
    App.modal({
      title: `Phiếu ${b.ma}`, size: 'lg',
      body: `
        ${b.biChan ? `<div class="alert alert--danger"><span><b>Phòng ${esc(b.soPhong)} hiện đang có khách ${esc(b.khachChiemPhong)} chưa check-out.</b> Khách của phiếu này chưa thể nhận phòng. Vui lòng cho khách cũ check-out hoặc <b>chuyển phiếu sang phòng khác</b>.</span></div>` : ''}
        ${b.quaHan ? `<div class="alert alert--danger"><span><b>Khách ở quá hạn ${b.soNgayQuaHan} ngày</b> (dự kiến trả ${esc(b.ngayTra)}). Phòng vẫn bị khóa cho đến khi check-out; hóa đơn sẽ tính thêm ${b.soNgayQuaHan} đêm. Có thể <b>Gia hạn</b> phiếu này, không tạo phiếu mới.</span></div>` : ''}
        <dl class="detail-list">
          <dt>Trạng thái</dt><dd>${this.statusBadge(b)}</dd>
          <dt>Khách hàng</dt><dd>${esc(b.khach)} (${esc(b.maKH)}) – ${esc(b.sdt)}, hạng ${esc(vi(b.hang))}</dd>
          <dt>Chi nhánh</dt><dd>${esc(branchName(b.chiNhanh))}</dd>
          <dt>Phòng</dt><dd>${esc(b.maPhong)} – số ${esc(b.soPhong)} (${esc(b.loaiPhong)})</dd>
          <dt>Ngày ở</dt><dd>${esc(b.ngayNhan)} → ${esc(b.ngayTra)} (${b.soDem} đêm, ${b.soNguoi} khách)</dd>
          <dt>Mùa</dt><dd>${esc(vi(b.mua))} (×${b.heSoMua})</dd>
          <dt>Đặt lúc</dt><dd>${esc(b.tgDat)} – ${esc(b.nhanVien)}</dd>
          ${b.tgCheckIn ? `<dt>Check-in</dt><dd>${esc(b.tgCheckIn)}</dd>` : ''}
          ${b.tgCheckOut ? `<dt>Check-out</dt><dd>${esc(b.tgCheckOut)} (hóa đơn ${esc(b.maHoaDon)})</dd>` : ''}
          ${b.tgHuy ? `<dt>Đã hủy</dt><dd>${esc(b.tgHuy)} – ${esc(b.lyDoHuy)}</dd>` : ''}
          <dt>Tiền cọc</dt><dd>${b.tienCoc ? `${money(b.tienCoc)} (${esc(vi(b.ptCoc))})${b.daHoanCoc ? ' – đã hoàn' : ''}` : 'Không cọc'}</dd>
          ${b.ghiChu ? `<dt>Ghi chú</dt><dd>${esc(b.ghiChu)}</dd>` : ''}
        </dl>
        <h3 class="section-title">Dịch vụ</h3>
        ${b.dichVu.length ? `<div class="table-wrap"><table class="table"><thead><tr><th>Dịch vụ</th><th>Thời gian</th><th class="num">SL</th><th class="num">Thành tiền</th></tr></thead><tbody>
          ${b.dichVu.map((d) => `<tr><td>${esc(d.ten)}</td><td class="muted">${esc(d.thoiGian)}</td><td class="num">${d.soLuong}</td><td class="num">${money(d.thanhTien)}</td></tr>`).join('')}</tbody></table></div>`
        : '<p class="muted">Chưa có dịch vụ.</p>'}
        <div class="invoice" style="margin-top:16px">
          <div class="invoice__block">
            <div class="invoice__line invoice__line--big"><span class="chip-room">Tiền phòng</span><span>${money(b.tienPhong)}</span></div>
            <div class="invoice__line invoice__line--big"><span class="chip-service">Tiền dịch vụ</span><span>${money(b.tienDichVu)}</span></div>
          </div>
        </div>`,
      footer,
    });
  },

  /* ---------- Gia han cho khach dang o (cung phieu, khong tao phieu moi) ---------- */
  extend(b) {
    const minNeed = b.quaHan ? b.soNgayQuaHan + 1 : 1;
    App.modal({
      title: `Gia hạn phiếu ${b.ma}`,
      body: `<form class="stack" id="ext-form" style="gap:14px" novalidate>
        <p style="margin:0">Khách <b>${esc(b.khach)}</b>, phòng ${esc(b.maPhong)} – ${esc(b.soPhong)}. Ngày trả hiện tại: <b>${esc(b.ngayTra)}</b> (${b.soDem} đêm).</p>
        ${b.quaHan ? `<div class="alert alert--danger" style="margin:0">Đã quá hạn ${b.soNgayQuaHan} ngày. Cần gia hạn ít nhất <b>${minNeed} đêm</b> để ngày trả mới nằm sau hôm nay.</div>` : ''}
        <label class="field"><span>Số đêm gia hạn thêm (1–30)</span><input name="themDem" inputmode="numeric" min="1" max="30" value="${minNeed}" autocomplete="off"></label>
        <div id="ext-preview" class="muted">Đang kiểm tra lịch phòng…</div>
      </form>`,
      onOpen: (body) => {
        const f = body.querySelector('#ext-form'), box = body.querySelector('#ext-preview');
        const okBtn = () => document.querySelector('#modal-footer .btn--primary');
        const check = debounce(async () => {
          const n = f.themDem.value;
          if (!/^\d{1,2}$/.test(n) || Number(n) < 1 || Number(n) > 30) { box.innerHTML = '<span style="color:var(--danger)">Nhập số đêm từ 1 đến 30.</span>'; if (okBtn()) okBtn().disabled = true; return; }
          try {
            const q = await Api.get('giahan', { ma: b.ma, themDem: n, xemTruoc: 1 });
            if (okBtn()) okBtn().disabled = !q.hopLe;
            box.innerHTML = q.hopLe
              ? `Ngày trả mới: <b>${esc(q.ngayTraMoi)}</b>. Tiền phòng: ${money(q.tienPhongCu)} → <b>${money(q.tienPhongMoi)}</b> (<span class="trend-down">+${money(q.chenhLech)}</span>), chưa gồm VAT/giảm giá.`
              : `<div class="alert alert--danger" style="margin:0">${esc(q.lyDo)}</div>`;
          } catch (e) { box.innerHTML = `<span style="color:var(--danger)">${esc(e.message)}</span>`; }
        }, 250);
        f.addEventListener('input', check); check();
      },
      footer: [
        { text: 'Hủy' },
        {
          text: 'Gia hạn', cls: 'btn--primary',
          onClick: async (body) => {
            const f = body.querySelector('#ext-form');
            if (App.showErrors(f, {})) return;
            const d = await App.run(() => Api.post('giahan', { ma: b.ma, themDem: f.themDem.value }), (p) => `Đã gia hạn phiếu ${p.ma}, ngày trả mới ${p.ngayTra}.`);
            if (d) { App.closeModal(); App.refresh(); }
          },
        },
      ],
    });
  },

  /* ---------- Chuyen phieu sang phong khac: cap nhat CHINH phieu hien tai (giu ma, khach, ngay, coc, trang thai) ---------- */
  async transfer(b) {
    const load = (cungLoai) => Api.get('phieu/phongchuyen', { ma: b.ma, cungLoai: cungLoai ? 1 : 0 });
    let d = await App.run(() => load(true));
    if (!d) return;
    if (d.loi) return toast(d.loi, 'error');
    const state = { chon: '', cungLoai: true };
    const draw = (body) => {
      const box = body.querySelector('#tf-rooms');
      box.innerHTML = d.phong.length ? `<div class="room-pick">${d.phong.map((x) => `<label>
          <input type="radio" name="_moi" value="${x.phong.ma}" ${x.phong.ma === state.chon ? 'checked' : ''}>
          <b>Phòng ${esc(x.phong.so)} – ${esc(x.phong.loai)}</b>
          <span class="muted">${x.phong.sucChua} người · ${esc(x.phong.ma)}</span>
          <span>${money(x.tienPhongMoi)} / ${b.soDem} đêm</span></label>`).join('')}</div>`
        : `<div class="room-empty">Không còn phòng ${state.cungLoai ? `loại ${esc(b.loaiPhong)} ` : ''}phù hợp cho toàn bộ khoảng ${esc(b.ngayNhan)} → ${esc(b.ngayTra)}.${state.cungLoai ? ' Có thể bật “Cho phép loại phòng khác”.' : ''}</div>`;
    };
    App.modal({
      title: `Chuyển phiếu ${b.ma} sang phòng khác`, size: 'lg',
      body: `<dl class="detail-list" style="margin-bottom:16px">
          <dt>Mã phiếu</dt><dd>${esc(b.ma)}</dd>
          <dt>Khách</dt><dd>${esc(b.khach)}</dd>
          <dt>Phòng hiện tại</dt><dd>${esc(b.soPhong)} – ${esc(b.loaiPhong)} (${esc(b.maPhong)})</dd>
          <dt>Ngày nhận</dt><dd>${esc(b.ngayNhan)}</dd>
          <dt>Ngày trả</dt><dd>${esc(b.ngayTra)} (${b.soDem} đêm, ${b.soNguoi} khách)</dd>
        </dl>
        <div class="toolbar" style="margin-bottom:10px"><b>Phòng có thể chuyển</b><span class="muted">đủ sức chứa, không có khách, không trùng phiếu khác</span><span class="spacer"></span>
          <label class="field--check" style="display:inline-flex;gap:8px;align-items:center"><input type="checkbox" id="tf-all"> Cho phép loại phòng khác</label></div>
        <div id="tf-rooms"></div>
        <p class="muted" style="margin:12px 0 0;font-size:12.5px">Hệ thống cập nhật phòng của chính phiếu ${esc(b.ma)}: giữ nguyên mã phiếu, khách hàng, ngày nhận/trả, tiền cọc và trạng thái.</p>`,
      onOpen: (body) => {
        draw(body);
        body.addEventListener('change', async (e) => {
          if (e.target.name === '_moi') state.chon = e.target.value;
          if (e.target.id === 'tf-all') {
            state.cungLoai = !e.target.checked; state.chon = '';
            d = await App.run(() => load(state.cungLoai)) || d;
            draw(body);
          }
        });
      },
      footer: [
        { text: 'Hủy' },
        {
          text: 'Chuyển phòng', cls: 'btn--primary',
          onClick: async () => {
            if (!state.chon) return toast('Vui lòng chọn phòng mới.', 'error');
            const moi = d.phong.find((x) => x.phong.ma === state.chon).phong;
            const ok = await App.confirm({ html: `<p>Bạn có chắc chắn muốn chuyển phiếu <b>${esc(b.ma)}</b><br>từ phòng <b>${esc(b.soPhong)}</b> sang phòng <b>${esc(moi.so)}</b> không?</p>` },
              { title: 'Xác nhận chuyển phòng', ok: 'Chuyển phòng' });
            if (!ok) { toast('Đã hủy thao tác chuyển phòng.', 'info'); return; }
            const r = await App.run(() => Api.post('phieu/chuyenphong', { ma: b.ma, maPhong: moi.ma }),
              (x) => `Chuyển phòng thành công! Phiếu ${x.ma} – phòng cũ: ${b.soPhong}, phòng mới: ${x.soPhong}.`);
            if (r) { App.closeModal(); App.refresh(); }
          },
        },
      ],
    });
  },

  async checkin(b) {
    if (!(await App.confirm(`Xác nhận check-in cho ${b.khach}, phòng ${b.maPhong} – ${b.soPhong}?`, { ok: 'Check-in', title: 'Xác nhận check-in' }))) return;
    if (await App.run(() => Api.post('checkin', { ma: b.ma }), `Check-in phiếu ${b.ma} thành công!`)) App.refresh();
  },

  cancel(b) {
    App.modal({
      title: `Xác nhận hủy phiếu ${b.ma}`,
      body: `<form class="stack" id="cancel-form" style="gap:14px">
        <p style="margin:0">Bạn có chắc chắn muốn hủy đặt phòng của <b>${esc(b.khach)}</b>, phòng ${esc(b.soPhong)} (${esc(b.ngayNhan)} → ${esc(b.ngayTra)})? Thao tác không thể hoàn tác.</p>
        <label class="field"><span>Lý do hủy</span><input name="lyDo" placeholder="Khách yêu cầu hủy"></label>
        ${b.tienCoc ? `<label class="field field--check"><input type="checkbox" name="hoanCoc"> Hoàn lại tiền cọc ${money(b.tienCoc)}</label>` : ''}
      </form>`,
      footer: [
        { text: 'Không hủy' },
        {
          text: 'Hủy phiếu', cls: 'btn--danger-solid',
          onClick: async (body) => {
            const v = App.formValues(body.querySelector('#cancel-form'));
            if (await App.run(() => Api.post('huyphieu', { ma: b.ma, ...v }), `Đã hủy phiếu ${b.ma}.`)) { App.closeModal(); App.refresh(); }
          },
        },
      ],
    });
  },

  async addService(b) {
    const services = await App.run(() => Api.get('dichvu'));
    if (!services) return;
    const row = (s) => `<tr data-row="${esc(s.ma)}">
          <td style="width:38px"><input type="checkbox" data-pick="${esc(s.ma)}" aria-label="Chọn ${esc(s.ten)}"></td>
          <td><b>${esc(s.ten)}</b><span class="sub">${esc(vi(s.loai))}</span></td>
          <td class="num">${money(s.donGia)} / ${esc(vi(s.donVi))}</td>
          <td class="num"><input class="input" style="width:84px;text-align:right" value="1" data-svc="${esc(s.ma)}" inputmode="numeric" maxlength="3" disabled aria-label="Số lượng ${esc(s.ten)}"></td>
          <td class="num" data-sum="${esc(s.ma)}">—</td>
        </tr>`;
    App.modal({
      title: `Thêm dịch vụ – phiếu ${b.ma}`, size: 'lg',
      body: `<div id="svc-form">
        <div class="table-wrap"><table class="table">
          <thead><tr><th></th><th>Dịch vụ</th><th class="num">Đơn giá</th><th class="num">Số lượng</th><th class="num">Thành tiền</th></tr></thead>
          <tbody>${services.map(row).join('') || '<tr><td colspan="5" class="muted">Chưa có dịch vụ.</td></tr>'}</tbody>
        </table></div>
        <div class="invoice__line strong" style="margin-top:12px"><span>Tổng tiền dịch vụ dự kiến (<span id="svc-count">0</span> dịch vụ, chưa gồm VAT/giảm giá)</span><span id="svc-total">${money(0)}</span></div>
        <p class="muted" style="margin:8px 0 0">${b.maTrangThai === 0 ? 'Đặt trước – tính vào hóa đơn khi trả phòng.' : 'Khách đang ở.'}</p>
        <p id="svc-error" style="margin:8px 0 0;color:var(--danger)" hidden></p>
      </div>`,
      onOpen: (body) => {
        const price = Object.fromEntries(services.map((s) => [s.ma, Number(s.donGia) || 0]));
        const qtyOf = (ma) => body.querySelector(`[data-svc="${ma}"]`);
        const sync = () => {
          let total = 0, count = 0;
          body.querySelectorAll('[data-pick]').forEach((c) => {
            const ma = c.dataset.pick, q = qtyOf(ma), cell = body.querySelector(`[data-sum="${ma}"]`);
            q.disabled = !c.checked;
            const n = Number(q.value);
            if (c.checked && Number.isInteger(n) && n >= 1 && n <= 100) { total += price[ma] * n; count++; cell.textContent = money(price[ma] * n); }
            else { cell.textContent = '—'; if (c.checked) count++; }
          });
          body.querySelector('#svc-total').textContent = money(total);
          body.querySelector('#svc-count').textContent = count;
        };
        body.addEventListener('change', (e) => { if (e.target.matches('[data-pick]')) { sync(); if (e.target.checked) qtyOf(e.target.dataset.pick).select(); } });
        body.addEventListener('input', (e) => { if (e.target.matches('[data-svc]')) { e.target.classList.remove('is-invalid'); body.querySelector('#svc-error').hidden = true; sync(); } });
        sync();
      },
      footer: [
        { text: 'Hủy' },
        {
          text: 'Thêm dịch vụ', cls: 'btn--primary',
          onClick: async (body) => {
            const err = body.querySelector('#svc-error');
            const picked = [...body.querySelectorAll('[data-pick]:checked')].map((c) => ({ maDV: c.dataset.pick, q: body.querySelector(`[data-svc="${c.dataset.pick}"]`) }));
            body.querySelectorAll('[data-svc]').forEach((i) => i.classList.remove('is-invalid'));
            if (!picked.length) { err.textContent = 'Hãy chọn ít nhất một dịch vụ.'; err.hidden = false; return; }
            const bad = picked.filter((x) => !/^\d{1,3}$/.test(x.q.value) || Number(x.q.value) < 1 || Number(x.q.value) > 100);
            if (bad.length) { bad.forEach((x) => x.q.classList.add('is-invalid')); err.textContent = 'Số lượng mỗi dịch vụ phải từ 1 đến 100.'; err.hidden = false; bad[0].q.focus(); return; }
            err.hidden = true;
            const btn = document.querySelector('#modal-footer .btn--primary');
            if (btn) btn.disabled = true;
            // API ghidichvu nhan tung dich vu: gui lan luot, dung ngay khi co loi de bao dung dich vu da/ chua luu
            let done = 0;
            try {
              for (const x of picked) { await Api.post('ghidichvu', { ma: b.ma, maDV: x.maDV, soLuong: Number(x.q.value) }); done++; }
              toast(`Đã thêm ${done} dịch vụ vào phiếu.`);
              App.closeModal(); App.refresh();
            } catch (e) {
              toast(done ? `Đã lưu ${done}/${picked.length} dịch vụ, dịch vụ còn lại chưa được thêm: ${e.message}` : e.message, 'error');
              if (done) { App.closeModal(); App.refresh(); } else if (btn) btn.disabled = false;
            }
          },
        },
      ],
    });
  },

  /* ---------- Form dat phong moi ---------- */
  // preset: {maCN, ngayNhan, ngayTra, soNguoi} tu thanh "Kiem tra phong trong"
  async form(preset = {}) {
    const p = { maCN: App.branch, ngayNhan: todayIso(), ngayTra: todayIso(1), soNguoi: 2, ...preset };
    const [services, customers] = await Promise.all([Api.get('dichvu'), Api.get('khachhang')]);
    const state = { maKH: '', khachMoi: false, phong: '', rooms: [] };
    const body = App.modal({
      title: 'Đặt phòng mới', size: 'xl',
      body: `<form id="bk-form" class="booking" novalidate>
        <div class="stack">
          <div class="form-grid">
            <div class="form-section">Phòng & ngày ở</div>
            <label class="field"><span>Chi nhánh</span><select name="maCN">${App.branchOptions(p.maCN)}</select></label>
            <label class="field"><span>Loại phòng</span><select name="loai"><option value="">Tất cả</option>${['Standard', 'Deluxe', 'Suite', 'Villa'].map((t) => `<option>${t}</option>`).join('')}</select></label>
            <label class="field"><span>Ngày nhận phòng</span><input type="date" name="ngayNhan" min="${todayIso()}" value="${esc(p.ngayNhan)}"></label>
            <label class="field"><span>Ngày trả phòng</span><input type="date" name="ngayTra" min="${addDays(p.ngayNhan, 1)}" value="${esc(p.ngayTra)}"></label>
            <label class="field"><span>Số người</span><input name="soNguoi" min="1" max="20" value="${Number(p.soNguoi) || 2}" inputmode="numeric"></label>
            <div class="field"><span>Mùa du lịch</span><div class="muted">Tự động xác định theo ngày nhận phòng và lịch mùa trong năm.</div></div>
            <div class="field full"><span>Phòng có thể đặt</span><div id="bk-rooms"><div class="muted">Đang kiểm tra phòng trống…</div></div><input type="hidden" name="maPhong"></div>

            <div class="form-section">Khách hàng</div>
            <div class="full"><div class="segmented" id="bk-kind"><button type="button" class="is-active" data-kind="old">Khách cũ</button><button type="button" data-kind="new">Khách mới</button></div></div>
            <div class="full" id="bk-old">
              <label class="field"><span>Tìm khách (tên, SĐT, CCCD, mã)</span><input id="bk-search" name="_tim" autocomplete="off" placeholder="Gõ để tìm…"></label>
              <div class="suggest" id="bk-suggest" hidden></div>
              <p class="muted" id="bk-chosen" style="margin:8px 0 0">Chưa chọn khách.</p>
            </div>
            <div class="form-grid full" id="bk-new" hidden>${CustomerForm.fields()}</div>

            <div class="form-section">Dịch vụ đi kèm (đặt trước)</div>
            <div class="svc-list full">${services.map((s) => `<label class="svc-row"><span><b>${esc(s.ten)}</b> <span class="muted">${esc(vi(s.loai))}</span></span><span class="muted">${money(s.donGia)} / ${esc(vi(s.donVi))}</span><input type="number" min="0" max="100" value="0" data-svc="${s.ma}" aria-label="Số lượng ${esc(s.ten)}"></label>`).join('') || '<p class="muted">Chưa có dịch vụ.</p>'}</div>

            <div class="form-section">Ghi chú & đặt cọc</div>
            <label class="field full"><span>Ghi chú</span><input name="ghiChu" placeholder="Yêu cầu đặc biệt của khách"></label>
            <label class="field field--check"><input type="checkbox" name="coCoc"> Khách đặt cọc</label>
            <label class="field field--check" id="bk-now-wrap"><input type="checkbox" name="nhanPhongNgay"> Nhận phòng ngay</label>
            <label class="field" data-coc hidden><span>Số tiền cọc (đ)</span><input name="tienCoc" min="0" inputmode="numeric" autocomplete="off"></label>
            <label class="field" data-coc hidden><span>Phương thức cọc</span><select name="ptCoc">${options(PAY_METHODS, 'Chuyen khoan')}</select></label>
          </div>
        </div>
        <aside class="booking__summary panel">
          <div class="panel__head"><h2>Tạm tính</h2></div>
          <div class="panel__body" id="bk-sum"><p class="muted">Chọn phòng để xem giá.</p></div>
        </aside>
      </form>`,
      footer: [{ text: 'Hủy' }, { text: 'Đặt phòng', cls: 'btn--primary', onClick: () => submit() }],
    });
    const form = body.querySelector('#bk-form');
    const $f = (n) => form.querySelector(`[name="${n}"]`);

    const nights = () => daysBetween($f('ngayNhan').value, $f('ngayTra').value);
    const svcString = () => [...form.querySelectorAll('[data-svc]')].filter((i) => Number(i.value) > 0).map((i) => `${i.dataset.svc}:${Number(i.value)}`).join(',');

    // 1. Lay lich trong tu C++ (lyDoKhongDatDuoc) va ve o chon phong
    const loadRooms = debounce(async () => {
      const box = body.querySelector('#bk-rooms');
      if (!box) return; // form dat phong da dong
      const n = nights();
      if (!(n > 0)) { box.innerHTML = '<div class="muted">Ngày trả phải sau ngày nhận.</div>'; return; }
      try {
        state.rooms = await Api.get('phong/kiemtra', { maCN: $f('maCN').value, ngayNhan: $f('ngayNhan').value, soDem: n, soNguoi: Number($f('soNguoi').value) || 1 });
      } catch (e) { box.innerHTML = `<div class="muted">${esc(e.message)}</div>`; return; }
      const loai = $f('loai').value;
      // CHI hien thi phong thuc su dat duoc (backend da loai phong khong du suc chua / dang co khach / bao tri / trung lich)
      const list = state.rooms.filter((x) => x.datDuoc && (!loai || x.phong.loai === loai));
      if (!list.find((x) => x.phong.ma === state.phong)) { state.phong = ''; $f('maPhong').value = ''; }
      box.innerHTML = list.length ? `<div class="room-pick">${list.map((x) => `<label>
          <input type="radio" name="_phong" value="${x.phong.ma}" ${x.phong.ma === state.phong ? 'checked' : ''}>
          <b>Phòng ${esc(x.phong.so)} – ${esc(x.phong.loai)}</b>
          <span class="muted">${x.phong.sucChua} người · ${money(x.phong.gia)}/đêm</span>
          <span>${money(x.tienPhong)} / ${n} đêm</span>
        </label>`).join('')}</div>`
        : '<div class="room-empty"><b>Không có phòng phù hợp với yêu cầu của khách.</b><br>Vui lòng thay đổi số người, ngày đặt hoặc loại phòng.</div>';
      quote();
    }, 250);

    // 2. Bao gia tu C++ (chuoi Decorator) -> hien tong tien
    const quote = debounce(async () => {
      const sum = body.querySelector('#bk-sum');
      if (!sum) return;
      if (!state.phong) { sum.innerHTML = '<p class="muted">Chọn phòng để xem giá.</p>'; return; }
      try {
        const q = await Api.post('baogia', { maPhong: state.phong, ngayNhan: $f('ngayNhan').value, soDem: nights(), dichVu: svcString(), maKH: state.khachMoi ? '' : state.maKH });
        const room = state.rooms.find((x) => x.phong.ma === state.phong).phong;
        state.quote = q;
        if ($f('coCoc').checked && !$f('tienCoc').value) $f('tienCoc').value = q.cocGoiY;
        sum.innerHTML = `
          <p style="margin:0 0 4px"><b>Phòng ${esc(room.so)} – ${esc(room.loai)}</b></p>
          <p class="muted" style="margin:0 0 12px">${esc(vi(room.chiNhanh))}, ${nights()} đêm đến ${esc(q.ngayTra)}<br>Mùa ${esc(vi(q.mua))} (×${q.heSo})</p>
          ${q.canhBaoMua ? `<div class="alert alert--warn" style="margin:0 0 12px;font-size:12.5px">${esc(q.canhBaoMua)}</div>` : ''}
          <div class="invoice__line invoice__line--big"><span class="chip-room">Tiền phòng</span><span>${money(q.tienPhong)}</span></div>
          <div class="invoice__line invoice__line--big"><span class="chip-service">Dịch vụ</span><span>${money(q.tienDichVu)}</span></div>
          ${q.dichVu.map((d) => `<div class="invoice__line invoice__line--sub"><span>${esc(d.ten)} × ${d.soLuong}</span><span>${money(d.thanhTien)}</span></div>`).join('')}
          ${q.giam ? `<div class="invoice__line"><span>Giảm giá thành viên ${Math.round(q.tyLeGiam * 100)}%</span><span class="trend-up">−${money(q.giam)}</span></div>` : ''}
          <div class="invoice__line"><span>VAT 10%</span><span>${money(q.thue)}</span></div>
          <div class="invoice__total" style="margin:12px -20px -20px;border-radius:0 0 var(--radius-lg) var(--radius-lg)"><span>Tổng dự kiến</span><span>${money(q.tong)}</span></div>
          <p class="muted" style="margin:28px 0 0;font-size:12px">Công thức: tạm tính → giảm giá → VAT → tổng (giống hóa đơn). Cọc tối đa ${money(q.tranCoc)} (tiền phòng đã gồm giảm giá và VAT).</p>`;
      } catch (e) { sum.innerHTML = `<p class="muted">${esc(e.message)}</p>`; }
    }, 250);

    const syncNow = () => { body.querySelector('#bk-now-wrap').hidden = $f('ngayNhan').value !== todayIso(); };
    form.addEventListener('change', (e) => {
      const n = e.target.name;
      if (n === 'ngayNhan') {
        if ($f('ngayTra').value <= $f('ngayNhan').value) $f('ngayTra').value = addDays($f('ngayNhan').value, 1);
        $f('ngayTra').min = addDays($f('ngayNhan').value, 1);
        syncNow();
      }
      if (['maCN', 'loai', 'ngayNhan', 'ngayTra', 'soNguoi'].includes(n)) loadRooms();
      if (n === '_phong') { state.phong = e.target.value; $f('maPhong').value = state.phong; quote(); }
      if (n === 'coCoc') { form.querySelectorAll('[data-coc]').forEach((x) => (x.hidden = !e.target.checked)); if (e.target.checked && state.quote) $f('tienCoc').value = state.quote.cocGoiY; }
    });
    form.addEventListener('input', (e) => { if (e.target.dataset.svc) quote(); });

    // Khach cu / khach moi
    body.querySelector('#bk-kind').addEventListener('click', (e) => {
      const b = e.target.closest('[data-kind]');
      if (!b) return;
      state.khachMoi = b.dataset.kind === 'new';
      body.querySelectorAll('#bk-kind button').forEach((x) => x.classList.toggle('is-active', x === b));
      body.querySelector('#bk-old').hidden = state.khachMoi;
      body.querySelector('#bk-new').hidden = !state.khachMoi;
      quote();
    });
    const sug = body.querySelector('#bk-suggest');
    body.querySelector('#bk-search').addEventListener('input', (e) => {
      const q = e.target.value.trim().toLowerCase();
      const hits = q ? customers.filter((k) => `${k.ma} ${k.hoTen} ${k.sdt} ${k.cccd}`.toLowerCase().includes(q)).slice(0, 8) : [];
      sug.hidden = !hits.length;
      sug.innerHTML = hits.map((k) => `<button type="button" data-kh="${k.ma}"><b>${esc(k.hoTen)}</b> <span class="muted">${esc(k.ma)} – ${esc(k.sdt)} – ${TIERS[k.maHang]}</span></button>`).join('');
    });
    sug.addEventListener('click', (e) => {
      const b = e.target.closest('[data-kh]');
      if (!b) return;
      const k = customers.find((x) => x.ma === b.dataset.kh);
      state.maKH = k.ma;
      body.querySelector('#bk-chosen').innerHTML = `Đã chọn: <b>${esc(k.hoTen)}</b> (${esc(k.ma)}), ${esc(k.sdt)} – hạng ${TIERS[k.maHang]}, giảm ${Math.round(k.giamGia * 100)}%`;
      sug.hidden = true;
      body.querySelector('#bk-search').value = '';
      quote();
    });

    // 3. Gui dat phong
    const submit = async () => {
      const v = App.formValues(form);
      const errs = {};
      if (v.ngayNhan < todayIso()) errs.ngayNhan = 'Ngày nhận không được ở quá khứ.';
      const n = nights();
      if (!(n > 0)) errs.ngayTra = 'Ngày trả phải sau ngày nhận.';
      else if (n > 60) errs.ngayTra = 'Tối đa 60 đêm.';
      if (!(Number(v.soNguoi) >= 1 && Number(v.soNguoi) <= 20)) errs.soNguoi = 'Số khách từ 1 đến 20.';
      if (!state.phong) errs.maPhong = 'Chọn một phòng còn trống.';
      if (state.khachMoi) Object.assign(errs, CustomerForm.validate(v));
      else if (!state.maKH) errs._tim = 'Chọn khách hàng hoặc chuyển sang “Khách mới”.';
      if (v.coCoc === '1' && !(Number(v.tienCoc) > 0)) errs.tienCoc = 'Nhập số tiền cọc.';
      if (v.coCoc === '1' && state.quote && Number(v.tienCoc) > state.quote.tranCoc) errs.tienCoc = `Tiền cọc tối đa ${money(state.quote.tranCoc)} (tổng tiền phòng phải trả).`;
      if (App.showErrors(form, errs)) return;
      const payload = {
        maPhong: state.phong, ngayNhan: v.ngayNhan, soDem: n, soNguoi: v.soNguoi, ghiChu: v.ghiChu,
        dichVu: svcString(), nhanPhongNgay: v.nhanPhongNgay === '1' && v.ngayNhan === todayIso(),
        tienCoc: v.coCoc === '1' ? v.tienCoc : 0, ptCoc: v.coCoc === '1' ? v.ptCoc : '',
      };
      if (state.khachMoi) Object.assign(payload, {
        hoTen: v.hoTen, sdt: v.sdt, cccd: v.cccd, email: v.email, quocTich: v.quocTich,
        gioiTinh: v.gioiTinh, diaChiThuongTru: v.diaChiThuongTru,
      });
      else payload.maKH = state.maKH;
      const room = state.rooms.find((x) => x.phong.ma === state.phong).phong;
      const khach = state.khachMoi ? v.hoTen : customers.find((k) => k.ma === state.maKH)?.hoTen;
      if (!(await App.confirm({ html: `<p>Xác nhận đặt <b>phòng ${esc(room.so)} – ${esc(room.loai)}</b> (${esc(branchName(room.chiNhanh))})<br>cho khách <b>${esc(khach || '')}</b>, ${esc(v.ngayNhan.split('-').reverse().join('/'))} → ${esc(v.ngayTra.split('-').reverse().join('/'))} (${n} đêm)?</p>${state.quote ? `<p class="muted">Tổng dự kiến ${money(state.quote.tong)}</p>` : ''}` },
        { title: 'Xác nhận đặt phòng', ok: 'Đặt phòng' }))) return;
      const d = await App.run(() => Api.post('datphong', payload),
        (p) => `Đặt phòng thành công! Mã phiếu ${p.ma}${p.maTrangThai === 1 ? ', khách đã nhận phòng' : ''}.`);
      if (d) { App.closeModal(); App.refresh(); }
    };

    syncNow();
    loadRooms();
  },
});
