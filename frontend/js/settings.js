/* =====================================================================
   settings.js - Cai dat (mau xam xanh) - chi ADMIN
   API: /api/taikhoan, /api/taikhoan/them|xoa|doimatkhau  (QuanLyTaiKhoan)
   ===================================================================== */
App.register('settings', {
  sub: 'Tài khoản đăng nhập và mật khẩu',
  async render(el) {
    const [accounts, staff] = await Promise.all([Api.get('taikhoan'), Api.get('nhanvien')]);
    const noAccount = staff.filter((n) => !n.taiKhoan);
    el.innerHTML = `<div class="page grid grid--2-1">
      <div class="panel">
        <div class="panel__head"><h2>Tài khoản đăng nhập</h2><button class="btn btn--primary btn--sm" id="acc-add" type="button" ${noAccount.length ? '' : 'disabled title="Mọi nhân viên đã có tài khoản"'}>${icon('plus', 15)} Tạo tài khoản</button></div>
        <div class="panel__body" style="padding:12px 0 0"><div id="acc-table"></div></div>
      </div>
      <div class="stack">
        <div class="panel">
          <div class="panel__head"><h2>Đổi mật khẩu</h2></div>
          <form class="panel__body stack" id="pw-form" style="gap:14px" novalidate>
            <label class="field"><span>Mật khẩu hiện tại</span><input type="password" name="matKhauCu" autocomplete="current-password"></label>
            <label class="field"><span>Mật khẩu mới (6–32 ký tự)</span><input type="password" name="matKhauMoi" autocomplete="new-password"></label>
            <label class="field"><span>Nhập lại mật khẩu mới</span><input type="password" name="nhapLai" autocomplete="new-password"></label>
            <button class="btn btn--primary" type="submit">Đổi mật khẩu</button>
          </form>
        </div>
        <div class="panel">
          <div class="panel__head"><h2>Về Ocean Pearl Resort</h2></div>
          <div class="panel__body">
            <p style="margin:0 0 12px;line-height:1.7;color:#3b4855">Ocean Pearl Resort mang đến không gian nghỉ dưỡng bên biển với phòng nghỉ tiện nghi, villa hướng biển và dịch vụ tận tâm. Tận hưởng trải nghiệm thư giãn trọn vẹn giữa biển xanh và nắng vàng tại Nha Trang và Quy Nhơn.</p>
            <p class="muted" style="margin:0;font-size:13.5px">Hệ thống quản lý phục vụ vận hành hai chi nhánh, hỗ trợ đặt phòng trực tuyến và chăm sóc khách hàng chuyên nghiệp.</p>
          </div>
        </div>
      </div></div>`;
    const page = el.firstElementChild;
    new DataTable(page.querySelector('#acc-table'), {
      rows: accounts,
      columns: [
        { key: 'username', label: 'Tên đăng nhập', render: (r) => `<b>${esc(r.username)}</b>${r.laToi ? ' <span class="tag">Bạn</span>' : ''}` },
        { key: 'quyen', label: 'Quyền', render: (r) => (r.quyen === 'ADMIN' ? badge('Quản lý (ADMIN)', 'warn') : badge('Lễ tân (USER)', 'info')) },
        { key: 'nhanVien', label: 'Nhân viên', render: (r) => `${esc(r.nhanVien)}<span class="sub">${esc(r.maNV)} – ${esc(vi(r.chucVu))}</span>` },
        { key: 'act', label: '', sort: false, align: 'num', render: (r) => (r.laToi ? '' : `<button class="btn btn--sm btn--danger" data-del="${esc(r.username)}">${icon('trash', 15)}</button>`) },
      ],
    });
    page.addEventListener('click', async (e) => {
      const b = e.target.closest('[data-del]');
      if (!b) return;
      if (await App.confirm(`Bạn có chắc chắn muốn xóa tài khoản “${b.dataset.del}”?`, { ok: 'Xóa tài khoản', danger: true }))
        if (await App.run(() => Api.post('taikhoan/xoa', { username: b.dataset.del }), `Đã xóa tài khoản ${b.dataset.del}.`)) App.refresh();
    });

    page.querySelector('#pw-form').addEventListener('submit', async (e) => {
      e.preventDefault();
      const f = e.target, v = App.formValues(f), errs = {};
      if (!v.matKhauCu) errs.matKhauCu = 'Nhập mật khẩu hiện tại.';
      if (!/^\S{6,32}$/.test(v.matKhauMoi)) errs.matKhauMoi = '6–32 ký tự, không có khoảng trắng.';
      if (v.matKhauMoi !== v.nhapLai) errs.nhapLai = 'Mật khẩu nhập lại không khớp.';
      if (App.showErrors(f, errs)) return;
      if (await App.run(() => Api.post('taikhoan/doimatkhau', v), 'Đổi mật khẩu thành công!')) f.reset();
    });

    page.querySelector('#acc-add').addEventListener('click', () => {
      App.modal({
        title: 'Tạo tài khoản',
        body: `<form class="form-grid" id="acc-form" novalidate>
          <label class="field full"><span>Nhân viên</span><select name="maNV">${noAccount.map((n) => `<option value="${n.ma}">${esc(n.hoTen)} (${n.ma}) – ${esc(vi(n.chucVu))}</option>`).join('')}</select></label>
          <label class="field"><span>Tên đăng nhập</span><input name="username" autocomplete="off"></label>
          <label class="field"><span>Mật khẩu</span><input name="password" type="password" autocomplete="new-password"></label>
          <label class="field full"><span>Quyền</span><select name="quyen"><option value="USER">Lễ tân (USER)</option><option value="ADMIN">Quản lý (ADMIN)</option></select></label>
        </form>`,
        footer: [
          { text: 'Hủy' },
          {
            text: 'Tạo tài khoản', cls: 'btn--primary',
            onClick: async (b) => {
              const f = b.querySelector('#acc-form'), v = App.formValues(f), errs = {};
              if (!/^[A-Za-z0-9_.]{3,20}$/.test(v.username)) errs.username = '3–20 ký tự: chữ, số, _ hoặc .';
              if (!/^\S{6,32}$/.test(v.password)) errs.password = '6–32 ký tự, không có khoảng trắng.';
              if (App.showErrors(f, errs)) return;
              if (await App.run(() => Api.post('taikhoan/them', v), `Đã tạo tài khoản ${v.username}.`)) { App.closeModal(); App.refresh(); }
            },
          },
        ],
      });
    });
  },
});
