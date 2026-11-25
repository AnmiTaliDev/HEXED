/* SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org> */
/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdlib.h>
#include <string.h>
#include "editor.h"
#include "fileio.h"

void editor_init(HexEditor *ed) {
    ed->view_offset   = 0;
    ed->cursor_offset = 0;
    ed->edit_mode     = MODE_HEX;
    ed->hex_nibble    = 0;
    ed->modified      = 0;
    ed->chunk_valid   = 0;
    ed->edits.entries = NULL;
    ed->edits.count   = 0;
    ed->edits.capacity = 0;
    ed->search.pattern_len = 0;
    ed->search.search_mode = SEARCH_HEX;
    ed->search.last_found  = -1;
    ed->status_msg[0] = '\0';
}

void editor_free(HexEditor *ed) {
    free(ed->edits.entries);
    ed->edits.entries = NULL;
}

static void ensure_view(HexEditor *ed) {
    long cursor = ed->cursor_offset;
    long view   = ed->view_offset;
    long page   = (long)PAGE_SIZE;

    if (cursor < view) {
        ed->view_offset = (cursor / BYTES_PER_ROW) * BYTES_PER_ROW;
    } else if (cursor >= view + page) {
        long last_row_start = (cursor / BYTES_PER_ROW) * BYTES_PER_ROW;
        long new_view = last_row_start - (long)((SCREEN_ROWS - 1) * BYTES_PER_ROW);
        if (new_view < 0) new_view = 0;
        ed->view_offset = new_view;
    }
}

void editor_move_cursor(HexEditor *ed, int dx, int dy) {
    long new_off = ed->cursor_offset + dx + (long)(dy * BYTES_PER_ROW);
    if (new_off < 0) new_off = 0;
    if (new_off >= ed->file_size) new_off = ed->file_size - 1;
    ed->cursor_offset = new_off;
    ed->hex_nibble = 0;
    ensure_view(ed);
}

void editor_scroll_page(HexEditor *ed, int dir) {
    long delta = (long)(dir * PAGE_SIZE);
    long new_off = ed->cursor_offset + delta;
    if (new_off < 0) new_off = 0;
    if (new_off >= ed->file_size) new_off = ed->file_size - 1;
    ed->cursor_offset = new_off;
    ed->hex_nibble = 0;
    ensure_view(ed);
}

void editor_goto_start(HexEditor *ed) {
    ed->cursor_offset = 0;
    ed->view_offset   = 0;
    ed->hex_nibble    = 0;
}

void editor_goto_end(HexEditor *ed) {
    ed->cursor_offset = ed->file_size - 1;
    ed->hex_nibble    = 0;
    ensure_view(ed);
}

void editor_home(HexEditor *ed) {
    ed->cursor_offset = (ed->cursor_offset / BYTES_PER_ROW) * BYTES_PER_ROW;
    ed->hex_nibble = 0;
}

void editor_end(HexEditor *ed) {
    long row_start = (ed->cursor_offset / BYTES_PER_ROW) * BYTES_PER_ROW;
    long row_end   = row_start + BYTES_PER_ROW - 1;
    if (row_end >= ed->file_size) row_end = ed->file_size - 1;
    ed->cursor_offset = row_end;
    ed->hex_nibble = 0;
}

static void edit_set_byte(HexEditor *ed, long offset, unsigned char new_val) {
    int i;
    unsigned char old_val = 0;

    for (i = 0; i < ed->edits.count; i++) {
        if (ed->edits.entries[i].offset == offset) {
            ed->edits.entries[i].new_val = new_val;
            ed->modified = 1;
            return;
        }
    }

    if (ed->edits.count >= ed->edits.capacity) {
        int new_cap = ed->edits.capacity == 0 ? 16 : ed->edits.capacity * 2;
        EditEntry *new_entries = (EditEntry *)realloc(ed->edits.entries,
                                                       new_cap * sizeof(EditEntry));
        if (!new_entries) return;
        ed->edits.entries  = new_entries;
        ed->edits.capacity = new_cap;
    }

    file_read_chunk(ed, offset, &old_val, 1);
    ed->edits.entries[ed->edits.count].offset  = offset;
    ed->edits.entries[ed->edits.count].old_val = old_val;
    ed->edits.entries[ed->edits.count].new_val = new_val;
    ed->edits.count++;
    ed->modified = 1;
}

void editor_input_hex(HexEditor *ed, int ch) {
    int nibble;
    unsigned char cur_val;

    if (ch >= '0' && ch <= '9') nibble = ch - '0';
    else if (ch >= 'a' && ch <= 'f') nibble = ch - 'a' + 10;
    else if (ch >= 'A' && ch <= 'F') nibble = ch - 'A' + 10;
    else return;

    cur_val = file_get_byte(ed, ed->cursor_offset);

    if (ed->hex_nibble == 0) {
        unsigned char new_val = (unsigned char)((nibble << 4) | (cur_val & 0x0F));
        edit_set_byte(ed, ed->cursor_offset, new_val);
        ed->hex_nibble = 1;
    } else {
        unsigned char new_val = (unsigned char)((cur_val & 0xF0) | nibble);
        edit_set_byte(ed, ed->cursor_offset, new_val);
        ed->hex_nibble = 0;
        editor_move_cursor(ed, 1, 0);
    }
}

void editor_input_ascii(HexEditor *ed, int ch) {
    if (ch < 0x20 || ch > 0x7E) return;
    edit_set_byte(ed, ed->cursor_offset, (unsigned char)ch);
    editor_move_cursor(ed, 1, 0);
}

void editor_toggle_mode(HexEditor *ed) {
    ed->edit_mode  = (ed->edit_mode == MODE_HEX) ? MODE_ASCII : MODE_HEX;
    ed->hex_nibble = 0;
}

int editor_is_modified_at(HexEditor *ed, long offset) {
    int i;
    for (i = 0; i < ed->edits.count; i++) {
        if (ed->edits.entries[i].offset == offset) return 1;
    }
    return 0;
}
