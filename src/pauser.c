/*
 * Console Pauser - 全能控制台暂停工具
 * 极致体积 Win32 x86 C
 * 20 种语言翻译
 * clang -Oz 编译
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ========== 20 种语言定义 ========== */
typedef enum {
    LANG_ZH_CN = 0,  /* 简体中文 */
    LANG_ZH_TW,      /* 繁体中文 */
    LANG_EN,         /* English */
    LANG_JA,         /* 日本語 */
    LANG_KO,         /* 한국어 */
    LANG_FR,         /* Français */
    LANG_DE,         /* Deutsch */
    LANG_ES,         /* Español */
    LANG_PT,         /* Português */
    LANG_IT,         /* Italiano */
    LANG_RU,         /* Русский */
    LANG_AR,         /* العربية */
    LANG_TH,         /* ไทย */
    LANG_VI,         /* Tiếng Việt */
    LANG_NL,         /* Nederlands */
    LANG_PL,         /* Polski */
    LANG_TR,         /* Türkçe */
    LANG_UK,         /* Українська */
    LANG_CS,         /* Čeština */
    LANG_SV,         /* Svenska */
    LANG_COUNT
} LanguageID;

/* 语言名称 */
static const char* LANG_NAMES[LANG_COUNT] = {
    "简体中文", "繁體中文", "English", "日本語", "한국어",
    "Français", "Deutsch", "Español", "Português", "Italiano",
    "Русский", "العربية", "ไทย", "Tiếng Việt", "Nederlands",
    "Polski", "Türkçe", "Українська", "Čeština", "Svenska"
};

/* 语言代码 */
static const char* LANG_CODES[LANG_COUNT] = {
    "zh-CN", "zh-TW", "en", "ja", "ko",
    "fr", "de", "es", "pt", "it",
    "ru", "ar", "th", "vi", "nl",
    "pl", "tr", "uk", "cs", "sv"
};

/* 暂停提示文本（20种语言） */
static const char* PAUSE_TEXT[LANG_COUNT] = {
    "按任意键继续...",
    "按任意鍵繼續...",
    "Press any key to continue...",
    "任意のキーを押して続行...",
    "계속하려면 아무 키나 누르세요...",
    "Appuyez sur une touche pour continuer...",
    "Drücken Sie eine beliebige Taste, um fortzufahren...",
    "Presione cualquier tecla para continuar...",
    "Pressione qualquer tecla para continuar...",
    "Premi un tasto qualsiasi per continuare...",
    "Нажмите любую клавишу для продолжения...",
    "اضغط على أي مفتاح للمتابعة...",
    "กดปุ่มใดๆ เพื่อดำเนินการต่อ...",
    "Nhấn phím bất kỳ để tiếp tục...",
    "Druk op een toets om door te gaan...",
    "Naciśnij dowolny klawisz, aby kontynuować...",
    "Devam etmek için herhangi bir tuşa basın...",
    "Натисніть будь-яку клавішу для продовження...",
    "Stiskněte libovolnou klávesu pro pokračování...",
    "Tryck på valfri tangent för att fortsätta..."
};

/* 超时提示 */
static const char* TIMEOUT_TEXT[LANG_COUNT] = {
    "超时，自动继续",
    "逾時，自動繼續",
    "Timeout, continuing automatically",
    "タイムアウト、自動的に続行します",
    "시간 초과, 자동으로 계속합니다",
    "Délai d'attente dépassé, continuation automatique",
    "Zeitüberschreitung, automatische Fortsetzung",
    "Tiempo de espera agotado, continuando automáticamente",
    "Tempo esgotado, continuando automaticamente",
    "Timeout, continuazione automatica",
    "Время ожидания истекло, автоматическое продолжение",
    "انتهت المهلة، المتابعة تلقائيًا",
    "หมดเวลา, ดำเนินการต่อโดยอัตโนมัติ",
    "Hết giờ, tự động tiếp tục",
    "Time-out, automatisch doorgaan",
    "Przekroczono limit czasu, automatyczne kontynuowanie",
    "Zaman aşımı, otomatik olarak devam ediliyor",
    "Час вийшов, автоматичне продовження",
    "Časový limit vypršel, automatické pokračování",
    "Tidsgränsen uppnådd, fortsätter automatiskt"
};

