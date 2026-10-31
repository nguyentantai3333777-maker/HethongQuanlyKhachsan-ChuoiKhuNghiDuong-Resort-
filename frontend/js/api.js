/* =====================================================================
   api.js - Giao tiep voi backend C++ (WebApi.cpp)
   - GET: tham so tren URL;  POST: form-urlencoded (C++ tach de dang)
   - Token dang nhap gui qua header X-Token
   - Backend tra ve {ok, data, thongBao} hoac {ok:false, loi}
   ===================================================================== */
const Api = (() => {
  let token = null;
  try { token = sessionStorage.getItem('hotel_token'); } catch (e) { token = null; }

  function setToken(t) {
    token = t;
    try { t ? sessionStorage.setItem('hotel_token', t) : sessionStorage.removeItem('hotel_token'); } catch (e) { /* bo qua */ }
  }

  async function request(method, path, params = {}) {
    const query = new URLSearchParams();
    Object.entries(params).forEach(([k, v]) => {
      if (v !== undefined && v !== null) query.append(k, typeof v === 'boolean' ? (v ? '1' : '0') : String(v));
    });
    const opts = { method, headers: {} };
    let url = '/api/' + path;
    if (method === 'GET') {
      const qs = query.toString();
      if (qs) url += '?' + qs;
    } else {
      opts.headers['Content-Type'] = 'application/x-www-form-urlencoded; charset=UTF-8';
      opts.body = query.toString();
    }
    if (token) opts.headers['X-Token'] = token;

    let res;
    try {
      res = await fetch(url, opts);
    } catch (e) {
      throw new Error('Không kết nối được máy chủ C++. Hãy kiểm tra chương trình đang chạy ở chế độ Web.');
    }
    let json;
    try { json = await res.json(); } catch (e) { throw new Error('Phản hồi từ máy chủ không hợp lệ.'); }
    if (res.status === 401 && path !== 'dangnhap') {
      setToken(null);
      if (typeof App !== 'undefined') App.showLogin('Phiên đăng nhập đã hết hạn. Vui lòng đăng nhập lại.');
    }
    if (!json.ok) throw new Error(json.loi || 'Không thể thực hiện thao tác. Vui lòng kiểm tra dữ liệu.');
    return json.data;
  }

  return {
    get: (path, params) => request('GET', path, params),
    post: (path, params) => request('POST', path, params),
    setToken,
    hasToken: () => !!token,
  };
})();
