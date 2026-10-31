/* =====================================================================
   services.js - Quan ly dich vu (mau hong)
   Lop C++: DichVu <- AmThuc / Spa / TourTuyen. API: /api/dichvu, /api/dichvu/them|sua|xoa
   ===================================================================== */
App.register('services', {
  sub: 'Ẩm thực, spa và tour cho khách lưu trú',
  async render(el) {
    const list = await Api.get('dichvu');
    const admin = App.isAdmin();
    el.innerHTML = `<div class="page">
      <section class="stats">
        ${['Am thuc', 'Spa', 'Tour'].map((l, i) => statCard({ label: vi(l), tone: ['gold', 'green', 'blue'][i], value: list.filter((s) => s.loai === l).length, unit: ' dịch vụ', ic: ['services', 'services', 'rooms'][i], foot: 'Đang cung cấp' })).join('')}
        ${statCard({ label: 'Tổng dịch vụ', value: list.length, ic: 'services', accent: true, tone: 'brown', foot: 'Giá chốt khi ghi vào phiếu' })}
      </section>
      <div class="toolbar">
        <input class="input input--search" id="s-q" type="search" placeholder="Tìm tên, mã dịch vụ…">
        <select class="select" id="s-loai"><option value="">Mọi loại</option><option value="Am thuc">Ẩm thực</option><option value="Spa">Spa</option><option value="Tour">Tour</option></select>
        <span class="spacer"></span>
        ${admin ? `<button class="btn btn--primary" id="s-add" type="button">${icon('plus', 16)} Thêm dịch vụ</button>` : ''}
      </div>
      <div class="panel" id="s-table"></div></div>`;
    const page = el.firstElementChild;
    const table = new DataTable(page.querySelector('#s-table'), {
      rows: list,
      searchText: (r) => `${r.ma} ${r.ten} ${r.moTa}`,
      columns: [
        { key: 'ma', label: 'Mã dịch vụ', render: (r) => `<b>${esc(r.ma)}</b>` },
        { key: 'ten', label: 'Tên dịch vụ' },
        { key: 'loai', label: 'Loại', render: (r) => `<span class="tag">${esc(vi(r.loai))}</span>` },
        { key: 'moTa', label: 'Mô tả', render: (r) => `<span class="muted">${esc(r.loai === 'Spa' ? r.thoiLuong + ' phút / lần' : r.loai === 'Tour' ? `${r.diemDen}, ${r.soNgay} ngày` : r.moTa)}</span>` },
        { key: 'donGia', label: 'Giá', align: 'num', render: (r) => money(r.donGia) },
        { key: 'donVi', label: 'Đơn vị', render: (r) => esc(vi(r.donVi)) },
        { key: 'tt', label: 'Trạng thái', sort: false, render: () => badge('Đang cung cấp', 'ok') },
        ...(admin ? [{ key: 'act', label: 'Thao tác', sort: false, align: 'num', render: (r) => `<div class="actions"><button class="btn btn--sm" data-edit="${r.ma}" title="Sửa">${icon('edit', 15)}</button><button class="btn btn--sm btn--danger" data-del="${r.ma}" title="Xóa">${icon('trash', 15)}</button></div>` }] : []),
      ],
    });
    const apply = () => {
      const l = page.querySelector('#s-loai').value;
      table.setFilter((r) => !l || r.loai === l);
      table.setQuery(page.querySelector('#s-q').value);
    };
    page.querySelector('#s-q').addEventListener('input', debounce(apply, 200));
    page.querySelector('#s-loai').addEventListener('change', apply);
    page.querySelector('#s-add')?.addEventListener('click', () => this.form(null));
    page.addEventListener('click', async (e) => {
      const ed = e.target.closest('[data-edit]'), del = e.target.closest('[data-del]');
      if (ed) this.form(list.find((s) => s.ma === ed.dataset.edit));
      if (del) {
        const s = list.find((x) => x.ma === del.dataset.del);
        if (await App.confirm(`Bạn có chắc chắn muốn xóa dịch vụ ${s.ma} – ${s.ten}? Hóa đơn cũ không bị ảnh hưởng.`, { ok: 'Xóa dịch vụ', danger: true }))
          if (await App.run(() => Api.post('dichvu/xoa', { ma: s.ma }), `Đã xóa dịch vụ ${s.ma}.`)) App.refresh();
      }
    });
  },

  form(s) {
    const isEdit = !!s;
    const extra = (loai) => ({
      'Am thuc': '<label class="field"><span>Loại bữa ăn</span><input name="loaiBuaAn"></label>',
      Spa: '<label class="field"><span>Thời lượng (phút)</span><input name="thoiLuong" type="number" min="15" max="480" value="60"></label>',
      Tour: '<label class="field"><span>Điểm đến</span><input name="diemDen" data-kieu="tenrieng"></label><label class="field"><span>Số ngày</span><input name="soNgay" type="number" min="1" max="15" value="1"></label>',
    }[loai]);
    App.modal({
      title: isEdit ? `Sửa dịch vụ ${s.ma}` : 'Thêm dịch vụ',
      body: `<form class="form-grid" id="svc-form" novalidate>
        ${isEdit ? '' : `<label class="field full"><span>Loại dịch vụ</span><select name="loai"><option value="Am thuc">Ẩm thực</option><option value="Spa">Spa</option><option value="Tour">Tour</option></select></label>`}
        <label class="field"><span>Tên dịch vụ</span><input name="ten" data-kieu="tenrieng" value="${esc(s?.ten || '')}"></label>
        <label class="field"><span>Giá (đ / ${isEdit ? esc(vi(s.donVi)) : 'đơn vị'})</span><input name="donGia" type="number" min="1000" step="1000" value="${s?.donGia || ''}"></label>
        <div class="form-grid full" id="svc-extra" style="grid-column:1/-1">${isEdit ? '' : extra('Am thuc')}</div>
        ${isEdit ? '<p class="muted full" style="margin:0">Theo lớp C++, dịch vụ chỉ sửa được tên và giá. Lần sử dụng cũ giữ nguyên giá đã chốt.</p>' : ''}
      </form>`,
      onOpen: (b) => b.querySelector('[name=loai]')?.addEventListener('change', (e) => { b.querySelector('#svc-extra').innerHTML = extra(e.target.value); }),
      footer: [
        { text: 'Hủy' },
        {
          text: isEdit ? 'Lưu thay đổi' : 'Thêm dịch vụ', cls: 'btn--primary',
          onClick: async (b) => {
            const form = b.querySelector('#svc-form');
            const v = App.formValues(form);
            const errs = {};
            if (!v.ten) errs.ten = 'Nhập tên dịch vụ.';
            if (!(Number(v.donGia) >= 1000)) errs.donGia = 'Giá tối thiểu 1.000 đ.';
            if ('loaiBuaAn' in v && !v.loaiBuaAn) errs.loaiBuaAn = 'Nhập loại bữa ăn.';
            if ('diemDen' in v && !v.diemDen) errs.diemDen = 'Nhập điểm đến.';
            if (App.showErrors(form, errs)) return;
            if (isEdit) v.ma = s.ma;
            const d = await App.run(() => Api.post(isEdit ? 'dichvu/sua' : 'dichvu/them', v),
              (x) => (isEdit ? `Cập nhật dịch vụ ${x.ma} thành công!` : `Thêm dịch vụ ${x.ma} thành công!`));
            if (d) { App.closeModal(); App.refresh(); }
          },
        },
      ],
    });
  },
});
