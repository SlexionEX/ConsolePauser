/*
 * Console Pauser - 全能控制台暂停器
 * 自动为控制台程序暂停并返回退出码
 *
 * 特性：
 *   - 20 种语言自动检测/手动指定
 *   - 极致体积（Win32 x86，clang -Oz）
 *   - 传递子进程退出码
 *   - 显示运行时间
 *   - 完善的错误处理
 *
 * 编译：clang++ -Oz -target i686-pc-windows-msvc -o consolepauser.exe main.cpp -lkernel32 -luser32
 */

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <conio.h>

/* ==================== 常量 ==================== */

#define APP_NAME L"Console Pauser"
#define APP_VERSION L"1.0.0"
#define MAX_LANG_NAME 64
#define MAX_MSG_LEN 512

/* 消息 ID */
enum {
    MSG_USAGE = 0,
    MSG_PRESS_ANY_KEY,
    MSG_PROGRAM_EXITED,
    MSG_EXIT_CODE,
    MSG_ELAPSED_TIME,
    MSG_ERROR_LAUNCH,
    MSG_ERROR_FILE_NOT_FOUND,
    MSG_ERROR_ACCESS_DENIED,
    MSG_ERROR_UNKNOWN,
    MSG_NO_PROGRAM_SPECIFIED,
    MSG_LANG_SET,
    MSG_HELP,
    MSG_VERSION_INFO,
    MSG_SECONDS,
    MSG_MILLISECONDS,
    MSG_COUNT
};

/* 语言 ID */
enum {
    LANG_ZH_CN = 0,  /* 简体中文 */
    LANG_ZH_TW,       /* 繁體中文 */
    LANG_EN,          /* English */
    LANG_JA,          /* 日本語 */
    LANG_KO,          /* 한국어 */
    LANG_FR,          /* Français */
    LANG_DE,          /* Deutsch */
    LANG_ES,          /* Español */
    LANG_PT,          /* Português */
    LANG_IT,          /* Italiano */
    LANG_RU,          /* Русский */
    LANG_AR,          /* العربية */
    LANG_TH,          /* ไทย */
    LANG_VI,          /* Tiếng Việt */
    LANG_NL,          /* Nederlands */
    LANG_PL,          /* Polski */
    LANG_TR,          /* Türkçe */
    LANG_UK,          /* Українська */
    LANG_CS,          /* Čeština */
    LANG_SV,          /* Svenska */
    LANG_COUNT
};

/* ==================== 全局变量 ==================== */

static int g_currentLang = LANG_EN;
static BOOL g_noPause = FALSE;
static BOOL g_showTime = TRUE;
static WCHAR g_langOverride[16] = {0};

/* ==================== 翻译表 ==================== */
/* 每种语言的消息字符串，使用 UTF-16 */