/* 退出码提示 */
static const char* EXIT_CODE_TEXT[LANG_COUNT] = {
    "退出码",
    "結束代碼",
    "Exit code",
    "終了コード",
    "종료 코드",
    "Code de sortie",
    "Exit-Code",
    "Código de salida",
    "Código de saída",
    "Codice di uscita",
    "Код выхода",
    "رمز الخروج",
    "รหัสทางออก",
    "Mã thoát",
    "Exitcode",
    "Kod wyjścia",
    "Çıkış kodu",
    "Код виходу",
    "Návratový kód",
    "Avslutningskod"
};

/* ========== 配置结构 ========== */
typedef struct {
    LanguageID lang;          /* 语言 */
    int timeout;               /* 超时秒数，0=无限 */
    int exit_code;             /* 返回的退出码 */
    int beep;                  /* 是否蜂鸣 */
    int silent;                /* 静默模式（不显示提示） */
    int show_time;             /* 显示当前时间 */
    int show_date;             /* 显示当前日期 */
    int no_color;              /* 禁用颜色 */
    char custom_text[512];     /* 自定义提示文本 */
    int has_custom_text;       /* 是否有自定义文本 */
} Config;

/* ========== 工具函数 ========== */

/* 获取控制台输出句柄 */
static HANDLE GetStdOut(void) {
    return GetStdHandle(STD_OUTPUT_HANDLE);
}

/* 获取控制台输入句柄 */
static HANDLE GetStdIn(void) {
    return GetStdHandle(STD_INPUT_HANDLE);
}

/* 设置控制台颜色 */
static void SetColor(WORD color) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hOut = GetStdOut();
    if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
        SetConsoleTextAttribute(hOut, color);
    }
}

/* 重置控制台颜色 */
static void ResetColor(void) {
    SetColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

/* 输出字符串 */
static void PrintStr(const char* str) {
    DWORD written;
    WriteConsoleA(GetStdOut(), str, (DWORD)strlen(str), &written, NULL);
}

/* 输出换行 */
static void PrintLn(void) {
    PrintStr("\r\n");
}

/* 检测是否在控制台中运行 */
static int IsConsole(void) {
    HANDLE hOut = GetStdOut();
    DWORD mode;
    return GetConsoleMode(hOut, &mode);
}

/* 检测是否有管道输入 */
static int HasPipeInput(void) {
    HANDLE hIn = GetStdIn();
    DWORD mode;
    if (!GetConsoleMode(hIn, &mode)) {
        /* 不是控制台输入，可能是管道 */
        return 1;
    }
    return 0;
}

/* 读取单个按键（不回显） */
static int ReadKey(void) {
    HANDLE hIn = GetStdIn();
    INPUT_RECORD ir;
    DWORD read;

    while (ReadConsoleInputA(hIn, &ir, 1, &read)) {
        if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) {
            return ir.Event.KeyEvent.wVirtualKeyCode;
        }
    }
    return 0;
}

/* 等待按键或超时 */
static int WaitKeyOrTimeout(int timeout_seconds) {
    HANDLE hIn = GetStdIn();
    INPUT_RECORD ir;
    DWORD read;
    DWORD start = GetTickCount();
    DWORD timeout_ms = timeout_seconds * 1000;

    /* 先清空输入缓冲区 */
    FlushConsoleInputBuffer(hIn);

    while (1) {
        DWORD elapsed = GetTickCount() - start;
        if (timeout_seconds > 0 && elapsed >= timeout_ms) {
            return -1; /* 超时 */
        }

        DWORD wait_time = (timeout_seconds > 0) ?
            (timeout_ms - elapsed) : INFINITE;

        DWORD result = WaitForSingleObject(hIn, wait_time);
        if (result == WAIT_TIMEOUT) {
            return -1; /* 超时 */
        }

        if (result == WAIT_OBJECT_0) {
            if (ReadConsoleInputA(hIn, &ir, 1, &read)) {
                if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) {
                    return ir.Event.KeyEvent.wVirtualKeyCode;
                }
            }
        }
    }
}

/* 蜂鸣 */
static void DoBeep(void) {
    Beep(800, 200);
}

