/* SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org> */
/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef FILEIO_H
#define FILEIO_H

#include "hexed.h"

int  file_open(HexEditor *ed, const char *filename);
void file_close(HexEditor *ed);
int  file_read_chunk(HexEditor *ed, long offset, unsigned char *buf, int len);
int  file_save(HexEditor *ed);
unsigned char file_get_byte(HexEditor *ed, long offset);

#endif