static const WCHAR* g_translations[LANG_COUNT][MSG_COUNT] = {
    /* 简体中文 */
    {
        L"用法: consolepauser [选项] -- <程序> [参数...]\n",
        L"按任意键继续...",
        L"程序已退出",
        L"退出码: %ld (0x%08lX)",
        L"运行时间: %s",
        L"错误: 无法启动程序",
        L"错误: 找不到指定的程序",
        L"错误: 访问被拒绝",
        L"错误: 未知错误 (0x%08lX)",
        L"错误: 未指定要运行的程序",
        L"语言已设置为: 简体中文",
        L"选项:\n"
        L"  --no-pause      程序退出后不暂停\n"
        L"  --no-time       不显示运行时间\n"
        L"  --lang <代码>   指定语言 (zh-CN, en, ja, ...)\n"
        L"  --help          显示此帮助信息\n"
        L"  --version       显示版本信息\n",
        L"Console Pauser v%s - 全能控制台暂停器\n",
        L"%lu.%03lu 秒",
        L"%lu 毫秒",
    },
    /* 繁體中文 */
    {
        L"用法: consolepauser [選項] -- <程式> [參數...]\n",
        L"按任意鍵繼續...",
        L"程式已結束",
        L"結束碼: %ld (0x%08lX)",
        L"執行時間: %s",
        L"錯誤: 無法啟動程式",
        L"錯誤: 找不到指定的程式",
        L"錯誤: 存取被拒",
        L"錯誤: 未知錯誤 (0x%08lX)",
        L"錯誤: 未指定要執行的程式",
        L"語言已設定為: 繁體中文",
        L"選項:\n"
        L"  --no-pause      程式結束後不暫停\n"
        L"  --no-time       不顯示執行時間\n"
        L"  --lang <代碼>   指定語言 (zh-TW, en, ja, ...)\n"
        L"  --help          顯示此說明資訊\n"
        L"  --version       顯示版本資訊\n",
        L"Console Pauser v%s - 全能控制台暫停器\n",
        L"%lu.%03lu 秒",
        L"%lu 毫秒",
    },
    /* English */
    {
        L"Usage: consolepauser [options] -- <program> [args...]\n",
        L"Press any key to continue...",
        L"Program exited",
        L"Exit code: %ld (0x%08lX)",
        L"Elapsed time: %s",
        L"Error: Failed to launch program",
        L"Error: Program not found",
        L"Error: Access denied",
        L"Error: Unknown error (0x%08lX)",
        L"Error: No program specified",
        L"Language set to: English",
        L"Options:\n"
        L"  --no-pause      Do not pause after program exits\n"
        L"  --no-time       Do not show elapsed time\n"
        L"  --lang <code>   Set language (en, zh-CN, ja, ...)\n"
        L"  --help          Show this help message\n"
        L"  --version       Show version information\n",
        L"Console Pauser v%s - Universal Console Pauser\n",
        L"%lu.%03lu seconds",
        L"%lu milliseconds",
    },
    /* 日本語 */
    {
        L"使い方: consolepauser [オプション] -- <プログラム> [引数...]\n",
        L"続行するには任意のキーを押してください...",
        L"プログラムが終了しました",
        L"終了コード: %ld (0x%08lX)",
        L"経過時間: %s",
        L"エラー: プログラムの起動に失敗しました",
        L"エラー: プログラムが見つかりません",
        L"エラー: アクセスが拒否されました",
        L"エラー: 不明なエラー (0x%08lX)",
        L"エラー: プログラムが指定されていません",
        L"言語を設定しました: 日本語",
        L"オプション:\n"
        L"  --no-pause      プログラム終了後に一時停止しない\n"
        L"  --no-time       経過時間を表示しない\n"
        L"  --lang <コード>  言語を指定 (ja, en, zh-CN, ...)\n"
        L"  --help          このヘルプを表示\n"
        L"  --version       バージョン情報を表示\n",
        L"Console Pauser v%s - 万能コンソールポーザー\n",
        L"%lu.%03lu 秒",
        L"%lu ミリ秒",
    },
    /* 한국어 */
    {
        L"사용법: consolepauser [옵션] -- <프로그램> [인수...]\n",
        L"계속하려면 아무 키나 누르세요...",
        L"프로그램이 종료되었습니다",
        L"종료 코드: %ld (0x%08lX)",
        L"경과 시간: %s",
        L"오류: 프로그램을 시작하지 못했습니다",
        L"오류: 프로그램을 찾을 수 없습니다",
        L"오류: 액세스가 거부되었습니다",
        L"오류: 알 수 없는 오류 (0x%08lX)",
        L"오류: 프로그램이 지정되지 않았습니다",
        L"언어가 설정되었습니다: 한국어",
        L"옵션:\n"
        L"  --no-pause      프로그램 종료 후 일시 중지하지 않음\n"
        L"  --no-time       경과 시간을 표시하지 않음\n"
        L"  --lang <코드>   언어 지정 (ko, en, ja, ...)\n"
        L"  --help          이 도움말 표시\n"
        L"  --version       버전 정보 표시\n",
        L"Console Pauser v%s - 만능 콘솔 포저\n",
        L"%lu.%03lu 초",
        L"%lu 밀리초",
    },
    /* Français */
    {
        L"Utilisation: consolepauser [options] -- <programme> [arguments...]\n",
        L"Appuyez sur une touche pour continuer...",
        L"Le programme s'est terminé",
        L"Code de sortie: %ld (0x%08lX)",
        L"Temps écoulé: %s",
        L"Erreur: Échec du lancement du programme",
        L"Erreur: Programme introuvable",
        L"Erreur: Accès refusé",
        L"Erreur: Erreur inconnue (0x%08lX)",
        L"Erreur: Aucun programme spécifié",
        L"Langue définie sur: Français",
        L"Options:\n"
        L"  --no-pause      Ne pas mettre en pause après la sortie\n"
        L"  --no-time       Ne pas afficher le temps écoulé\n"
        L"  --lang <code>   Définir la langue (fr, en, zh-CN, ...)\n"
        L"  --help          Afficher cette aide\n"
        L"  --version       Afficher la version\n",
        L"Console Pauser v%s - Pauseur de console universel\n",
        L"%lu.%03lu secondes",
        L"%lu millisecondes",
    },
    /* Deutsch */
    {
        L"Verwendung: consolepauser [Optionen] -- <Programm> [Argumente...]\n",
        L"Drücken Sie eine beliebige Taste, um fortzufahren...",
        L"Programm beendet",
        L"Exit-Code: %ld (0x%08lX)",
        L"Verstrichene Zeit: %s",
        L"Fehler: Programm konnte nicht gestartet werden",
        L"Fehler: Programm nicht gefunden",
        L"Fehler: Zugriff verweigert",
        L"Fehler: Unbekannter Fehler (0x%08lX)",
        L"Fehler: Kein Programm angegeben",
        L"Sprache festgelegt auf: Deutsch",
        L"Optionen:\n"
        L"  --no-pause      Nach Beendigung nicht pausieren\n"
        L"  --no-time       Verstrichene Zeit nicht anzeigen\n"
        L"  --lang <Code>   Sprache festlegen (de, en, zh-CN, ...)\n"
        L"  --help          Diese Hilfe anzeigen\n"
        L"  --version       Versionsinformationen anzeigen\n",
        L"Console Pauser v%s - Universeller Konsolen-Pausierer\n",
        L"%lu.%03lu Sekunden",
        L"%lu Millisekunden",
    },
    /* Español */
    {
        L"Uso: consolepauser [opciones] -- <programa> [argumentos...]\n",
        L"Presione cualquier tecla para continuar...",
        L"El programa ha terminado",
        L"Código de salida: %ld (0x%08lX)",
        L"Tiempo transcurrido: %s",
        L"Error: No se pudo iniciar el programa",
        L"Error: Programa no encontrado",
        L"Error: Acceso denegado",
        L"Error: Error desconocido (0x%08lX)",
        L"Error: No se especificó ningún programa",
        L"Idioma establecido en: Español",
        L"Opciones:\n"
        L"  --no-pause      No pausar después de salir\n"
        L"  --no-time       No mostrar el tiempo transcurrido\n"
        L"  --lang <código> Establecer idioma (es, en, zh-CN, ...)\n"
        L"  --help          Mostrar esta ayuda\n"
        L"  --version       Mostrar información de versión\n",
        L"Console Pauser v%s - Pausador de consola universal\n",
        L"%lu.%03lu segundos",
        L"%lu milisegundos",
    },
    /* Português */
    {
        L"Uso: consolepauser [opções] -- <programa> [argumentos...]\n",
        L"Pressione qualquer tecla para continuar...",
        L"O programa terminou",
        L"Código de saída: %ld (0x%08lX)",
        L"Tempo decorrido: %s",
        L"Erro: Falha ao iniciar o programa",
        L"Erro: Programa não encontrado",
        L"Erro: Acesso negado",
        L"Erro: Erro desconhecido (0x%08lX)",
        L"Erro: Nenhum programa especificado",
        L"Idioma definido para: Português",
        L"Opções:\n"
        L"  --no-pause      Não pausar após sair\n"
        L"  --no-time       Não mostrar o tempo decorrido\n"
        L"  --lang <código> Definir idioma (pt, en, zh-CN, ...)\n"
        L"  --help          Mostrar esta ajuda\n"
        L"  --version       Mostrar informações da versão\n",
        L"Console Pauser v%s - Pausador de console universal\n",
        L"%lu.%03lu segundos",
        L"%lu milissegundos",
    },
    /* Italiano */
    {
        L"Uso: consolepauser [opzioni] -- <programma> [argomenti...]\n",
        L"Premi un tasto per continuare...",
        L"Programma terminato",
        L"Codice di uscita: %ld (0x%08lX)",
        L"Tempo trascorso: %s",
        L"Errore: Impossibile avviare il programma",
        L"Errore: Programma non trovato",
        L"Errore: Accesso negato",
        L"Errore: Errore sconosciuto (0x%08lX)",
        L"Errore: Nessun programma specificato",
        L"Lingua impostata su: Italiano",
        L"Opzioni:\n"
        L"  --no-pause      Non mettere in pausa dopo l'uscita\n"
        L"  --no-time       Non mostrare il tempo trascorso\n"
        L"  --lang <codice> Imposta lingua (it, en, zh-CN, ...)\n"
        L"  --help          Mostra questo aiuto\n"
        L"  --version       Mostra informazioni sulla versione\n",
        L"Console Pauser v%s - Pausatore console universale\n",
        L"%lu.%03lu secondi",
        L"%lu millisecondi",
    },
    /* Русский */
    {
        L"Использование: consolepauser [опции] -- <программа> [аргументы...]\n",
        L"Нажмите любую клавишу для продолжения...",
        L"Программа завершена",
        L"Код выхода: %ld (0x%08lX)",
        L"Прошло времени: %s",
        L"Ошибка: Не удалось запустить программу",
        L"Ошибка: Программа не найдена",
        L"Ошибка: Доступ запрещён",
        L"Ошибка: Неизвестная ошибка (0x%08lX)",
        L"Ошибка: Программа не указана",
        L"Язык установлен: Русский",
        L"Опции:\n"
        L"  --no-pause      Не ставить на паузу после выхода\n"
        L"  --no-time       Не показывать прошедшее время\n"
        L"  --lang <код>    Установить язык (ru, en, zh-CN, ...)\n"
        L"  --help          Показать эту справку\n"
        L"  --version       Показать информацию о версии\n",
        L"Console Pauser v%s - Универсальная пауза консоли\n",
        L"%lu.%03lu секунд",
        L"%lu миллисекунд",
    },
    /* العربية */
    {
        L"الاستخدام: consolepauser [الخيارات] -- <البرنامج> [الوسائط...]\n",
        L"اضغط على أي مفتاح للمتابعة...",
        L"تم إنهاء البرنامج",
        L"رمز الخروج: %ld (0x%08lX)",
        L"الوقت المنقضي: %s",
        L"خطأ: فشل تشغيل البرنامج",
        L"خطأ: البرنامج غير موجود",
        L"خطأ: تم رفض الوصول",
        L"خطأ: خطأ غير معروف (0x%08lX)",
        L"خطأ: لم يتم تحديد برنامج",
        L"تم تعيين اللغة إلى: العربية",
        L"الخيارات:\n"
        L"  --no-pause      عدم الإيقاف المؤقت بعد الخروج\n"
        L"  --no-time       عدم عرض الوقت المنقضي\n"
        L"  --lang <الرمز>   تعيين اللغة (ar, en, zh-CN, ...)\n"
        L"  --help          عرض هذه المساعدة\n"
        L"  --version       عرض معلومات الإصدار\n",
        L"Console Pauser v%s - مؤشر إيقاف وحدة التحكم الشامل\n",
        L"%lu.%03lu ثانية",
        L"%lu مللي ثانية",
    },
    /* ไทย */
    {
        L"การใช้งาน: consolepauser [ตัวเลือก] -- <โปรแกรม> [อาร์กิวเมนต์...]\n",
        L"กดปุ่มใดๆ เพื่อดำเนินการต่อ...",
        L"โปรแกรมจบการทำงาน",
        L"รหัสทางออก: %ld (0x%08lX)",
        L"เวลาที่ใช้: %s",
        L"ข้อผิดพลาด: ไม่สามารถเริ่มโปรแกรมได้",
        L"ข้อผิดพลาด: ไม่พบโปรแกรม",
        L"ข้อผิดพลาด: การเข้าถึงถูกปฏิเสธ",
        L"ข้อผิดพลาด: ข้อผิดพลาดที่ไม่รู้จัก (0x%08lX)",
        L"ข้อผิดพลาด: ไม่ได้ระบุโปรแกรม",
        L"ตั้งค่าภาษาเป็น: ไทย",
        L"ตัวเลือก:\n"
        L"  --no-pause      ไม่หยุดชั่วคราวหลังจากโปรแกรมจบ\n"
        L"  --no-time       ไม่แสดงเวลาที่ใช้\n"
        L"  --lang <รหัส>    ตั้งค่าภาษา (th, en, zh-CN, ...)\n"
        L"  --help          แสดงความช่วยเหลือนี้\n"
        L"  --version       แสดงข้อมูลเวอร์ชัน\n",
        L"Console Pauser v%s - ตัวหยุดคอนโซลสากล\n",
        L"%lu.%03lu วินาที",
        L"%lu มิลลิวินาที",
    },
    /* Tiếng Việt */
    {
        L"Cách dùng: consolepauser [tùy chọn] -- <chương trình> [đối số...]\n",
        L"Nhấn phím bất kỳ để tiếp tục...",
        L"Chương trình đã thoát",
        L"Mã thoát: %ld (0x%08lX)",
        L"Thời gian đã trôi qua: %s",
        L"Lỗi: Không thể khởi động chương trình",
        L"Lỗi: Không tìm thấy chương trình",
        L"Lỗi: Truy cập bị từ chối",
        L"Lỗi: Lỗi không xác định (0x%08lX)",
        L"Lỗi: Không có chương trình nào được chỉ định",
        L"Ngôn ngữ đã được đặt thành: Tiếng Việt",
        L"Tùy chọn:\n"
        L"  --no-pause      Không tạm dừng sau khi thoát\n"
        L"  --no-time       Không hiển thị thời gian đã trôi qua\n"
        L"  --lang <mã>     Đặt ngôn ngữ (vi, en, zh-CN, ...)\n"
        L"  --help          Hiển thị trợ giúp này\n"
        L"  --version       Hiển thị thông tin phiên bản\n",
        L"Console Pauser v%s - Trình tạm dừng console đa năng\n",
        L"%lu.%03lu giây",
        L"%lu mili giây",
    },
    /* Nederlands */
    {
        L"Gebruik: consolepauser [opties] -- <programma> [argumenten...]\n",
        L"Druk op een willekeurige toets om door te gaan...",
        L"Programma afgesloten",
        L"Exit-code: %ld (0x%08lX)",
        L"Verstreken tijd: %s",
        L"Fout: Kan programma niet starten",
        L"Fout: Programma niet gevonden",
        L"Fout: Toegang geweigerd",
        L"Fout: Onbekende fout (0x%08lX)",
        L"Fout: Geen programma opgegeven",
        L"Taal ingesteld op: Nederlands",
        L"Opties:\n"
        L"  --no-pause      Niet pauzeren na afsluiten\n"
        L"  --no-time       Verstreken tijd niet weergeven\n"
        L"  --lang <code>   Taal instellen (nl, en, zh-CN, ...)\n"
        L"  --help          Deze help weergeven\n"
        L"  --version       Versie-informatie weergeven\n",
        L"Console Pauser v%s - Universele console-pauser\n",
        L"%lu.%03lu seconden",
        L"%lu milliseconden",
    },
    /* Polski */
    {
        L"Użycie: consolepauser [opcje] -- <program> [argumenty...]\n",
        L"Naciśnij dowolny klawisz, aby kontynuować...",
        L"Program zakończył działanie",
        L"Kod wyjścia: %ld (0x%08lX)",
        L"Czas trwania: %s",
        L"Błąd: Nie udało się uruchomić programu",
        L"Błąd: Nie znaleziono programu",
        L"Błąd: Odmowa dostępu",
        L"Błąd: Nieznany błąd (0x%08lX)",
        L"Błąd: Nie określono programu",
        L"Ustawiono język na: Polski",
        L"Opcje:\n"
        L"  --no-pause      Nie wstrzymuj po zakończeniu\n"
        L"  --no-time       Nie pokazuj czasu trwania\n"
        L"  --lang <kod>    Ustaw język (pl, en, zh-CN, ...)\n"
        L"  --help          Pokaż tę pomoc\n"
        L"  --version       Pokaż informacje o wersji\n",
        L"Console Pauser v%s - Uniwersalny pauzer konsoli\n",
        L"%lu.%03lu sekundy",
        L"%lu milisekundy",
    },
    /* Türkçe */
    {
        L"Kullanım: consolepauser [seçenekler] -- <program> [argümanlar...]\n",
        L"Devam etmek için herhangi bir tuşa basın...",
        L"Program sonlandı",
        L"Çıkış kodu: %ld (0x%08lX)",
        L"Geçen süre: %s",
        L"Hata: Program başlatılamadı",
        L"Hata: Program bulunamadı",
        L"Hata: Erişim reddedildi",
        L"Hata: Bilinmeyen hata (0x%08lX)",
        L"Hata: Program belirtilmedi",
        L"Dil ayarlandı: Türkçe",
        L"Seçenekler:\n"
        L"  --no-pause      Çıktıktan sonra duraklatma\n"
        L"  --no-time       Geçen süreyi gösterme\n"
        L"  --lang <kod>    Dil ayarla (tr, en, zh-CN, ...)\n"
        L"  --help          Bu yardımı göster\n"
        L"  --version       Sürüm bilgilerini göster\n",
        L"Console Pauser v%s - Evrensel konsol duraklatıcı\n",
        L"%lu.%03lu saniye",
        L"%lu milisaniye",
    },
    /* Українська */
    {
        L"Використання: consolepauser [опції] -- <програма> [аргументи...]\n",
        L"Натисніть будь-яку клавішу для продовження...",
        L"Програма завершена",
        L"Код виходу: %ld (0x%08lX)",
        L"Минуло часу: %s",
        L"Помилка: Не вдалося запустити програму",
        L"Помилка: Програму не знайдено",
        L"Помилка: Доступ заборонено",
        L"Помилка: Невідома помилка (0x%08lX)",
        L"Помилка: Програму не вказано",
        L"Мову встановлено: Українська",
        L"Опції:\n"
        L"  --no-pause      Не призупиняти після виходу\n"
        L"  --no-time       Не показувати минулий час\n"
        L"  --lang <код>    Встановити мову (uk, en, zh-CN, ...)\n"
        L"  --help          Показати цю довідку\n"
        L"  --version       Показати інформацію про версію\n",
        L"Console Pauser v%s - Універсальна пауза консолі\n",
        L"%lu.%03lu секунд",
        L"%lu мілісекунд",
    },
    /* Čeština */
    {
        L"Použití: consolepauser [možnosti] -- <program> [argumenty...]\n",
        L"Stiskněte libovolnou klávesu pro pokračování...",
        L"Program skončil",
        L"Návratový kód: %ld (0x%08lX)",
        L"Uplynulý čas: %s",
        L"Chyba: Nepodařilo se spustit program",
        L"Chyba: Program nenalezen",
        L"Chyba: Přístup odepřen",
        L"Chyba: Neznámá chyba (0x%08lX)",
        L"Chyba: Není zadán žádný program",
        L"Jazyk nastaven na: Čeština",
        L"Možnosti:\n"
        L"  --no-pause      Nepozastavovat po ukončení\n"
        L"  --no-time       Nezobrazovat uplynulý čas\n"
        L"  --lang <kód>    Nastavit jazyk (cs, en, zh-CN, ...)\n"
        L"  --help          Zobrazit tuto nápovědu\n"
        L"  --version       Zobrazit informace o verzi\n",
        L"Console Pauser v%s - Univerzální pozastavení konzole\n",
        L"%lu.%03lu sekundy",
        L"%lu milisekundy",
    },
    /* Svenska */
    {
        L"Användning: consolepauser [alternativ] -- <program> [argument...]\n",
        L"Tryck på valfri tangent för att fortsätta...",
        L"Programmet avslutades",
        L"Avslutningskod: %ld (0x%08lX)",
        L"Förfluten tid: %s",
        L"Fel: Kunde inte starta programmet",
        L"Fel: Programmet hittades inte",
        L"Fel: Åtkomst nekades",
        L"Fel: Okänt fel (0x%08lX)",
        L"Fel: Inget program angivet",
        L"Språk inställt på: Svenska",
        L"Alternativ:\n"
        L"  --no-pause      Pausa inte efter avslut\n"
        L"  --no-time       Visa inte förfluten tid\n"
        L"  --lang <kod>    Ange språk (sv, en, zh-CN, ...)\n"
        L"  --help          Visa detta hjälpmeddelande\n"
        L"  --version       Visa versionsinformation\n",
        L"Console Pauser v%s - Universell konsolpausare\n",
        L"%lu.%03lu sekunder",
        L"%lu millisekunder",
    },
};

