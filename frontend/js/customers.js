/* =====================================================================
   customers.js - Quan ly khach hang (mau xam ghi)
   API: /api/khachhang, /api/khachhang/chitiet, /api/khachhang/them|sua
   (C++ khong co ham xoa khach hang -> web khong co nut xoa, giu dung nghiep vu)
   ===================================================================== */
const CustomerForm = {
  // Kiem tra phia trinh duyet - cung quy tac voi InputHelper.cpp (laSoDienThoai, laCCCD, laEmail)
  validate(v) {
    const e = {};
    if (!/^[\p{L} ]{2,50}$/u.test(v.hoTen || '')) e.hoTen = 'Họ tên chỉ gồm chữ cái và khoảng trắng.';
    if (!/^0\d{9}$/.test(v.sdt || '')) e.sdt = 'Số điện thoại gồm 10 chữ số, bắt đầu bằng 0.';
    if (!/^(\d{9}|\d{12}|[A-Za-z][A-Za-z0-9]{7,8})$/.test(v.cccd || '')) e.cccd = 'CMND/CCCD gồm 9 hoặc 12 chữ số.';
    if (v.email && !/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(v.email)) e.email = 'Email không hợp lệ.';
    if (v.quocTich && !/^[\p{L} ]+$/u.test(v.quocTich)) e.quocTich = 'Quốc tịch chỉ gồm chữ cái.';
    if (v.gioiTinh && !['Nam', 'Nữ', 'Khác'].includes(v.gioiTinh)) e.gioiTinh = 'Giới tính không hợp lệ.';
    if (v.diaChiThuongTru && !/^[\p{L} ]{5,120}$/u.test(v.diaChiThuongTru)) e.diaChiThuongTru = 'Địa chỉ chỉ gồm chữ cái và khoảng trắng.';
    return e;
  },
  fields(k = {}, withTier = false) {
    return `
      <label class="field"><span>Họ tên</span><input name="hoTen" value="${esc(k.hoTen || '')}"></label>
      <label class="field"><span>Số điện thoại</span><input name="sdt" inputmode="tel" value="${esc(k.sdt || '')}"></label>
      <label class="field"><span>CCCD / CMND / Hộ chiếu</span><input name="cccd" inputmode="numeric" value="${esc(k.cccd || '')}"></label>
      <label class="field"><span>Giới tính</span><select name="gioiTinh"><option value="">Chưa cập nhật</option>${['Nam', 'Nữ', 'Khác'].map((x) => `<option${x === k.gioiTinh ? ' selected' : ''}>${x}</option>`).join('')}</select></label>
      <label class="field full"><span>Địa chỉ thường trú</span><input name="diaChiThuongTru" value="${esc(k.diaChiThuongTru || '')}"></label>
      <label class="field"><span>Email</span><input name="email" type="email" value="${esc(k.email || '')}" placeholder="Không bắt buộc"></label>
      <label class="field"><span>Quốc tịch</span><input name="quocTich" value="${esc(k.quocTich || '')}" placeholder="Viet Nam"></label>
      ${withTier ? `<label class="field"><span>Hạng thành viên</span><select name="maHang">${TIERS.map((t, i) => `<option value="${i}"${i === k.maHang ? ' selected' : ''}>${t} (giảm ${[0, 5, 10, 15][i]}%)</option>`).join('')}</select></label>` : ''}`;
  },
};

