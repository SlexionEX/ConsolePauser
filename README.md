<<<<<<< HEAD
# ConsolePauser
Console Pauser for Slexion
=======
# Console Pauser - 全能控制台暂停器

自动为控制台程序暂停并返回退出码，支持 20 种语言，极致体积。

## 特性

- **20 种语言**：自动检测系统语言，支持手动指定
- **极致体积**：Win32 x86，仅 142 KB，单文件无依赖
- **传递退出码**：程序退出后返回子进程的退出码
- **显示运行时间**：精确到毫秒
- **完善的错误处理**：程序不存在、访问拒绝等错误提示
- **UTF-8 输出**：兼容控制台和输出重定向

## 支持的语言

简体中文、繁體中文、English、日本語、한국어、Français、Deutsch、Español、Português、Italiano、Русский、العربية、ไทย、Tiếng Việt、Nederlands、Polski、Türkçe、Українська、Čeština、Svenska

## 用法

```
consolepauser [选项] -- <程序> [参数...]
```

## 选项

| 选项 | 说明 |
|------|------|
| `--no-pause` | 程序退出后不暂停 |
| `--no-time` | 不显示运行时间 |
| `--lang <代码>` | 指定语言（zh-CN, en, ja, ...） |
| `--help` | 显示帮助信息 |
| `--version` | 显示版本信息 |

## 示例

```bash
# 基本用法（暂停等待按键）
consolepauser -- myprogram.exe arg1 arg2

# 不暂停（只显示结果）
consolepauser --no-pause -- myprogram.exe

# 指定语言
consolepauser --lang ja -- myprogram.exe

# 在 IDE 中使用（如 Red Panda C++）
consolepauser --no-pause -- "$(TargetPath)"
```

## 编译

使用 clang + Visual Studio 环境编译：

```bash
# 设置 VS 环境
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x86

# 编译（极致体积优化）
clang++ -Oz -target i686-pc-windows-msvc ^
    -fno-exceptions -fno-rtti ^
    -flto=thin -fuse-ld=lld ^
    -fdata-sections -ffunction-sections ^
    -Wl,/OPT:REF,/OPT:ICF ^
    -o consolepauser.exe src/main.cpp ^
    -lkernel32 -luser32
```

## 技术细节

- **编译器**：LLVM clang 23.0
- **优化级别**：-Oz（极致体积优化）
- **目标平台**：i686-pc-windows-msvc（Win32 x86）
- **异常/RTTI**：禁用（减小体积）
- **LTO**：Thin LTO + LLD 链接器
- **输出编码**：UTF-8（SetConsoleOutputCP + WideCharToMultiByte）
- **进程管理**：CreateProcess + WaitForSingleObject + GetExitCodeProcess

## 项目结构

```
ConsolePauser/
├── src/
│   └── main.cpp          # 主程序（单文件实现）
├── include/               # 头文件目录（预留）
├── consolepauser.exe     # 编译产物
├── build.bat              # 编译脚本
└── README.md              # 本文件
```

## 许可证

MIT License
>>>>>>> dc42130 (feat: Console Pauser 初始提交 - 全能控制台暂停器)