/* 语言代码映射 */
static const struct {
    const WCHAR* code;
    int langId;
} g_langCodes[] = {
    { L"zh-CN", LANG_ZH_CN }, { L"zh-cn", LANG_ZH_CN }, { L"zh", LANG_ZH_CN },
    { L"zh-TW", LANG_ZH_TW }, { L"zh-tw", LANG_ZH_TW }, { L"zh-HK", LANG_ZH_TW },
    { L"en", LANG_EN }, { L"en-US", LANG_EN }, { L"en-GB", LANG_EN },
    { L"ja", LANG_JA }, { L"ja-JP", LANG_JA },
    { L"ko", LANG_KO }, { L"ko-KR", LANG_KO },
    { L"fr", LANG_FR }, { L"fr-FR", LANG_FR },
    { L"de", LANG_DE }, { L"de-DE", LANG_DE },
    { L"es", LANG_ES }, { L"es-ES", LANG_ES },
    { L"pt", LANG_PT }, { L"pt-BR", LANG_PT }, { L"pt-PT", LANG_PT },
    { L"it", LANG_IT }, { L"it-IT", LANG_IT },
    { L"ru", LANG_RU }, { L"ru-RU", LANG_RU },
    { L"ar", LANG_AR }, { L"ar-SA", LANG_AR },
    { L"th", LANG_TH }, { L"th-TH", LANG_TH },
    { L"vi", LANG_VI }, { L"vi-VN", LANG_VI },
    { L"nl", LANG_NL }, { L"nl-NL", LANG_NL },
    { L"pl", LANG_PL }, { L"pl-PL", LANG_PL },
    { L"tr", LANG_TR }, { L"tr-TR", LANG_TR },
    { L"uk", LANG_UK }, { L"uk-UA", LANG_UK },
    { L"cs", LANG_CS }, { L"cs-CZ", LANG_CS },
    { L"sv", LANG_SV }, { L"sv-SE", LANG_SV },
    { NULL, -1 }
};

