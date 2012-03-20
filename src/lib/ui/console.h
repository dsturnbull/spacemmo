#ifndef __src_lib_ui_console_h
#define __src_lib_ui_console_h

#include <histedit.h>
#include <ruby/ruby.h>

#include "src/lib/spacemmo.h"

struct console_st {
    ui_t *ui;
    char *prompt;
    History *history;
    HistEvent ev;
    char *hist_file;
    EditLine *el;
    Tokenizer *t;
};

console_t * init_console(ui_t *, char *);

void init_ruby_console(console_t *);
VALUE spacemmo_client_status(VALUE);
VALUE spacemmo_client_entity(VALUE);

VALUE spacemmo_entity_initialize(VALUE);
VALUE spacemmo_entity_status(VALUE);
VALUE spacemmo_entity_thrust(VALUE, VALUE, int);

char * prompt(EditLine *);
void update_console(console_t *, double);
void process_input(console_t *);
void shutdown_console(console_t *);

#endif

