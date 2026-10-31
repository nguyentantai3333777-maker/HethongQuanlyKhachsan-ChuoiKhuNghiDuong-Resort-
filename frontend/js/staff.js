/* =====================================================================
   staff.js - Quan ly / xem nhan vien
   - ADMIN: day du (luong, them/sua/nghi viec, loc, sap xep)
   - USER: chi xem dong nghiep cung chi nhanh, khong luong / khong thao tac quan tri
   API: GET /api/nhanvien (USER + ADMIN), them|sua|nghiviec (ADMIN)
   ===================================================================== */
App.register('staff', {
  sub: 'Nhân sự theo chi nhánh',
  async render(el) {
    const isAdmin = App.isAdmin();
    try { App.cache.branches = await Api.get('chinhanh'); } catch (e) { /* giu cache */ }

    let list = [];
    try {
      list = await Api.get('nhanvien', isAdmin ? { caNguoiDaNghi: 1 } : {});
    } catch (e) {
      el.innerHTML = `<div class="page"><div class="panel empty-state">${esc(e.message || 'Không tải được danh sách nhân viên.')}</div></div>`;
      return;
    }

    el.innerHTML = `<div class="page page--staff">
      ${isAdmin ? `<section class="stats" id="st-branches"></section>` : ''}
      <div class="toolbar toolbar--staff">
        <input class="input input--search" id="st-q" type="search" placeholder="Tìm mã NV hoặc họ tên…">
        ${isAdmin ? `
          <select class="select" id="st-cv"><option value="">Mọi chức vụ</option></select>
          <select class="select" id="st-cn">${App.branchOptions('', true)}</select>
          <select class="select" id="st-tt">
            <option value="1">Đang làm việc</option>
            <option value="0">Đã nghỉ việc</option>
            <option value="">Tất cả trạng thái</option>
          </select>
        ` : `
          <span class="muted st-scope">Chi nhánh: <b>${esc(branchName(App.user?.nhanVien?.chiNhanh || ''))}</b></span>
        `}
        <span class="spacer"></span>
        ${isAdmin ? `
          <a class="btn" href="#/branches">${icon('building', 16)} Quản lý chi nhánh</a>
          <button class="btn btn--primary" id="st-add" type="button">${icon('plus', 16)} Thêm nhân viên</button>
        ` : ''}
      </div>
      <div class="panel panel--staff-table">
        <div class="panel__head panel__head--staff">
          <div>
            <h2>${isAdmin ? 'Danh sách nhân viên' : 'Đồng nghiệp tại chi nhánh'}</h2>
            <span class="muted" id="st-count"></span>
          </div>
        </div>
        <div class="panel__body panel__body--table" id="st-table"></div>
      </div>
    </div>`;

    const page = el.firstElementChild;

    if (isAdmin && page.querySelector('#st-branches')) {
      page.querySelector('#st-branches').innerHTML = App.cache.branches.map((b, i) =>
        statCard({
          tone: ['brown', 'blue', 'green'][i % 3],
          label: branchName(b.ten),
          value: b.soNhanVien,
          unit: ' nhân viên',
          ic: 'staff',
          foot: `${b.soPhong} phòng – hotline ${esc(b.hotline)}`,
        })).join('');
    }

    // Chức vụ unique cho filter admin
    if (isAdmin) {
      const cvs = [...new Set(list.map((r) => r.chucVu).filter(Boolean))].sort((a, b) => String(a).localeCompare(String(b), 'vi'));
      const cvSel = page.querySelector('#st-cv');
      cvs.forEach((c) => {
        const o = document.createElement('option');
        o.value = c;
        o.textContent = vi(c);
        cvSel.appendChild(o);
      });
    }

    const columns = isAdmin
      ? [
          { key: 'ma', label: 'Mã NV', render: (r) => `<b class="st-ma">${esc(r.ma)}</b>` },
          { key: 'hoTen', label: 'Họ tên', render: (r) => `<span class="st-name">${esc(r.hoTen)}</span>` },
          { key: 'chucVu', label: 'Chức vụ', render: (r) => `<span class="st-role">${esc(vi(r.chucVu))}</span>` },
          {
            key: 'luong',
            label: 'Lương cơ bản',
            align: 'num',
            value: (r) => r.luong || 0,
            render: (r) => `<span class="st-salary">${money(r.luong)}</span>`,
          },
          {
            key: 'act',
            label: 'Thao tác',
            sort: false,
            align: 'num',
            render: (r) => {
              if (r.chucVu === 'Kênh Đặt Phòng Online' || r.chucVu === 'Dat phong online') {
                return '<span class="muted">Kênh hệ thống</span>';
              }
              const view = `<button type="button" class="btn-icon" data-view="${esc(r.ma)}" title="Xem chi tiết" aria-label="Xem chi tiết">${icon('eye', 16)}</button>`;
              if (!r.dangLam) return `<div class="actions actions--staff">${view}</div>`;
              const edit = `<button type="button" class="btn-icon" data-edit="${esc(r.ma)}" title="Chỉnh sửa" aria-label="Chỉnh sửa">${icon('edit', 16)}</button>`;
              const off = (App.user?.nhanVien?.ma && r.ma === App.user.nhanVien.ma)
                ? ''
                : `<button type="button" class="btn btn--sm btn--outline-danger" data-off="${esc(r.ma)}" title="Cho nghỉ việc">Nghỉ việc</button>`;
              return `<div class="actions actions--staff">${view}${edit}${off}</div>`;
            },
          },
        ]
      : [
          { key: 'ma', label: 'Mã NV', render: (r) => `<b class="st-ma">${esc(r.ma)}</b>` },
          { key: 'hoTen', label: 'Họ tên', render: (r) => `<span class="st-name">${esc(r.hoTen)}</span>` },
          { key: 'chucVu', label: 'Chức vụ', render: (r) => `<span class="st-role">${esc(vi(r.chucVu))}</span>` },
          {
            key: 'trangThai',
            label: 'Trạng thái',
            sort: false,
            render: (r) => (r.tamNghi ? badge('Tạm nghỉ', 'warn') : badge('Đang làm việc', 'ok')),
          },
          {
            key: 'act',
            label: 'Thao tác',
            sort: false,
            align: 'num',
            render: (r) => `<div class="actions actions--staff">
              <button type="button" class="btn-icon" data-view="${esc(r.ma)}" title="Xem thông tin" aria-label="Xem thông tin">${icon('eye', 16)}</button>
            </div>`,
          },
        ];

    const table = new DataTable(page.querySelector('#st-table'), {
      rows: list,
      pageSize: 10,
      empty: isAdmin ? 'Không có nhân viên phù hợp.' : 'Không có đồng nghiệp nào trong chi nhánh.',
      searchText: (r) => `${r.ma} ${r.hoTen} ${r.chucVu || ''}`,
      columns,
    });

    const updateCount = () => {
      const n = table.visibleRows().length;
      const elc = page.querySelector('#st-count');
      if (elc) elc.textContent = n ? `${n} nhân viên` : 'Không có kết quả';
    };

    const apply = () => {
      if (isAdmin) {
        const cn = page.querySelector('#st-cn')?.value || '';
        const tt = page.querySelector('#st-tt')?.value ?? '1';
        const cv = page.querySelector('#st-cv')?.value || '';
        table.setFilter((r) =>
          (!cn || r.maCN === cn)
          && (tt === '' || r.dangLam === (tt === '1'))
          && (!cv || r.chucVu === cv));
      } else {
        table.setFilter(null);
      }
      table.setQuery(page.querySelector('#st-q').value);
      updateCount();
    };

    page.querySelector('#st-q').addEventListener('input', debounce(apply, 200));
    if (isAdmin) {
      page.querySelector('#st-cn')?.addEventListener('change', apply);
      page.querySelector('#st-tt')?.addEventListener('change', apply);
      page.querySelector('#st-cv')?.addEventListener('change', apply);
    }
    // DataTable re-renders on sort/page — hook count after each render
    const origRender = table.render.bind(table);
    table.render = () => { origRender(); updateCount(); };
    apply();

    if (isAdmin) {
      page.querySelector('#st-add')?.addEventListener('click', () => this.form(null));
    }

    page.addEventListener('click', async (e) => {
      const vw = e.target.closest('[data-view]');
      const ed = e.target.closest('[data-edit]');
      const off = e.target.closest('[data-off]');
      if (vw) {
        const n = list.find((x) => x.ma === vw.dataset.view);
        if (n) this.detail(n, isAdmin);
      }
      if (ed && isAdmin) this.form(list.find((n) => n.ma === ed.dataset.edit));
      if (off && isAdmin) {
        const n = list.find((x) => x.ma === off.dataset.off);
        if (!n) return;
        if (await App.confirm(
          `Bạn có chắc chắn muốn cho ${n.hoTen} (${n.ma}) nghỉ việc?${n.taiKhoan ? ` Tài khoản “${n.taiKhoan}” sẽ bị xóa.` : ''} Phiếu và hóa đơn cũ vẫn giữ tên nhân viên.`,
          { ok: 'Cho nghỉ việc', danger: true, title: 'Xác nhận cho nghỉ việc' },
        )) {
          if (await App.run(() => Api.post('nhanvien/nghiviec', { ma: n.ma }), `${n.hoTen} đã nghỉ việc.`)) App.refresh();
        }
      }
    });
  },

  detail(n, isAdmin = App.isAdmin()) {
    if (!n) return;
    if (!isAdmin) {
      App.modal({
        title: `Thông tin nhân viên ${n.ma}`,
        body: `<dl class="detail-list">
          <dt>Mã nhân viên</dt><dd>${esc(n.ma)}</dd>
          <dt>Họ và tên</dt><dd>${esc(n.hoTen)}</dd>
          <dt>Chức vụ</dt><dd>${esc(vi(n.chucVu))}</dd>
          <dt>Chi nhánh</dt><dd>${esc(branchName(n.chiNhanh))}</dd>
          <dt>Trạng thái</dt><dd>${n.tamNghi ? badge('Tạm nghỉ', 'warn') : badge('Đang làm việc', 'ok')}</dd>
        </dl>`,
        footer: [{ text: 'Đóng', cls: 'btn--primary' }],
      });
      return;
    }
    const heSo = (n.chucVu && (n.chucVu.includes('Quản lý') || n.chucVu.includes('Quan ly'))) ? 3.0 : 2.0;
    const luongThuc = (n.luong || 0) * heSo;
    App.modal({
      title: `Chi tiết nhân viên ${n.ma}`,
      body: `<dl class="detail-list">
        <dt>Mã nhân viên</dt><dd>${esc(n.ma)}</dd>
        <dt>Họ và tên</dt><dd>${esc(n.hoTen)}</dd>
        <dt>Số điện thoại</dt><dd>${esc(n.sdt)}</dd>
        <dt>Email</dt><dd>${esc(n.email || '—')}</dd>
        <dt>CCCD / CMND</dt><dd>${esc(n.cccd || 'Chưa cập nhật')}</dd>
        <dt>Giới tính</dt><dd>${esc(n.gioiTinh || 'Chưa cập nhật')}</dd>
        <dt>Ngày sinh</dt><dd>${esc(n.ngaySinh || 'Chưa cập nhật')}</dd>
        <dt>Chức vụ</dt><dd>${esc(vi(n.chucVu))}</dd>
        <dt>Chi nhánh</dt><dd>${esc(branchName(n.chiNhanh))}</dd>
        <dt>Hệ số lương</dt><dd>${heSo}</dd>
        <dt>Lương cơ bản</dt><dd>${money(n.luong)}</dd>
        <dt>Lương thực nhận</dt><dd><b>${money(luongThuc)}</b></dd>
        <dt>Tài khoản</dt><dd>${esc(n.taiKhoan || 'Không có tài khoản đăng nhập')}</dd>
        <dt>Trạng thái</dt><dd>${!n.dangLam ? badge('Đã nghỉ việc', 'neutral') : n.tamNghi ? badge('Tạm nghỉ', 'warn') : badge('Đang làm việc', 'ok')}</dd>
      </dl>`,
      footer: [{ text: 'Đóng', cls: 'btn--primary' }],
    });
  },

  form(n) {
    const isEdit = !!n;
    App.modal({
      title: isEdit ? `Sửa nhân viên ${n.ma}` : 'Thêm nhân viên', size: 'lg',
      body: `<form class="form-grid" id="nv-form" novalidate>
        <label class="field"><span>Họ tên</span><input name="hoTen" value="${esc(n?.hoTen || '')}"></label>
        <label class="field"><span>Chi nhánh</span><select name="maCN" ${isEdit ? 'disabled' : ''}>${App.branchOptions(n?.maCN)}</select></label>
        <label class="field"><span>Chức vụ</span><input name="chucVu" list="chuc-vu" value="${esc(n?.chucVu || '')}"></label>
        <datalist id="chuc-vu"><option value="Quản lý"><option value="Lễ tân"><option value="Buồng phòng"><option value="Kỹ thuật"><option value="Bếp"><option value="Phục vụ"><option value="An ninh"></datalist>
        <label class="field"><span>CCCD / CMND</span><input name="cccd" data-kieu="cccdSo" inputmode="numeric" value="${esc(n?.cccd || '')}"></label>
        <label class="field"><span>Giới tính</span><select name="gioiTinh"><option value="">Chưa cập nhật</option>${['Nam', 'Nữ', 'Khác'].map((x) => `<option${x === n?.gioiTinh ? ' selected' : ''}>${x}</option>`).join('')}</select></label>
        <label class="field"><span>Ngày sinh</span><input name="ngaySinh" value="${esc(n?.ngaySinh || '')}" maxlength="10"></label>
        <label class="field"><span>Lương cơ bản (đ)</span><input name="luong" type="number" min="7000000" step="100000" value="${n?.luong || ''}"></label>
        <label class="field"><span>Số điện thoại</span><input name="sdt" inputmode="numeric" pattern="0[0-9]{9}" value="${esc(n?.sdt || '')}"></label>
        <label class="field"><span>Email</span><input name="email" type="email" value="${esc(n?.email || '')}" placeholder="Không bắt buộc"></label>
        ${isEdit && n.dangLam ? `<label class="field field--check full"><input type="checkbox" name="tamNghi" ${n.tamNghi ? 'checked' : ''}> Tạm nghỉ</label>` : ''}
        ${isEdit ? '' : `
        <div class="form-section">Phân quyền đăng nhập</div>
        <label class="field field--check full" data-account-toggle><input type="checkbox" name="taoTaiKhoan"> Tạo tài khoản cho nhân viên này</label>
        <label class="field" data-acc hidden><span>Tên đăng nhập</span><input name="username" autocomplete="off"></label>
        <label class="field" data-acc hidden><span>Mật khẩu (6–32 ký tự)</span><input name="password" type="password" autocomplete="new-password"></label>
        `}
      </form>`,
      onOpen: (b) => {
        const toggle = b.querySelector('[name=taoTaiKhoan]');
        const syncAccountFields = () => {
          const eligible = /quan ly|quản lý|le tan|lễ tân/i.test(b.querySelector('[name=chucVu]').value);
          b.querySelector('[data-account-toggle]').hidden = isEdit || !eligible;
          if (!eligible && toggle) toggle.checked = false;
          b.querySelectorAll('[data-acc]').forEach((x) => (x.hidden = !toggle?.checked || !eligible));
        };
        toggle?.addEventListener('change', syncAccountFields);
        b.querySelector('[name=chucVu]').addEventListener('input', syncAccountFields);
        syncAccountFields();
      },
      footer: [
        { text: 'Hủy' },
        {
          text: isEdit ? 'Lưu thay đổi' : 'Thêm nhân viên', cls: 'btn--primary',
          onClick: async (b) => {
            const form = b.querySelector('#nv-form');
            const v = App.formValues(form);
            // formValues trả checkbox dạng '1'/'0' — không dùng !! vì !!'0' === true
            const isOn = (x) => x === true || x === '1' || x === 'true' || x === 'on';
            v.tamNghi = isOn(v.tamNghi);
            v.taoTaiKhoan = isOn(v.taoTaiKhoan);
            const errs = {};
            if (!/^[\p{L} ]{2,50}$/u.test(v.hoTen)) errs.hoTen = 'Họ tên chỉ gồm chữ cái.';
            if (!/^[\p{L} ]+$/u.test(v.chucVu)) errs.chucVu = 'Chức vụ chỉ gồm chữ cái.';
            if (!v.maCN && !isEdit) errs.maCN = 'Chọn chi nhánh.';
            if (!(Number(v.luong) >= 7000000)) errs.luong = 'Lương cơ bản từ 7.000.000 đ.';
            if (!/^0\d{9}$/.test(v.sdt || '')) errs.sdt = 'SĐT 10 số, bắt đầu bằng 0.';
            if (v.cccd && !/^(\d{9}|\d{12})$/.test(v.cccd)) errs.cccd = 'CMND/CCCD nhân viên gồm 9 hoặc 12 chữ số.';
            if (v.email && !/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(v.email)) errs.email = 'Email không hợp lệ.';
            if (!isEdit && v.taoTaiKhoan) {
              if (!v.username) errs.username = 'Nhập tên đăng nhập.';
              if (!v.password || v.password.length < 6) errs.password = 'Mật khẩu 6–32 ký tự.';
            }
            if (App.showErrors(form, errs)) return;
            if (isEdit) v.ma = n.ma;
            const d = await App.run(() => Api.post(isEdit ? 'nhanvien/sua' : 'nhanvien/them', v),
              (x) => (isEdit ? `Cập nhật nhân viên ${x.ma} thành công!` : `Thêm nhân viên ${x.ma} thành công!`));
            if (d) { App.closeModal(); App.refresh(); }
          },
        },
      ],
    });
  },
});
