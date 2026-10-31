/* =====================================================================
   core.js - Tien ich dung chung cho WEBSITE KHACH (index.html) va TRANG QUAN TRI (admin.html)
   Icon SVG, dinh dang tien/ngay, doi chuoi khong dau cua C++ sang tieng Viet co dau.
   ===================================================================== */

/* ---------- Icon SVG net manh (ve tay, khong dung thu vien) ---------- */
const ICONS = {
  dashboard: '<rect x="3" y="3" width="7" height="9" rx="1"/><rect x="14" y="3" width="7" height="5" rx="1"/><rect x="14" y="12" width="7" height="9" rx="1"/><rect x="3" y="16" width="7" height="5" rx="1"/>',
  rooms: '<path d="M3 20V8l9-5 9 5v12"/><path d="M9 20v-6h6v6"/><path d="M3 20h18"/>',
  customers: '<circle cx="9" cy="8" r="3.5"/><path d="M2.5 20c.8-3.6 3.4-5.5 6.5-5.5s5.7 1.9 6.5 5.5"/><path d="M16 4.5a3.5 3.5 0 0 1 0 7"/><path d="M18.5 14.8c1.6.8 2.6 2.6 3 5.2"/>',
  bookings: '<rect x="3" y="5" width="18" height="16" rx="1"/><path d="M3 10h18M8 3v4M16 3v4"/><path d="M8 14h3v3H8z"/>',
  services: '<path d="M12 3c1.2 3.4 3.6 5.8 7 7-3.4 1.2-5.8 3.6-7 7-1.2-3.4-3.6-5.8-7-7 3.4-1.2 5.8-3.6 7-7z"/><path d="M19 17v4M17 19h4"/>',
  staff: '<rect x="4" y="3" width="16" height="18" rx="1"/><circle cx="12" cy="10" r="3"/><path d="M8 17c.6-2 2.1-3 4-3s3.4 1 4 3"/>',
  payments: '<rect x="2.5" y="5" width="19" height="14" rx="1"/><path d="M2.5 10h19"/><path d="M6.5 15h4"/>',
  reports: '<path d="M4 20V10M10 20V4M16 20v-7M22 20H2"/>',
  settings: '<circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.6 1.6 0 0 0 .3 1.8l.1.1a2 2 0 1 1-2.8 2.8l-.1-.1a1.6 1.6 0 0 0-1.8-.3 1.6 1.6 0 0 0-1 1.5V21a2 2 0 1 1-4 0v-.1a1.6 1.6 0 0 0-1-1.5 1.6 1.6 0 0 0-1.8.3l-.1.1a2 2 0 1 1-2.8-2.8l.1-.1a1.6 1.6 0 0 0 .3-1.8 1.6 1.6 0 0 0-1.5-1H3a2 2 0 1 1 0-4h.1a1.6 1.6 0 0 0 1.5-1 1.6 1.6 0 0 0-.3-1.8l-.1-.1a2 2 0 1 1 2.8-2.8l.1.1a1.6 1.6 0 0 0 1.8.3H9a1.6 1.6 0 0 0 1-1.5V3a2 2 0 1 1 4 0v.1a1.6 1.6 0 0 0 1 1.5 1.6 1.6 0 0 0 1.8-.3l.1-.1a2 2 0 1 1 2.8 2.8l-.1.1a1.6 1.6 0 0 0-.3 1.8V9a1.6 1.6 0 0 0 1.5 1H21a2 2 0 1 1 0 4h-.1a1.6 1.6 0 0 0-1.5 1z"/>',
  logout: '<path d="M9 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h4"/><path d="M16 17l5-5-5-5M21 12H9"/>',
  menu: '<path d="M4 6h16M4 12h16M4 18h16"/>',
  search: '<circle cx="11" cy="11" r="7"/><path d="M20 20l-3.5-3.5"/>',
  bell: '<path d="M6 8a6 6 0 1 1 12 0c0 7 3 9 3 9H3s3-2 3-9"/><path d="M10.3 21a1.9 1.9 0 0 0 3.4 0"/>',
  close: '<path d="M6 6l12 12M18 6L6 18"/>',
  plus: '<path d="M12 5v14M5 12h14"/>',
  edit: '<path d="M4 20h4L19 9l-4-4L4 16v4z"/><path d="M13.5 6.5l4 4"/>',
  trash: '<path d="M4 7h16M10 11v6M14 11v6"/><path d="M6 7l1 13h10l1-13M9 7V4h6v3"/>',
  eye: '<path d="M2 12s3.6-7 10-7 10 7 10 7-3.6 7-10 7S2 12 2 12z"/><circle cx="12" cy="12" r="3"/>',
  key: '<circle cx="8" cy="15" r="4"/><path d="M11 12l9-9M17 6l3 3M15 8l2 2"/>',
  bed: '<path d="M3 18V7M21 18v-5a3 3 0 0 0-3-3H10v8"/><path d="M3 14h18"/><circle cx="6.5" cy="10.5" r="1.8"/>',
  door: '<path d="M5 21V4a1 1 0 0 1 1-1h12a1 1 0 0 1 1 1v17"/><path d="M3 21h18"/><circle cx="15" cy="12.5" r="1"/>',
  tool: '<path d="M14.7 6.3a4 4 0 0 0-5.4 5.4L3 18l3 3 6.3-6.3a4 4 0 0 0 5.4-5.4l-2.6 2.6-2.4-.6-.6-2.4 2.6-2.6z"/>',
  wallet: '<path d="M20 7H5a2 2 0 0 1 0-4h13v4"/><path d="M3 5v14a2 2 0 0 0 2 2h15V7"/><circle cx="16" cy="14" r="1.2"/>',
  calendarCheck: '<rect x="3" y="5" width="18" height="16" rx="1"/><path d="M3 10h18M8 3v4M16 3v4"/><path d="M9 15l2 2 4-4"/>',
  checkin: '<path d="M15 3h4a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2h-4"/><path d="M10 17l5-5-5-5M15 12H3"/>',
  table: '<rect x="3" y="4" width="18" height="16" rx="1"/><path d="M3 10h18M3 15h18M9 4v16"/>',
  cards: '<rect x="3" y="3" width="8" height="8" rx="1"/><rect x="13" y="3" width="8" height="8" rx="1"/><rect x="3" y="13" width="8" height="8" rx="1"/><rect x="13" y="13" width="8" height="8" rx="1"/>',
  area: '<path d="M4 4h16v16H4z"/><path d="M4 9h5V4M20 15h-5v5"/>',
  guests: '<circle cx="9" cy="8" r="3.5"/><path d="M2.5 20c.8-3.6 3.4-5.5 6.5-5.5s5.7 1.9 6.5 5.5"/><circle cx="17" cy="9" r="2.5"/><path d="M16 14.6c2.6.1 4.6 1.9 5.4 5.4"/>',
  wifi: '<path d="M2 9a15 15 0 0 1 20 0M5 12.5a10 10 0 0 1 14 0M8.5 16a5 5 0 0 1 7 0"/><circle cx="12" cy="19.5" r=".8"/>',
  phone: '<path d="M5 4h4l2 5-2.5 1.5a11 11 0 0 0 5 5L15 13l5 2v4a2 2 0 0 1-2 2A16 16 0 0 1 3 6a2 2 0 0 1 2-2"/>',
  mail: '<rect x="3" y="5" width="18" height="14" rx="1"/><path d="M3 7l9 6 9-6"/>',
  pin: '<path d="M12 21s-7-6.2-7-11.5a7 7 0 0 1 14 0C19 14.8 12 21 12 21z"/><circle cx="12" cy="9.5" r="2.5"/>',
  layers: '<path d="M12 3l9 5-9 5-9-5 9-5z"/><path d="M3 13l9 5 9-5"/>',
  check: '<path d="M5 12.5l4.5 4.5L19 7.5"/>',
  chevronLeft: '<path d="M15 5l-7 7 7 7"/>',
  chevronRight: '<path d="M9 5l7 7-7 7"/>',
  arrowUp: '<path d="M12 19V5M5 12l7-7 7 7"/>',
  arrowRight: '<path d="M5 12h14M13 6l6 6-6 6"/>',
  gift: '<rect x="3" y="8" width="18" height="13" rx="1"/><path d="M3 12h18M12 8v13"/><path d="M12 8c-1.5-3-5-4-5.5-1.5S9 8 12 8zM12 8c1.5-3 5-4 5.5-1.5S15 8 12 8z"/>',
  utensils: '<path d="M7 3v8a2 2 0 0 0 2 2v8M5 3v5M9 3v5"/><path d="M17 21V3c-2 1.5-3 4-3 7s1 4 3 4"/>',
  lotus: '<path d="M12 20c-4 0-8-2.5-9-6 3 0 6 1 9 4 3-3 6-4 9-4-1 3.5-5 6-9 6z"/><path d="M12 18c-2.5-2.5-3-6-0-11 3 5 2.5 8.5 0 11z"/><path d="M8.5 15.5C6 13 5.5 10 6.5 7.5c2 1 3.5 2.5 4 4.5M15.5 15.5C18 13 18.5 10 17.5 7.5c-2 1-3.5 2.5-4 4.5"/>',
  wave: '<path d="M2 15c2.5-2 5-2 7.5 0s5 2 7.5 0 3.5-1.5 5-1"/><path d="M2 19c2.5-2 5-2 7.5 0s5 2 7.5 0 3.5-1.5 5-1"/><circle cx="15" cy="6" r="2.5"/><path d="M6 12l4-5 4 3"/>',
  car: '<path d="M5 17h14M3 17v-4l2-5h14l2 5v4h-2"/><circle cx="7.5" cy="17.5" r="1.8"/><circle cx="16.5" cy="17.5" r="1.8"/><path d="M4 12h16"/>',
  swap: '<path d="M4 8h13l-3-3M20 16H7l3 3"/>',
  building: '<path d="M4 21V5l8-3v19M12 21V8l8 3v10M2 21h20"/><path d="M7 8h2M7 12h2M7 16h2M15 13h2M15 17h2"/>',
  alert: '<path d="M12 3l10 18H2L12 3z"/><path d="M12 10v5M12 18v.5"/>',
  info: '<circle cx="12" cy="12" r="9"/><path d="M12 11v6M12 7.5v.5"/>',
};
function icon(name, size = 18) {
  return `<svg class="svg-ic" viewBox="0 0 24 24" width="${size}" height="${size}" aria-hidden="true">${ICONS[name] || ''}</svg>`;
}

