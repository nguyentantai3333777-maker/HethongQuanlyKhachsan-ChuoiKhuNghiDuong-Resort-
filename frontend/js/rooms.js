/* =====================================================================
   rooms.js - Quan ly phong (mau nau)
   API: /api/phong, /api/phong/chitiet, /api/phong/them|sua|xoa, /api/phong/trangthai
   Trang thai phong do State Pattern trong C++ quyet dinh; web chi goi hanh dong.
   ===================================================================== */
App.register('rooms', {
  sub: 'Chọn chi nhánh để xem phòng, trạng thái và khách đang ở',
  view: 'table',

  async render(el, params) {
    // Chon chi nhanh truoc: chi tai va hien thi phong cua MOT chi nhanh (khong tron cac chi nhanh)
    if (params.q && /^P\d+$/i.test(params.q)) { // tim nhanh theo ma phong -> mo dung chi nhanh cua phong do
      const all = await Api.get('phong');
      const hit = all.find((r) => r.ma.toUpperCase() === params.q.toUpperCase());
      if (hit) App.setBranch(hit.maCN);
    }
    const maCN = App.branch;
    const cn = App.branchInfo(maCN);
    const rooms = maCN ? await Api.get('phong', { maCN }) : [];
    const admin = App.isAdmin();
    document.getElementById('page-title').textContent = cn ? `Phòng – ${branchName(cn.ten)}` : 'Phòng';
    el.innerHTML = `<div class="page">
      ${App.branchTabs(maCN, { label: 'Chọn chi nhánh' })}
      <div class="toolbar">
        <input class="input input--search" id="r-q" type="search" placeholder="Tìm mã phòng, số phòng, khách đang ở…" value="${esc(params.q || '')}">
        <select class="select" id="r-st"><option value="">Mọi trạng thái</option>${ROOM_STATUS.map((s, i) => `<option value="${i}"${params.st === String(i) ? ' selected' : ''}>${s.t}</option>`).join('')}</select>
        <select class="select" id="r-type"><option value="">Mọi loại phòng</option>${['Standard', 'Deluxe', 'Suite', 'Villa'].map((t) => `<option>${t}</option>`).join('')}</select>
        <span class="spacer"></span>
        <div class="segmented" role="group" aria-label="Kiểu hiển thị">
          <button type="button" data-view="table" title="Dạng bảng">${icon('table', 16)} Bảng</button>
          <button type="button" data-view="cards" title="Dạng thẻ">${icon('cards', 16)} Thẻ</button>
        </div>
        ${admin ? `<button class="btn btn--primary" id="r-add" type="button">${icon('plus', 16)} Thêm phòng</button>` : ''}
      </div>
      <div class="panel" id="r-table"></div>
      <div class="room-band room-band--admin" id="r-cards" hidden></div></div>`;
    const page = el.firstElementChild; // gan su kien vao phan tu moi cua trang (tranh lap su kien tren #content)

    const table = new DataTable(el.querySelector('#r-table'), {
      rows: rooms, pageSize: 12,
      searchText: (r) => `${r.ma} ${r.so} ${r.khachHienTai} ${r.loai}`,
      empty: cn ? 'Không có phòng phù hợp với bộ lọc.' : 'Hệ thống chưa có chi nhánh nào.',
      columns: [
        { key: 'ma', label: 'Mã phòng', render: (r) => `<b>${esc(r.ma)}</b><span class="sub">Số ${esc(r.so)}</span>` },
        { key: 'loai', label: 'Loại phòng', render: (r) => `${esc(r.loai)}<span class="sub">${r.sucChua} khách, ${r.dienTich} m²</span>` },
        { key: 'tang', label: 'Tầng', align: 'num' },
        { key: 'gia', label: 'Giá / đêm', align: 'num', render: (r) => money(r.gia) },
        { key: 'maTrangThai', label: 'Trạng thái', render: (r) => roomBadge(r.maTrangThai) },
        { key: 'khachHienTai', label: 'Khách đang ở', render: (r) => this.guestCell(r) },
        { key: 'act', label: 'Thao tác', sort: false, align: 'num', render: (r) => this.actions(r) },
      ],
    });

    const applyFilter = () => {
      const st = el.querySelector('#r-st').value, type = el.querySelector('#r-type').value;
      table.setFilter((r) => (st === '' || r.maTrangThai === Number(st)) && (!type || r.loai === type));
      table.setQuery(el.querySelector('#r-q').value);
      this.renderCards(el, table.visibleRows());
    };
    el.querySelector('#r-q').addEventListener('input', debounce(applyFilter, 200));
    ['#r-st', '#r-type'].forEach((s) => el.querySelector(s).addEventListener('change', applyFilter));
    App.bindBranchTabs(page, () => App.refresh());

    const setView = (v) => {
      this.view = v;
      el.querySelectorAll('[data-view]').forEach((b) => b.classList.toggle('is-active', b.dataset.view === v));
      el.querySelector('#r-table').hidden = v !== 'table';
      el.querySelector('#r-cards').hidden = v !== 'cards';
    };
    el.querySelectorAll('[data-view]').forEach((b) => b.addEventListener('click', () => setView(b.dataset.view)));
    setView(this.view);
    applyFilter();

    // Uy quyen su kien cho cac nut thao tac (ca bang lan the)
    page.addEventListener('click', (e) => {
      const b = e.target.closest('[data-act]');
      if (!b) return;
      const room = rooms.find((r) => r.ma === b.dataset.ma);
      ({ view: () => { if (room && window.confirm(`Bạn có muốn xem đầy đủ thông tin phòng ${room.ma} không?`)) this.detail(room); }, status: () => this.statusDialog(room), edit: () => this.form(room), del: () => this.remove(room) })[b.dataset.act]();
    });
    el.querySelector('#r-add')?.addEventListener('click', () => this.form(null));
  },

  guestCell(r) {
    if (r.khachDangO) return `${esc(r.khachDangO)}<span class="sub">Trả phòng ${esc(r.ngayTraHienTai)}</span>`;
    if (r.khachHienTai) return `<span class="muted">${esc(r.khachHienTai)} (đến hôm nay)</span>`;
    if (r.maTrangThai === 4) return `<span class="muted">${esc(r.ghiChuTrangThai)}</span>`;
    if (r.datSapToi) return `<span class="muted">Đặt ${esc(r.datSapToi)}</span>`;
    return '<span class="muted">—</span>';
  },

  actions(r) {
    const admin = App.isAdmin();
    return `<div class="actions">
      <button class="btn btn--sm" data-act="view" data-ma="${r.ma}" title="Xem chi tiết">${icon('eye', 15)}</button>
      <button class="btn btn--sm" data-act="status" data-ma="${r.ma}">Trạng thái</button>
      ${admin ? `<button class="btn btn--sm" data-act="edit" data-ma="${r.ma}" title="Sửa">${icon('edit', 15)}</button>
      <button class="btn btn--sm btn--danger" data-act="del" data-ma="${r.ma}" title="Xóa">${icon('trash', 15)}</button>` : ''}
    </div>`;
  },

  // The phong kieu anh mau (Eagle): anh phong + so phong, than trang, gia ben phai, dai nau phia sau.
  // Anh phong: mapping chung trong core.js (RoomImages), thieu anh thi dung anh minh hoa.
  renderCards(el, list) {
    const box = el.querySelector('#r-cards');
    const img = (r) => RoomImages.css(RoomImages.forRoom(r.loai, r.ma));
    const guest = (r) => (r.khachDangO ? `Khách: <b>${esc(r.khachDangO)}</b>, trả ${esc(r.ngayTraHienTai)}${r.quaHan ? ` <span class="tag" style="background:var(--danger-bg);color:var(--danger)">quá hạn ${r.soNgayQuaHan} ngày</span>` : ''}`
      : r.maTrangThai === 4 ? esc(r.ghiChuTrangThai) : r.datSapToi ? `Đặt ${esc(r.datSapToi)}` : '<span class="muted">Chưa có khách</span>');
    box.innerHTML = list.length ? `<div class="room-grid">${list.map((r) => `
      <article class="room-card room-card--admin">
        <div class="room-card__img" style="${img(r)}">${roomBadge(r.maTrangThai)}<span class="room-card__no">${esc(r.so)}</span></div>
        <div class="room-card__body">
          <h3 class="room-card__title">${esc(r.loai)}</h3>
          <div class="room-card__meta">${esc(r.ma)} · tầng ${r.tang}</div>
          <ul class="room-card__specs" style="margin-top:12px">
            <li>${icon('area', 15)}${r.dienTich} m²</li><li>${icon('guests', 15)}Tối đa ${r.sucChua} khách</li>
            <li>${icon('wallet', 15)}<span><b>${money(r.gia)}</b> / đêm</span></li>
          </ul>
          <div class="room-card__guest">${guest(r)}</div>
          <div class="room-card__foot">${this.actions(r).replace('<div class="actions">', '').replace(/<\/div>\s*$/, '')}</div>
        </div>
      </article>`).join('')}</div>` : '<div class="panel empty-state">Không có phòng phù hợp với bộ lọc.</div>';
  },

  async detail(room) {
    const d = await App.run(() => Api.get('phong/chitiet', { ma: room.ma }));
    if (!d) return;
    const p = d.phong;
    App.modal({
      title: `Phòng ${p.so} – ${branchName(p.chiNhanh)}`, size: 'lg',
      body: `
        <dl class="detail-list">
          <dt>Mã phòng</dt><dd>${esc(p.ma)}</dd>
          <dt>Loại phòng</dt><dd>${esc(p.loai)}, tầng ${p.tang}</dd>
          <dt>Giá cơ bản</dt><dd>${money(p.gia)} / đêm</dd>
          <dt>Sức chứa</dt><dd>${p.sucChua} khách, ${p.dienTich} m²</dd>
          <dt>Trạng thái</dt><dd>${roomBadge(p.maTrangThai)} <span class="muted">${esc(p.ghiChuTrangThai)}</span></dd>
          ${p.thongTinRieng.map((x) => `<dt>${esc(x.k)}</dt><dd>${esc(vi(x.v))}</dd>`).join('')}
          <dt>Tiện nghi</dt><dd>${p.tienNghi.map((t) => `<span class="tag">${esc(t)}</span>`).join(' ')}</dd>
        </dl>
        <h3 class="section-title">Giá theo mùa</h3>
        <div class="table-wrap"><table class="table"><thead><tr><th>Mùa</th><th class="num">Hệ số</th><th class="num">1 đêm</th><th class="num">7 đêm (giảm 10%)</th></tr></thead>
        <tbody>${d.giaTheoMua.map((g) => `<tr><td>${esc(vi(g.mua))}</td><td class="num">×${g.heSo}</td><td class="num">${money(g.motDem)}</td><td class="num">${money(g.bayDem)}</td></tr>`).join('')}</tbody></table></div>
        <h3 class="section-title">Lịch đặt phòng</h3>
        ${d.lichDat.length ? `<div class="table-wrap"><table class="table"><thead><tr><th>Phiếu</th><th>Khách</th><th>Ngày ở</th><th>Trạng thái</th></tr></thead><tbody>
          ${d.lichDat.map((b) => `<tr><td><b>${esc(b.ma)}</b></td><td>${esc(b.khach)}</td><td>${esc(b.ngayNhan)} → ${esc(b.ngayTra)}</td><td>${bookingBadge(b.maTrangThai)}</td></tr>`).join('')}
        </tbody></table></div>` : '<p class="muted">Chưa có ai đặt – phòng trống mọi ngày.</p>'}`,
      footer: [{ text: 'Đóng' }],
    });
  },

  statusDialog(room) {
    const st = room.maTrangThai;
    let body = `<p style="margin-top:0">Phòng <b>${esc(room.ma)} – ${esc(room.so)}</b> đang ở trạng thái ${roomBadge(st)}.</p>`;
    const footer = [{ text: 'Đóng' }];
    const act = (hanhDong, msg) => async (b) => {
      const extra = hanhDong === 'baotri' ? { lyDo: b.querySelector('[name=lyDo]').value.trim() } : {};
      if (hanhDong === 'baotri' && !extra.lyDo) return App.showErrors(b.querySelector('form'), { lyDo: 'Nhập lý do bảo trì.' });
      const ok = await App.run(() => Api.post('phong/trangthai', { ma: room.ma, hanhDong, ...extra }), msg);
      if (ok) { App.closeModal(); App.refresh(); }
    };
    if (st === 0 || st === 3) {
      body += `<form class="field" onsubmit="return false"><label class="field"><span>Lý do bảo trì (nếu chuyển bảo trì)</span><input name="lyDo"></label></form>`;
      footer.push({ text: 'Chuyển bảo trì', onClick: act('baotri', `Phòng ${room.ma} đã chuyển sang bảo trì.`) });
    }
    if (st === 3) footer.push({ text: 'Hoàn tất dọn phòng', cls: 'btn--primary', onClick: act('dondep', `Phòng ${room.ma} đã dọn xong, sẵn sàng đón khách.`) });
    if (st === 4) footer.push({ text: 'Hoàn tất bảo trì', cls: 'btn--primary', onClick: act('hoantat', `Phòng ${room.ma} đã bảo trì xong.`) });
    if (st === 1 || st === 2) body += '<p class="muted">Trạng thái này thay đổi qua nghiệp vụ check-in / check-out / hủy đặt phòng.</p>';
    App.modal({ title: 'Cập nhật trạng thái phòng', body, footer });
  },

  form(room) {
    const isEdit = !!room;
    const r = room || { loai: 'Standard', rieng: {}, maCN: App.branch };
    const f = r.rieng || {};
    const typeFields = (loai) => ({
      Standard: `<label class="field full"><span>Hướng nhìn</span><input name="huongNhin" value="${esc(f.huongNhin || '')}"></label>`,
      Deluxe: `<label class="field"><span>Hướng nhìn</span><input name="huongNhin" value="${esc(f.huongNhin || '')}"></label>
               <label class="field field--check" style="align-self:end"><input type="checkbox" name="banCong" ${f.banCong !== false ? 'checked' : ''}> Có ban công</label>`,
            Suite: `<label class="field"><span>Số phòng ngủ</span><input name="soPhongNgu" type="number" min="1" value="${f.soPhongNgu || 1}"></label>
              <label class="field"><span>Phí buffet sáng / đêm</span><input name="phiBuffet" type="number" min="1" value="${f.phiBuffet ?? 300000}"></label>
              <label class="field field--check"><input type="checkbox" name="phongKhach" ${f.phongKhach !== false ? 'checked' : ''}> Có phòng khách</label>`,
      Villa: `<label class="field"><span>Số phòng ngủ</span><input name="soPhongNgu" type="number" min="1" value="${f.soPhongNgu || 3}"></label>
              <label class="field"><span>Diện tích sân vườn (m²)</span><input name="sanVuon" type="number" min="1" value="${f.sanVuon ?? 100}"></label>
              <label class="field"><span>Phí quản gia / đêm</span><input name="phiQuanGia" type="number" min="1" value="${f.phiQuanGia ?? 500000}"></label>
              <label class="field field--check" style="align-self:end"><input type="checkbox" name="hoBoi" ${f.hoBoi !== false ? 'checked' : ''}> Có hồ bơi riêng</label>`,
    }[loai]);

    App.modal({
      title: isEdit ? `Sửa phòng ${r.ma}` : 'Thêm phòng mới', size: 'lg',
      body: `<form class="form-grid" id="room-form" novalidate>
        ${isEdit ? `<input type="hidden" name="ma" value="${esc(r.ma)}">` : ''}
        <label class="field"><span>Chi nhánh</span><select name="maCN" disabled>${App.branchOptions(r.maCN)}</select></label>
        <label class="field"><span>Loại phòng</span><select name="loai" ${isEdit ? 'disabled' : ''}>${['Standard', 'Deluxe', 'Suite', 'Villa'].map((t) => `<option${t === r.loai ? ' selected' : ''}>${t}</option>`).join('')}</select></label>
        <label class="field"><span>Số phòng</span><input name="so" inputmode="numeric" value="${esc(r.so || '')}"></label>
        <label class="field"><span>Tầng</span><input name="tang" type="number" min="1" max="5" step="1" value="${r.tang ?? 1}"></label>
        <label class="field"><span>Giá cơ bản / đêm (đ)</span><input name="gia" type="number" min="100000" step="10000" value="${r.gia || ''}"></label>
        <label class="field"><span>Sức chứa (khách)</span><input name="sucChua" type="number" min="1" max="20" value="${r.sucChua || 2}"></label>
        <label class="field"><span>Diện tích (m²)</span><input name="dienTich" type="number" min="5" step="0.5" value="${r.dienTich || ''}"></label>
        <div class="form-section">Thông tin riêng theo loại phòng</div>
        <div class="form-grid full" id="type-fields" style="grid-column:1/-1">${typeFields(r.loai)}</div>
        ${isEdit && room.soPhieuHoatDong ? `<p class="muted full" style="margin:0">Phòng còn ${room.soPhieuHoatDong} phiếu đặt nên không đổi được giá, sức chứa và phụ phí.</p>` : ''}
      </form>`,
      onOpen: (b) => {
        b.querySelector('[name=loai]').addEventListener('change', (e) => { b.querySelector('#type-fields').innerHTML = typeFields(e.target.value); });
      },
      footer: [
        { text: 'Hủy' },
        {
          text: isEdit ? 'Lưu thay đổi' : 'Thêm phòng', cls: 'btn--primary',
          onClick: async (b) => {
            const form = b.querySelector('#room-form');
            const v = App.formValues(form);
            v.maCN = r.maCN; // phong luon thuoc chi nhanh dang chon (hien ro tren form)
            if (isEdit) v.loai = r.loai;
            const errs = {};
            if (!/^\d{1,9}$/.test(v.so) || Number(v.so) === 0) errs.so = 'Số phòng là số nguyên dương.';
            if (!/^\d+$/.test(v.tang) || !(Number(v.tang) >= 1 && Number(v.tang) <= 5)) errs.tang = 'Tầng phải là số nguyên từ 1 đến 5.';
            if (!(Number(v.gia) >= 100000)) errs.gia = 'Giá tối thiểu 100.000 đ.';
            const sc = Number(v.sucChua), loai = v.loai || (r && r.loai);
            if (loai === 'Standard' && !(sc >= 1 && sc <= 2)) errs.sucChua = 'Standard: 1–2 khách.';
            else if ((loai === 'Deluxe' || loai === 'Suite') && !(sc >= 2 && sc <= 6)) errs.sucChua = loai + ': 2–6 khách.';
            else if (loai === 'Villa' && !(sc >= 8 && sc <= 20)) errs.sucChua = 'Villa: 8–20 khách.';
            else if (!(sc >= 1 && sc <= 20)) errs.sucChua = 'Từ 1 đến 20 khách.';
            if (!(Number(v.dienTich) >= 5)) errs.dienTich = 'Tối thiểu 5 m².';
            if ('phiBuffet' in v && !(Number(v.phiBuffet) > 0)) errs.phiBuffet = 'Phí phải lớn hơn 0.';
            if ('phiQuanGia' in v && !(Number(v.phiQuanGia) > 0)) errs.phiQuanGia = 'Phí phải lớn hơn 0.';
            if ('sanVuon' in v && !(Number(v.sanVuon) > 0)) errs.sanVuon = 'Diện tích phải lớn hơn 0.';
            if ('huongNhin' in v && !v.huongNhin) errs.huongNhin = 'Nhập hướng nhìn.';
            if (App.showErrors(form, errs)) return;
            const ok = await App.confirm({ html: isEdit
              ? `<p>Bạn có chắc chắn muốn lưu thay đổi cho phòng <b>${esc(r.ma)} – số ${esc(v.so)}</b>?</p>`
              : `<p>Bạn có chắc chắn muốn thêm phòng <b>số ${esc(v.so)} (${esc(v.loai)})</b> vào <b>${esc(branchName(App.branchInfo(r.maCN)?.ten))}</b>?</p>` },
            { title: isEdit ? 'Xác nhận sửa phòng' : 'Xác nhận thêm phòng', ok: isEdit ? 'Lưu thay đổi' : 'Thêm phòng' });
            if (!ok) return;
            const d = await App.run(() => Api.post(isEdit ? 'phong/sua' : 'phong/them', v),
              (p) => (isEdit ? `Cập nhật phòng ${p.ma} thành công!` : `Thêm phòng ${p.ma} thành công!`));
            if (d) { App.closeModal(); App.refresh(); }
          },
        },
      ],
    });
  },

  async remove(room) {
    if (!(await App.confirm({ html: `<p>Bạn có chắc chắn muốn xóa phòng <b>${esc(room.ma)} – số ${esc(room.so)}</b> của <b>${esc(branchName(room.chiNhanh))}</b>?</p>` },
      { ok: 'Xóa phòng', danger: true, title: 'Xác nhận xóa phòng', note: room.soPhieuHoatDong ? `Phòng còn ${room.soPhieuHoatDong} phiếu đặt / khách đang ở – hệ thống sẽ từ chối xóa.` : 'Lịch sử phiếu cũ của phòng vẫn được giữ lại.' }))) return;
    if (await App.run(() => Api.post('phong/xoa', { ma: room.ma }), `Đã xóa phòng ${room.ma}.`)) App.refresh();
  },
});
