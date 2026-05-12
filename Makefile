# SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
# SPDX-License-Identifier: GPL-3.0-or-later

CC      = i586-pc-msdosdjgpp-gcc
CFLAGS  = -O2 -Wall -Wextra -Iinclude
LDFLAGS = -s

SRC = src/main.c \
      src/display.c \
      src/fileio.c \
      src/input.c \
      src/editor.c \
      src/dialog.c \
      src/search.c

OBJ = $(SRC:.c=.o)
TARGET = HEXED.EXE

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)
