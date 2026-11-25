/* SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org> */
/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef DIALOG_H
#define DIALOG_H

#include "hexed.h"

int  dialog_confirm(const char *title, const char *msg);
int  dialog_save_discard_cancel(void);
int  dialog_input(const char *title, const char *prompt, char *buf, int maxlen);
void dialog_message(const char *title, const char *msg);
void dialog_draw_box(int row, int col, int h, int w, unsigned char attr);
int  dialog_goto(HexEditor *ed);
int  dialog_search(HexEditor *ed);

#endif
