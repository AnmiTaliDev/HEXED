/* SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org> */
/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "hexed.h"

void display_init(void);
void display_restore(void);
void display_clear(void);
void display_putchar(int row, int col, unsigned char ch, unsigned char attr);
void display_puts(int row, int col, const char *s, unsigned char attr);
void display_fill(int row, int col, int len, unsigned char ch, unsigned char attr);
void display_flush(void);
void draw_screen(HexEditor *ed);
void draw_status(HexEditor *ed);

#endif
