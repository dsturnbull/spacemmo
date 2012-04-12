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
}

