#ifndef __src_lib_ui_console_h
#define __src_lib_ui_console_h

#include <histedit.h>

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

typedef enum cmd_e {
    CMD_NOTFOUND,
    CMD_BECOME,
    CMD_STATUS,
    CMD_SCAN,
    CMD_THRUST,
    CMD_QUIT,
} cmd_t;

static const char *cmds[] = {
    "become",
    "status",
    "scan",
    "thrust",
    "quit",
};

console_t * init_console(ui_t *, char *);

cmd_t lookup(const char *);
void cmd_become(console_t *, int, char **);
void cmd_status(console_t *, int, char **);
void cmd_scan(console_t *, int, char **);
void cmd_thrust(console_t *, int, char **);

char * prompt(EditLine *);
void update_console(console_t *, double);
void process_input(console_t *);
void shutdown_console(console_t *);

#endif

