/* =====================================================================
   app.js - Khung ung dung: dang nhap, menu, doi theme theo menu, router,
   modal, toast, xac nhan, bang du lieu (search/sort/pagination), tien ich.
   Moi trang (dashboard.js, rooms.js...) dang ky qua App.register(...)
   ===================================================================== */

/* Icon, esc, money, todayIso, vi... nam o core.js (dung chung voi website khach) */
const PAY_METHODS = [
  { v: 'Tien mat', t: 'Tiền mặt' }, { v: 'Chuyen khoan', t: 'Chuyển khoản' },
  { v: 'The tin dung', t: 'Thẻ' }, { v: 'Vi dien tu', t: 'Ví điện tử' },
];
const ROOM_STATUS = [
  { t: 'Trống', c: 'ok' }, { t: 'Đã đặt', c: 'warn' }, { t: 'Đang sử dụng', c: 'info' },
  { t: 'Dọn phòng', c: 'purple' }, { t: 'Bảo trì', c: 'danger' },
];
const BOOKING_STATUS = [
  { t: 'Chờ nhận phòng', c: 'warn' }, { t: 'Đang ở', c: 'info' }, { t: 'Đã trả phòng', c: 'ok' }, { t: 'Đã hủy', c: 'danger' },
];
const TIERS = ['Thành viên', 'Bạc', 'Vàng', 'Kim cương'];
const badge = (text, cls) => `<span class="badge badge--${cls}">${esc(text)}</span>`;
const roomBadge = (code) => badge(ROOM_STATUS[code].t, ROOM_STATUS[code].c);
const bookingBadge = (code) => badge(BOOKING_STATUS[code].t, BOOKING_STATUS[code].c);
const options = (list, selected) => list.map((o) => `<option value="${esc(o.v)}"${String(o.v) === String(selected) ? ' selected' : ''}>${esc(o.t)}</option>`).join('');

/* ---------- Bang du lieu: tim kiem, loc, sap xep, phan trang ---------- */
class DataTable {
  constructor(el, { columns, rows = [], pageSize = 10, empty = 'Không có dữ liệu phù hợp.', searchText }) {
    Object.assign(this, { el, columns, rows, pageSize, empty, searchText });
    this.query = ''; this.filter = null; this.sortKey = null; this.sortDir = 1; this.page = 1;
    this.el.addEventListener('click', (e) => {
      const th = e.target.closest('th[data-key]');
      if (th) { this.sortBy(th.dataset.key); return; }
      const pg = e.target.closest('[data-page]');
      if (pg && !pg.disabled) { this.page = Number(pg.dataset.page); this.render(); }
    });
    this.render();
  }
  setRows(rows) { this.rows = rows; this.page = 1; this.render(); }
  setQuery(q) { this.query = q.trim().toLowerCase(); this.page = 1; this.render(); }
  setFilter(fn) { this.filter = fn; this.page = 1; this.render(); }
  sortBy(key) {
    this.sortDir = this.sortKey === key ? -this.sortDir : 1;
    this.sortKey = key; this.render();
  }
  visibleRows() {
    let list = this.rows;
    if (this.filter) list = list.filter(this.filter);
    if (this.query) {
      const text = this.searchText || ((r) => Object.values(r).join(' '));
      list = list.filter((r) => text(r).toLowerCase().includes(this.query));
    }
    if (this.sortKey) {
      const col = this.columns.find((c) => c.key === this.sortKey);
      const val = col.value || ((r) => r[col.key]);
      list = [...list].sort((a, b) => {
        const x = val(a), y = val(b);
        return (typeof x === 'number' ? x - y : String(x).localeCompare(String(y), 'vi')) * this.sortDir;
      });
    }
    return list;
  }
  render() {
    const list = this.visibleRows();
    const pages = Math.max(1, Math.ceil(list.length / this.pageSize));
    this.page = Math.min(this.page, pages);
    const slice = list.slice((this.page - 1) * this.pageSize, this.page * this.pageSize);
    const head = this.columns.map((c) => {
      const sorted = this.sortKey === c.key;
      const arrow = sorted ? (this.sortDir > 0 ? '▲' : '▼') : '↕';
      return c.sort === false
        ? `<th class="${c.align || ''}">${esc(c.label)}</th>`
        : `<th data-key="${c.key}" class="sortable ${c.align || ''} ${sorted ? 'is-sorted' : ''}">${esc(c.label)}<span class="sort">${arrow}</span></th>`;
    }).join('');
    const body = slice.length
      ? slice.map((r) => `<tr>${this.columns.map((c) => `<td class="${c.align || ''}">${c.render ? c.render(r) : esc(r[c.key])}</td>`).join('')}</tr>`).join('')
      : `<tr><td colspan="${this.columns.length}" class="table-empty">${esc(this.empty)}</td></tr>`;
    let pager = '';
    if (list.length > this.pageSize) {
      const btns = Array.from({ length: pages }, (_, i) => i + 1)
        .filter((p) => p === 1 || p === pages || Math.abs(p - this.page) <= 1)
        .map((p, i, arr) => (i && p - arr[i - 1] > 1 ? '<span>…</span>' : '') + `<button type="button" data-page="${p}" class="${p === this.page ? 'is-active' : ''}">${p}</button>`)
        .join('');
      pager = `<div class="pager"><span>${list.length} dòng</span><span class="spacer"></span>
        <button type="button" data-page="${this.page - 1}" ${this.page === 1 ? 'disabled' : ''} aria-label="Trang trước">‹</button>${btns}
        <button type="button" data-page="${this.page + 1}" ${this.page === pages ? 'disabled' : ''} aria-label="Trang sau">›</button></div>`;
    } else if (list.length) {
      pager = `<div class="pager"><span>${list.length} dòng</span></div>`;
    }
    this.el.innerHTML = `<div class="table-wrap"><table class="table"><thead><tr>${head}</tr></thead><tbody>${body}</tbody></table></div>${pager}`;
  }
}

