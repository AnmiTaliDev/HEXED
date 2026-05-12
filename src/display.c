/* SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org> */
/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <string.h>
#include <stdio.h>
#include "display.h"
#include "hexed.h"
#include "fileio.h"
#include "editor.h"

#ifdef __DJGPP__
#include <go32.h>
#include <dpmi.h>
#include <sys/farptr.h>
#define VIDEO_SEG 0xB800
#define VPUT(off, val) _farpokew(_dos_ds, (VIDEO_SEG << 4) + (off), (val))
#define VGET(off)      _farpeekw(_dos_ds, (VIDEO_SEG << 4) + (off))
#else
/* Turbo C real mode */
#include <dos.h>
static unsigned char far *video = (unsigned char far *)0xB8000000L;
#define VPUT(off, val) do { video[(off)] = (val) & 0xFF; video[(off)+1] = ((val) >> 8) & 0xFF; } while(0)
#define VGET(off)      ((unsigned short)(video[(off)] | (video[(off)+1] << 8)))
#endif

static ScreenCell back_buf[25][80];
static ScreenCell front_buf[25][80];
static int display_dirty = 1;

static void vga_write(int row, int col, unsigned char ch, unsigned char attr) {
    int off = (row * 80 + col) * 2;
    unsigned short word = (unsigned short)((attr << 8) | ch);
    VPUT(off, word);
}

void display_init(void) {
    int r, c;
    /* Save current screen */
    for (r = 0; r < 25; r++) {
        for (c = 0; c < 80; c++) {
            int off = (r * 80 + c) * 2;
            unsigned short w = VGET(off);
            back_buf[r][c].ch   = (unsigned char)(w & 0xFF);
            back_buf[r][c].attr = (unsigned char)((w >> 8) & 0xFF);
        }
    }
    /* Hide cursor */
#ifdef __DJGPP__
    {
        __dpmi_regs regs;
        regs.h.ah = 0x01;
        regs.h.ch = 0x20;
        regs.h.cl = 0x00;
        __dpmi_int(0x10, &regs);
    }
#else
    {
        union REGS regs;
        regs.h.ah = 0x01;
        regs.h.ch = 0x20;
        regs.h.cl = 0x00;
        int86(0x10, &regs, &regs);
    }
#endif
    memset(front_buf, 0, sizeof(front_buf));
    display_dirty = 1;
}

void display_restore(void) {
    int r, c;
    for (r = 0; r < 25; r++) {
        for (c = 0; c < 80; c++) {
            vga_write(r, c, back_buf[r][c].ch, back_buf[r][c].attr);
        }
    }
    /* Restore cursor */
#ifdef __DJGPP__
    {
        __dpmi_regs regs;
        regs.h.ah = 0x01;
        regs.h.ch = 0x06;
        regs.h.cl = 0x07;
        __dpmi_int(0x10, &regs);
    }
#else
    {
        union REGS regs;
        regs.h.ah = 0x01;
        regs.h.ch = 0x06;
        regs.h.cl = 0x07;
        int86(0x10, &regs, &regs);
    }
#endif
}

void display_clear(void) {
    int r, c;
    for (r = 0; r < 25; r++) {
        for (c = 0; c < 80; c++) {
            front_buf[r][c].ch   = ' ';
            front_buf[r][c].attr = COL_HEX;
        }
    }
    display_dirty = 1;
}

void display_putchar(int row, int col, unsigned char ch, unsigned char attr) {
    if (row < 0 || row >= 25 || col < 0 || col >= 80) return;
    front_buf[row][col].ch   = ch;
    front_buf[row][col].attr = attr;
}

void display_puts(int row, int col, const char *s, unsigned char attr) {
    while (*s && col < 80) {
        display_putchar(row, col++, (unsigned char)*s++, attr);
    }
}

void display_fill(int row, int col, int len, unsigned char ch, unsigned char attr) {
    int i;
    for (i = 0; i < len && col + i < 80; i++) {
        display_putchar(row, col + i, ch, attr);
    }
}

void display_flush(void) {
    int r, c;
    for (r = 0; r < 25; r++) {
        for (c = 0; c < 80; c++) {
            vga_write(r, c, front_buf[r][c].ch, front_buf[r][c].attr);
        }
    }
}

