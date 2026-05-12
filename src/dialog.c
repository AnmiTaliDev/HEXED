/* SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org> */
/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "dialog.h"
#include "display.h"
#include "input.h"

void dialog_draw_box(int row, int col, int h, int w, unsigned char attr) {
    int r, c;
    display_putchar(row, col, 0xC9, attr);
    for (c = col + 1; c < col + w - 1; c++) display_putchar(row, c, 0xCD, attr);
    display_putchar(row, col + w - 1, 0xBB, attr);

    for (r = row + 1; r < row + h - 1; r++) {
        display_putchar(r, col, 0xBA, attr);
        display_fill(r, col + 1, w - 2, ' ', attr);
        display_putchar(r, col + w - 1, 0xBA, attr);
    }

    display_putchar(row + h - 1, col, 0xC8, attr);
    for (c = col + 1; c < col + w - 1; c++) display_putchar(row + h - 1, c, 0xCD, attr);
    display_putchar(row + h - 1, col + w - 1, 0xBC, attr);
}

void dialog_message(const char *title, const char *msg) {
    int w = 40, h = 5;
    int r = 10, c = 20;
    dialog_draw_box(r, c, h, w, COL_DIALOG_BG);
    display_puts(r, c + 2, title, COL_DIALOG_BG);
    display_puts(r + 2, c + 2, msg, COL_DIALOG_BG);
    display_puts(r + 3, c + 2, "Press any key...", COL_DIALOG_BG);
    display_flush();
    input_get_key();
}

int dialog_confirm(const char *title, const char *msg) {
    int w = 50, h = 5;
    int r = 10, c = 15;
    int key;
    dialog_draw_box(r, c, h, w, COL_DIALOG_BG);
    display_puts(r, c + 2, title, COL_DIALOG_BG);
    display_puts(r + 2, c + 2, msg, COL_DIALOG_BG);
    display_puts(r + 3, c + 2, "[Y] Yes   [N] No", COL_DIALOG_BG);
    display_flush();
    for (;;) {
        key = input_get_key();
        if (key == 'y' || key == 'Y') return 1;
        if (key == 'n' || key == 'N' || key == KEY_ESC) return 0;
    }
}

int dialog_save_discard_cancel(void) {
    int w = 52, h = 5;
    int r = 10, c = 14;
    int key;
    dialog_draw_box(r, c, h, w, COL_DIALOG_BG);
    display_puts(r, c + 2, "Unsaved changes", COL_DIALOG_BG);
    display_puts(r + 2, c + 2, "Save changes before exit?", COL_DIALOG_BG);
    display_puts(r + 3, c + 2, "[S] Save   [D] Discard   [C] Cancel", COL_DIALOG_BG);
    display_flush();
    for (;;) {
        key = input_get_key();
        if (key == 's' || key == 'S') return 0;
        if (key == 'd' || key == 'D') return 1;
        if (key == 'c' || key == 'C' || key == KEY_ESC) return 2;
    }
}

int dialog_input(const char *title, const char *prompt, char *buf, int maxlen) {
    int w = 52, h = 6;
    int r = 9, c = 14;
    int len = 0;
    int key;

    buf[0] = '\0';

    for (;;) {
        dialog_draw_box(r, c, h, w, COL_DIALOG_BG);
        display_puts(r, c + 2, title, COL_DIALOG_BG);
        display_puts(r + 2, c + 2, prompt, COL_DIALOG_BG);
        display_fill(r + 3, c + 2, w - 4, ' ', COL_DIALOG_INP);
        display_puts(r + 3, c + 2, buf, COL_DIALOG_INP);
        display_puts(r + 4, c + 2, "Enter=OK   Esc=Cancel", COL_DIALOG_BG);
        display_flush();

        key = input_get_key();
        if (key == KEY_ESC) return 0;
        if (key == KEY_ENTER) return 1;
        if (key == KEY_BACKSPACE && len > 0) {
            buf[--len] = '\0';
        } else if (key >= 0x20 && key <= 0x7E && len < maxlen - 1) {
            buf[len++] = (char)key;
            buf[len]   = '\0';
        }
    }
}

static long parse_offset(const char *s, long current, long file_size, int *ok) {
    long val = 0;
    int  relative = 0;
    int  sign = 1;
    const char *p = s;
    *ok = 0;

    while (*p == ' ') p++;
    if (*p == '+') { relative = 1; p++; }
    else if (*p == '-') { relative = 1; sign = -1; p++; }

    if (!*p) return 0;

    while (*p) {
        int digit;
        if (*p >= '0' && *p <= '9') digit = *p - '0';
        else if (*p >= 'a' && *p <= 'f') digit = *p - 'a' + 10;
        else if (*p >= 'A' && *p <= 'F') digit = *p - 'A' + 10;
        else return 0;
        val = val * 16 + digit;
        p++;
    }

    if (relative) val = current + sign * val;

    if (val < 0) val = 0;
    if (val >= file_size) val = file_size - 1;
    *ok = 1;
    return val;
}

