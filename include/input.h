/* SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org> */
/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef INPUT_H
#define INPUT_H

#define KEY_UP        0x0100
#define KEY_DOWN      0x0101
#define KEY_LEFT      0x0102
#define KEY_RIGHT     0x0103
#define KEY_PGUP      0x0104
#define KEY_PGDN      0x0105
#define KEY_HOME      0x0106
#define KEY_END       0x0107
#define KEY_CTRL_HOME 0x0108
#define KEY_CTRL_END  0x0109
#define KEY_F2        0x010A
#define KEY_F5        0x010B
#define KEY_F6        0x010C
#define KEY_F7        0x010D
#define KEY_F8        0x010E
#define KEY_F10       0x010F
#define KEY_TAB       0x0009
#define KEY_ENTER     0x000D
#define KEY_ESC       0x001B
#define KEY_BACKSPACE 0x0008

int input_get_key(void);

#endif
