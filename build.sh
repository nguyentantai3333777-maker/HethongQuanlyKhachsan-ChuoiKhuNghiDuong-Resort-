#!/bin/sh
# Bien dich tren Linux / macOS
cd "$(dirname "$0")" && g++ -std=c++17 -O2 -pthread $(find . -maxdepth 1 -name '*.cpp' ! -name 'test_build.cpp' -printf '%f ') -o main && echo "Bien dich xong: ./main"
