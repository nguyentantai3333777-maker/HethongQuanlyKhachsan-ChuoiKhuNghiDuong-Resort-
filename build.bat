@echo off
REM Bien dich nhom lop Thuc the + Nghiep vu va chay kiem thu (Windows / MinGW)
cd /d "%~dp0"
g++ -std=c++17 -O2 -pthread -Isrc/entity -Isrc/business -Isrc/support src/entity/*.cpp src/business/*.cpp src/support/*.cpp tests/test_nghiep_vu.cpp -o test_nghiep_vu.exe -lws2_32
if %errorlevel%==0 echo Bien dich xong: test_nghiep_vu.exe
