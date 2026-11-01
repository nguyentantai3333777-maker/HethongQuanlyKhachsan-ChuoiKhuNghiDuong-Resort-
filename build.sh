#!/bin/sh
# Bien dich nhom lop Thuc the + Nghiep vu va chay kiem thu (Linux / macOS)
cd "$(dirname "$0")" || exit 1
INC="-Isrc/entity -Isrc/business -Isrc/support"
g++ -std=c++17 -O2 -pthread $INC src/entity/*.cpp src/business/*.cpp src/support/*.cpp \
    tests/test_nghiep_vu.cpp -o test_nghiep_vu && echo "Bien dich xong: ./test_nghiep_vu"
