#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "src/lib/computer.h"

computer_t *
init_computer()
{
    computer_t *this = calloc(1, sizeof(computer_t));
    return this;
}

void
update_computer(computer_t *this, double dt)
{
}

