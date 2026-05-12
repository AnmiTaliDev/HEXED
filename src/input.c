/* SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org> */
/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "input.h"

#ifdef __DJGPP__
#include <dpmi.h>
#include <string.h>

int input_get_key(void) {
    __dpmi_regs r;
    memset(&r, 0, sizeof(r));
    r.h.ah = 0x00;
    __dpmi_int(0x16, &r);
    /* Read AL and AH directly to avoid packing issues */
    if (r.h.al == 0x00 || r.h.al == 0xE0) {
        switch (r.h.ah) {
            case 0x48: return KEY_UP;
            case 0x50: return KEY_DOWN;
            case 0x4B: return KEY_LEFT;
            case 0x4D: return KEY_RIGHT;
            case 0x49: return KEY_PGUP;
            case 0x51: return KEY_PGDN;
            case 0x47: return KEY_HOME;
            case 0x4F: return KEY_END;
            case 0x77: return KEY_CTRL_HOME;
            case 0x75: return KEY_CTRL_END;
            case 0x3C: return KEY_F2;
            case 0x3F: return KEY_F5;
            case 0x40: return KEY_F6;
            case 0x41: return KEY_F7;
            case 0x42: return KEY_F8;
            case 0x44: return KEY_F10;
        }
        return -1;
    }
    return (int)(unsigned char)r.h.al;
}

#else
/* Turbo C / BIOS */
#include <dos.h>
#include <conio.h>

int input_get_key(void) {
    int c = getch();
    if (c == 0 || c == 0xE0) {
        int ext = getch();
        switch (ext) {
            case 0x48: return KEY_UP;
            case 0x50: return KEY_DOWN;
            case 0x4B: return KEY_LEFT;
            case 0x4D: return KEY_RIGHT;
            case 0x49: return KEY_PGUP;
            case 0x51: return KEY_PGDN;
            case 0x47: return KEY_HOME;
            case 0x4F: return KEY_END;
            case 0x77: return KEY_CTRL_HOME;
            case 0x75: return KEY_CTRL_END;
            case 0x3C: return KEY_F2;
            case 0x3F: return KEY_F5;
            case 0x40: return KEY_F6;
            case 0x41: return KEY_F7;
            case 0x42: return KEY_F8;
            case 0x44: return KEY_F10;
        }
        return -1;
    }
    return c;
}
#endif
