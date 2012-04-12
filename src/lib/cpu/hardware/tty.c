#include <stdio.h>
#include <stdlib.h>

#include "src/lib/cpu/hardware/tty.h"

tty_t *
init_tty()
{
    tty_t *tty = calloc(1, sizeof(tty_t));
    tty->cursor = &tty->display[0];
    return tty;
}

void
ttyp(tty_t *tty, uint8_t c)
{
    // TODO ansi
    // TODO scroll the screen

    if (c == 0xa) {
        int cur_pos = tty->cursor - &tty->display[0];
        int cur_row = cur_pos / SCREEN_WIDTH;
        int new_pos = 0;

        new_pos = (cur_row + 1) * SCREEN_WIDTH;
        tty->cursor = &tty->display[new_pos];
    } else if (c == 0x7f) {
        *(--tty->cursor) = '\0';
        while (*(tty->cursor - 1) == '\0')
            *(tty->cursor--) = '\0';
    } else {
        *(tty->cursor++) = c;
    }

    FILE *out = fopen("/tmp/screen", "w");
    write_tty(tty, out);
    fclose(out);
}

void
print_screen(tty_t *tty)
{
    write_tty(tty, stdout);
}

void
write_tty(tty_t *tty, FILE *out)
{
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        for (int j = 0; j < SCREEN_WIDTH; j++) {
            char c = tty->display[i * SCREEN_WIDTH + j];
            if (c == '\0')
                break;
            fprintf(out, "%c", c);
        }
        fprintf(out, "\n");
    }
}

