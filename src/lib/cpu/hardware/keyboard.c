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

void
keyboard_state(keyboard_t *kbd, uint8_t buf[])
{
    /*
    (0..0xf).map do |i|
        p [4, i, i / 4 * 4, 4 - (i % 4) - 1 + i / 4 * 4]
    end

    [4, 0, 0, 3]
    [4, 1, 0, 2]
    [4, 2, 0, 1]
    [4, 3, 0, 0]
    [4, 4, 4, 7]
    [4, 5, 4, 6]
    [4, 6, 4, 5]
    [4, 7, 4, 4]
    [4, 8, 8, 11]
    [4, 9, 8, 10]
    [4, 10, 8, 9]
    [4, 11, 8, 8]
    [4, 12, 12, 15]
    [4, 13, 12, 14]
    [4, 14, 12, 13]
    [4, 15, 12, 12]
    */

    for (uint8_t i = 0; i < UINT8_MAX; i++) {
        buf[(4 - (i % 4) - 1) + (i / 4 * 4)] = kbd->input->keys[i];
        kbd->input->keys[i] = 0;
    }
}

