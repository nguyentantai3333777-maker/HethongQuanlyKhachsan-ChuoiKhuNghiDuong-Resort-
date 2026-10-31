/* Ô ngày hiển thị dd/mm/yyyy cho mọi <input type="date">.
 * Giữ nguyên input gốc (ẩn): .value vẫn là yyyy-mm-dd, .min / sự kiện change / form giữ nguyên
 * nên toàn bộ logic đặt phòng hiện có không phải đổi. Chỉ phần HIỂN THỊ và NHẬP là dd/mm/yyyy. */
(function () {
  const valueDesc = Object.getOwnPropertyDescriptor(HTMLInputElement.prototype, 'value');
  const CAL = '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><rect x="3" y="4.5" width="18" height="16" rx="2.5"/><path d="M3 9.5h18M8 2.5v4M16 2.5v4"/></svg>';

  function isoToDmy(v) {
    const m = /^(\d{4})-(\d{2})-(\d{2})$/.exec(v || '');
    return m ? `${m[3]}/${m[2]}/${m[1]}` : '';
  }
  function dmyToIso(s) {
    const m = /^(\d{1,2})\/(\d{1,2})\/(\d{4})$/.exec((s || '').trim());
    if (!m) return null;
    const d = +m[1], mo = +m[2], y = +m[3];
    const t = new Date(y, mo - 1, d);
    if (t.getFullYear() !== y || t.getMonth() !== mo - 1 || t.getDate() !== d) return null;
    return `${y}-${String(mo).padStart(2, '0')}-${String(d).padStart(2, '0')}`;
  }
  // 31102026 -> 31/10/2026 (tự chèn dấu /)
  function mask(raw) {
    const dg = raw.replace(/\D/g, '').slice(0, 8);
    if (dg.length <= 2) return dg;
    if (dg.length <= 4) return `${dg.slice(0, 2)}/${dg.slice(2)}`;
    return `${dg.slice(0, 2)}/${dg.slice(2, 4)}/${dg.slice(4)}`;
  }

  function enhance(inp) {
    if (inp.dataset.dfDone || !inp.parentNode) return;
    inp.dataset.dfDone = '1';

    const wrap = document.createElement('x-date'); // thẻ tùy biến: không dính CSS `label span` của thanh tìm phòng
    wrap.className = 'df';
    const txt = document.createElement('input');
    txt.type = 'text';
    txt.className = (inp.className + ' df__text').trim();
    txt.placeholder = 'dd/mm/yyyy';
    txt.maxLength = 10;
    txt.inputMode = 'numeric';
    txt.autocomplete = 'off';
    if (inp.id) txt.setAttribute('aria-label', 'Ngày (dd/mm/yyyy)');
    const btn = document.createElement('button');
    btn.type = 'button';
    btn.className = 'df__btn';
    btn.setAttribute('aria-label', 'Chọn ngày từ lịch');
    btn.tabIndex = -1;
    btn.innerHTML = CAL;

    inp.parentNode.insertBefore(wrap, inp);
    wrap.append(txt, inp, btn);

    // Input gốc: ẩn khỏi giao diện nhưng vẫn render để showPicker() hiện lịch ngay dưới ô
    const hide = { position: 'absolute', left: '0', bottom: '0', width: '100%', height: '1px', opacity: '0', 'pointer-events': 'none', padding: '0', margin: '0', border: '0', 'min-height': '0' };
    Object.entries(hide).forEach(([k, v]) => inp.style.setProperty(k, v, 'important'));
    inp.tabIndex = -1;
    inp.setAttribute('aria-hidden', 'true');

    const show = () => { txt.value = isoToDmy(valueDesc.get.call(inp)); wrap.classList.remove('is-invalid'); };
    // Gán .value bằng code (vd. f.ngayNhan.value = todayIso()) cũng cập nhật ô hiển thị
    Object.defineProperty(inp, 'value', {
      configurable: true,
      get() { return valueDesc.get.call(inp); },
      set(v) { valueDesc.set.call(inp, v); show(); },
    });
    show();

    txt.addEventListener('input', () => {
      txt.value = mask(txt.value);
      const iso = txt.value.length === 10 ? dmyToIso(txt.value) : null;
      wrap.classList.toggle('is-invalid', txt.value.length === 10 && !iso);
      if (iso && iso !== valueDesc.get.call(inp)) {
        valueDesc.set.call(inp, iso);
        inp.dispatchEvent(new Event('input', { bubbles: true }));
        inp.dispatchEvent(new Event('change', { bubbles: true }));
      }
    });
    txt.addEventListener('blur', show); // chưa đủ / sai ngày -> trả về ngày hợp lệ gần nhất
    txt.addEventListener('keydown', (e) => { if (e.key === 'Enter') show(); });
    inp.addEventListener('change', show); // chọn từ lịch

    btn.addEventListener('click', (e) => {
      e.preventDefault();
      e.stopPropagation();
      try { inp.showPicker(); } catch (_) { txt.focus(); }
    });
  }

  function scan(root) {
    (root.querySelectorAll ? root : document).querySelectorAll('input[type="date"]').forEach(enhance);
  }
  function start() {
    scan(document);
    new MutationObserver((list) => {
      for (const m of list) m.addedNodes.forEach((n) => {
        if (n.nodeType !== 1) return;
        if (n.matches?.('input[type="date"]')) enhance(n);
        else scan(n);
      });
    }).observe(document.body, { childList: true, subtree: true });
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start);
  else start();
})();
