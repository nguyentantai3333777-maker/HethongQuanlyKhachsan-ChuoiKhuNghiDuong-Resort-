@echo off
rem Bien dich tren Windows (MSYS2 UCRT64 / MinGW-w64). -lws2_32: thu vien socket cho che do Web.
rem Bo qua test_build.cpp neu co (file thu nghiem).
setlocal EnableDelayedExpansion
set SRCS=
for %%f in (*.cpp) do (
  if /I not "%%f"=="test_build.cpp" set SRCS=!SRCS! %%f
)
g++ -std=c++17 -O2 %SRCS% -o main.exe -lws2_32
if %errorlevel% neq 0 (echo Bien dich that bai & pause & exit /b 1)
echo Bien dich xong: main.exe
