/* SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org> */
/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef EDITOR_H
#define EDITOR_H

#include "hexed.h"

void editor_init(HexEditor *ed);
void editor_free(HexEditor *ed);
void editor_move_cursor(HexEditor *ed, int dx, int dy);
void editor_scroll_page(HexEditor *ed, int dir);
void editor_goto_start(HexEditor *ed);
void editor_goto_end(HexEditor *ed);
void editor_home(HexEditor *ed);
void editor_end(HexEditor *ed);
void editor_input_hex(HexEditor *ed, int ch);
void editor_input_ascii(HexEditor *ed, int ch);
void editor_toggle_mode(HexEditor *ed);
int  editor_is_modified_at(HexEditor *ed, long offset);

#endif