/* ==================== 工具函数 ==================== */

static const WCHAR* GetMsg(int msgId) {
    if (msgId < 0 || msgId >= MSG_COUNT) return L"";
    if (g_currentLang < 0 || g_currentLang >= LANG_COUNT) return g_translations[LANG_EN][msgId];
    return g_translations[g_currentLang][msgId];
}

static int LangCodeToId(const WCHAR* code) {
    if (!code || !*code) return -1;
    for (int i = 0; g_langCodes[i].code; i++) {
        if (_wcsicmp(code, g_langCodes[i].code) == 0) {
            return g_langCodes[i].langId;
        }
    }
    return -1;
}

static void DetectSystemLanguage(void) {
    LANGID langId = GetUserDefaultUILanguage();
    WCHAR langCode[16] = {0};

    /* 从 LANGID 获取语言代码 */
    GetLocaleInfoW(MAKELCID(langId, SORT_DEFAULT), LOCALE_SISO639LANGNAME, langCode, 16);

    /* 特殊处理中文 */
    if (PRIMARYLANGID(langId) == LANG_CHINESE) {
        if (SUBLANGID(langId) == SUBLANG_CHINESE_SIMPLIFIED ||
            SUBLANGID(langId) == SUBLANG_CHINESE_SINGAPORE) {
            g_currentLang = LANG_ZH_CN;
        } else {
            g_currentLang = LANG_ZH_TW;
        }
        return;
    }

    int id = LangCodeToId(langCode);
    if (id >= 0) {
        g_currentLang = id;
    } else {
        g_currentLang = LANG_EN;
    }
}

