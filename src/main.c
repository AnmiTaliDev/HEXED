/* SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org> */
/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hexed.h"
#include "display.h"
#include "fileio.h"
#include "input.h"
#include "editor.h"
#include "dialog.h"
#include "search.h"

static void set_status(HexEditor *ed, const char *msg) {
    strncpy(ed->status_msg, msg, sizeof(ed->status_msg) - 1);
    ed->status_msg[sizeof(ed->status_msg) - 1] = '\0';
}

static void clear_status(HexEditor *ed) {
    ed->status_msg[0] = '\0';
}

static int do_save(HexEditor *ed) {
    if (!dialog_confirm("Save File", "Write changes to disk? [Y/N]")) return 0;
    if (file_save(ed)) {
        set_status(ed, "File saved.");
        return 1;
    }
    set_status(ed, "Error: could not save file.");
    return 0;
}

static int do_quit(HexEditor *ed) {
    if (ed->modified) {
        int choice = dialog_save_discard_cancel();
        if (choice == 2) return 0;
        if (choice == 0) {
            if (!file_save(ed)) {
                set_status(ed, "Error: could not save file.");
                return 0;
            }
        }
    }
    return 1;
}

static int open_file_dialog(HexEditor *ed) {
    char buf[128];
    if (!dialog_input("Open File", "Enter filename:", buf, sizeof(buf))) return 0;
    if (!file_open(ed, buf)) {
        dialog_message("Error", "Cannot open file.");
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    HexEditor ed;
    int running = 1;

    memset(&ed, 0, sizeof(ed));
    editor_init(&ed);

    display_init();
    display_clear();
    display_flush();

    if (argc >= 2) {
        if (!file_open(&ed, argv[1])) {
            display_restore();
            fprintf(stderr, "HEXED: cannot open '%s'\n", argv[1]);
            return 1;
        }
    } else {
        if (!open_file_dialog(&ed)) {
            display_restore();
            return 0;
        }
    }

    while (running) {
        clear_status(&ed);
        draw_screen(&ed);

        int key = input_get_key();

        switch (key) {
            case KEY_UP:        editor_move_cursor(&ed,  0, -1); break;
            case KEY_DOWN:      editor_move_cursor(&ed,  0,  1); break;
            case KEY_LEFT:      editor_move_cursor(&ed, -1,  0); break;
            case KEY_RIGHT:     editor_move_cursor(&ed,  1,  0); break;
            case KEY_PGUP:      editor_scroll_page(&ed, -1);     break;
            case KEY_PGDN:      editor_scroll_page(&ed,  1);     break;
            case KEY_HOME:      editor_home(&ed);                break;
            case KEY_END:       editor_end(&ed);                 break;
            case KEY_CTRL_HOME: editor_goto_start(&ed);          break;
            case KEY_CTRL_END:  editor_goto_end(&ed);            break;
            case KEY_TAB:       editor_toggle_mode(&ed);         break;

            case KEY_F2:
                do_save(&ed);
                break;

            case KEY_F5:
                dialog_goto(&ed);
                break;

            case KEY_F6:
                if (dialog_search(&ed)) {
                    if (!search_find_next(&ed)) {
                        set_status(&ed, "Not found.");
                    }
                }
                break;

            case KEY_F7:
                if (ed.search.pattern_len == 0) {
                    set_status(&ed, "No search pattern. Use F6 first.");
                } else if (!search_find_prev(&ed)) {
                    set_status(&ed, "Not found.");
                }
                break;

            case KEY_F8:
                if (ed.search.pattern_len == 0) {
                    set_status(&ed, "No search pattern. Use F6 first.");
                } else if (!search_find_next(&ed)) {
                    set_status(&ed, "Not found.");
                }
                break;

            case KEY_F10:
            case KEY_ESC:
                if (do_quit(&ed)) running = 0;
                break;

            default:
                if (ed.edit_mode == MODE_HEX) {
                    editor_input_hex(&ed, key);
                } else {
                    editor_input_ascii(&ed, key);
                }
                break;
        }
    }

    file_close(&ed);
    editor_free(&ed);
    display_restore();
    return 0;
}
