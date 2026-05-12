/* SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org> */
/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <string.h>
#include "search.h"
#include "fileio.h"
#include "editor.h"

static int match_at(HexEditor *ed, long offset) {
    int i;
    unsigned char b;
    if (offset + ed->search.pattern_len > ed->file_size) return 0;
    for (i = 0; i < ed->search.pattern_len; i++) {
        if (!editor_get_byte_at(ed, offset + i, &b)) return 0;
        if (b != ed->search.pattern[i]) return 0;
    }
    return 1;
}

int search_find_next(HexEditor *ed) {
    long start, off;
    if (ed->search.pattern_len == 0) return 0;
    start = ed->cursor_offset + 1;
    for (off = start; off <= ed->file_size - ed->search.pattern_len; off++) {
        if (match_at(ed, off)) {
            ed->search.last_found = off;
            ed->cursor_offset = off;
            {
                long new_view = (off / BYTES_PER_ROW) * BYTES_PER_ROW;
                long half = (long)((SCREEN_ROWS / 2) * BYTES_PER_ROW);
                new_view -= half;
                if (new_view < 0) new_view = 0;
                ed->view_offset = new_view;
            }
            return 1;
        }
    }
    return 0;
}

int search_find_prev(HexEditor *ed) {
    long start, off;
    if (ed->search.pattern_len == 0) return 0;
    start = ed->cursor_offset - 1;
    for (off = start; off >= 0; off--) {
        if (match_at(ed, off)) {
            ed->search.last_found = off;
            ed->cursor_offset = off;
            {
                long new_view = (off / BYTES_PER_ROW) * BYTES_PER_ROW;
                long half = (long)((SCREEN_ROWS / 2) * BYTES_PER_ROW);
                new_view -= half;
                if (new_view < 0) new_view = 0;
                ed->view_offset = new_view;
            }
            return 1;
        }
    }
    return 0;
}
