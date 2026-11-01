// Utf8Text.h - Xu ly chuoi UTF-8 theo DO RONG HIEN THI (so cot tren console), dung cho moi bang / khung.
//
// Vi sao can: std::setw() dem theo BYTE. Chu tieng Viet co dau ("Nguyễn") ton 2-3 byte moi ky tu, nen
// setw(20) << "Nguyễn" chi them it khoang trang hon can thiet -> cot bi lech. Module nay:
//   1. Giai ma UTF-8 thanh tung code point (docKyTu).
//   2. Tinh do rong hien thi cua tung ky tu (doRongKyTu): dau to hop (NFD) = 0 cot,
//      ky tu Dong A / emoji = 2 cot, con lai = 1 cot. Ma mau ANSI (\033[...m) = 0 cot.
//   3. Can le bang std::setw + std::left / std::right, nhung BU them so byte "du" cua chuoi UTF-8
//      (setw(w + soByte - doRong)) -> cot luon dung w o hien thi, khong phu thuoc chu co dau hay khong.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace Utf8
{
    // Doc 1 ky tu tai vi tri i (chuoi UTF-8); tra ve so byte cua ky tu (>= 1), ghi code point vao cp.
    // Byte loi duoc xem nhu 1 ky tu U+FFFD de khong bao gio lap vo han.
    size_t docKyTu(const std::string &s, size_t i, std::uint32_t &cp);
    // Do dai cua ma dieu khien ANSI bat dau tai i (0 neu khong phai ma ANSI)
    size_t doDaiMaAnsi(const std::string &s, size_t i);
    // So cot hien thi cua 1 code point: 0 (dau to hop, zero-width), 1 hoac 2 (chu Dong A, emoji)
    int doRongKyTu(std::uint32_t cp);
    // So cot hien thi cua ca chuoi (bo qua ma mau ANSI va ky tu danh dau \x02 \x03 cua ConsoleUI)
    int doRong(const std::string &s);

    // Cat chuoi de vua w cot (khong cat giua ky tu nhieu byte, giu ma mau); dauCat: chuoi them vao khi bi cat
    std::string catVua(const std::string &s, int w, const std::string &dauCat = "..");
    // Can le dung w cot bang std::setw (da bu byte UTF-8). Chuoi dai hon w duoc cat (".." o cuoi).
    std::string canTrai(const std::string &s, int w);
    std::string canPhai(const std::string &s, int w);
    std::string canGiua(const std::string &s, int w);
    // Ngat chuoi thanh cac dong <= w cot, ngat theo tu; tu dai hon w thi cat cung theo do rong.
    // '\n' trong chuoi = xuong dong co chu dich.
    std::vector<std::string> ngatDong(const std::string &s, int w);
}