// Gan icon cho moi phan tu co data-icon="ten" (dung trong HTML tinh)
function hydrateIcons(root = document) {
  root.querySelectorAll('[data-icon]').forEach((el) => {
    if (!el.querySelector('.svg-ic')) el.insertAdjacentHTML('afterbegin', icon(el.dataset.icon));
  });
}

/* ---------- Dinh dang ---------- */
const esc = (s) => String(s ?? '').replace(/[&<>"']/g, (c) => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
const nf = new Intl.NumberFormat('vi-VN');
// Number(n)||0 coi 0 la falsy nhung van dung; NaN -> 0. Khong ep am->duong (loi nhuan co the am).
const money = (n) => {
  const v = Number(n);
  return nf.format(Math.round(Number.isFinite(v) ? v : 0)) + ' đ';
};
const num = (n) => {
  const v = Number(n);
  return nf.format(Math.round(Number.isFinite(v) ? v : 0));
};
const debounce = (fn, ms = 300) => { let t; return (...a) => { clearTimeout(t); t = setTimeout(() => fn(...a), ms); }; };
const isoOf = (d) => `${d.getFullYear()}-${String(d.getMonth() + 1).padStart(2, '0')}-${String(d.getDate()).padStart(2, '0')}`;
const todayIso = (offset = 0) => { const d = new Date(); d.setDate(d.getDate() + offset); return isoOf(d); };
const addDays = (iso, n) => { const d = new Date(iso + 'T00:00:00'); d.setDate(d.getDate() + n); return isoOf(d); };
const daysBetween = (a, b) => Math.round((Date.parse(b) - Date.parse(a)) / 86400000);
const initials = (name) => String(name || '?').trim().split(/\s+/).slice(-2).map((w) => w[0]).join('').toUpperCase();

/* Backend C++ luu chuoi khong dau (de console Windows hien thi on dinh) -> doi sang tieng Viet co dau khi hien thi */
const VI = {
  'Thanh vien': 'Thành viên', Bac: 'Bạc', Vang: 'Vàng', 'Kim cuong': 'Kim cương',
  'Am thuc': 'Ẩm thực', 'Thap diem': 'Thấp điểm', 'Binh thuong': 'Bình thường', 'Cao diem': 'Cao điểm', 'Le/Tet': 'Lễ/Tết',
  'Viet Nam': 'Việt Nam', 'Tien mat': 'Tiền mặt', 'Chuyen khoan': 'Chuyển khoản', 'The tin dung': 'Thẻ tín dụng',
  'Vi dien tu': 'Ví điện tử', 'Tien coc': 'Tiền cọc', 'Quan ly': 'Quản lý', 'Le tan': 'Lễ tân', 'Buong phong': 'Buồng phòng',
  'Ky thuat': 'Kỹ thuật', 'Phuc vu': 'Phục vụ', 'Quy Nhon': 'Quy Nhơn', 'Ky Co - Eo Gio': 'Kỳ Co - Eo Gió', 'Tour Ky Co Eo Gio': 'Tour Kỳ Co - Eo Gió', 'Dang luu tru': 'Đang lưu trú', 'Sap den': 'Sắp đến',
  'Kênh Đặt Phòng Online': 'Kênh Đặt Phòng Online', 'Dat phong online': 'Kênh Đặt Phòng Online',
  suat: 'suất', lan: 'lần', nguoi: 'người',
  Anh: 'Anh', 'Thanh pho': 'Thành phố', Vuon: 'Vườn', Bien: 'Biển', 'Ho boi': 'Hồ bơi', Rung: 'Rừng',
  // Tien nghi phong
  'Nuoc uong': 'Nước uống', 'Bon tam': 'Bồn tắm', 'Ban cong': 'Ban công', 'Buffet sang': 'Buffet sáng',
  'Phong khach rieng': 'Phòng khách riêng', 'Ho boi rieng': 'Hồ bơi riêng', 'Xe dua don': 'Xe đưa đón', 'Quan gia rieng': 'Quản gia riêng',
  'Huong nhin': 'Hướng nhìn', 'Phi buffet sang/dem': 'Phí buffet sáng/đêm', 'Phi quan gia/dem': 'Phí quản gia/đêm', 'So phong ngu': 'Số phòng ngủ', 'Phong khach': 'Phòng khách', 'San vuon': 'Sân vườn',
  Co: 'Có', Khong: 'Không',
  // Dich vu mau
  'Set menu hai san': 'Set menu hải sản', 'Massage da nong': 'Massage đá nóng', 'Tri lieu thao duoc': 'Trị liệu thảo dược',
  'Tour 4 dao': 'Tour 4 đảo', 'Lan ngam san ho': 'Lặn ngắm san hô', 'Vinh Nha Trang, 1 ngay': 'Vịnh Nha Trang, 1 ngày',
  'Hai san': 'Hải sản', 'Vinh Nha Trang': 'Vịnh Nha Trang', 'Hon Mun': 'Hòn Mun',
};
// Chuoi tien te C++ dang "350,000 VND" -> "350.000 đ"
const vi = (s) => VI[s] || String(s ?? '').replace(/^([\d,]+) VND$/, (_, n) => n.replace(/,/g, '.') + ' đ');
// Ten hien thi chi nhanh = thuong hieu + ten trong du lieu C++ (khong hard-code danh sach chi nhanh)
const branchName = (ten) => { const t = vi(ten || ''); return /^ocean pearl/i.test(t) ? t : `Ocean Pearl Resort ${t}`; };

/* =====================================================================
   Input - Quy tac nhap lieu theo KIEU truong, dung chung cho website khach va trang quan tri.
   - Tu loc ky tu sai ngay khi go / dan (truong so khong nhan chu, truong chu khong nhan so...)
   - Input.check(form) tra ve loi tung truong; App.showErrors tu goi nen moi form deu duoc kiem tra.
   - Day CHI la lop tien loi: backend C++ van kiem tra lai moi truong (tat JS / sua request van bi tu choi).
   ===================================================================== */
const Input = (() => {
  const L = '\\p{L}';
  const R = {
    sdt:      { clean: (v) => v.replace(/\D/g, ''), max: 10, ok: /^0\d{9}$/, msg: 'Số điện thoại gồm đúng 10 chữ số, bắt đầu bằng 0.', mode: 'numeric' },
    cccd:     { clean: (v) => v.replace(/[^A-Za-z0-9]/g, ''), max: 12, ok: /^(\d{9}|\d{12}|[A-Za-z][A-Za-z0-9]{7,8})$/, msg: 'CMND/CCCD gồm 9 hoặc 12 chữ số, hoặc hộ chiếu hợp lệ.', optional: true },
    cccdSo:   { clean: (v) => v.replace(/\D/g, ''), max: 12, ok: /^(\d{9}|\d{12})$/, msg: 'CMND/CCCD nhân viên gồm 9 hoặc 12 chữ số.', optional: true, mode: 'numeric' },
    so:       { clean: (v) => v.replace(/\D/g, ''), max: 9, ok: /^\d{1,9}$/, msg: 'Chỉ nhập chữ số (số nguyên không âm).', mode: 'numeric', num: true },
    tien:     { clean: (v) => v.replace(/\D/g, ''), max: 10, ok: /^\d{1,10}$/, msg: 'Chỉ nhập chữ số (số tiền, không dấu).', mode: 'numeric', num: true },
    thuc:     { clean: (v) => v.replace(/[^\d.]/g, '').replace(/^(\d*\.\d*)\..*$/, '$1'), max: 8, ok: /^\d+(\.\d+)?$/, msg: 'Chỉ nhập chữ số (có thể có một dấu chấm).', mode: 'decimal', num: true },
    chu:      { clean: (v) => v.replace(new RegExp(`[^${L} ]`, 'gu'), '').replace(/ {2,}/g, ' '), max: 50, ok: new RegExp(`^(?=.*${L})[${L} ]{2,50}$`, 'u'), msg: 'Chỉ nhập chữ cái và khoảng trắng (2–50 ký tự), không dùng số.' },
    tenrieng: { clean: (v) => v.replace(new RegExp(`[^${L}\\d .'-]`, 'gu'), ''), max: 50, ok: new RegExp(`^(?=.*${L})[${L}\\d .'-]{2,50}$`, 'u'), msg: 'Gồm chữ, số, khoảng trắng và - . \' (2–50 ký tự, phải có chữ).' },
    diachi:   { clean: (v) => v.replace(new RegExp(`[^${L} ]`, 'gu'), '').replace(/ {2,}/g, ' '), max: 120, ok: new RegExp(`^(?=.*${L})[${L} ]{5,120}$`, 'u'), msg: 'Địa chỉ chỉ gồm chữ cái và khoảng trắng (5–120 ký tự).', optional: true },
    ngay:     { clean: (v) => v.replace(/[^0-9/]/g, '').slice(0, 10), max: 10, ok: /^\d{2}\/\d{2}\/\d{4}$/, msg: 'Ngày sinh theo dạng ngày/tháng/năm.', optional: true },
    username: { clean: (v) => v.replace(/[^A-Za-z0-9_.]/g, ''), max: 20, ok: /^[A-Za-z0-9_.]{3,20}$/, msg: '3–20 ký tự: chữ không dấu, số, _ hoặc .' },
    email:    { clean: (v) => v.replace(/\s/g, ''), max: 80, ok: /^[^\s@]+@[^\s@]+\.[^\s@]+$/, msg: 'Email không hợp lệ.', optional: true },
    text:     { clean: (v) => v.replace(/[|<>\x00-\x1f]/g, ''), max: 200, ok: /^[^|<>\x00-\x1f]{0,200}$/, msg: 'Không dùng các ký tự | < >.', optional: true },
  };
  const BY_NAME = {
    sdt: 'sdt', hotline: 'sdt', cccd: 'cccd',
    hoTen: 'chu', quocTich: 'chu', chucVu: 'chu', huongNhin: 'chu', loaiBuaAn: 'chu',
    so: 'so', tang: 'so', sucChua: 'so', soPhongNgu: 'so', soLuong: 'so', soNguoi: 'so', themDem: 'so', thoiLuong: 'so', soNgay: 'so',
    gia: 'tien', luong: 'tien', tienCoc: 'tien', donGia: 'tien', phiBuffet: 'tien', phiQuanGia: 'tien', khachDua: 'tien',
    dienTich: 'thuc', sanVuon: 'thuc', username: 'username', email: 'email', diaChi: 'diachi',
    diaChiThuongTru: 'diachi', ngaySinh: 'ngay', lyDo: 'text', ghiChu: 'text',
  };
  const OPTIONAL = new Set(['quocTich', 'diaChiThuongTru', 'cccd', 'ngaySinh', 'ghiChu', 'lyDo', 'tienCoc', 'khachDua']);
  const kieuOf = (el) => {
    if (!el || el.tagName !== 'INPUT') return null;
    if (el.dataset.kieu) return el.dataset.kieu;
    if (el.dataset.svc !== undefined) return 'so';
    if (el.type === 'password' || el.type === 'checkbox' || el.type === 'radio' || el.type === 'hidden' || el.type === 'date') return null;
    return BY_NAME[el.name] || null;
  };

  // Lan dau focus: doi type=number thanh text (type=number van cho go chu e, +, -) va dat maxlength / inputmode
  document.addEventListener('focusin', (e) => {
    const el = e.target, k = kieuOf(el), r = k && R[k];
    if (!r) return;
    if (r.num && el.type === 'number') el.type = 'text';
    if (r.mode) el.inputMode = r.mode;
    if (!el.maxLength || el.maxLength < 0) el.maxLength = r.max;
    el.autocomplete = el.autocomplete || 'off';
  });
  // Loc ky tu sai khi go / dan: giu vi tri con tro
  document.addEventListener('input', (e) => {
    const el = e.target, k = kieuOf(el), r = k && R[k];
    if (!r || typeof el.value !== 'string') return;
    const before = el.value;
    let after = r.clean(before);
    if (after.length > r.max) after = after.slice(0, r.max);
    if (after !== before) {
      const pos = el.selectionStart ?? before.length;
      el.value = after;
      const np = Math.max(0, pos - (before.length - after.length));
      try { el.setSelectionRange(np, np); } catch (err) { /* type khong ho tro */ }
    }
  });

  // Kiem tra ca form: tra ve {ten: thongBao}; bo qua o an / bi khoa
  function check(form) {
    const errs = {};
    form.querySelectorAll('input').forEach((el) => {
      const k = kieuOf(el), r = k && R[k];
      if (!r || !el.name || el.disabled || el.closest('[hidden]')) return;
      const v = el.value.trim();
      if (!v) {
        if (!(r.optional || OPTIONAL.has(el.name) || el.dataset.tuyChon !== undefined)) errs[el.name] = 'Vui lòng nhập thông tin này.';
        return;
      }
      if (!r.ok.test(v)) { errs[el.name] = r.msg; return; }
      if (r.num) {
        const n = Number(v);
        if (el.min !== '' && n < Number(el.min)) errs[el.name] = `Tối thiểu ${nf.format(Number(el.min))}.`;
        else if (el.max !== '' && el.max !== undefined && Number(el.max) > 0 && n > Number(el.max)) errs[el.name] = `Tối đa ${nf.format(Number(el.max))}.`;
      }
    });
    return errs;
  }
  return { check, rules: R };
})();

/* =====================================================================
   RoomImages - NOI DUY NHAT quan ly anh phong (mapping loai phong / phong -> anh).
   - POOL: danh sach anh cua tung loai (duong dan tinh tu assets/images/). Them / bot anh chi sua o day.
   - forRoom(loai, maPhong): moi phong mot anh co dinh (khong ngau nhien) nen tai lai trang van ra dung anh cu;
     cac phong cung loai duoc tron deu qua nhieu anh khac nhau.
   - cover(loai): anh dai dien cua loai (the loai phong, trang chu).
   - Thieu file anh -> CSS tu roi ve fallback/room.svg (xem imgCss).
   ===================================================================== */
const RoomImages = (() => {
  const POOL = {
    Standard: ['room-standard.jpg', 'rooms/standard-2.jpg', 'rooms/standard-3.jpg'],
    Deluxe: ['room-deluxe.jpg', 'rooms/deluxe-2.jpg', 'rooms/deluxe-3.jpg', 'rooms/deluxe-4.jpg'],
    Suite: ['room-suite.jpg', 'rooms/suite-2.jpg', 'rooms/suite-3.jpg'],
    Villa: ['room-villa.jpg', 'rooms/villa-2.jpg', 'rooms/villa-3.jpg', 'rooms/villa-4.jpg', 'rooms/villa-5.jpg', 'rooms/villa-6.jpg'],
  };
  const FALLBACK = 'room-standard.jpg';
  const gcd = (a, b) => (b ? gcd(b, a % b) : a);
  const pool = (loai) => POOL[loai] || POOL[String(loai || '').replace(/^./, (c) => c.toUpperCase())] || [FALLBACK];
  // Buoc nhay nguyen to cung nhau voi so anh -> thu tu "tron" nhung van deu, hai phong lien tiep khong trung anh
  const step = (n) => { let s = 2; while (s < n && gcd(s, n) !== 1) s++; return n > 1 ? s : 1; };
  const num = (ma) => parseInt(String(ma || '').replace(/\D/g, ''), 10) || 0;
  const forRoom = (loai, ma) => { const p = pool(loai); return p[(num(ma) * step(p.length)) % p.length]; };
  const cover = (loai) => pool(loai)[0];
  const css = (file) => `background-image:url('assets/images/${file}'),url('assets/images/fallback/room.svg')`;
  return { POOL, forRoom, cover, css };
})();