/* ---------- Ung dung ---------- */
const App = {
  user: null,
  pages: {},
  current: null,
  cache: { branches: [] },

  // Sidebar (theo yeu cau): Dashboard, Phong, Khach hang, Dat phong, Hoa don, Dich vu, Chi nhanh, Nhan vien, Thong ke, Cai dat.
  // "page" quyet dinh noi dung; cac bo loc phu (phong trong, bao tri...) nam tren thanh cong cu cua tung trang.
  PAGES: {
    dashboard: { label: 'Dashboard', theme: 'dashboard' },
    rooms: { label: 'Phòng', theme: 'rooms' },
    customers: { label: 'Khách hàng', theme: 'customers' },
    bookings: { label: 'Đặt phòng', theme: 'bookings' },
    payments: { label: 'Hóa đơn & thanh toán', theme: 'payments' },
    services: { label: 'Dịch vụ', theme: 'services' },
    branches: { label: 'Chi nhánh', theme: 'staff', admin: true },
    staff: { label: 'Nhân viên', theme: 'staff' },
    reports: { label: 'Thống kê', theme: 'reports', admin: true },
    settings: { label: 'Cài đặt', theme: 'settings', admin: true },
  },
  NAV_GROUPS: [
    { title: '', items: [
      { href: '#/dashboard', label: 'Dashboard', icon: 'dashboard' },
      { href: '#/rooms', label: 'Phòng', icon: 'rooms' },
      { href: '#/customers', label: 'Khách hàng', icon: 'customers' },
      { href: '#/bookings', label: 'Đặt phòng', icon: 'bookings' },
      { href: '#/payments', label: 'Hóa đơn', icon: 'wallet' },
      { href: '#/services', label: 'Dịch vụ', icon: 'services' },
      { href: '#/branches', label: 'Chi nhánh', icon: 'building', admin: true },
      { href: '#/staff', label: 'Nhân viên', icon: 'staff' },
      { href: '#/reports', label: 'Thống kê', icon: 'reports', admin: true },
      { href: '#/settings', label: 'Cài đặt', icon: 'settings', admin: true },
    ] },
  ],

  /* ----- Chi nhanh dang xem (phong / dat phong / hoa don): KHONG mac dinh hien tat ca chi nhanh ----- */
  get branch() {
    const list = this.cache.branches;
    if (!list.length) return '';
    let ma = this._branch;
    if (!ma) { try { ma = localStorage.getItem('bl_branch') || ''; } catch (e) { ma = ''; } }
    if (!list.find((b) => b.ma === ma)) ma = (list.find((b) => b.ma === this.user?.nhanVien?.maCN) || list[0]).ma;
    return ma;
  },
  setBranch(ma) {
    this._branch = ma;
    try { localStorage.setItem('bl_branch', ma); } catch (e) { /* bo qua */ }
  },
  branchInfo(ma = this.branch) { return this.cache.branches.find((b) => b.ma === ma); },
  // Thanh chon chi nhanh dang tab tron; extra = nut thao tac them ben phai
  branchTabs(selected = this.branch, { all = false, label = 'Chi nhánh', extra = '' } = {}) {
    const tabs = (all ? [{ ma: '', ten: '' }] : []).concat(this.cache.branches);
    return `<div class="branch-tabs" role="tablist" aria-label="${esc(label)}">
      <span class="branch-tabs__label">${esc(label)}:</span>
      ${tabs.map((b) => `<button type="button" role="tab" data-branch-tab="${esc(b.ma)}" class="${b.ma === selected ? 'is-active' : ''}" aria-selected="${b.ma === selected}">${icon(b.ma ? 'pin' : 'layers', 15)}${esc(b.ma ? branchName(b.ten) : 'Tất cả chi nhánh')}</button>`).join('')}
      ${extra ? `<div class="branch-tabs__extra">${extra}</div>` : ''}
    </div>`;
  },
  bindBranchTabs(root, onChange, { remember = true } = {}) {
    root.querySelectorAll('[data-branch-tab]').forEach((b) => b.addEventListener('click', () => {
      root.querySelectorAll('[data-branch-tab]').forEach((x) => { x.classList.toggle('is-active', x === b); x.setAttribute('aria-selected', x === b); });
      if (remember && b.dataset.branchTab) this.setBranch(b.dataset.branchTab);
      onChange(b.dataset.branchTab);
    }));
  },

  register(id, page) { this.pages[id] = page; },
  isAdmin() { return !!(this.user && this.user.admin); },

  /* ----- Khoi dong ----- */
  async start() {
    this.bindShell();
    if (Api.hasToken()) {
      try {
        this.user = await Api.get('toi');
        return this.enter();
      } catch (e) { /* token cu het han */ }
    }
    this.showLogin();
  },

  showLogin(msg) {
    this.user = null;
    document.documentElement.dataset.theme = 'dashboard';
    document.getElementById('app').hidden = true;
    document.getElementById('login-screen').hidden = false;
    if (msg) toast(msg, 'info');
    document.querySelector('#login-form input[name=username]').focus();
  },

  async enter() {
    document.getElementById('login-screen').hidden = true;
    document.getElementById('app').hidden = false;
    const nv = this.user.nhanVien || {};
    document.getElementById('user-name').textContent = nv.hoTen || this.user.username;
    document.getElementById('user-role').textContent = this.isAdmin() ? 'Quản lý' : 'Lễ tân';
    document.getElementById('user-avatar').textContent = initials(nv.hoTen);
    document.getElementById('user-info').innerHTML = `<b>${esc(nv.hoTen)}</b><br><span class="muted">${esc(vi(nv.chucVu))} – ${esc(branchName(nv.chiNhanh))}<br>Tài khoản: ${esc(this.user.username)}</span>`;
    this.renderNav();
    try { this.cache.branches = await Api.get('chinhanh'); } catch (e) { this.cache.branches = []; }
    if (!location.hash) location.hash = '#/dashboard';
    else this.route();
  },

  /* ----- Gan su kien cho khung ----- */
  bindShell() {
    document.getElementById('btn-menu').innerHTML = icon('menu', 22);
    document.querySelector('.a-search__icon').innerHTML = icon('search', 16);
    document.querySelector('#notify [data-toggle]').innerHTML = icon('bell', 20);
    document.querySelector('#modal .modal__header .icon-btn').innerHTML = icon('close', 18);
    hydrateIcons(document);

    document.getElementById('login-form').addEventListener('submit', async (e) => {
      e.preventDefault();
      const f = e.target;
      const v = App.formValues(f);
      const errs = {};
      if (!v.username) errs.username = 'Nhập tên đăng nhập.';
      if (!v.password) errs.password = 'Nhập mật khẩu.';
      if (App.showErrors(f, errs)) return;
      const btn = f.querySelector('button[type=submit]');
      btn.disabled = true; btn.textContent = 'Đang đăng nhập…';
      try {
        const d = await Api.post('dangnhap', v);
        Api.setToken(d.token);
        this.user = d;
        f.reset();
        toast(`Xin chào ${d.nhanVien.hoTen}!`);
        this.enter();
      } catch (err) {
        toast(err.message, 'error');
      } finally {
        btn.disabled = false; btn.textContent = 'Đăng nhập';
      }
    });

    // Sidebar: tren dien thoai truot tu trai (nut hamburger); tren may tinh thu gon / mo rong (nho lua chon)
    const app = document.getElementById('app');
    const burger = document.getElementById('btn-menu');
    const collapseBtn = document.getElementById('btn-collapse');
    this.setMenuOpen = (open) => {
      app.classList.toggle('is-menu-open', open);
      burger.setAttribute('aria-expanded', open);
    };
    const setCollapsed = (c) => {
      app.classList.toggle('is-collapsed', c);
      collapseBtn.innerHTML = icon(c ? 'chevronRight' : 'chevronLeft', 18);
      collapseBtn.setAttribute('aria-label', c ? 'Mở rộng menu' : 'Thu gọn menu');
      try { localStorage.setItem('hotel_sidebar_collapsed', c ? '1' : '0'); } catch (e) { /* bo qua */ }
    };
    let saved = false;
    try { saved = localStorage.getItem('hotel_sidebar_collapsed') === '1'; } catch (e) { /* bo qua */ }
    setCollapsed(saved);
    collapseBtn.addEventListener('click', () => setCollapsed(!app.classList.contains('is-collapsed')));
    burger.addEventListener('click', () => this.setMenuOpen(!app.classList.contains('is-menu-open')));
    document.getElementById('sidebar-backdrop').addEventListener('click', () => this.setMenuOpen(false));
    document.getElementById('nav').addEventListener('click', (e) => { if (e.target.closest('[data-action=logout]')) this.logout(); });
    document.querySelector('#user-menu [data-action=logout]').addEventListener('click', () => this.logout());
    // Nut vang "Dat phong" tren header: mo form dat phong (ke ca khi dang o trang Dat phong)
    document.querySelector('.a-cta').addEventListener('click', (e) => {
      if (this.current === 'bookings') { e.preventDefault(); this.pages.bookings.form(); }
    });

    // Dropdown chung: bam nut de mo/dong, bam ra ngoai de dong
    document.addEventListener('click', (e) => {
      const toggle = e.target.closest('[data-toggle]');
      document.querySelectorAll('.dropdown.is-open').forEach((d) => { if (!d.contains(e.target)) d.classList.remove('is-open'); });
      if (toggle) {
        const dd = toggle.closest('.dropdown');
        const open = dd.classList.toggle('is-open');
        if (dd.id === 'notify' && open) this.loadNotifications();
      }
    });

    // Modal: dong khi bam nen / nut dong / phim Esc
    const modal = document.getElementById('modal');
    modal.addEventListener('click', (e) => { if (e.target.closest('[data-close]')) this.closeModal(); });
    document.addEventListener('keydown', (e) => { if (e.key === 'Escape' && !modal.hidden) this.closeModal(); });

    // Tim kiem nhanh theo ma
    document.getElementById('global-search').addEventListener('submit', (e) => {
      e.preventDefault();
      const input = e.target.querySelector('input');
      const q = input.value.trim();
      if (!q) return;
      const up = q.toUpperCase();
      if (/^P\d+$/.test(up)) location.hash = `#/rooms?q=${encodeURIComponent(up)}`;
      else if (/^DP\d+$/.test(up)) location.hash = `#/bookings?ma=${encodeURIComponent(up)}`;
      else if (/^HD\d+$/.test(up)) location.hash = `#/payments?ma=${encodeURIComponent(up)}`;
      else location.hash = `#/customers?q=${encodeURIComponent(q)}`;
      input.value = '';
    });

    window.addEventListener('hashchange', () => this.route());
  },

  renderNav() {
    const nav = document.getElementById('nav');
    nav.innerHTML = this.NAV_GROUPS.map((g) => {
      const items = g.items.filter((i) => !i.admin || this.isAdmin());
      if (!items.length) return '';
      return `<div class="nav-group">${g.title ? `<div class="nav-group__title">${esc(g.title)}</div>` : ''}${items.map((i) =>
        `<a class="nav-item" href="${i.href}" data-href="${i.href}" title="${esc(i.label)}">${icon(i.icon)}<span>${esc(i.label)}</span></a>`).join('')}</div>`;
    }).join('')
      + `<div class="nav-group nav-group--bottom"><a class="nav-item" href="index.html" target="_blank" rel="noopener" title="Website khách sạn">${icon('pin')}<span>Website khách sạn</span></a>`
      + `<button class="nav-item nav-item--logout" type="button" data-action="logout" title="Đăng xuất">${icon('logout')}<span>Đăng xuất</span></button></div>`;
  },

  // Danh dau muc dang chon: trung khop ca tham so truoc, neu khong thi muc cung trang khong co tham so
  highlightNav() {
    const cur = location.hash || '#/dashboard';
    const [curPage, curQuery = ''] = cur.replace(/^#\//, '').split('?');
    const curParams = new URLSearchParams(curQuery);
    let best = null, bestScore = -1;
    document.querySelectorAll('#nav [data-href]').forEach((a) => {
      const [pg, q = ''] = a.dataset.href.replace(/^#\//, '').split('?');
      if (pg !== curPage) return;
      const want = [...new URLSearchParams(q).entries()];
      if (want.every(([k, v]) => curParams.get(k) === v) && want.length >= bestScore) { best = a; bestScore = want.length; }
    });
    document.querySelectorAll('#nav .nav-item').forEach((a) => a.classList.toggle('is-active', a === best));
  },

  async logout() {
    try { await Api.post('dangxuat'); } catch (e) { /* bo qua */ }
    Api.setToken(null);
    location.hash = '';
    this.showLogin('Bạn đã đăng xuất.');
  },

  /* ----- Router: #/trang?thamso ----- */
  async route() {
    if (!this.user) return;
    const [pathPart, queryPart] = location.hash.replace(/^#\/?/, '').split('?');
    let id = pathPart || 'dashboard';
    const nav = this.PAGES[id];
    if (!nav || !this.pages[id] || (nav.admin && !this.isAdmin())) { location.hash = '#/dashboard'; return; }
    const params = Object.fromEntries(new URLSearchParams(queryPart || ''));
    this.current = id;

    // Doi mau chu dao theo menu dang mo
    document.documentElement.dataset.theme = nav.theme;
    this.highlightNav();
    this.setMenuOpen(false);
    window.scrollTo(0, 0);
    document.querySelectorAll('.dropdown.is-open').forEach((d) => d.classList.remove('is-open'));
    const page = this.pages[id];
    document.getElementById('page-title').textContent = nav.label;
    document.getElementById('page-sub').textContent = (typeof page.sub === 'function' ? page.sub() : page.sub) || '';
    document.title = `${nav.label} – Ocean Pearl Resort`;

    const content = document.getElementById('content');
    content.innerHTML = '<div class="loading">Đang tải dữ liệu…</div>';
    try {
      await page.render(content, params);
      content.focus({ preventScroll: true });
    } catch (e) {
      content.innerHTML = `<div class="panel"><div class="empty-state">${esc(e.message)}</div></div>`;
    }
    this.updateBadge();
  },
  refresh() { this.route(); },

  /* ----- Thong bao: lay tu du lieu that (khach den, phong can don, hoa don con no) ----- */
  async updateBadge() {
    try {
      const d = await Api.get('tongquan');
      this._overview = d;
      const n = d.khachDenHomNay + d.phongDangDon + d.doanhThu.conNo + (d.khachQuaHan || 0);
      const dot = document.querySelector('#notify .badge-dot');
      dot.hidden = n === 0; dot.textContent = n;
    } catch (e) { /* bo qua */ }
  },
  async loadNotifications() {
    const menu = document.querySelector('#notify [data-menu]');
    menu.innerHTML = '<div class="loading">Đang tải…</div>';
    await this.updateBadge();
    const d = this._overview;
    if (!d) return;
    const items = [];
    if (d.khachQuaHan) items.push({ c: 'var(--danger)', t: `${d.khachQuaHan} khách ở quá hạn, chưa check-out`, h: '#/bookings?tt=1' });
    if (d.khachDenHomNay) items.push({ c: 'var(--warn)', t: `${d.khachDenHomNay} khách đến nhận phòng hôm nay`, h: '#/bookings?tt=0' });
    if (d.phongDangDon) items.push({ c: 'var(--purple)', t: `${d.phongDangDon} phòng đang chờ dọn xong`, h: '#/rooms?st=3' });
    if (d.doanhThu.conNo) items.push({ c: 'var(--danger)', t: `${d.doanhThu.conNo} hóa đơn chưa thanh toán`, h: '#/payments?tab=invoices' });
    if (d.phongBaoTri) items.push({ c: 'var(--neutral)', t: `${d.phongBaoTri} phòng đang bảo trì`, h: '#/rooms?st=4' });
    menu.innerHTML = items.length
      ? items.map((i) => `<a class="notify-item" href="${i.h}" style="color:inherit;text-decoration:none"><i style="background:${i.c}"></i><span>${esc(i.t)}</span></a>`).join('')
      : '<div class="empty-state">Không có việc cần xử lý.</div>';
  },

  /* ----- Modal ----- */
  modal({ title, body, size = '', footer = [], onOpen }) {
    const modal = document.getElementById('modal');
    modal.className = 'modal' + (size ? ` modal--${size}` : '');
    document.getElementById('modal-title').textContent = title;
    const bodyEl = document.getElementById('modal-body');
    bodyEl.innerHTML = body;
    const foot = document.getElementById('modal-footer');
    foot.innerHTML = '';
    footer.forEach((b) => {
      const btn = document.createElement('button');
      btn.type = 'button';
      btn.className = 'btn ' + (b.cls || '');
      btn.textContent = b.text;
      if (b.id) btn.id = b.id;
      btn.addEventListener('click', async () => {
        if (!b.onClick) return this.closeModal();
        btn.disabled = true;
        try { await b.onClick(bodyEl); } finally { btn.disabled = false; }
      });
      foot.appendChild(btn);
    });
    modal.hidden = false;
    this._lastFocus = document.activeElement;
    if (onOpen) onOpen(bodyEl);
    const first = bodyEl.querySelector('input:not([type=hidden]), select, textarea');
    (first || foot.querySelector('button') || bodyEl).focus?.();
    return bodyEl;
  },
  closeModal() {
    document.getElementById('modal').hidden = true;
    this._lastFocus?.focus?.();
  },
  // Modal xac nhan cho thao tac anh huong du lieu - lop RIENG nam tren modal dang mo (form khong bi mat khi bam Huy).
  // message: chuoi thuong (tu escape) hoac {html} da escape san. note: dong phu.
  // reasons: danh sach ly do -> chi hien nut Dong, KHONG cho thuc hien (vd chi nhanh con du lieu lien quan).
  confirm(message, { ok = 'Đồng ý', danger = false, title = 'Xác nhận', note = '', reasons = null, cancel = 'Hủy' } = {}) {
    let el = document.getElementById('confirm-modal');
    if (!el) {
      el = document.createElement('div');
      el.id = 'confirm-modal';
      el.className = 'modal modal--confirm';
      el.hidden = true;
      el.innerHTML = `<div class="modal__backdrop" data-cf="0"></div>
        <div class="modal__dialog" role="alertdialog" aria-modal="true" aria-labelledby="cf-title">
          <header class="modal__header"><h2 id="cf-title"></h2><button class="icon-btn" type="button" data-cf="0" aria-label="Đóng">${icon('close', 18)}</button></header>
          <div class="modal__body" id="cf-body"></div><footer class="modal__footer" id="cf-foot"></footer></div>`;
      document.body.appendChild(el);
    }
    const html = typeof message === 'object' && message ? message.html : `<p>${esc(message)}</p>`;
    const blocked = Array.isArray(reasons) && reasons.length > 0;
    el.querySelector('#cf-title').textContent = title;
    el.querySelector('#cf-body').innerHTML = `<div class="confirm${danger || blocked ? ' confirm--danger' : ''}">
        <span class="confirm__icon">${icon(danger || blocked ? 'alert' : 'info', 26)}</span>
        <div class="confirm__text">${html}${note ? `<p class="muted">${esc(note)}</p>` : ''}
          ${blocked ? `<ul>${reasons.map((r) => `<li>${esc(r)}</li>`).join('')}</ul>` : ''}</div></div>`;
    el.querySelector('#cf-foot').innerHTML = blocked
      ? '<button type="button" class="btn" data-cf="0">Đóng</button>'
      : `<button type="button" class="btn" data-cf="0">${esc(cancel)}</button><button type="button" class="btn ${danger ? 'btn--danger-solid' : 'btn--primary'}" data-cf="1">${esc(ok)}</button>`;
    el.hidden = false;
    const last = document.activeElement;
    el.querySelector('#cf-foot [data-cf="1"], #cf-foot [data-cf="0"]').focus();
    return new Promise((resolve) => {
      const done = (v) => {
        el.hidden = true;
        el.removeEventListener('click', onClick);
        document.removeEventListener('keydown', onKey, true);
        last?.focus?.();
        resolve(v);
      };
      const onClick = (e) => { const t = e.target.closest('[data-cf]'); if (t) done(t.dataset.cf === '1'); };
      const onKey = (e) => { if (e.key === 'Escape') { e.stopPropagation(); done(false); } };
      el.addEventListener('click', onClick);
      document.addEventListener('keydown', onKey, true);
    });
  },
  /* ----- Form ----- */
  formValues(form) {
    const v = {};
    form.querySelectorAll('[name]').forEach((el) => {
      if (el.type === 'checkbox') v[el.name] = el.checked ? '1' : '0';
      else if (el.type === 'radio') { if (el.checked) v[el.name] = el.value; }
      else v[el.name] = el.value.trim();
    });
    return v;
  },
  // Hien loi duoi tung o; tra ve true neu co loi
  showErrors(form, errors) {
    // Gop kiem tra theo KIEU truong (so / chu / sdt / dia chi...) - moi form deu di qua day
    const auto = Input.check(form);
    Object.keys(auto).forEach((k) => { if (!errors[k]) errors[k] = auto[k]; });
    form.querySelectorAll('.field.is-invalid').forEach((f) => f.classList.remove('is-invalid'));
    form.querySelectorAll('.field__error').forEach((e) => e.remove());
    const keys = Object.keys(errors);
    keys.forEach((k) => {
      const el = form.querySelector(`[name="${k}"]`);
      const field = el && el.closest('.field');
      if (!field) return;
      field.classList.add('is-invalid');
      field.insertAdjacentHTML('beforeend', `<small class="field__error">${esc(errors[k])}</small>`);
    });
    if (keys.length) form.querySelector('.is-invalid input, .is-invalid select')?.focus();
    return keys.length > 0;
  },
  branchOptions(selected, withAll = false) {
    return (withAll ? '<option value="">Tất cả chi nhánh</option>' : '') +
      this.cache.branches.map((b) => `<option value="${esc(b.ma)}"${b.ma === selected ? ' selected' : ''}>${esc(branchName(b.ten))}</option>`).join('');
  },
  // Goi API co thong bao: tra ve data hoac null neu loi
  async run(fn, successMsg) {
    try {
      const d = await fn();
      if (successMsg) toast(typeof successMsg === 'function' ? successMsg(d) : successMsg);
      return d ?? true;
    } catch (e) {
      toast(e.message, 'error');
      return null;
    }
  },
};

/* ---------- Toast ---------- */
function toast(message, type = 'success') {
  const titles = { success: 'Thành công', error: 'Không thể thực hiện', info: 'Thông báo' };
  const el = document.createElement('div');
  el.className = `toast toast--${type}`;
  el.setAttribute('role', type === 'error' ? 'alert' : 'status');
  el.innerHTML = `<span class="toast__icon">${icon({ success: 'check', error: 'alert', info: 'info' }[type], 16)}</span><div><b>${titles[type]}</b><p>${esc(message)}</p></div>`;
  document.getElementById('toasts').appendChild(el);
  setTimeout(() => el.remove(), type === 'error' ? 6000 : 3500);
}

/* ---------- The thong ke dung chung ---------- */
function statCard({ label, value, unit = '', foot = '', ic = 'dashboard', accent = false, tone = '' }) {
  return `<div class="stat${accent ? ' stat--accent' : ''}${tone ? ' stat--' + tone : ''}">
    <div class="stat__top"><span>${esc(label)}</span><span class="stat__icon">${icon(ic, 17)}</span></div>
    <div class="stat__value">${value}${unit ? `<small>${esc(unit)}</small>` : ''}</div>
    ${foot ? `<div class="stat__foot">${foot}</div>` : ''}
  </div>`;
}
