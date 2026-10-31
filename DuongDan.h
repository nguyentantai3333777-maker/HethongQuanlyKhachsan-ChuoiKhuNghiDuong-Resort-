// DuongDan.h - Xac dinh thu muc du lieu CSV mot cach on dinh (khong phu thuoc thu muc lam viec hien tai)
#pragma once
#include <string>
#include <vector>

namespace DuongDan
{
    // Thu muc chua file .exe / binary dang chay ("" neu khong xac dinh duoc)
    std::string thuMucChayChuongTrinh(const std::string &argv0);

    // Thu tu uu tien:
    //   1. chiDinh (doi so dong lenh)         2. bien moi truong OCEANPEARL_DATA
    //   3. <thu muc exe>/data, <exe>/backend/data (va len toi 6 cap cha)
    //   4. <thu muc hien tai>/data, backend/data (va len toi 6 cap cha)
    // Chi chon thu muc co san chinhanh.csv (hoac chinhanh.txt). KHONG tao thu muc moi.
    // Neu khong tim thay: tra ve <thu muc exe>/data (de thong bao loi chi ro duong dan) va liet ke cac noi da tim.
    std::string timThuMucDuLieu(const std::string &argv0, const std::string &chiDinh,
                                std::vector<std::string> *daTim = nullptr);
}
