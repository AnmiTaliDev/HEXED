# HEXED

Full-screen hex editor for FreeDOS and MS-DOS, running in VGA 80x25 text mode.

## Dependencies

Build:

- DJGPP cross-compiler (`i586-pc-msdosdjgpp-gcc`) on Linux
- Make

Runtime:

- FreeDOS, MS-DOS 5.0+, DOSBox, or DOSBox-X
- `CWSDPMI.EXE` (DPMI host required by the DJGPP runtime)

## Build

```sh
make
```

Output: `HEXED.EXE`

## Running

Download `CWSDPMI.EXE` from [sandmann.dotster.com](https://sandmann.dotster.com/cwsdpmi/) and place it in the same directory as `HEXED.EXE`.

```
HEXED.EXE [file]
```

Without an argument, the editor asks for a filename.

## Keybindings

| Key | Action |
|-----|--------|
| Arrow keys | Move cursor one byte |
| PgUp / PgDn | Scroll one page (320 bytes) |
| Home / End | Start / end of current row |
| Ctrl+Home | Beginning of file |
| Ctrl+End | End of file |
| Tab | Toggle hex / ASCII edit mode |
| F2 | Save file |
| F5 | Goto offset |
| F6 | Open search dialog |
| F7 | Find previous match |
| F8 | Find next match |
| F10 / Esc | Quit |

In hex mode, type two hex digits (0-9, A-F) to overwrite the current byte. In ASCII mode, type any printable character.

## License

GNU General Public License v3.0, see [LICENSE](LICENSE).
