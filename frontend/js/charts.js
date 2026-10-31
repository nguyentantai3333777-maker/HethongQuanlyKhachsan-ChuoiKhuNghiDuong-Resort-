/* =====================================================================
   charts.js - Bieu do ve bang <canvas> thuan (khong thu vien)
   Charts.line / Charts.bar / Charts.doughnut  - tu ve lai khi doi kich thuoc,
   co tooltip khi re chuot. Mau mac dinh lay tu bien CSS cua menu hien tai.
   ===================================================================== */
const Charts = (() => {
  // 6 sac nau dam nhat khac nhau ro - dung cho Bao cao & thong ke
  const BROWNS = ['#cfe4f3', '#9cc8e6', '#5fa8d8', '#1f6fa3', '#144a75', '#0e3557'];
  // Bang mau phan biet ro cac nhom so lieu (xanh la, vang, xanh duong, nau, do, tim)
  const PALETTE = ['#2f8a5e', '#c9a24a', '#1f6fa3', '#0e3557', '#c0443f', '#8a7bc2'];
  const cssVar = (n) => getComputedStyle(document.documentElement).getPropertyValue(n).trim();
  const compact = (v) => {
    const a = Math.abs(v);
    if (a >= 1e9) return (v / 1e9).toFixed(1).replace('.0', '') + ' tỷ';
    if (a >= 1e6) return (v / 1e6).toFixed(1).replace('.0', '') + ' tr';
    if (a >= 1e3) return Math.round(v / 1e3) + 'k';
    return String(Math.round(v));
  };

  // Chuan bi canvas theo kich thuoc khung + devicePixelRatio (net tren man hinh retina)
  function setup(el, height, draw) {
    el.classList.add('chart');
    el.innerHTML = '<canvas></canvas><div class="chart-tip"></div>';
    const canvas = el.querySelector('canvas');
    const tip = el.querySelector('.chart-tip');
    let hit = [];
    const paint = () => {
      const w = el.clientWidth || 300;
      const dpr = window.devicePixelRatio || 1;
      canvas.width = w * dpr; canvas.height = height * dpr;
      canvas.style.height = height + 'px';
      const ctx = canvas.getContext('2d');
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
      ctx.clearRect(0, 0, w, height);
      ctx.font = '12px "Be Vietnam Pro", system-ui, sans-serif';
      hit = draw(ctx, w, height) || [];
    };
    paint();
    if ('ResizeObserver' in window) new ResizeObserver(() => paint()).observe(el);
    canvas.addEventListener('mousemove', (e) => {
      const r = canvas.getBoundingClientRect();
      const x = e.clientX - r.left, y = e.clientY - r.top;
      const h = hit.find((b) => b.test(x, y));
      if (h) {
        tip.textContent = h.text; tip.style.left = h.x + 'px'; tip.style.top = h.y + 'px'; tip.style.opacity = 1;
      } else tip.style.opacity = 0;
    });
    canvas.addEventListener('mouseleave', () => { tip.style.opacity = 0; });
  }

  // Truc Y va luoi ngang
  function axes(ctx, box, max, fmt) {
    const steps = 4;
    ctx.strokeStyle = '#e3eaf1'; ctx.fillStyle = '#6c7b8a'; ctx.lineWidth = 1;
    ctx.textAlign = 'right'; ctx.textBaseline = 'middle';
    for (let i = 0; i <= steps; i++) {
      const v = (max / steps) * i;
      const y = box.y + box.h - (box.h * i) / steps;
      ctx.beginPath(); ctx.moveTo(box.x, y + 0.5); ctx.lineTo(box.x + box.w, y + 0.5); ctx.stroke();
      ctx.fillText(fmt(v), box.x - 8, y);
    }
  }
  const niceMax = (m) => {
    if (m <= 0) return 1;
    const p = Math.pow(10, Math.floor(Math.log10(m)));
    return Math.ceil(m / p * 1.1) * p;
  };

  function line(el, { labels, values, color, height = 260, money = true }) {
    setup(el, height, (ctx, w, h) => {
      const c = color || cssVar('--primary-color');
      const box = { x: 56, y: 14, w: w - 72, h: h - 44 };
      const max = niceMax(Math.max(...values, 0));
      const fmt = money ? compact : (v) => String(Math.round(v));
      axes(ctx, box, max, fmt);
      const n = values.length;
      const px = (i) => box.x + (n === 1 ? box.w / 2 : (box.w * i) / (n - 1));
      const py = (v) => box.y + box.h - (box.h * v) / max;
      // Vung to nhat duoi duong
      const grad = ctx.createLinearGradient(0, box.y, 0, box.y + box.h);
      grad.addColorStop(0, c + '33'); grad.addColorStop(1, c + '00');
      ctx.beginPath(); ctx.moveTo(px(0), box.y + box.h);
      values.forEach((v, i) => ctx.lineTo(px(i), py(v)));
      ctx.lineTo(px(n - 1), box.y + box.h); ctx.closePath(); ctx.fillStyle = grad; ctx.fill();
      ctx.beginPath(); values.forEach((v, i) => (i ? ctx.lineTo(px(i), py(v)) : ctx.moveTo(px(i), py(v))));
      ctx.strokeStyle = c; ctx.lineWidth = 2.5; ctx.lineJoin = 'round'; ctx.stroke();
      // Diem + nhan truc X
      ctx.textAlign = 'center'; ctx.textBaseline = 'top'; ctx.fillStyle = '#7b868a';
      const every = Math.ceil(n / Math.max(1, Math.floor(box.w / 70)));
      const hits = [];
      values.forEach((v, i) => {
        ctx.beginPath(); ctx.arc(px(i), py(v), 3.5, 0, Math.PI * 2);
        ctx.fillStyle = '#fff'; ctx.fill(); ctx.strokeStyle = c; ctx.lineWidth = 2; ctx.stroke();
        ctx.fillStyle = '#7b868a';
        if (i % every === 0 || i === n - 1) ctx.fillText(labels[i], px(i), box.y + box.h + 10);
        hits.push({ x: px(i), y: py(v), text: `${labels[i]}: ${money ? money_(v) : v}`, test: (x) => Math.abs(x - px(i)) < box.w / n / 2 + 2 });
      });
      return hits;
    });
  }

  function bar(el, { labels, values, colors, height = 260, money = true, unit = '' }) {
    setup(el, height, (ctx, w, h) => {
      const box = { x: 56, y: 14, w: w - 72, h: h - 44 };
      const max = niceMax(Math.max(...values, 0));
      const fmt = money ? compact : (v) => Math.round(v) + unit;
      axes(ctx, box, max, fmt);
      const n = values.length, slot = box.w / n, bw = Math.min(46, slot * 0.6);
      const base = cssVar('--primary-color');
      const hits = [];
      ctx.textAlign = 'center'; ctx.textBaseline = 'top';
      values.forEach((v, i) => {
        const x = box.x + slot * i + (slot - bw) / 2, bh = (box.h * v) / max, y = box.y + box.h - bh;
        ctx.fillStyle = colors ? colors[i % colors.length] : base;
        const r = Math.min(2, bw / 2, bh); // goc gan vuong nhu anh mau
        ctx.beginPath(); ctx.moveTo(x, y + bh); ctx.lineTo(x, y + r); ctx.quadraticCurveTo(x, y, x + r, y);
        ctx.lineTo(x + bw - r, y); ctx.quadraticCurveTo(x + bw, y, x + bw, y + r); ctx.lineTo(x + bw, y + bh); ctx.fill();
        ctx.fillStyle = '#7b868a';
        let lb = String(labels[i]);
        while (ctx.measureText(lb).width > slot - 4 && lb.length > 3) lb = lb.slice(0, -2) + '…';
        ctx.fillText(lb, x + bw / 2, box.y + box.h + 10);
        hits.push({ x: x + bw / 2, y, text: `${labels[i]}: ${money ? money_(v) : Math.round(v * 10) / 10 + unit}`, test: (mx, my) => mx >= x - 4 && mx <= x + bw + 4 && my >= box.y && my <= box.y + box.h });
      });
      return hits;
    });
  }

  function doughnut(el, { labels, values, colors, size = 170, center = '' }) {
    setup(el, size, (ctx, w, h) => {
      const total = values.reduce((a, b) => a + b, 0) || 1;
      const cx = w / 2, cy = h / 2, R = Math.min(w, h) / 2 - 4, r = R * 0.62;
      let a0 = -Math.PI / 2;
      const segs = [];
      values.forEach((v, i) => {
        const a1 = a0 + (Math.PI * 2 * v) / total;
        ctx.beginPath(); ctx.arc(cx, cy, R, a0, a1); ctx.arc(cx, cy, r, a1, a0, true); ctx.closePath();
        ctx.fillStyle = colors[i % colors.length]; ctx.fill();
        segs.push({ a0, a1, i });
        a0 = a1;
      });
      ctx.fillStyle = '#262b30'; ctx.textAlign = 'center'; ctx.textBaseline = 'middle';
      ctx.font = '500 24px "Playfair Display", Georgia, serif';
      ctx.fillText(center, cx, cy);
      return segs.map((s) => ({
        x: cx, y: cy - r / 2,
        text: `${labels[s.i]}: ${values[s.i]} (${Math.round((values[s.i] / total) * 100)}%)`,
        test: (x, y) => {
          const d = Math.hypot(x - cx, y - cy);
          let ang = Math.atan2(y - cy, x - cx); if (ang < -Math.PI / 2) ang += Math.PI * 2;
          return d >= r && d <= R && ang >= s.a0 && ang < s.a1;
        },
      }));
    });
  }

  // Chu thich (legend) dang danh sach
  function legend(labels, values, colors, fmt = (v) => v) {
    const total = values.reduce((a, b) => a + b, 0) || 1;
    return `<div class="legend">${labels.map((l, i) => `<div class="legend__item"><i style="background:${colors[i % colors.length]}"></i><span>${esc(l)}</span><b>${fmt(values[i])} <span class="muted">${Math.round((values[i] / total) * 100)}%</span></b></div>`).join('')}</div>`;
  }

  const money_ = (v) => nf.format(Math.round(v)) + ' đ';
  return { line, bar, doughnut, legend, BROWNS, PALETTE, cssVar };
})();