int dialog_goto(HexEditor *ed) {
    char buf[32];
    int ok;
    long new_off;

    for (;;) {
        if (!dialog_input("Goto Offset", "Enter hex offset (e.g. 1A3F, +10, -20):",
                          buf, sizeof(buf))) {
            return 0;
        }
        new_off = parse_offset(buf, ed->cursor_offset, ed->file_size, &ok);
        if (ok) {
            ed->cursor_offset = new_off;
            {
                long new_view = (new_off / BYTES_PER_ROW) * BYTES_PER_ROW;
                long half_page = (long)((SCREEN_ROWS / 2) * BYTES_PER_ROW);
                new_view -= half_page;
                if (new_view < 0) new_view = 0;
                ed->view_offset = new_view;
            }
            return 1;
        }
        dialog_message("Error", "Invalid offset. Use hex digits, +N or -N.");
    }
}

static int parse_hex_pattern(const char *s, unsigned char *pat, int *len) {
    int count = 0;
    const char *p = s;
    while (*p && count < 255) {
        int hi, lo;
        while (*p == ' ') p++;
        if (!*p) break;
        if (*p >= '0' && *p <= '9') hi = *p - '0';
        else if (*p >= 'a' && *p <= 'f') hi = *p - 'a' + 10;
        else if (*p >= 'A' && *p <= 'F') hi = *p - 'A' + 10;
        else return 0;
        p++;
        if (*p >= '0' && *p <= '9') lo = *p - '0';
        else if (*p >= 'a' && *p <= 'f') lo = *p - 'a' + 10;
        else if (*p >= 'A' && *p <= 'F') lo = *p - 'A' + 10;
        else return 0;
        p++;
        pat[count++] = (unsigned char)((hi << 4) | lo);
    }
    *len = count;
    return count > 0;
}

int dialog_search(HexEditor *ed) {
    int w = 56, h = 8;
    int r = 8, c = 12;
    char buf[256];
    int mode = ed->search.search_mode;
    int key;
    int len = 0;

    buf[0] = '\0';

    for (;;) {
        dialog_draw_box(r, c, h, w, COL_DIALOG_BG);
        display_puts(r, c + 2, "Search", COL_DIALOG_BG);
        display_puts(r + 2, c + 2,
                     mode == SEARCH_HEX ? "Mode: [HEX]  ASCII  (Tab to switch)" :
                                          "Mode:  HEX  [ASCII] (Tab to switch)",
                     COL_DIALOG_BG);
        display_puts(r + 3, c + 2,
                     mode == SEARCH_HEX ? "Pattern (hex bytes, e.g. 4D 5A 90):" :
                                          "Pattern (text string):",
                     COL_DIALOG_BG);
        display_fill(r + 4, c + 2, w - 4, ' ', COL_DIALOG_INP);
        display_puts(r + 4, c + 2, buf, COL_DIALOG_INP);
        display_puts(r + 5, c + 2, "Enter=Search   Tab=Switch Mode   Esc=Cancel",
                     COL_DIALOG_BG);
        display_flush();

        key = input_get_key();
        if (key == KEY_ESC) return 0;
        if (key == KEY_TAB) {
            mode = (mode == SEARCH_HEX) ? SEARCH_ASCII : SEARCH_HEX;
            buf[0] = '\0';
            len = 0;
            continue;
        }
        if (key == KEY_ENTER) {
            if (len == 0) return 0;
            if (mode == SEARCH_HEX) {
                int plen;
                if (!parse_hex_pattern(buf, ed->search.pattern, &plen)) {
                    dialog_message("Error", "Invalid hex pattern.");
                    continue;
                }
                ed->search.pattern_len = plen;
            } else {
                int i;
                ed->search.pattern_len = len;
                for (i = 0; i < len; i++) ed->search.pattern[i] = (unsigned char)buf[i];
            }
            ed->search.search_mode = mode;
            ed->search.last_found  = -1;
            return 1;
        }
        if (key == KEY_BACKSPACE && len > 0) {
            buf[--len] = '\0';
        } else if (key >= 0x20 && key <= 0x7E && len < (int)sizeof(buf) - 1) {
            buf[len++] = (char)key;
            buf[len]   = '\0';
        }
    }
}
