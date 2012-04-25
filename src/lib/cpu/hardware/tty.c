#include <stdbool.h>
#include <stdlib.h>
#include <util.h>
#include <unistd.h>
#include <string.h>

#include "src/lib/cpu/hardware/tty.h"

tty_t *
init_tty()
{
    tty_t *tty = calloc(1, sizeof(tty_t));

    if (openpty(&tty->master, &tty->slave, NULL, NULL, NULL) != 0) {
        perror("openpty");
        return NULL;
    }

    //fcntl(tty->master, F_SETFL, O_NONBLOCK);
    tty->fn = strdup(ttyname(tty->slave));
    printf("%s\n", tty->fn);

    //struct termios tio;
    //tcgetattr(tty->master, &tio);
    //tio.c_lflag &= ECHO;
    //tcsetattr(tty->master, TCSANOW, &tio);

    return tty;
}

bool
read_tty(tty_t *tty, char *c)
{

    if (read(tty->master, c, 1) == 1)
        return true;
    return false;
}

void
write_tty(tty_t *tty, char c)
{
    // TODO fix the terminal settings
    if (c == 0xd) {
        char d = 0xa;
        write(tty->master, &d, 1);
    }
    write(tty->master, &c, 1);
}

