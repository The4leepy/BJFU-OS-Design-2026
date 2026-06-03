#include "splash.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>

/* ============================================================
 *  "Mini OS" — ANSI Shadow figlet font + drop-shadow separator
 *
 *  9 lines total: 6 letter lines + separator + shadow + info
 *  Animation:  print dim → pause → clear & redraw bright line-by-line
 * ============================================================ */
static constexpr int SPLASH_H = 9;
static constexpr int PAUSE_MS    = 600;   // dim display time
static constexpr int LINE_MS     = 160;   // per-line bright reveal

static const char* g_art[SPLASH_H] = {
    "███╗   ███╗   ██╗  ███╗   ██╗   ██╗        █████╗   ██████╗ ",
    "████╗ ████║   ██║  ████╗  ██║   ██║       ██╔══██╗  ██╔═══╝ ",
    "██╔████╔██║   ██║  ██╔██╗ ██║   ██║       ██║  ██║  ███████╗",
    "██║╚██╔╝██║   ██║  ██║╚██╗██║   ██║       ██║  ██║  ╚════██║",
    "██║ ╚═╝ ██║   ██║  ██║ ╚████║   ██║       ╚█████╔╝  ██████╔╝",
    "╚═╝     ╚═╝   ╚═╝  ╚═╝  ╚═══╝   ╚═╝        ╚════╝   ╚═════╝ ",
    "██████████████████████████████████████████████████████████████",
    "░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░",
    "          Mini OS  --  Type 'help' for commands                ",
};

/// Replace block/box chars with dim ░ (both are 3-byte UTF-8 → length preserved)
static std::string dim_line(const char* s) {
    std::string r;
    const char* p = s;
    while (*p) {
        unsigned char c = static_cast<unsigned char>(*p);
        if (c >= 0xE0) {       // 3-byte UTF-8 leader → ░ (also 3 bytes)
            r += "░";
            p += 3;
        } else {
            r += *p;            // ASCII (1 byte)
            p++;
        }
    }
    return r;
}

static void clear_screen() {
    std::cout << "\033[3J\033[2J\033[H" << std::flush;
}

/* ---- public API ---- */

void show_splash() {
    // Phase 1: dim ghost
    clear_screen();
    for (int i = 0; i < SPLASH_H; ++i)
        std::cout << dim_line(g_art[i]) << '\n';
    std::cout << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(PAUSE_MS));

    // Phase 2: bright reveal, line by line
    clear_screen();
    for (int i = 0; i < SPLASH_H; ++i) {
        std::cout << g_art[i] << '\n' << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(LINE_MS));
    }
    std::cout << '\n';
}

void print_splash_banner() {
    for (int i = 0; i < SPLASH_H; ++i)
        std::cout << g_art[i] << '\n';
}
