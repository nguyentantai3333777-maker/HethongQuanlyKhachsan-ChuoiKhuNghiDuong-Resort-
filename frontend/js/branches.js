/* =====================================================================
   branches.js - Chi nhanh (chi ADMIN)
   API: GET /api/chinhanh, GET /api/chinhanh/kiemtraxoa, POST /api/chinhanh/them, POST /api/chinhanh/xoa
   - Them chi nhanh: dien form -> modal XAC NHAN -> moi goi API (khong them ngay khi bam).
   - Xoa chi nhanh: backend kiem tra quan he du lieu (phong, nhan vien, lich su hoa don) truoc;
     con du lieu -> modal chi hien ly do, khong cho xoa. Hop le -> modal xac nhan -> xoa.
   ===================================================================== */
App.register('branches', {
  sub: 'Hệ thống chi nhánh Ocean Pearl Resort – thêm, xem và xóa chi nhánh',
  IMGS: ['hero.jpg', 'banner.jpg', 'room-villa.jpg', 'room-suite.jpg'],

  async render(el) {
    const list = await Api.get('chinhanh');
    App.cache.branches = list;
    el.innerHTML = `<div class="page">
      <div class="toolbar">
        <div class="scope-title" style="margin:0"><h2>${list.length} chi nhánh</h2><span class="muted">Danh sách lấy từ dữ liệu hệ thống</span></div>
        <span class="spacer"></span>
        <button class="btn btn--primary" id="cn-add" type="button">${icon('plus', 16)} Thêm chi nhánh</button>
      </div>
      <section class="bl-branches">${list.map((b, i) => this.card(b, i)).join('') || '<div class="panel empty-state">Chưa có chi nhánh nào.</div>'}</section></div>`;
    const page = el.firstElementChild;
    page.querySelector('#cn-add').addEventListener('click', () => this.form());
    page.addEventListener('click', (e) => {
      const t = e.target.closest('[data-cn-act]');
      if (!t) return;
      const b = list.find((x) => x.ma === t.dataset.ma);
      if (t.dataset.cnAct === 'rooms') { App.setBranch(b.ma); location.hash = '#/rooms'; }
      if (t.dataset.cnAct === 'bills') { App.setBranch(b.ma); location.hash = '#/payments?tab=invoices'; }
      if (t.dataset.cnAct === 'del') this.remove(b);
    });
  },

  card(b, i) {
    const img = this.IMGS[i % this.IMGS.length];
    return `<article class="bl-branch">
      <div class="bl-branch__img" style="background-image:linear-gradient(180deg,rgba(14,53,87,0),rgba(14,53,87,.35)),url('assets/images/${img}'),url('assets/images/fallback/hero.svg')"></div>
      <div class="bl-branch__body">
        <h3>${esc(branchName(b.ten))} <span class="muted" style="font:500 13px var(--font)">${esc(b.ma)}</span></h3>
        <p>${icon('pin')}${esc(b.diaChi)}</p>
        <p>${icon('phone')}${esc(b.hotline)}</p>
        <div class="bl-branch__stats">
          <div><b>${b.soPhong}</b><span>phòng · ${b.phongTrong} trống</span></div>
          <div><b>${b.soNhanVien}</b><span>nhân viên</span></div>
          <div><b>${b.soHoaDon}</b><span>hóa đơn</span></div>
        </div>
        <div class="bl-branch__foot">
          <button class="btn btn--sm" type="button" data-cn-act="rooms" data-ma="${esc(b.ma)}">${icon('rooms', 15)} Phòng</button>
          <button class="btn btn--sm" type="button" data-cn-act="bills" data-ma="${esc(b.ma)}">${icon('wallet', 15)} Hóa đơn</button>
          <button class="btn btn--sm btn--danger" type="button" data-cn-act="del" data-ma="${esc(b.ma)}" title="${b.xoaDuoc ? 'Xóa chi nhánh' : 'Chi nhánh còn dữ liệu liên quan'}">${icon('trash', 15)} Xóa</button>
        </div>
      </div>
    </article>`;
  },

  form() {
    App.modal({
      title: 'Thêm chi nhánh',
      body: `<form class="form-grid" id="cn-form" novalidate>
        <label class="field full"><span>Tên chi nhánh</span><input name="ten" data-kieu="chu" autocomplete="off"></label>
        <label class="field full"><span>Địa chỉ</span><input name="diaChi" autocomplete="off"></label>
        <label class="field"><span>Hotline</span><input name="hotline" inputmode="numeric" pattern="0[0-9]{9}"></label>
      </form>`,
      footer: [
        { text: 'Hủy' },
        {
          text: 'Tiếp tục', cls: 'btn--primary',
          onClick: async (body) => {
            const form = body.querySelector('#cn-form');
            const v = App.formValues(form);
            const errs = {};
            if (!v.ten) errs.ten = 'Nhập tên chi nhánh.';
            else if (!/^(?=.*\p{L})[\p{L} ]{2,40}$/u.test(v.ten)) errs.ten = 'Tên chi nhánh chỉ gồm chữ cái và khoảng trắng (2–40 ký tự), không dùng số.';
            else if (App.cache.branches.some((b) => vi(b.ten).toLowerCase() === v.ten.toLowerCase() || b.ten.toLowerCase() === v.ten.toLowerCase())) errs.ten = 'Tên chi nhánh đã tồn tại.';
            if (!/^[\p{L} ]{5,120}$/u.test(v.diaChi || '')) errs.diaChi = 'Địa chỉ chỉ gồm chữ cái và khoảng trắng (5–120 ký tự).';
            if (!/^0\d{9}$/.test(v.hotline)) errs.hotline = 'Hotline gồm đúng 10 chữ số, bắt đầu bằng 0.';
            if (App.showErrors(form, errs)) return;
            // Xac nhan truoc khi ghi
            const ok = await App.confirm({ html: `<p>Thêm chi nhánh <b>“${esc(branchName(v.ten))}”</b>.</p><p><b>Bạn có chắc chắn muốn xác nhận thực hiện hay không?</b></p><p class="muted">${esc(v.diaChi)} · Hotline ${esc(v.hotline)}</p>` },
              { title: 'Xác nhận thêm chi nhánh', ok: 'Thêm chi nhánh' });
            if (!ok) { toast('Đã hủy thao tác. Không thêm chi nhánh.', 'info'); return; }
            if (await App.run(() => Api.post('chinhanh/them', v), 'Đã thêm chi nhánh thành công!')) {
              App.cache.branches = await Api.get('chinhanh');
              App.closeModal(); App.refresh();
            }
          },
        },
      ],
    });
  },

  async remove(b) {
    // Kiem tra quan he du lieu o backend TRUOC khi hoi xac nhan
    const chk = await App.run(() => Api.get('chinhanh/kiemtraxoa', { ma: b.ma }));
    if (!chk) return;
    const ok = await App.confirm({ html: `<p>Xóa chi nhánh <b>“${esc(branchName(b.ten))}”</b>.</p><p><b>Bạn có chắc chắn muốn xác nhận thực hiện hay không?</b></p>` }, {
      title: 'Xác nhận xóa chi nhánh', ok: 'Xóa chi nhánh', danger: true,
      note: chk.xoaDuoc ? 'Thao tác không thể hoàn tác.' : 'Không thể xóa vì chi nhánh còn dữ liệu liên quan:',
      reasons: chk.xoaDuoc ? null : chk.lyDo,
    });
    if (!ok) return;
    if (await App.run(() => Api.post('chinhanh/xoa', { ma: b.ma }), 'Đã xóa chi nhánh thành công!')) {
      App.cache.branches = await Api.get('chinhanh');
      App.refresh();
    }
  },
});