static void PrintString(const WCHAR* str) {
    if (!str || !*str) return;
    /* 转换为 UTF-8 并用 WriteFile 输出（兼容控制台和重定向） */
    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
    if (utf8Len <= 0) return;
    char* utf8 = (char*)_alloca(utf8Len);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, utf8, utf8Len, NULL, NULL);
    DWORD written;
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), utf8, (DWORD)(utf8Len - 1), &written, NULL);
}

static void PrintMsg(int msgId, ...) {
    WCHAR buffer[MAX_MSG_LEN];
    va_list args;
    va_start(args, msgId);
    _vsnwprintf_s(buffer, MAX_MSG_LEN, _TRUNCATE, GetMsg(msgId), args);
    va_end(args);
    PrintString(buffer);
}

static void FormatElapsedTime(WCHAR* buffer, int bufferLen, DWORD ms) {
    if (ms >= 1000) {
        _snwprintf_s(buffer, bufferLen, _TRUNCATE, GetMsg(MSG_SECONDS), ms / 1000, ms % 1000);
    } else {
        _snwprintf_s(buffer, bufferLen, _TRUNCATE, GetMsg(MSG_MILLISECONDS), ms);
    }
}

/* ==================== 进程管理 ==================== */

static DWORD LaunchAndWait(const WCHAR* commandLine, DWORD* exitCode) {
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    DWORD result = 0;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    /* 创建子进程 */
    if (!CreateProcessW(
        NULL,
        (LPWSTR)commandLine,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    )) {
        result = GetLastError();
        return result;
    }

    /* 等待子进程退出 */
    WaitForSingleObject(pi.hProcess, INFINITE);

    /* 获取退出码 */
    if (exitCode) {
        GetExitCodeProcess(pi.hProcess, exitCode);
    }

    /* 清理 */
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}