App.register('customers', {
  sub: 'Thông tin khách, hạng thành viên và lịch sử lưu trú',
  async render(el, params) {
    const list = await Api.get('khachhang');
    el.innerHTML = `<div class="page">
      <div class="toolbar">
        <input class="input input--search" id="c-q" type="search" placeholder="Tìm tên, SĐT, CCCD, mã khách…" value="${esc(params.q || '')}">
        <select class="select" id="c-cn" aria-label="Lọc theo chi nhánh">${App.branchOptions('', true)}</select>
        <select class="select" id="c-tier"><option value="">Mọi hạng</option>${TIERS.map((t, i) => `<option value="${i}">${t}</option>`).join('')}</select>
        <select class="select" id="c-st"><option value="">Mọi trạng thái</option><option value="Dang luu tru">Đang lưu trú</option><option value="Sap den">Sắp đến</option><option value="Binh thuong">Bình thường</option></select>
        <span class="spacer"></span>
        <button class="btn btn--primary" id="c-add" type="button">${icon('plus', 16)} Thêm khách hàng</button>
      </div>
      <div class="panel" id="c-table"></div></div>`;
    const page = el.firstElementChild;
    const table = new DataTable(page.querySelector('#c-table'), {
      rows: list,
      searchText: (r) => `${r.ma} ${r.hoTen} ${r.sdt} ${r.cccd} ${r.email}`,
      empty: 'Không tìm thấy khách hàng phù hợp.',
      columns: [
        { key: 'ma', label: 'Mã KH', render: (r) => `<b>${esc(r.ma)}</b>` },
        { key: 'hoTen', label: 'Họ tên', render: (r) => `${esc(r.hoTen)}<span class="sub">${esc(r.email || '—')}</span>` },
        { key: 'sdt', label: 'Số điện thoại' },
        { key: 'cccd', label: 'CCCD / Passport' },
        { key: 'quocTich', label: 'Quốc tịch', render: (r) => esc(vi(r.quocTich)) },
        { key: 'soLanLuuTru', label: 'Lưu trú', align: 'num' },
        { key: 'tongChiTieu', label: 'Tổng chi tiêu', align: 'num', render: (r) => money(r.tongChiTieu) },
        { key: 'maHang', label: 'Hạng', render: (r) => `<span class="tag">${TIERS[r.maHang]}</span><span class="sub">${r.diem} điểm</span>` },
        { key: 'trangThai', label: 'Trạng thái', render: (r) => badge(vi(r.trangThai), { 'Dang luu tru': 'info', 'Sap den': 'warn' }[r.trangThai] || 'neutral') },
        {
          key: 'act', label: 'Thao tác', sort: false, align: 'num',
          render: (r) => `<div class="actions"><button class="btn btn--sm" data-view="${r.ma}" title="Xem chi tiết">${icon('eye', 15)}</button><button class="btn btn--sm" data-edit="${r.ma}" title="Sửa">${icon('edit', 15)}</button></div>`,
        },
        {
          key: 'dsChiNhanh', label: 'Chi nhánh', sort: false,
          render: (r) => ((r.dsChiNhanh || []).map((m) => `<span class="tag">${esc(vi(App.branchInfo(m)?.ten || m))}</span>`).join(' ') || '<span class="muted">—</span>'),
        },
      ],
    });
    const apply = () => {
      const tier = page.querySelector('#c-tier').value, st = page.querySelector('#c-st').value, cn = page.querySelector('#c-cn').value;
      // Loc theo chi nhanh: uu tien maChiNhanh da gan (CSV); sau do dsChiNhanh (lich su dat)
      // Khop Console: chi hien khach thuoc CN dang chon khi loc
      table.setFilter((r) => {
        if (tier !== '' && r.maHang !== Number(tier)) return false;
        if (st && r.trangThai !== st) return false;
        if (cn) {
          const maGan = r.maChiNhanh || '';
          const ds = r.dsChiNhanh || [];
          if (maGan) return maGan === cn;
          return ds.includes(cn);
        }
        return true;
      });
      table.setQuery(page.querySelector('#c-q').value);
    };
    page.querySelector('#c-q').addEventListener('input', debounce(apply, 200));
    page.querySelector('#c-tier').addEventListener('change', apply);
    page.querySelector('#c-st').addEventListener('change', apply);
    page.querySelector('#c-cn').addEventListener('change', apply);
    apply();

    page.addEventListener('click', (e) => {
      const v = e.target.closest('[data-view]'), ed = e.target.closest('[data-edit]');
      if (v) { const k = list.find((x) => x.ma === v.dataset.view); if (k && window.confirm(`Bạn có muốn xem đầy đủ thông tin khách hàng ${k.ma} – ${k.hoTen} không?`)) this.detail(v.dataset.view); }
      if (ed) this.form(list.find((k) => k.ma === ed.dataset.edit));
    });
    page.querySelector('#c-add').addEventListener('click', () => this.form(null));
  },

  async detail(ma) {
    const d = await App.run(() => Api.get('khachhang/chitiet', { ma }));
    if (!d) return;
    const k = d.khach;
    App.modal({
      title: k.hoTen, size: 'lg',
      body: `
        <dl class="detail-list">
          <dt>Mã khách hàng</dt><dd>${esc(k.ma)}</dd>
          <dt>Liên hệ</dt><dd>${esc(k.sdt)}${k.email ? ' – ' + esc(k.email) : ''}</dd>
          <dt>Giới tính</dt><dd>${esc(k.gioiTinh || 'Chưa cập nhật')}</dd>
          <dt>CCCD / CMND / Hộ chiếu</dt><dd>${esc(k.cccd)} (${esc(vi(k.quocTich))})</dd>
          <dt>Địa chỉ thường trú</dt><dd>${esc(k.diaChiThuongTru || 'Chưa cập nhật')}</dd>
          <dt>Hạng thành viên</dt><dd>${TIERS[k.maHang]} – ${k.diem} điểm, giảm ${Math.round(k.giamGia * 100)}% khi thanh toán</dd>
          <dt>Lưu trú</dt><dd>${k.soLanLuuTru} lần trả phòng / ${k.soLanDat} lần đặt</dd>
          <dt>Tổng chi tiêu</dt><dd>${money(k.tongChiTieu)}</dd>
        </dl>
        <h3 class="section-title">Lịch sử đặt & trả phòng</h3>
        ${d.lichSu.length ? `<div class="table-wrap"><table class="table"><thead><tr><th>Phiếu</th><th>Chi nhánh / phòng</th><th>Đặt lúc</th><th>Nhận phòng</th><th>Trả phòng</th><th>Trạng thái</th></tr></thead><tbody>
          ${d.lichSu.map((b) => `<tr><td><b>${esc(b.ma)}</b></td><td>${esc(branchName(b.chiNhanh))}<span class="sub">${esc(b.maPhong)} – ${esc(b.soPhong)} ${esc(b.loaiPhong)}</span></td>
            <td>${esc(b.tgDat)}</td><td>${b.tgCheckIn ? esc(b.tgCheckIn) : `<span class="muted">${esc(b.ngayNhan)}</span>`}</td>
            <td>${b.tgCheckOut ? esc(b.tgCheckOut) : `<span class="muted">${esc(b.ngayTra)}</span>`}</td><td>${bookingBadge(b.maTrangThai)}</td></tr>`).join('')}
        </tbody></table></div>` : '<p class="muted">Khách chưa đặt phòng lần nào.</p>'}`,
      footer: [{ text: 'Đóng' }],
    });
  },

  form(k) {
    const isEdit = !!k;
    App.modal({
      title: isEdit ? `Sửa khách hàng ${k.ma}` : 'Thêm khách hàng',
      size: 'lg',
      body: `<form class="form-grid" id="cust-form" novalidate>${CustomerForm.fields(k || {}, isEdit)}</form>`,
      footer: [
        { text: 'Hủy' },
        {
          text: isEdit ? 'Lưu thay đổi' : 'Thêm khách hàng', cls: 'btn--primary',
          onClick: async (b) => {
            const form = b.querySelector('#cust-form');
            const v = App.formValues(form);
            if (App.showErrors(form, CustomerForm.validate(v))) return;
            if (isEdit && !(await App.confirm({ html: `<p>Bạn có chắc chắn muốn lưu thay đổi thông tin khách <b>${esc(k.ma)} – ${esc(v.hoTen)}</b>?</p>` }, { title: 'Xác nhận cập nhật khách hàng', ok: 'Lưu thay đổi' }))) return;
            if (isEdit) v.ma = k.ma;
            const d = await App.run(() => Api.post(isEdit ? 'khachhang/sua' : 'khachhang/them', v),
              (x) => (isEdit ? `Cập nhật khách hàng ${x.ma} thành công!` : `Thêm khách hàng ${x.ma} thành công!`));
            if (d) { App.closeModal(); App.refresh(); }
          },
        },
      ],
    });
  },
});