static void draw_border(void) {
    int c, r;
    /* Top border row 0 */
    display_putchar(0, 0, 0xC9, COL_BORDER);
    for (c = 1; c < 79; c++) display_putchar(0, c, 0xCD, COL_BORDER);
    display_putchar(0, 79, 0xBB, COL_BORDER);
    /* Title in row 0 */
    display_puts(0, 2, " HEXED v1.0 ", COL_BORDER);

    /* Header row 1 */
    display_putchar(1, 0, 0xBA, COL_BORDER);
    display_fill(1, 1, 78, ' ', COL_HEADER);
    display_putchar(1, 79, 0xBA, COL_BORDER);

    /* Separator row 2 */
    display_putchar(2, 0, 0xC7, COL_BORDER);
    /* col 1-10: offset col */
    for (c = 1; c <= 10; c++) display_putchar(2, c, 0xCD, COL_BORDER);
    display_putchar(2, 11, 0xD1, COL_BORDER);
    /* col 12-62: hex col (51 chars) */
    for (c = 12; c <= 62; c++) display_putchar(2, c, 0xCD, COL_BORDER);
    display_putchar(2, 63, 0xD1, COL_BORDER);
    /* col 64-78: ascii col */
    for (c = 64; c <= 78; c++) display_putchar(2, c, 0xCD, COL_BORDER);
    display_putchar(2, 79, 0xB6, COL_BORDER);

    /* Column labels row 3 */
    display_putchar(3, 0, 0xBA, COL_BORDER);
    display_puts(3, 1, " Offset   ", COL_HEADER);
    display_putchar(3, 11, 0xB3, COL_BORDER);
    display_puts(3, 12, " 00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F ", COL_HEADER);
    display_putchar(3, 63, 0xB3, COL_BORDER);
    display_puts(3, 64, " ASCII      ", COL_HEADER);
    display_putchar(3, 79, 0xBA, COL_BORDER);

    /* Separator row 4 */
    display_putchar(4, 0, 0xC7, COL_BORDER);
    for (c = 1; c <= 10; c++) display_putchar(4, c, 0xCD, COL_BORDER);
    display_putchar(4, 11, 0xD8, COL_BORDER);
    for (c = 12; c <= 62; c++) display_putchar(4, c, 0xCD, COL_BORDER);
    display_putchar(4, 63, 0xD8, COL_BORDER);
    for (c = 64; c <= 78; c++) display_putchar(4, c, 0xCD, COL_BORDER);
    display_putchar(4, 79, 0xB6, COL_BORDER);

    /* Data rows 5..24 */
    for (r = 5; r <= 24; r++) {
        display_putchar(r, 0, 0xBA, COL_BORDER);
        display_fill(r, 1, 10, ' ', COL_OFFSET);
        display_putchar(r, 11, 0xB3, COL_BORDER);
        display_fill(r, 12, 51, ' ', COL_HEX);
        display_putchar(r, 63, 0xB3, COL_BORDER);
        display_fill(r, 64, 15, ' ', COL_ASCII);
        display_putchar(r, 79, 0xBA, COL_BORDER);
    }

    /* Bottom border - replaced by hint row */
    /* Row 24 is the last data row, we only have 20 data rows so row 24 = hint */
    /* Actually: rows 5-24 = 20 rows. Row 24 is the hints bar */
    display_putchar(24, 0, 0xC8, COL_BORDER);
    for (c = 1; c < 79; c++) display_putchar(24, c, 0xCD, COL_BORDER);
    display_putchar(24, 79, 0xBC, COL_BORDER);
}

static void draw_hints(void) {
    /* Row 23 is the hint row (last before bottom border) */
    /* Actually layout: 0=top border, 1=header, 2=sep, 3=col labels, 4=sep,
       5..24 = data rows (20 rows). No room for separate hints without pushing.
       Use row 24 as bottom border with hints embedded, or use row 23 as hints
       and row 24 as bottom border (only 19 data rows).
       Per spec: 20 data rows. Let's fit hints into bottom border line. */
    const char *hints = " F2:Save  F5:Goto  F6:Find  F7:Prev  F8:Next  Tab:Mode  F10:Quit";
    int col = 1;
    const char *p = hints;
    /* Draw hints inside bottom border - we'll use a separate row scheme:
       rows 5..24 = 20 data rows, row 25 would be hints but screen is 25 rows (0-24).
       So: 0=top, 1=header, 2=sep1, 3=col-labels, 4=sep2, 5..24=data(20rows).
       No room for hints. We'll squeeze: use rows 5..23=data(19 rows), 24=hints+border. */
    /* The spec says 20 rows. We'll keep 20 data rows and put hints in bottom border. */
    while (*p && col < 79) {
        unsigned char attr;
        if (*p == 'F' && *(p+1) >= '1' && *(p+1) <= '9') {
            attr = COL_HINT_KEY;
        } else if (*p == 'T' && *(p+1) == 'a') {
            attr = COL_HINT_KEY;
        } else {
            attr = COL_HINT_TEXT;
        }
        display_putchar(24, col++, (unsigned char)*p++, attr);
    }
}

static char hex_char(int n) {
    return n < 10 ? '0' + n : 'A' + n - 10;
}

static void fmt_offset(char *buf, long off) {
    int i;
    for (i = 7; i >= 0; i--) {
        buf[i] = hex_char((int)(off & 0xF));
        off >>= 4;
    }
    buf[8] = '\0';
}

