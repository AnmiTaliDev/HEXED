/* SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org> */
/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef HEXED_H
#define HEXED_H

#include <stdio.h>

#define MODE_HEX     0
#define MODE_ASCII   1
#define SEARCH_HEX   0
#define SEARCH_ASCII 1

#define SCREEN_ROWS  20
#define BYTES_PER_ROW 16
#define PAGE_SIZE    (SCREEN_ROWS * BYTES_PER_ROW)

#define ATTR(bg, fg)   (((bg) << 4) | (fg))
#define COL_BORDER     ATTR(1, 3)
#define COL_HEADER     ATTR(1, 7)
#define COL_OFFSET     ATTR(0, 14)
#define COL_HEX        ATTR(0, 7)
#define COL_MODIFIED   ATTR(0, 14)
#define COL_CURSOR     ATTR(9, 15)
#define COL_ASCII      ATTR(0, 10)
#define COL_FOUND      ATTR(2, 15)
#define COL_HINT_KEY   ATTR(1, 14)
#define COL_HINT_TEXT  ATTR(1, 8)
#define COL_DIALOG_BG  ATTR(7, 0)
#define COL_DIALOG_INP ATTR(0, 15)
#define COL_STATUS_ERR ATTR(0, 12)

typedef struct {
    long          offset;
    unsigned char old_val;
    unsigned char new_val;
} EditEntry;

typedef struct {
    EditEntry *entries;
    int        count;
    int        capacity;
} EditBuffer;

typedef struct {
    unsigned char pattern[256];
    int           pattern_len;
    int           search_mode;
    long          last_found;
} SearchState;

typedef struct {
    FILE        *fp;
    long         file_size;
    long         view_offset;
    long         cursor_offset;
    int          edit_mode;
    int          hex_nibble;
    int          modified;
    char         filename[128];
    unsigned char chunk[PAGE_SIZE];
    int          chunk_valid;
    EditBuffer   edits;
    SearchState  search;
    char         status_msg[80];
} HexEditor;

typedef struct {
    unsigned char ch;
    unsigned char attr;
} ScreenCell;

#endif