/* ==================== 命令行解析 ==================== */

static void ShowHelp(void) {
    PrintMsg(MSG_VERSION_INFO, APP_VERSION);
    PrintString(L"\n");
    PrintMsg(MSG_HELP);
}

static void ShowVersion(void) {
    PrintMsg(MSG_VERSION_INFO, APP_VERSION);
}

/* ==================== 主入口 ==================== */

int wmain(int argc, wchar_t* argv[]) {
    int programArgIndex = -1;
    WCHAR commandLine[32768] = {0};
    DWORD startTime, endTime, elapsedMs;
    DWORD exitCode = 0;
    DWORD launchResult;

    /* 初始化 */
    SetConsoleOutputCP(CP_UTF8);
    DetectSystemLanguage();

    /* 解析命令行参数 */
    for (int i = 1; i < argc; i++) {
        if (wcscmp(argv[i], L"--") == 0) {
            programArgIndex = i + 1;
            break;
        } else if (wcscmp(argv[i], L"--no-pause") == 0) {
            g_noPause = TRUE;
        } else if (wcscmp(argv[i], L"--no-time") == 0) {
            g_showTime = FALSE;
        } else if (wcscmp(argv[i], L"--lang") == 0 && i + 1 < argc) {
            int id = LangCodeToId(argv[i + 1]);
            if (id >= 0) {
                g_currentLang = id;
            }
            i++;
        } else if (wcscmp(argv[i], L"--help") == 0 || wcscmp(argv[i], L"-h") == 0) {
            ShowHelp();
            return 0;
        } else if (wcscmp(argv[i], L"--version") == 0 || wcscmp(argv[i], L"-v") == 0) {
            ShowVersion();
            return 0;
        }
    }

    /* 检查是否指定了程序 */
    if (programArgIndex < 0 || programArgIndex >= argc) {
        PrintMsg(MSG_USAGE);
        PrintString(L"\n");
        PrintMsg(MSG_NO_PROGRAM_SPECIFIED);
        PrintString(L"\n");
        return 1;
    }

    /* 构建命令行 */
    commandLine[0] = L'\0';
    for (int i = programArgIndex; i < argc; i++) {
        if (i > programArgIndex) {
            wcscat_s(commandLine, _countof(commandLine), L" ");
        }
        /* 如果参数包含空格，用引号包裹 */
        if (wcschr(argv[i], L' ') || wcschr(argv[i], L'\t')) {
            wcscat_s(commandLine, _countof(commandLine), L"\"");
            wcscat_s(commandLine, _countof(commandLine), argv[i]);
            wcscat_s(commandLine, _countof(commandLine), L"\"");
        } else {
            wcscat_s(commandLine, _countof(commandLine), argv[i]);
        }
    }

    /* 记录开始时间 */
    startTime = GetTickCount();

    /* 启动并等待程序 */
    launchResult = LaunchAndWait(commandLine, &exitCode);

    /* 记录结束时间 */
    endTime = GetTickCount();
    elapsedMs = endTime - startTime;

    /* 处理启动错误 */
    if (launchResult != 0) {
        PrintString(L"\n");
        switch (launchResult) {
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
                PrintMsg(MSG_ERROR_FILE_NOT_FOUND);
                break;
            case ERROR_ACCESS_DENIED:
                PrintMsg(MSG_ERROR_ACCESS_DENIED);
                break;
            default:
                PrintMsg(MSG_ERROR_UNKNOWN, launchResult);
                break;
        }
        PrintString(L"\n");
        if (!g_noPause) {
            PrintString(L"\n");
            PrintMsg(MSG_PRESS_ANY_KEY);
            PrintString(L"\n");
            _getch();
        }
        return launchResult;
    }

    /* 显示结果 */
    PrintString(L"\n");
    PrintMsg(MSG_PROGRAM_EXITED);
    PrintString(L"\n");
    PrintMsg(MSG_EXIT_CODE, exitCode, exitCode);
    PrintString(L"\n");

    if (g_showTime) {
        WCHAR timeBuf[64];
        FormatElapsedTime(timeBuf, _countof(timeBuf), elapsedMs);
        PrintMsg(MSG_ELAPSED_TIME, timeBuf);
        PrintString(L"\n");
    }

    /* 暂停 */
    if (!g_noPause) {
        PrintString(L"\n");
        PrintMsg(MSG_PRESS_ANY_KEY);
        PrintString(L"\n");
        _getch();
    }

    /* 返回子进程的退出码 */
    return (int)exitCode;
}
