@echo off
REM Console Pauser 编译脚本
REM 使用 clang -Oz 编译 Win32 x86 极致体积版本

set CLANG=D:\Project\LLVM\build\bin\clang++.exe
set SRC=src\main.cpp
set OUT=consolepauser.exe

echo ========================================
echo Console Pauser 编译
echo ========================================
echo.

echo [1/3] 编译 Win32 x86 版本 (clang -Oz)...
"%CLANG%" -Oz -target i686-pc-windows-msvc ^
    -fno-exceptions -fno-rtti ^
    -static-libgcc -static-libstdc++ ^
    -o "%OUT%" "%SRC%" ^
    -lkernel32 -luser32 -lmsvcrt

if %errorlevel% neq 0 (
    echo.
    echo [错误] 编译失败！
    exit /b 1
)

echo.
echo [2/3] 编译成功！
echo.

echo [3/3] 文件信息：
dir "%OUT%" | findstr ".exe"
echo.

for %%A in ("%OUT%") do echo 文件大小: %%~zA 字节
echo.
echo ========================================
echo 编译完成！
echo ========================================