/* 获取系统语言 */
static LanguageID GetSystemLanguage(void) {
    LANGID langid = GetUserDefaultUILanguage();
    WORD primary = PRIMARYLANGID(langid);
    WORD sub = SUBLANGID(langid);

    switch (primary) {
        case LANG_CHINESE:
            return (sub == SUBLANG_CHINESE_TRADITIONAL ||
                    sub == SUBLANG_CHINESE_HONGKONG ||
                    sub == SUBLANG_CHINESE_SINGAPORE) ? LANG_ZH_TW : LANG_ZH_CN;
        case LANG_JAPANESE: return LANG_JA;
        case LANG_KOREAN: return LANG_KO;
        case LANG_FRENCH: return LANG_FR;
        case LANG_GERMAN: return LANG_DE;
        case LANG_SPANISH: return LANG_ES;
        case LANG_PORTUGUESE: return LANG_PT;
        case LANG_ITALIAN: return LANG_IT;
        case LANG_RUSSIAN: return LANG_RU;
        case LANG_ARABIC: return LANG_AR;
        case LANG_THAI: return LANG_TH;
        case LANG_VIETNAMESE: return LANG_VI;
        case LANG_DUTCH: return LANG_NL;
        case LANG_POLISH: return LANG_PL;
        case LANG_TURKISH: return LANG_TR;
        case LANG_UKRAINIAN: return LANG_UK;
        case LANG_CZECH: return LANG_CS;
        case LANG_SWEDISH: return LANG_SV;
        case LANG_ENGLISH:
        default:
            return LANG_EN;
    }
}

/* 根据代码查找语言 */
static LanguageID FindLangByCode(const char* code) {
    int i;
    for (i = 0; i < LANG_COUNT; i++) {
        if (_stricmp(code, LANG_CODES[i]) == 0) {
            return (LanguageID)i;
        }
    }
    return LANG_EN; /* 默认英语 */
}

/* 打印帮助信息 */
static void PrintHelp(void) {
    PrintStr("Console Pauser v1.0 - 全能控制台暂停工具\r\n");
    PrintStr("========================================\r\n\r\n");
    PrintStr("用法: pauser [选项]\r\n\r\n");
    PrintStr("选项:\r\n");
    PrintStr("  -t, --timeout <秒>     超时自动继续（0=无限，默认0）\r\n");
    PrintStr("  -c, --code <码>        返回的退出码（默认0）\r\n");
    PrintStr("  -l, --lang <代码>      指定语言（zh-CN, en, ja, ko, fr, de...）\r\n");
    PrintStr("  -m, --message <文本>   自定义提示文本\r\n");
    PrintStr("  -b, --beep             暂停时蜂鸣\r\n");
    PrintStr("  -s, --silent           静默模式（不显示提示）\r\n");
    PrintStr("      --time             显示当前时间\r\n");
    PrintStr("      --date             显示当前日期\r\n");
    PrintStr("      --no-color         禁用颜色输出\r\n");
    PrintStr("  -h, --help             显示帮助信息\r\n");
    PrintStr("  -v, --version          显示版本信息\r\n");
    PrintStr("      --list-lang        列出所有支持的语言\r\n\r\n");
    PrintStr("示例:\r\n");
    PrintStr("  pauser                    暂停，按任意键继续\r\n");
    PrintStr("  pauser -t 10             10秒后自动继续\r\n");
    PrintStr("  pauser -c 1              返回退出码1\r\n");
    PrintStr("  pauser -l ja             日语提示\r\n");
    PrintStr("  pauser -m \"按回车继续\"  自定义提示\r\n");
    PrintStr("  pauser -b -t 5 --time   蜂鸣+5秒超时+显示时间\r\n");
}

/* 打印版本信息 */
static void PrintVersion(void) {
    PrintStr("Console Pauser v1.0\r\n");
    PrintStr("极致体积 Win32 x86 C 程序\r\n");
    PrintStr("支持 20 种语言\r\n");
    PrintStr("使用 clang -Oz 编译\r\n");
}

/* 列出所有语言 */
static void ListLanguages(void) {
    int i;
    PrintStr("支持的语言（共20种）:\r\n");
    for (i = 0; i < LANG_COUNT; i++) {
        char buf[128];
        sprintf(buf, "  %-8s %s\r\n", LANG_CODES[i], LANG_NAMES[i]);
        PrintStr(buf);
    }
}

/* 显示当前时间 */
static void PrintCurrentTime(void) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    char buf[64];
    sprintf(buf, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
    PrintStr(buf);
}

/* 显示当前日期 */
static void PrintCurrentDate(void) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    char buf[64];
    sprintf(buf, "%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
    PrintStr(buf);
}

