/* =====================================================================
   guest.js - Website cho khach (khong can dang nhap)
   API cong khai cua C++ (WebApi::apiKhach):
     GET  /api/khach/thongtin   chi nhanh, loai phong (gia tu), dich vu
     GET  /api/khach/phongtrong phong DAT DUOC theo ngay (C++ kiem tra trung lich, suc chua, bao tri)
     POST /api/khach/baogia     tien phong + dich vu (chuoi Decorator) + VAT
     POST /api/khach/datphong   tao/tim khach -> QuanLyKhachSan::datPhong (nhu le tan)
   ===================================================================== */
const Guest = (() => {
  const $ = (s, root = document) => root.querySelector(s);
  const state = { info: null, search: null, rooms: [], type: '', listType: '', listRooms: [], listBranch: '' };

  // Mo ta + anh cho tung loai phong (anh that dat trong assets/images, thieu thi dung anh minh hoa)
  const ROOM_TYPES = {
    Standard: { img: RoomImages.cover('Standard'), desc: 'Phòng gọn gàng cho hai người, cửa sổ nhìn ra phố biển hoặc khu vườn.' },
    Deluxe: { img: RoomImages.cover('Deluxe'), desc: 'Rộng hơn, có bồn tắm, nhiều phòng có ban công hướng biển.' },
    Suite: { img: RoomImages.cover('Suite'), desc: 'Phòng khách riêng, buffet sáng và minibar cho kỳ nghỉ dài ngày.' },
    Villa: { img: RoomImages.cover('Villa'), desc: 'Biệt thự sân vườn có hồ bơi riêng và quản gia, hợp cho gia đình.' },
  };
  const SVC_IMG = { 'Am thuc': 'dv-am-thuc.jpg', Spa: 'dv-spa.jpg', Tour: 'dv-tour.jpg' };
  // Nhom dich vu (loai lay tu du lieu C++) -> tieu de, icon, mo ta hien thi kieu Ocean Pearl Resort
  const SVC_GROUP = {
    'Am thuc': { title: 'Dịch vụ ẩm thực', icon: 'utensils', desc: 'Thưởng thức ẩm thực đa dạng từ Á đến Âu, được chế biến bởi đội ngũ đầu bếp chuyên nghiệp.' },
    Spa: { title: 'Spa & Thư giãn', icon: 'lotus', desc: 'Giải tỏa căng thẳng với các liệu trình spa chuyên nghiệp, mang đến sự thư thái và tái tạo năng lượng.' },
    Tour: { title: 'Hoạt động giải trí', icon: 'wave', desc: 'Khám phá những hoạt động thú vị trên biển để tận hưởng trọn vẹn vẻ đẹp đại dương.' },
  };
  const BRANCH_IMG = ['hero.jpg', 'banner.jpg']; // Nha Trang: bai bien (hero.jpg), Quy Nhon: ho boi resort (banner.jpg)
  const imgStyle = (file, fallback) => `background-image:url('assets/images/${file}'),url('assets/images/fallback/${fallback}')`;
  const dmy = (iso) => iso.split('-').reverse().join('/');
  const phone = (s) => String(s).replace(/^(\d{4})(\d{3})(\d+)$/, '$1 $2 $3');

  /* ---------- Modal + toast (cung markup voi trang quan tri) ---------- */
  function modal({ title, body, footer = [], size = 'lg' }) {
    const m = $('#modal');
    m.className = `modal modal--${size}`;
    $('#modal-title').textContent = title;
    $('#modal-body').innerHTML = body;
    const foot = $('#modal-footer');
    foot.innerHTML = '';
    footer.forEach((b) => {
      const btn = document.createElement('button');
      btn.type = 'button'; btn.className = 'btn ' + (b.cls || ''); btn.textContent = b.text;
      btn.addEventListener('click', async () => {
        if (!b.onClick) return closeModal();
        btn.disabled = true;
        try { await b.onClick(); } finally { btn.disabled = false; }
      });
      foot.appendChild(btn);
    });
    m.hidden = false;
    document.body.style.overflow = 'hidden';
    return $('#modal-body');
  }
  function closeModal() { $('#modal').hidden = true; document.body.style.overflow = ''; }
  function toast(message, type = 'success') {
    const titles = { success: 'Thành công', error: 'Chưa đặt được', info: 'Thông báo' };
    const el = document.createElement('div');
    el.className = `toast toast--${type}`;
    el.setAttribute('role', type === 'error' ? 'alert' : 'status');
    el.innerHTML = `<b>${titles[type]}</b><p>${esc(message)}</p>`;
    $('#toasts').appendChild(el);
    setTimeout(() => el.remove(), type === 'error' ? 6500 : 4000);
  }

  /* ---------- The kieu Ocean Pearl Resort: anh lon, icon tron, tieu de navy, gia vang, nut CTA ---------- */
  // price: { tu: true/false, so: money, donVi: '/ đêm' }; action: nut chinh; more: thuoc tinh cua nut mui ten tron
  function blCard({ title, img, fallback = 'room.svg', ic = 'bed', desc, specs = [], list = [], price, action, more }) {
    return `<article class="bl-card">
      <div class="bl-card__img"><i style="${imgStyle(img, fallback)}"></i></div>
      <span class="bl-card__icon" aria-hidden="true">${icon(ic, 30)}</span>
      <div class="bl-card__body">
        <h3 class="bl-card__title">${esc(title)}</h3>
        ${desc ? `<p class="bl-card__desc">${esc(desc)}</p>` : ''}
        ${specs.length ? `<div class="bl-card__meta">${specs.map(([sic, t]) => `<span>${icon(sic, 14)}${esc(t)}</span>`).join('')}</div>` : ''}
        ${list.length ? `<ul class="bl-card__list">${list.map(([a, b]) => `<li><span>${esc(a)}</span><b>${esc(b)}</b></li>`).join('')}</ul>` : ''}
        ${price ? `<div class="bl-price">${price.tu ? '<small>Từ</small>' : ''}<b>${price.so}</b><span>${esc(price.donVi || '')}</span></div>` : ''}
        <div class="bl-card__foot">${action}${more ? `<button class="icon-circle" type="button" ${more} aria-label="Xem thêm">${icon('arrowRight', 18)}</button>` : ''}</div>
      </div>
    </article>`;
  }

  function renderInfo() {
    const d = state.info;
    // Chi nhanh: select, the chi nhanh, footer (danh sach lay tu du lieu C++)
    // Chọn chi nhánh dạng pill (tên đầy đủ) — đồng bộ với input ẩn #g-cn
    renderBranchPills(d.chiNhanh, state.branchFocus || d.chiNhanh[0]?.ma || '');
    if (d.chiNhanh[0] && $('#g-hotline')) {
      $('#g-hotline').textContent = phone(d.chiNhanh[0].hotline);
      $('#g-hotline').href = 'tel:' + d.chiNhanh[0].hotline;
      $('#g-hotline').insertAdjacentHTML('afterbegin', icon('phone'));
    }
    $('#g-branches').innerHTML = d.chiNhanh.map((c, i) => `
      <article class="g-branch">
        <div class="g-branch__img" style="${imgStyle(BRANCH_IMG[i % 2], 'hero.svg')}"></div>
        <div class="g-branch__body">
          <h3>${esc(branchName(c.ten))}</h3>
          <p>${icon('pin')}${esc(c.diaChi)}</p>
          <p>${icon('phone')}<a href="tel:${esc(c.hotline)}">${esc(phone(c.hotline))}</a></p>
          <p>${icon('door')}${c.soPhong} phòng và villa</p>
          <button class="btn btn--primary" type="button" data-branch="${c.ma}">Xem phòng trống ${icon('arrowRight', 16)}</button>
        </div>
      </article>`).join('');
    $('#g-footer-branches').innerHTML = '<h3>Liên hệ</h3>' + d.chiNhanh.map((c) =>
      `<p class="g-footer__branch"><b>${esc(branchName(c.ten))}</b><br>${esc(c.diaChi)}<br><a href="tel:${esc(c.hotline)}">${esc(phone(c.hotline))}</a></p>`).join('');

    const totalRooms = d.chiNhanh.reduce((s, c) => s + c.soPhong, 0);
    $('#g-points').innerHTML = [
      [d.chiNhanh.length, 'khu nghỉ dưỡng ven biển'], [totalRooms, 'phòng, suite và villa'],
      [d.loaiPhong.length, 'hạng phòng để lựa chọn'], [d.dichVu.length, 'dịch vụ đặt trước được'],
    ].map(([n, t]) => `<li><b>${n}</b>${t}</li>`).join('');

    // Loai phong — mọi vị trí trên thẻ đều mở danh sách phòng thực tế của loại đó
    $('#g-room-types').innerHTML = d.loaiPhong.map((l) => {
      const t = ROOM_TYPES[l.loai] || { img: 'room-standard.jpg', desc: '' };
      const area = l.dienTichTu === l.dienTichDen ? `${l.dienTichTu} m²` : `${l.dienTichTu}–${l.dienTichDen} m²`;
      return `<article class="bl-card bl-card--clickable" data-room-type="${esc(l.loai)}" role="button" tabindex="0" aria-label="Xem danh sách phòng ${esc(l.loai)}">
      <div class="bl-card__img"><i style="${imgStyle(t.img, 'room.svg')}"></i></div>
      <span class="bl-card__icon" aria-hidden="true">${icon('bed', 30)}</span>
      <div class="bl-card__body">
        <h3 class="bl-card__title">${esc(l.loai)}</h3>
        <p class="bl-card__desc">${esc(t.desc)}</p>
        <div class="bl-card__meta">
          <span>${icon('area', 14)}${esc(area)}</span>
          <span>${icon('guests', 14)}Tối đa ${l.sucChua} khách</span>
          <span>${icon('door', 14)}${l.soPhong} phòng</span>
        </div>
        <div class="bl-price"><small>Từ</small><b>${money(l.giaTu)}</b><span>/ đêm</span></div>
        <div class="bl-card__foot">
          <button class="btn btn--primary btn--sm" type="button" data-room-type="${esc(l.loai)}">Đặt ngay ${icon('arrowRight', 15)}</button>
          <button class="icon-circle" type="button" data-room-type="${esc(l.loai)}" aria-label="Xem danh sách phòng ${esc(l.loai)}">${icon('arrowRight', 18)}</button>
        </div>
      </div>
    </article>`;
    }).join('');

    // Dich vu theo nhom — chi 3 nhom: Am thuc, Spa, Tour (an card "Tien ich")
    state.svcGroups = {};
    d.dichVu.forEach((s) => {
      if (s.loai === 'Tien ich') return;
      (state.svcGroups[s.loai] = state.svcGroups[s.loai] || []).push(s);
    });
    const order = ['Am thuc', 'Spa', 'Tour'];
    const entries = order
      .filter((loai) => state.svcGroups[loai]?.length)
      .map((loai) => [loai, state.svcGroups[loai]]);
    Object.entries(state.svcGroups).forEach(([loai, list]) => {
      if (!order.includes(loai)) entries.push([loai, list]);
    });
    const grid = $('#g-services');
    if (grid) {
      grid.classList.add('g-services-grid');
      grid.innerHTML = entries.map(([loai, list]) => {
        const g = SVC_GROUP[loai] || { title: vi(loai), icon: 'services', desc: '' };
        const re = list.reduce((a, b) => (b.donGia < a.donGia ? b : a));
        return blCard({
          title: g.title, img: SVC_IMG[loai] || 'dv-spa.jpg', fallback: 'service.svg', ic: g.icon, desc: g.desc,
          list: list.slice(0, 3).map((x) => [vi(x.ten), money(x.donGia)]),
          price: { tu: true, so: money(re.donGia), donVi: `/ ${vi(re.donVi)}` },
          action: `<button class="btn btn--primary btn--sm" type="button" data-goto-rooms>Đặt ngay ${icon('arrowRight', 15)}</button>`,
          more: `data-svc-group="${esc(loai)}"`,
        });
      }).join('');
    }
  }

  function renderBranchPills(list, selected) {
    const box = $('#g-cn-pills');
    const input = $('#g-cn');
    if (!box || !input) return;
    const items = Array.isArray(list) ? list : [];
    let ma = selected || input.value || '';
    if (ma && !items.find((c) => c.ma === ma)) ma = items[0]?.ma || '';
    if (!ma && items[0]) ma = items[0].ma;
    input.value = ma;
    state.branchFocus = ma;
    box.innerHTML = items.map((c) => {
      const active = c.ma === ma;
      return `<button type="button" class="g-cn-pill${active ? ' is-active' : ''}" data-set-cn="${esc(c.ma)}" aria-pressed="${active}">
        ${icon('pin', 14)}<span>${esc(branchName(c.ten))}</span>
      </button>`;
    }).join('') || '<span class="muted">Chưa có chi nhánh</span>';
    hydrateIcons(box);
  }

  function setGuestBranch(maCN) {
    const input = $('#g-cn');
    if (!input) return;
    input.value = maCN || '';
    state.branchFocus = maCN || '';
    document.querySelectorAll('#g-cn-pills [data-set-cn]').forEach((btn) => {
      const on = btn.dataset.setCn === maCN;
      btn.classList.toggle('is-active', on);
      btn.setAttribute('aria-pressed', on ? 'true' : 'false');
    });
  }

  /**
   * 1) Tìm phòng — giữ maCN/ngày/số khách, lọc qua API, cuộn tới Phòng & Suite + kết quả.
   */
  async function handleSearchRooms() {
    const f = ensureSearchDates();
    if (!f) return toast('Không tìm thấy form tìm phòng.', 'error');
    state.type = '';
    await search({ scroll: false });
    try { history.replaceState(null, '', '#rooms'); } catch (_) { location.hash = '#rooms'; }
    const roomsSec = document.getElementById('rooms');
    if (roomsSec) roomsSec.scrollIntoView({ behavior: 'smooth', block: 'start' });
    // Sau khi cuộn tới Phòng & Suite, hiện kết quả phòng trống (nếu có)
    const res = document.getElementById('results');
    if (res) {
      res.hidden = false;
      setTimeout(() => res.scrollIntoView({ behavior: 'smooth', block: 'start' }), 350);
    }
    if (!(state.rooms || []).length) {
      toast('Không tìm thấy phòng trống phù hợp với điều kiện tìm kiếm', 'error');
    }
  }

  /** Hiển thị nhãn trạng thái thân thiện cho khách */
  function statusLabel(r) {
    if (r.coTheDat) return { text: 'Còn trống', cls: 'st--free' };
    const tt = (r.trangThai || '').toLowerCase();
    if (tt.includes('bao tri')) return { text: 'Bảo trì', cls: 'st--maint' };
    if (tt.includes('dang o')) return { text: 'Đang sử dụng', cls: 'st--busy' };
    if (tt.includes('giu cho') || tt.includes('giữ chỗ')) return { text: 'Đã đặt', cls: 'st--booked' };
    if (!r.coTheDat) return { text: 'Đã đặt', cls: 'st--booked' };
    return { text: vi(r.trangThai) || 'Không khả dụng', cls: 'st--busy' };
  }

  /**
   * Bước 2: Sau khi chọn loại phòng → hiển thị danh sách phòng thực tế theo chi nhánh.
   */
  async function showRoomListByType(roomType) {
    if (!roomType) return;
    const f = ensureSearchDates();
    if (!f) return toast('Không tìm thấy form tìm phòng.', 'error');
    const v = Object.fromEntries(new FormData(f));
    const nights = daysBetween(v.ngayNhan, v.ngayTra);
    if (!v.ngayNhan || v.ngayNhan < todayIso()) return toast('Ngày nhận phòng phải từ hôm nay trở đi.', 'error');
    if (!(nights > 0)) return toast('Ngày trả phòng phải sau ngày nhận phòng.', 'error');
    if (nights > 60) return toast('Đặt trực tuyến tối đa 60 đêm.', 'error');
    const soNguoi = Number(v.soNguoi) || 1;

    state.listType = roomType;
    state.type = roomType;
    const sec = $('#room-list');
    const grid = $('#g-room-list-grid');
    if (grid) grid.innerHTML = `<div class="panel empty-state" style="grid-column:1/-1">Đang tải danh sách phòng ${esc(roomType)}…</div>`;
    if (sec) { sec.hidden = false; sec.scrollIntoView({ behavior: 'smooth', block: 'start' }); }

    try {
      const d = await Api.get('khach/danhsachphong', {
        loai: roomType,
        maCN: v.maCN || '',
        ngayNhan: v.ngayNhan,
        soDem: nights,
        soNguoi,
      });
      // Giữ ngày ISO từ form (v.ngayTra); API trả dd/mm/yyyy không dùng cho dmy()
      state.search = { ...v, soNguoi, soDem: nights, mua: d.mua, heSo: d.heSo, ngayTra: v.ngayTra };
      state.listRooms = Array.isArray(d.phong) ? d.phong : [];
      // Đồng bộ state.rooms với các phòng có thể đặt để openBooking hoạt động
      state.rooms = state.listRooms.filter((r) => r.coTheDat);
      state.listBranch = '';
      renderRoomListByType();
      try { history.replaceState(null, '', '#room-list'); } catch (_) { location.hash = '#room-list'; }
    } catch (e) {
      if (grid) grid.innerHTML = `<div class="panel empty-state" style="grid-column:1/-1">${esc(e.message || 'Không tải được danh sách phòng.')}</div>`;
      toast(e.message || 'Không tải được danh sách phòng.', 'error');
    }
  }

  function renderRoomListByType() {
    const roomType = state.listType;
    const rooms = state.listRooms || [];
    const s = state.search;
    const title = $('#g-room-list-title');
    const desc = $('#g-room-list-desc');
    const meta = $('#g-room-list-meta');
    const tabs = $('#g-room-list-tabs');
    const grid = $('#g-room-list-grid');
    if (title) title.textContent = `DANH SÁCH PHÒNG ${String(roomType || '').toUpperCase()}`;
    if (desc) desc.textContent = 'Khám phá và lựa chọn căn phòng phù hợp với kỳ nghỉ của bạn.';

    if (meta && s) {
      meta.innerHTML = `<span>${icon('calendarCheck', 14)}${dmy(s.ngayNhan)} → ${dmy(s.ngayTra || s.ngayNhan)} · ${s.soDem} đêm</span>
        <span>${icon('guests', 14)}${s.soNguoi} khách</span>
        <span>Mùa ${vi(s.mua).toLowerCase()} (×${s.heSo})</span>`;
    }

    // Nhóm theo chi nhánh
    const byBranch = {};
    rooms.forEach((r) => {
      const key = r.maCN || r.chiNhanh || 'khac';
      (byBranch[key] = byBranch[key] || []).push(r);
    });
    const branches = (state.info?.chiNhanh || []).filter((c) => byBranch[c.ma]);
    // Thêm chi nhánh lạ nếu có
    Object.keys(byBranch).forEach((k) => {
      if (!branches.find((c) => c.ma === k)) {
        const sample = byBranch[k][0];
        branches.push({ ma: k, ten: sample?.chiNhanh || k });
      }
    });

    if (tabs) {
      if (branches.length > 1) {
        tabs.innerHTML = [
          `<button type="button" data-list-branch="" class="${!state.listBranch ? 'is-active' : ''}">Tất cả (${rooms.length})</button>`,
          ...branches.map((c) => {
            const n = (byBranch[c.ma] || []).length;
            return `<button type="button" data-list-branch="${esc(c.ma)}" class="${state.listBranch === c.ma ? 'is-active' : ''}">${esc(branchName(c.ten))} (${n})</button>`;
          }),
        ].join('');
      } else {
        tabs.innerHTML = '';
      }
    }

    const filtered = state.listBranch
      ? rooms.filter((r) => r.maCN === state.listBranch)
      : rooms;

    if (!grid) return;
    if (!filtered.length) {
      grid.innerHTML = `<div class="panel empty-state" style="grid-column:1/-1">Không có phòng ${esc(roomType)} nào${state.listBranch ? ' tại chi nhánh này' : ''}.</div>`;
      return;
    }

    // Hiển thị theo nhóm chi nhánh khi xem "Tất cả"
    if (!state.listBranch && branches.length > 1) {
      grid.innerHTML = branches.map((c) => {
        const list = byBranch[c.ma] || [];
        if (!list.length) return '';
        return `<div class="g-room-branch" style="grid-column:1/-1">
          <div class="g-room-branch__head">
            <h3>${esc(branchName(c.ten))}</h3>
            <span class="muted">${list.length} phòng</span>
          </div>
          <div class="bl-grid g-room-branch__grid">${list.map(roomCardHtml).join('')}</div>
        </div>`;
      }).join('');
    } else {
      grid.innerHTML = filtered.map(roomCardHtml).join('');
    }
    hydrateIcons(grid);
  }

  function roomCardHtml(r) {
    const st = statusLabel(r);
    const s = state.search;
    const canBook = !!r.coTheDat;
    const specs = [
      ['area', `${r.dienTich} m²`],
      ['guests', `Tối đa ${r.sucChua} khách`],
      ...(r.tienNghi || []).slice(0, 2).map((t) => ['check', vi(t)]),
    ];
    return `<article class="bl-card g-room-item ${canBook ? '' : 'is-unavailable'}">
      <div class="bl-card__img">
        <i style="${imgStyle(RoomImages.forRoom(r.loai, r.ma), 'room.svg')}"></i>
        <span class="badge g-room-status ${st.cls}">${esc(st.text)}</span>
      </div>
      <span class="bl-card__icon" aria-hidden="true">${icon('key', 30)}</span>
      <div class="bl-card__body">
        <h3 class="bl-card__title">${esc(r.loai)} ${esc(r.so)}</h3>
        <p class="bl-card__desc">${esc(branchName(r.chiNhanh))} · Tầng ${r.tang}${r.ma ? ` · Mã ${r.ma}` : ''}</p>
        <div class="bl-card__meta">${specs.map(([sic, t]) => `<span>${icon(sic, 14)}${esc(t)}</span>`).join('')}</div>
        <div class="bl-price"><b>${money(r.gia)}</b><span>/ đêm</span></div>
        ${s ? `<p class="muted" style="margin:0 0 12px;font-size:13px">${money(r.tienPhong)} / ${s.soDem} đêm</p>` : ''}
        ${!canBook && r.lyDoKhongDat ? `<p class="g-room-reason muted">${esc(vi(r.lyDoKhongDat))}</p>` : ''}
        <div class="bl-card__foot">
          ${canBook
            ? `<button class="btn btn--gold btn--sm" type="button" data-select-room="${esc(r.ma)}">Chọn phòng ${icon('arrowRight', 15)}</button>`
            : `<button class="btn btn--ghost btn--sm" type="button" disabled>Không thể đặt</button>`}
          <button class="btn btn--ghost btn--sm" type="button" data-room-detail="${esc(r.ma)}">Xem chi tiết</button>
        </div>
      </div>
    </article>`;
  }

  /** Bước 4: Chi tiết phòng đã chọn */
  function showRoomDetail(maPhong) {
    const r = (state.listRooms || []).find((x) => x.ma === maPhong)
      || (state.rooms || []).find((x) => x.ma === maPhong);
    if (!r) return toast('Không tìm thấy thông tin phòng.', 'error');
    const st = statusLabel(r);
    const s = state.search;
    const canBook = !!r.coTheDat;
    const tienNghi = (r.tienNghi || []).map((t) => `<li>${esc(vi(t))}</li>`).join('') || '<li class="muted">—</li>';
    const rieng = (r.thongTinRieng || []).map((x) => `<li><span>${esc(vi(x.k))}</span><b>${esc(vi(x.v))}</b></li>`).join('');
    modal({
      title: `${r.loai} ${r.so}`,
      size: 'lg',
      body: `<div class="g-room-detail">
        <div class="g-room-detail__img" style="${imgStyle(RoomImages.forRoom(r.loai, r.ma), 'room.svg')}"></div>
        <div class="g-room-detail__body">
          <p class="g-room-status ${st.cls}" style="display:inline-flex;margin:0 0 10px">${esc(st.text)}</p>
          <dl class="g-lookup__meta">
            <div><dt>Mã phòng</dt><dd>${esc(r.ma)}</dd></div>
            <div><dt>Loại</dt><dd>${esc(r.loai)}</dd></div>
            <div><dt>Chi nhánh</dt><dd>${esc(branchName(r.chiNhanh))}</dd></div>
            <div><dt>Tầng</dt><dd>${r.tang}</dd></div>
            <div><dt>Diện tích</dt><dd>${r.dienTich} m²</dd></div>
            <div><dt>Sức chứa</dt><dd>Tối đa ${r.sucChua} khách</dd></div>
            <div><dt>Giá / đêm</dt><dd>${money(r.gia)}</dd></div>
            ${s ? `<div><dt>Tổng ${s.soDem} đêm</dt><dd>${money(r.tienPhong)}</dd></div>` : ''}
          </dl>
          <h4 style="margin:16px 0 8px">Tiện nghi nổi bật</h4>
          <ul class="g-room-amenities">${tienNghi}</ul>
          ${rieng ? `<ul class="bl-card__list" style="margin-top:12px">${rieng}</ul>` : ''}
          ${!canBook && r.lyDoKhongDat ? `<p class="g-room-reason" style="margin-top:12px">${esc(vi(r.lyDoKhongDat))}</p>` : ''}
        </div>
      </div>`,
      footer: canBook
        ? [
            { text: 'Đóng' },
            { text: 'Đặt phòng', cls: 'btn--gold', onClick: () => { closeModal(); openBookingFromList(r.ma); } },
          ]
        : [{ text: 'Đóng' }],
    });
  }

  /** Bước 5–6: Mở form đặt phòng với phòng đã chọn (chỉ khi còn trống) */
  function openBookingFromList(maPhong) {
    const r = (state.listRooms || []).find((x) => x.ma === maPhong);
    if (!r) return toast('Không tìm thấy phòng.', 'error');
    if (!r.coTheDat) return toast(r.lyDoKhongDat ? vi(r.lyDoKhongDat) : 'Phòng này không thể đặt trong khoảng thời gian đã chọn.', 'error');
    // Đảm bảo state.rooms chứa phòng này để openBooking tìm thấy
    if (!state.rooms.find((x) => x.ma === maPhong)) state.rooms = [...state.rooms, r];
    if (!state.search) return toast('Vui lòng chọn ngày nhận/trả phòng trước.', 'error');
    openBooking(maPhong);
  }

  /**
   * Đặt ngay từ dịch vụ (không qua loại phòng) — giữ hành vi tìm phòng trống rồi mở form.
   */
  async function handleOpenBookingForm(roomType) {
    const f = ensureSearchDates();
    if (!f) return toast('Không tìm thấy form tìm phòng.', 'error');
    state.type = roomType || '';
    const branchId = f.maCN?.value || '';
    toast(roomType ? `Đang tìm phòng ${roomType}…` : 'Đang tìm phòng trống…');
    await search({ scroll: false });
    const list = (state.rooms || []).filter((r) => !roomType || r.loai === roomType);
    if (!list.length) {
      toast(branchId
        ? `Không còn phòng${roomType ? ' ' + roomType : ''} trống tại chi nhánh đã chọn trong khoảng ngày này.`
        : `Không tìm thấy phòng trống phù hợp với điều kiện tìm kiếm`, 'error');
      const res = document.getElementById('results');
      if (res) { res.hidden = false; res.scrollIntoView({ behavior: 'smooth', block: 'start' }); }
      return;
    }
    openBooking(list[0].ma);
  }

  // Từ mục Dịch vụ: đưa khách tới Phòng & Suite (không tự chọn phòng / mở form)
  function goToRooms() {
    if (!$('#modal')?.hidden) closeModal();
    const sec = document.getElementById('rooms');
    if (sec) sec.scrollIntoView({ behavior: 'smooth', block: 'start' });
    if (history.replaceState) history.replaceState(null, '', '#rooms');
  }

  function showServiceGroup(loai) {
    const list = state.svcGroups?.[loai] || [];
    const g = SVC_GROUP[loai] || { title: vi(loai) };
    modal({
      title: g.title, size: '',
      body: `<p class="muted" style="margin-top:0">${esc(g.desc || '')}</p>
        <div class="table-wrap"><table class="table"><thead><tr><th>Dịch vụ</th><th>Chi tiết</th><th class="num">Giá</th></tr></thead><tbody>
        ${list.map((x) => `<tr><td><b>${esc(vi(x.ten))}</b></td><td class="muted">${esc(vi(x.moTa))}</td><td class="num">${money(x.donGia)} / ${esc(vi(x.donVi))}</td></tr>`).join('')}
        </tbody></table></div>
        <p class="muted" style="margin:14px 0 0">Chọn dịch vụ ở bước điền thông tin đặt phòng; tiền dịch vụ tính vào hóa đơn khi trả phòng.</p>`,
      footer: [{ text: 'Đóng' }, { text: 'Đặt phòng', cls: 'btn--gold', onClick: () => { closeModal(); goToRooms(); } }],
    });
  }

  /* ---------- Tim phong trong ---------- */
  function ensureSearchDates() {
    const f = $('#g-check-form');
    if (!f) return null;
    if (!f.ngayNhan.value) {
      f.ngayNhan.value = todayIso();
      f.ngayNhan.min = todayIso();
    }
    if (!f.ngayTra.value || f.ngayTra.value <= f.ngayNhan.value) {
      f.ngayTra.value = addDays(f.ngayNhan.value, 2);
      f.ngayTra.min = addDays(f.ngayNhan.value, 1);
    }
    // Đồng bộ chi nhánh: nếu chưa chọn, lấy chi nhánh đang active trên pill
    if (f.maCN && !f.maCN.value) {
      const active = document.querySelector('#g-cn-pills .g-cn-pill.is-active');
      if (active?.dataset?.setCn) f.maCN.value = active.dataset.setCn;
      else if (state.info?.chiNhanh?.[0]) f.maCN.value = state.info.chiNhanh[0].ma;
    }
    return f;
  }

  async function search({ scroll = true } = {}) {
    const f = ensureSearchDates();
    if (!f) return toast('Không tìm thấy form tìm phòng.', 'error');
    const v = Object.fromEntries(new FormData(f));
    const nights = daysBetween(v.ngayNhan, v.ngayTra);
    if (!v.ngayNhan || v.ngayNhan < todayIso()) return toast('Ngày nhận phòng phải từ hôm nay trở đi.', 'error');
    if (!(nights > 0)) return toast('Ngày trả phòng phải sau ngày nhận phòng.', 'error');
    if (nights > 60) return toast('Đặt trực tuyến tối đa 60 đêm.', 'error');
    const soNguoi = Number(v.soNguoi) || 1;
    if (soNguoi < 1 || soNguoi > 20) return toast('Số khách phải từ 1 đến 20.', 'error');
    const btn = f.querySelector('button[type=submit], button.btn--primary');
    if (btn) { btn.disabled = true; btn.textContent = 'Đang tìm…'; }
    try {
      const d = await Api.get('khach/phongtrong', {
        maCN: v.maCN || '',
        ngayNhan: v.ngayNhan,
        soDem: nights,
        soNguoi,
      });
      state.search = { ...v, soNguoi, soDem: nights, mua: d.mua, heSo: d.heSo };
      state.rooms = Array.isArray(d.phong) ? d.phong : [];
      renderResults();
      const res = $('#results');
      if (res) res.hidden = false;
      setStep(2);
      if (scroll) {
        const target = res || $('#check');
        if (target) target.scrollIntoView({ behavior: 'smooth', block: 'start' });
      }
    } catch (e) {
      toast(e.message || 'Không tìm được phòng. Vui lòng thử lại.', 'error');
    } finally {
      if (btn) { btn.disabled = false; btn.textContent = 'Tìm phòng'; }
    }
  }

  function renderResults() {
    const s = state.search;
    if (!s) return;
    const types = [...new Set([...(state.info?.loaiPhong || []).map((x) => x.loai), ...state.rooms.map((r) => r.loai)])];
    const cn = (state.info?.chiNhanh || []).find((c) => c.ma === s.maCN);
    const sub = $('#g-result-sub');
    if (sub) {
      const tenCN = cn ? branchName(cn.ten) : '';
      sub.textContent = (tenCN ? `${tenCN} · ` : '')
        + `${dmy(s.ngayNhan)} → ${dmy(s.ngayTra)}, ${s.soDem} đêm, ${s.soNguoi} khách`
        + `. Mùa ${vi(s.mua).toLowerCase()} (giá ×${s.heSo}).`
        + (tenCN ? ' Chỉ hiển thị phòng thuộc chi nhánh này.' : '');
    }
    const filt = $('#g-type-filter');
    if (filt) {
      filt.innerHTML = types.length > 1
        ? ['', ...types].map((t) => `<button type="button" data-filter="${esc(t)}" class="${t === state.type ? 'is-active' : ''}">${t || 'Tất cả'}</button>`).join('')
        : '';
    }
    const list = state.rooms.filter((r) => !state.type || r.loai === state.type);
    const listEl = $('#g-result-list');
    if (!listEl) return;
    listEl.innerHTML = list.length ? list.map((r) => blCard({
      title: `${r.loai} ${r.so}`, img: RoomImages.forRoom(r.loai, r.ma), ic: 'key',
      desc: `${branchName(r.chiNhanh)}, tầng ${r.tang}. ${(r.thongTinRieng || []).map((x) => `${vi(x.k)}: ${vi(x.v)}`).join(', ')}`,
      specs: [
        ['area', `${r.dienTich} m²`],
        ['guests', `Tối đa ${r.sucChua} khách`],
        ...(r.tienNghi || []).slice(0, 2).map((t) => ['check', vi(t)]),
      ],
      price: { so: money(r.tienPhong), donVi: `/ ${s.soDem} đêm` },
      action: `<button class="btn btn--gold btn--sm" type="button" data-book="${r.ma}">Đặt phòng ${icon('arrowRight', 15)}</button>`,
    })).join('') : `<div class="panel empty-state" style="grid-column:1/-1">Không tìm thấy phòng trống phù hợp với điều kiện tìm kiếm<br><span class="muted">Vui lòng đổi ngày, số khách hoặc chi nhánh.</span></div>`;
  }

  /* ---------- Dat phong ---------- */
  function validate(v) {
    const e = {};
    if (!/^[\p{L} ]{2,50}$/u.test(v.hoTen || '')) e.hoTen = 'Họ tên chỉ gồm chữ cái và khoảng trắng.';
    if (!/^0\d{9}$/.test(v.sdt || '')) e.sdt = 'Số điện thoại gồm 10 chữ số, bắt đầu bằng 0.';
    if (!/^(\d{9}|\d{12}|[A-Za-z][A-Za-z0-9]{7,8})$/.test(v.cccd || '')) e.cccd = 'CMND/CCCD gồm 9 hoặc 12 chữ số.';
    if (v.email && !/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(v.email)) e.email = 'Email không hợp lệ.';
    if (v.quocTich && !/^[\p{L} ]+$/u.test(v.quocTich)) e.quocTich = 'Quốc tịch chỉ gồm chữ cái.';
    if (v.gioiTinh && !['Nam', 'Nữ', 'Khác'].includes(v.gioiTinh)) e.gioiTinh = 'Giới tính không hợp lệ.';
    if (v.diaChiThuongTru && !/^[\p{L} ]{5,120}$/u.test(v.diaChiThuongTru)) e.diaChiThuongTru = 'Địa chỉ chỉ gồm chữ cái và khoảng trắng.';
    if (!v.dongY) e.dongY = 'Vui lòng xác nhận thông tin.';
    return e;
  }
  function showErrors(form, errors) {
    form.querySelectorAll('.field.is-invalid').forEach((f) => f.classList.remove('is-invalid'));
    form.querySelectorAll('.field__error').forEach((x) => x.remove());
    Object.entries(errors).forEach(([k, msg]) => {
      const field = form.querySelector(`[name="${k}"]`)?.closest('.field');
      if (!field) return;
      field.classList.add('is-invalid');
      field.insertAdjacentHTML('beforeend', `<small class="field__error">${esc(msg)}</small>`);
    });
    form.querySelector('.is-invalid input')?.focus();
    return Object.keys(errors).length > 0;
  }

  function openBooking(maPhong) {
    const r = state.rooms.find((x) => x.ma === maPhong);
    const s = state.search;
    if (!r || !s) {
      toast('Vui lòng tìm phòng trống trước khi đặt.', 'error');
      return;
    }
    const services = state.info?.dichVu || [];
    setStep(3);
    const body = modal({
      title: 'Bước 3 · Điền thông tin đặt phòng',
      body: `<form class="g-book" id="g-book" novalidate>
        <div>
          <div class="g-book__room">
            <div class="g-book__thumb" style="${imgStyle(RoomImages.forRoom(r.loai, r.ma), 'room.svg')}"></div>
            <div><h3>${esc(r.loai)} ${esc(r.so)}</h3>
              <p class="muted" style="margin:4px 0 0">${esc(branchName(r.chiNhanh))} · ${dmy(s.ngayNhan)} → ${dmy(s.ngayTra)} · ${s.soDem} đêm · ${s.soNguoi} khách</p></div>
          </div>
          <div class="form-grid">
            <div class="form-section">Thông tin người đặt</div>
            <label class="field full"><span>Họ tên</span><input name="hoTen" autocomplete="name"></label>
            <label class="field"><span>Số điện thoại</span><input name="sdt" inputmode="numeric" autocomplete="tel"></label>
            <label class="field"><span>CCCD / CMND / Hộ chiếu</span><input name="cccd" inputmode="numeric"></label>
            <label class="field"><span>Giới tính</span><select name="gioiTinh"><option value="">Chưa cập nhật</option><option>Nam</option><option>Nữ</option><option>Khác</option></select></label>
            <label class="field full"><span>Địa chỉ thường trú</span><input name="diaChiThuongTru"></label>
            <label class="field"><span>Email</span><input name="email" type="email" autocomplete="email" placeholder="Không bắt buộc"></label>
            <label class="field"><span>Quốc tịch</span><input name="quocTich" value="Việt Nam"></label>
            <label class="field full"><span>Yêu cầu thêm</span><input name="ghiChu" placeholder="Giờ đến dự kiến, giường phụ…"></label>
            <div class="form-section">Dịch vụ đặt trước</div>
            <div class="g-svc-pick full">${services.map((d) => `<label><span>${esc(vi(d.ten))} <span class="muted">${esc(vi(d.loai))}</span></span>
              <span class="muted">${money(d.donGia)} / ${esc(vi(d.donVi))}</span>
              <input type="number" min="0" max="100" value="0" data-svc="${d.ma}" aria-label="Số lượng ${esc(vi(d.ten))}"></label>`).join('')}</div>
            <label class="field field--check full"><input type="checkbox" name="dongY"> Tôi xác nhận thông tin trên là chính xác. Thanh toán khi trả phòng.</label>
          </div>
        </div>
        <aside class="g-book__summary" id="g-sum" aria-live="polite"><p class="muted">Đang tính giá…</p></aside>
      </form>`,
      footer: [{ text: 'Đóng' }, { text: 'Xác nhận đặt phòng', cls: 'btn--primary', onClick: () => submit() }],
    });
    const form = $('#g-book', body);
    const svcString = () => [...form.querySelectorAll('[data-svc]')].filter((i) => Number(i.value) > 0)
      .map((i) => `${i.dataset.svc}:${Math.min(100, Math.floor(Number(i.value)))}`).join(',');

    const quote = debounce(async () => {
      const sum = $('#g-sum', body);
      if (!sum) return; // modal da dong / da chuyen sang man hinh thanh cong
      try {
        const sdt = (form.sdt.value || '').trim();
        const q = await Api.post('khach/baogia', { maPhong, ngayNhan: s.ngayNhan, soDem: s.soDem, dichVu: svcString(), sdt: /^0\d{9}$/.test(sdt) ? sdt : '' });
        sum.innerHTML = `<h3>Tạm tính</h3>
          <div class="invoice__line invoice__line--big"><span class="chip-room">Tiền phòng</span><span>${money(q.tienPhong)}</span></div>
          <div class="invoice__line invoice__line--sub"><span>${s.soDem} đêm, mùa ${esc(vi(q.mua).toLowerCase())}</span><span></span></div>
          <div class="invoice__line invoice__line--big"><span class="chip-service">Dịch vụ</span><span>${money(q.tienDichVu)}</span></div>
          ${q.giam ? `<div class="invoice__line"><span>Ưu đãi thành viên ${Math.round(q.tyLeGiam * 100)}%</span><span class="trend-up">−${money(q.giam)}</span></div>` : ''}
          <div class="invoice__line"><span>VAT 10%</span><span>${money(q.thue)}</span></div>
          <div class="invoice__total"><span>Tổng dự kiến</span><span>${money(q.tong)}</span></div>
          ${q.canhBaoMua ? `<p class="muted" style="font-size:12.5px;margin:10px 0 0">${esc(q.canhBaoMua)}</p>` : ''}
          <p class="muted" style="font-size:12px;margin:8px 0 0">Giá đã tính theo cùng công thức với hóa đơn. Là thành viên? Nhập số điện thoại để áp dụng ưu đãi.</p>`;
      } catch (e) { sum.innerHTML = `<p class="muted">${esc(e.message)}</p>`; }
    }, 200);
    form.addEventListener('input', (e) => { if (e.target.dataset.svc || e.target.name === 'sdt') quote(); });
    // Sua o nao thi bo bao loi o do
    form.addEventListener('change', (e) => {
      const field = e.target.closest('.field.is-invalid');
      if (field) { field.classList.remove('is-invalid'); field.querySelector('.field__error')?.remove(); }
    });
    quote();

    const submit = async () => {
      const v = Object.fromEntries(new FormData(form));
      Object.keys(v).forEach((k) => (v[k] = String(v[k]).trim()));
      if (showErrors(form, validate(v))) return;
      try {
        const d = await Api.post('khach/datphong', {
          maPhong, ngayNhan: s.ngayNhan, soDem: s.soDem, soNguoi: s.soNguoi,
          hoTen: v.hoTen, sdt: v.sdt, cccd: v.cccd, email: v.email, quocTich: v.quocTich,
          gioiTinh: v.gioiTinh, diaChiThuongTru: v.diaChiThuongTru, ghiChu: v.ghiChu, dichVu: svcString(),
        });
        showSuccess(d);
        search({ scroll: false }); // cap nhat lai danh sach phong trong
      } catch (e) {
        toast(e.message, 'error');
        if (/vua co khach dat|vừa có khách đặt/i.test(e.message)) { closeModal(); search(); }
      }
    };
  }

  function setStep(n) {
    document.querySelectorAll('#g-steps .g-steps__item').forEach((el) => {
      const s = Number(el.dataset.step);
      el.classList.toggle('is-active', s === n);
      el.classList.toggle('is-done', s < n);
    });
  }

  function showSuccess(d) {
    setStep(3);
    const code = d.maPhieu || '';
    modal({
      title: 'Đặt phòng thành công', size: '',
      body: `<div class="g-success">
        <div class="g-success__icon">${icon('check')}</div>
        <h3>Cảm ơn ${esc(d.khach)}</h3>
        <p class="muted" style="margin:0">Phòng đã được giữ. Vui lòng lưu mã phiếu để nhận phòng tại lễ tân.</p>
        <div class="g-success__code" id="g-ma-phieu">${esc(code)}</div>
        <dl class="detail-list">
          <dt>Chi nhánh</dt><dd>${esc(branchName(d.chiNhanh))}</dd>
          <dt>Phòng</dt><dd>${esc(d.loai)} ${esc(d.phong)}</dd>
          <dt>Ngày ở</dt><dd>${esc(d.ngayNhan)} → ${esc(d.ngayTra)}</dd>
          <dt>Tổng dự kiến</dt><dd>${money(d.tong)}</dd>
          ${d.giamGia > 0 ? `<dt>Ưu đãi</dt><dd>Khách thành viên được giảm ${Math.round(d.giamGia * 100)}% khi thanh toán</dd>` : ''}
        </dl>
        <div class="g-success__hint">Mang theo <b>CCCD/hộ chiếu</b> và mã phiếu <b>${esc(code)}</b> khi nhận phòng.
          Nếu không đến đúng ngày nhận, hệ thống tự hủy phiếu và <b>không hoàn cọc</b> (đặt online hiện chưa thu cọc).</div>
        <div class="g-success__actions">
          <button type="button" class="btn btn--outline" id="g-copy-ma">Sao chép mã phiếu</button>
        </div>
      </div>`,
      footer: [{ text: 'Hoàn tất', cls: 'btn--primary' }],
    });
    const btn = document.getElementById('g-copy-ma');
    if (btn) btn.addEventListener('click', async () => {
      try {
        await navigator.clipboard.writeText(code);
        toast('Đã sao chép mã ' + code, 'success');
      } catch (_) {
        toast('Không sao chép được. Hãy ghi lại mã ' + code, 'info');
      }
    });
  }

  /* ---------- Tra cuu phieu / hoa don ---------- */
  function moneySafe(v) { return money(Number(v) || 0); }
  function renderHoaDon(hd) {
    const lines = (hd.chiTiet || []).map((c) =>
      `<div class="invoice__line"><span>${esc(vi(c.noiDung || c.loai))} ×${c.soLuong}</span><span>${moneySafe(c.thanhTien)}</span></div>`).join('');
    return `<div class="g-lookup__bill">
      <h3>Hóa đơn ${esc(hd.ma)}</h3>
      <dl class="g-lookup__meta">
        <div><dt>Khách</dt><dd>${esc(hd.khach || '')}</dd></div>
        <div><dt>Phòng</dt><dd>${esc(hd.loaiPhong || '')} ${esc(hd.soPhong || '')}</dd></div>
        <div><dt>Chi nhánh</dt><dd>${esc(vi(hd.chiNhanh || ''))}</dd></div>
        <div><dt>Ngày ở</dt><dd>${esc(hd.ngayNhan || '')} → ${esc(hd.ngayTra || '')}</dd></div>
        <div><dt>Trạng thái</dt><dd>${hd.daThanhToan ? 'Đã thanh toán' + (hd.phuongThuc ? ' (' + esc(vi(hd.phuongThuc)) + ')' : '') : 'Chưa thanh toán'}</dd></div>
        <div><dt>Lập lúc</dt><dd>${esc(hd.tgLap || '')}</dd></div>
      </dl>
      ${lines}
      <div class="invoice__line"><span>Tạm tính</span><span>${moneySafe(hd.tamTinh)}</span></div>
      <div class="invoice__line"><span>Giảm thành viên (${Math.round((hd.tyLeGiam || 0) * 100)}%)</span><span>-${moneySafe(hd.tienGiam)}</span></div>
      <div class="invoice__line"><span>VAT 10%</span><span>${moneySafe(hd.thue)}</span></div>
      <div class="invoice__line"><span>Đã cọc</span><span>${moneySafe(hd.coc)}</span></div>
      <div class="invoice__total"><span>Tổng / còn lại</span><span>${moneySafe(hd.tong)} / ${moneySafe(hd.conLai)}</span></div>
    </div>`;
  }
  function renderPhieu(ph) {
    return `<div class="g-lookup__bill">
      <h3>Phiếu ${esc(ph.ma)} · ${esc(ph.trangThai || '')}</h3>
      <dl class="g-lookup__meta">
        <div><dt>Khách</dt><dd>${esc(ph.khach || '')}</dd></div>
        <div><dt>SĐT</dt><dd>${esc(ph.sdt || '')}</dd></div>
        <div><dt>Phòng</dt><dd>${esc(ph.loaiPhong || '')} ${esc(ph.soPhong || '')}</dd></div>
        <div><dt>Chi nhánh</dt><dd>${esc(vi(ph.chiNhanh || ''))}</dd></div>
        <div><dt>Ngày ở</dt><dd>${esc(ph.ngayNhan || '')} → ${esc(ph.ngayTra || '')} (${ph.soDem || 0} đêm)</dd></div>
        <div><dt>Tiền phòng / DV</dt><dd>${moneySafe(ph.tienPhong)} / ${moneySafe(ph.tienDichVu)}</dd></div>
        <div><dt>Cọc</dt><dd>${moneySafe(ph.tienCoc)}${ph.ptCoc ? ' · ' + esc(vi(ph.ptCoc)) : ''}</dd></div>
        <div><dt>Mã hóa đơn</dt><dd>${ph.maHoaDon ? esc(ph.maHoaDon) : 'Chưa có (chưa check-out)'}</dd></div>
      </dl>
    </div>`;
  }
  async function lookupBill(e) {
    e.preventDefault();
    const f = e.target;
    const ma = String(new FormData(f).get('ma') || '').trim();
    const sdt = String(new FormData(f).get('sdt') || '').trim();
    const box = $('#g-lookup-result');
    if (!ma || !sdt) return toast('Vui lòng nhập đủ mã và số điện thoại.', 'error');
    const btn = f.querySelector('button');
    btn.disabled = true; btn.textContent = 'Đang tra…';
    try {
      // Thuuuuuuuuuu: ma HD -> hoa don; ma DP -> phieu (+ hoa don neu co)
      const isHD = /^HD/i.test(ma);
      if (isHD) {
        const hd = await Api.get('khach/tracuhoadon', { ma, sdt });
        box.innerHTML = renderHoaDon(hd);
      } else {
        const d = await Api.get('khach/tracuuphieu', { ma, sdt });
        box.innerHTML = renderPhieu(d.phieu) + (d.hoaDon ? renderHoaDon(d.hoaDon) : '');
      }
      box.hidden = false;
      box.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
    } catch (err) {
      box.hidden = true;
      toast(err.message || 'Không tra cứu được.', 'error');
    } finally {
      btn.disabled = false; btn.textContent = 'Tra cứu';
    }
  }

  /* ---------- Header, menu, cuon trang ---------- */
  function bindLayout() {
    $('#g-burger').innerHTML = icon('menu', 22);
    const toTop = $('#g-totop');
    if (toTop) toTop.innerHTML = icon('arrowUp', 18);
    const sticky = $('#g-sticky-cta');
    const onScrollCta = () => {
      if (!sticky) return;
      const zone = $('#dat-phong');
      const pastHero = window.scrollY > 420;
      const inZone = zone && zone.getBoundingClientRect().top < 120 && zone.getBoundingClientRect().bottom > 80;
      sticky.classList.toggle('is-visible', pastHero && !inZone);
    };
    window.addEventListener('scroll', onScrollCta, { passive: true });
    onScrollCta();
    $('#modal .modal__header .icon-btn').innerHTML = icon('close', 18);
    hydrateIcons(document);
    $('#g-year').textContent = new Date().getFullYear();

    const header = $('#g-header');
    $('#g-burger').addEventListener('click', () => {
      const open = header.classList.toggle('is-open');
      $('#g-burger').setAttribute('aria-expanded', open);
      $('#g-burger').innerHTML = icon(open ? 'close' : 'menu', 22);
    });
    $('#g-nav').addEventListener('click', (e) => {
      if (!e.target.closest('a')) return;
      header.classList.remove('is-open');
      $('#g-burger').innerHTML = icon('menu', 22);
    });

    // Header dac khi cuon qua banner; danh dau menu theo muc dang xem
    const sections = ['categories', 'offers', 'rooms', 'services', 'tra-cuu', 'branches', 'contact'].map((id) => document.getElementById(id)).filter(Boolean);
    const onScroll = () => {
      const y = window.scrollY;
      header.classList.toggle('is-solid', y > 120);
      $('#g-totop').classList.toggle('is-visible', y > 600);
      let cur = 'top';
      sections.forEach((s) => { if (s.getBoundingClientRect().top < 140) cur = s.id; });
      document.querySelectorAll('.g-nav a').forEach((a) => a.classList.toggle('is-active', a.getAttribute('href') === '#' + cur));
    };
    window.addEventListener('scroll', onScroll, { passive: true });
    onScroll();

    $('#modal').addEventListener('click', (e) => { if (e.target.closest('[data-close]')) closeModal(); });
    document.addEventListener('keydown', (e) => { if (e.key === 'Escape' && !$('#modal').hidden) closeModal(); });

    // Form kiem tra phong
    const f = $('#g-check-form');
    f.ngayNhan.value = todayIso(); f.ngayNhan.min = todayIso();
    f.ngayTra.value = todayIso(2); f.ngayTra.min = todayIso(1);
    f.ngayNhan.addEventListener('change', () => {
      f.ngayTra.min = addDays(f.ngayNhan.value, 1);
      if (f.ngayTra.value <= f.ngayNhan.value) f.ngayTra.value = addDays(f.ngayNhan.value, 1);
    });
    f.addEventListener('submit', (e) => { e.preventDefault(); handleSearchRooms(); });
    const lf = $('#g-lookup-form');
    if (lf) lf.addEventListener('submit', lookupBill);

    // Ủy quyền sự kiện: thẻ loại phòng → danh sách; chọn phòng → chi tiết / đặt
    document.addEventListener('click', (e) => {
      // Pill chọn chi nhánh trên form tìm phòng
      const setCn = e.target.closest('[data-set-cn]');
      if (setCn) {
        e.preventDefault();
        setGuestBranch(setCn.dataset.setCn || '');
        return;
      }
      // Quay lại trang loại phòng
      if (e.target.closest('#g-room-list-back')) {
        e.preventDefault();
        const sec = $('#room-list');
        if (sec) sec.hidden = true;
        const roomsSec = $('#rooms');
        if (roomsSec) roomsSec.scrollIntoView({ behavior: 'smooth', block: 'start' });
        try { history.replaceState(null, '', '#rooms'); } catch (_) { location.hash = '#rooms'; }
        return;
      }

      // Tab chi nhánh trên danh sách phòng
      const listBr = e.target.closest('[data-list-branch]');
      if (listBr) {
        e.preventDefault();
        state.listBranch = listBr.dataset.listBranch || '';
        renderRoomListByType();
        return;
      }

      // Xem chi tiết phòng
      const detailBtn = e.target.closest('[data-room-detail]');
      if (detailBtn) {
        e.preventDefault();
        e.stopPropagation();
        showRoomDetail(detailBtn.dataset.roomDetail);
        return;
      }

      // Chọn phòng → mở form đặt (bước 5–6)
      const selectBtn = e.target.closest('[data-select-room]');
      if (selectBtn) {
        e.preventDefault();
        e.stopPropagation();
        openBookingFromList(selectBtn.dataset.selectRoom);
        return;
      }

      // Thẻ loại phòng (hình / tên / Đặt ngay / mũi tên / cả thẻ) → danh sách phòng
      const roomTypeEl = e.target.closest('[data-room-type]');
      if (roomTypeEl) {
        e.preventDefault();
        e.stopPropagation();
        if (!$('#modal')?.hidden) closeModal();
        showRoomListByType(roomTypeEl.dataset.roomType || '');
        return;
      }

      // Dịch vụ: Đặt ngay → cuộn tới mục Phòng & Suite để khách tự chọn phòng
      if (e.target.closest('[data-goto-rooms]')) {
        e.preventDefault();
        goToRooms();
        return;
      }
      const sg = e.target.closest('[data-svc-group]');
      if (sg) return showServiceGroup(sg.dataset.svcGroup);

      const t = e.target.closest('[data-book],[data-branch],[data-filter]');
      if (!t) return;
      if (t.dataset.book) return openBooking(t.dataset.book);
      if (t.dataset.filter !== undefined) {
        state.type = t.dataset.filter;
        return renderResults();
      }

      ensureSearchDates();

      // "Xem phòng trống" tại thẻ chi nhánh: gán branchId + tìm + tới Phòng & Suite
      if (t.dataset.branch) {
        const maCN = t.dataset.branch;
        setGuestBranch(maCN);
        state.type = '';
        handleSearchRooms();
        return;
      }
    });
  }

  async function start() {
    bindLayout();
    try {
      state.info = await Api.get('khach/thongtin');
      renderInfo();
    } catch (e) {
      $('#g-room-types').innerHTML = `<div class="panel empty-state">${esc(e.message)}</div>`;
    }
  }
  return { start };
})();

Guest.start();
