// main.cpp - Diem khoi chay chuong trinh
//   main                       : tu tim thu muc data (canh file .exe, backend/data, thu muc hien tai...)
//   main <thu muc data>        : chi dinh thu muc CSV
//   main --kiem-tra [thu muc]  : chi doc va bao cao so ban ghi tung file (khong ghi, khong mo menu)
#include "ChuongTrinh.h"
#include "DuongDan.h"
#include <cstring>
#include <iostream>

int main(int argc, char **argv)
{
    std::string chiDinh;
    bool kiemTra = false;
    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "--kiem-tra") || !strcmp(argv[i], "--check"))
            kiemTra = true;
        else if (argv[i][0] != '-')
            chiDinh = argv[i];
    }
    std::vector<std::string> daTim;
    const std::string thuMuc = DuongDan::timThuMucDuLieu(argc > 0 ? argv[0] : "", chiDinh, &daTim);
    ChuongTrinh app(thuMuc);
    if (kiemTra)
        return app.kiemTraDuLieu();
    app.chay();
    return 0;
}