void draw_screen(HexEditor *ed) {
    char buf[16];
    int row, col, i;
    long base = ed->view_offset;
    long cursor = ed->cursor_offset;

    display_clear();
    draw_border();

    /* Header row 1 */
    {
        char hdr[79];
        int pos = 0;
        hdr[pos++] = ' ';
        hdr[pos++] = 'F';
        hdr[pos++] = 'i';
        hdr[pos++] = 'l';
        hdr[pos++] = 'e';
        hdr[pos++] = ':';
        hdr[pos++] = ' ';
        i = 0;
        while (ed->filename[i] && pos < 40) hdr[pos++] = ed->filename[i++];
        hdr[pos++] = ' ';
        hdr[pos++] = ' ';
        hdr[pos++] = 'S';
        hdr[pos++] = 'i';
        hdr[pos++] = 'z';
        hdr[pos++] = 'e';
        hdr[pos++] = ':';
        hdr[pos++] = ' ';
        fmt_offset(buf, ed->file_size);
        for (i = 0; i < 8; i++) hdr[pos++] = buf[i];
        hdr[pos++] = 'h';
        hdr[pos++] = ' ';
        hdr[pos++] = ' ';
        hdr[pos++] = 'O';
        hdr[pos++] = 'f';
        hdr[pos++] = 'f';
        hdr[pos++] = 's';
        hdr[pos++] = 'e';
        hdr[pos++] = 't';
        hdr[pos++] = ':';
        hdr[pos++] = ' ';
        fmt_offset(buf, cursor);
        for (i = 0; i < 8; i++) hdr[pos++] = buf[i];
        hdr[pos++] = 'h';
        if (ed->modified) {
            hdr[pos++] = ' ';
            hdr[pos++] = ' ';
            hdr[pos++] = '[';
            hdr[pos++] = 'm';
            hdr[pos++] = 'o';
            hdr[pos++] = 'd';
            hdr[pos++] = 'i';
            hdr[pos++] = 'f';
            hdr[pos++] = 'i';
            hdr[pos++] = 'e';
            hdr[pos++] = 'd';
            hdr[pos++] = ']';
        }
        if (ed->edit_mode == MODE_ASCII) {
            hdr[pos++] = ' ';
            hdr[pos++] = '[';
            hdr[pos++] = 'A';
            hdr[pos++] = 'S';
            hdr[pos++] = 'C';
            hdr[pos++] = 'I';
            hdr[pos++] = 'I';
            hdr[pos++] = ']';
        }
        hdr[pos] = '\0';
        display_puts(1, 1, hdr, COL_HEADER);
    }

    /* Data rows 5..24 */
    for (row = 0; row < SCREEN_ROWS; row++) {
        long row_off = base + (long)(row * BYTES_PER_ROW);
        int screen_row = 5 + row;

        if (row_off >= ed->file_size) break;

        /* Offset column */
        fmt_offset(buf, row_off);
        display_putchar(screen_row, 1, ' ', COL_OFFSET);
        display_puts(screen_row, 2, buf, COL_OFFSET);
        display_putchar(screen_row, 10, ' ', COL_OFFSET);

        /* Hex + ASCII columns */
        for (col = 0; col < BYTES_PER_ROW; col++) {
            long byte_off = row_off + col;
            unsigned char val;
            unsigned char hex_attr, asc_attr;
            int hex_col, asc_col;
            int is_cursor = (byte_off == cursor);
            int is_found = 0;

            /* Check if in found range */
            if (ed->search.last_found >= 0 &&
                byte_off >= ed->search.last_found &&
                byte_off < ed->search.last_found + ed->search.pattern_len) {
                is_found = 1;
            }

            if (byte_off >= ed->file_size) {
                break;
            }

            /* Get byte value (from edit buffer if modified) */
            val = 0;
            {
                int found_edit = 0;
                int ei;
                for (ei = 0; ei < ed->edits.count; ei++) {
                    if (ed->edits.entries[ei].offset == byte_off) {
                        val = ed->edits.entries[ei].new_val;
                        found_edit = 1;
                        break;
                    }
                }
                if (!found_edit) {
                    file_read_chunk(ed, byte_off, &val, 1);
                }
            }

            /* Determine attributes */
            if (is_cursor) {
                hex_attr = COL_CURSOR;
                asc_attr = COL_CURSOR;
            } else if (is_found) {
                hex_attr = COL_FOUND;
                asc_attr = COL_FOUND;
            } else if (editor_is_modified_at(ed, byte_off)) {
                hex_attr = COL_MODIFIED;
                asc_attr = COL_ASCII;
            } else {
                hex_attr = COL_HEX;
                asc_attr = COL_ASCII;
            }

            /* Hex column position */
            hex_col = 13 + col * 3 + (col >= 8 ? 1 : 0);
            display_putchar(screen_row, hex_col,     hex_char(val >> 4),   hex_attr);
            display_putchar(screen_row, hex_col + 1, hex_char(val & 0x0F), hex_attr);

            /* ASCII column position */
            asc_col = 65 + col;
            {
                unsigned char asc = (val >= 0x20 && val <= 0x7E) ? val : '.';
                display_putchar(screen_row, asc_col, asc, asc_attr);
            }
        }
    }

    /* Status message */
    if (ed->status_msg[0]) {
        display_fill(24, 1, 78, ' ', COL_HINT_TEXT);
        display_puts(24, 2, ed->status_msg, COL_STATUS_ERR);
    } else {
        draw_hints();
    }

    display_flush();
}

void draw_status(HexEditor *ed) {
    draw_screen(ed);
}