/* ========== 参数解析 ========== */
static int ParseArgs(int argc, char* argv[], Config* cfg) {
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            PrintHelp();
            return 0; /* 显示帮助后退出 */
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            PrintVersion();
            return 0;
        }
        else if (strcmp(argv[i], "--list-lang") == 0) {
            ListLanguages();
            return 0;
        }
        else if ((strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--timeout") == 0) && i + 1 < argc) {
            cfg->timeout = atoi(argv[++i]);
            if (cfg->timeout < 0) cfg->timeout = 0;
        }
        else if ((strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--code") == 0) && i + 1 < argc) {
            cfg->exit_code = atoi(argv[++i]);
        }
        else if ((strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--lang") == 0) && i + 1 < argc) {
            cfg->lang = FindLangByCode(argv[++i]);
        }
        else if ((strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--message") == 0) && i + 1 < argc) {
            strncpy(cfg->custom_text, argv[++i], sizeof(cfg->custom_text) - 1);
            cfg->has_custom_text = 1;
        }
        else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--beep") == 0) {
            cfg->beep = 1;
        }
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--silent") == 0) {
            cfg->silent = 1;
        }
        else if (strcmp(argv[i], "--time") == 0) {
            cfg->show_time = 1;
        }
        else if (strcmp(argv[i], "--date") == 0) {
            cfg->show_date = 1;
        }
        else if (strcmp(argv[i], "--no-color") == 0) {
            cfg->no_color = 1;
        }
        else {
            /* 未知参数，忽略 */
        }
    }

    return 1; /* 继续执行 */
}

/* ========== 主函数 ========== */
int main(int argc, char* argv[]) {
    Config cfg;
    int key;

    /* 初始化配置 */
    ZeroMemory(&cfg, sizeof(cfg));
    cfg.lang = GetSystemLanguage();
    cfg.timeout = 0;
    cfg.exit_code = 0;
    cfg.beep = 0;
    cfg.silent = 0;
    cfg.show_time = 0;
    cfg.show_date = 0;
    cfg.no_color = 0;
    cfg.has_custom_text = 0;

    /* 解析参数 */
    if (!ParseArgs(argc, argv, &cfg)) {
        return 0; /* 显示帮助/版本后退出 */
    }

    /* 如果不是控制台，直接返回 */
    if (!IsConsole()) {
        return cfg.exit_code;
    }

    /* 设置控制台为 UTF-8 代码页 */
    SetConsoleOutputCP(CP_UTF8);

    /* 静默模式：直接等待按键 */
    if (cfg.silent) {
        if (cfg.beep) DoBeep();
        WaitKeyOrTimeout(cfg.timeout);
        return cfg.exit_code;
    }

    /* 显示日期/时间 */
    if (cfg.show_date || cfg.show_time) {
        if (!cfg.no_color) SetColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        PrintStr("[");
        if (cfg.show_date) {
            PrintCurrentDate();
            if (cfg.show_time) PrintStr(" ");
        }
        if (cfg.show_time) {
            PrintCurrentTime();
        }
        PrintStr("] ");
        if (!cfg.no_color) ResetColor();
    }

    /* 显示提示文本 */
    if (!cfg.no_color) SetColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    if (cfg.has_custom_text) {
        PrintStr(cfg.custom_text);
    } else {
        PrintStr(PAUSE_TEXT[cfg.lang]);
    }
    if (!cfg.no_color) ResetColor();

    /* 显示超时信息 */
    if (cfg.timeout > 0) {
        char buf[64];
        if (!cfg.no_color) SetColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        sprintf(buf, " (%ds)", cfg.timeout);
        PrintStr(buf);
        if (!cfg.no_color) ResetColor();
    }

    PrintLn();

    /* 蜂鸣 */
    if (cfg.beep) DoBeep();

    /* 等待按键或超时 */
    key = WaitKeyOrTimeout(cfg.timeout);

    /* 超时处理 */
    if (key == -1) {
        if (!cfg.no_color) SetColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        PrintStr(TIMEOUT_TEXT[cfg.lang]);
        if (!cfg.no_color) ResetColor();
        PrintLn();
    }

    /* 显示退出码（非0时） */
    if (cfg.exit_code != 0) {
        char buf[128];
        if (!cfg.no_color) SetColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        sprintf(buf, "%s: %d\r\n", EXIT_CODE_TEXT[cfg.lang], cfg.exit_code);
        PrintStr(buf);
        if (!cfg.no_color) ResetColor();
    }

    return cfg.exit_code;
}
