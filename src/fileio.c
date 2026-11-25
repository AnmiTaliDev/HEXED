/* SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org> */
/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <string.h>
#include <stdlib.h>
#include "fileio.h"
#include "editor.h"

int file_open(HexEditor *ed, const char *filename) {
    strncpy(ed->filename, filename, sizeof(ed->filename) - 1);
    ed->filename[sizeof(ed->filename) - 1] = '\0';

    ed->fp = fopen(filename, "r+b");
    if (!ed->fp) {
        ed->fp = fopen(filename, "rb");
        if (!ed->fp) return 0;
    }

    fseek(ed->fp, 0, SEEK_END);
    ed->file_size = ftell(ed->fp);
    rewind(ed->fp);

    if (ed->file_size == 0) {
        fclose(ed->fp);
        ed->fp = NULL;
        return 0;
    }

    return 1;
}

void file_close(HexEditor *ed) {
    fclose(ed->fp);
    ed->fp = NULL;
}

int file_read_chunk(HexEditor *ed, long offset, unsigned char *buf, int len) {
    int actually;
    fseek(ed->fp, offset, SEEK_SET);
    actually = (int)fread(buf, 1, len, ed->fp);
    return actually;
}

unsigned char file_get_byte(HexEditor *ed, long offset) {
    unsigned char b = 0;
    int ei;

    for (ei = 0; ei < ed->edits.count; ei++) {
        if (ed->edits.entries[ei].offset == offset) {
            return ed->edits.entries[ei].new_val;
        }
    }

    file_read_chunk(ed, offset, &b, 1);
    return b;
}

int file_save(HexEditor *ed) {
    FILE *fp;
    int i;

    fp = fopen(ed->filename, "r+b");
    if (!fp) return 0;

    for (i = 0; i < ed->edits.count; i++) {
        fseek(fp, ed->edits.entries[i].offset, SEEK_SET);
        if (fwrite(&ed->edits.entries[i].new_val, 1, 1, fp) != 1) {
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    ed->edits.count = 0;
    ed->modified = 0;
    return 1;
}
