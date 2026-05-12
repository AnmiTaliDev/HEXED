/* SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org> */
/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef DIALOG_H
#define DIALOG_H

#include "hexed.h"

/* Returns 1 on confirm, 0 on cancel */
int  dialog_confirm(const char *title, const char *msg);
/* Returns 0=save, 1=discard, 2=cancel */
int  dialog_save_discard_cancel(void);
/* Returns 1 on ok with result in buf, 0 on cancel */
int  dialog_input(const char *title, const char *prompt, char *buf, int maxlen);
void dialog_message(const char *title, const char *msg);
void dialog_draw_box(int row, int col, int h, int w, unsigned char attr);
int  dialog_goto(HexEditor *ed);
int  dialog_search(HexEditor *ed);

#endif
