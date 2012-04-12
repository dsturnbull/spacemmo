#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

#include "src/lib/ui/input.h"
#include "src/lib/cpu/hardware/keyboard.h"

keyboard_t *
init_keyboard()
{
    keyboard_t *kbd = calloc(1, sizeof(keyboard_t));
    return kbd;
}

uint8_t
readchar(keyboard_t *kbd)
{
    if (kbd->input) {
        return 0;
    } else {
        struct termios old_tio, new_tio;
        tcgetattr(1, &old_tio);
        new_tio = old_tio;
        new_tio.c_lflag &= (~ICANON & ~ECHO);

        tcsetattr(1, TCSANOW, &new_tio);
        uint8_t c = getchar();
        tcsetattr(1, TCSANOW, &old_tio);

        return c;
    }
}

