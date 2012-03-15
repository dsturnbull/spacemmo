#ifndef __src_lib_console_h
#define __src_lib_console_h

#include <histedit.h>
#include "src/lib/spacemmo.h"

struct console_st {
    ui_t *ui;
    char *prompt;
    History *history;
    HistEvent ev;
    char *hist_file;
    EditLine *el;
};

console_t * init_console(ui_t *, char *);
char * prompt(EditLine *);
void update_console(console_t *, double);
void process_input(console_t *);
void shutdown_console(console_t *);

#endif

