#ifndef __src_lib_console_h
#define __src_lib_console_h

#include <histedit.h>
#include "src/lib/spacemmo.h"

struct console_st {
    char *prompt;
    History *history;
    HistEvent ev;
    struct client_st *client;
    char *hist_file;
    EditLine *el;
};

void init_console(console_t **, char *);
char * prompt(EditLine *);
void update_console(console_t *, double);
void process_input(EditLine *);
void shutdown_console(EditLine *);

#endif

