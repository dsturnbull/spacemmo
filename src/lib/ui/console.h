#ifndef __src_lib_ui_console_h
#define __src_lib_ui_console_h

#include <histedit.h>

#include "src/lib/spacemmo.h"

typedef struct ui_st ui_t;

typedef struct console_st {
    ui_t *ui;
    char *prompt;
    History *history;
    HistEvent ev;
    char *hist_file;
    EditLine *el;
    Tokenizer *t;
} console_t;

typedef enum cmd_e {
    CMD_NOTFOUND,
    CMD_BECOME,
    CMD_STATUS,
    CMD_SCAN,
    CMD_THRUST,
    CMD_CPU_LOAD,
    CMD_CPU_START,
    CMD_CPU_STOP,
    CMD_CPU_RESET,
    CMD_CPU_STEP,
    CMD_QUIT,
} cmd_t;

static const char *cmds[] = {
    "become",
    "status",
    "scan",
    "thrust",
    "cpu_load",
    "cpu_start",
    "cpu_stop",
    "cpu_reset",
    "cpu_step",
    "quit",
};

console_t * init_console(ui_t *, char *);

cmd_t lookup(const char *);
void cmd_become(console_t *, int, char **);
void cmd_status(console_t *, int, char **);
void cmd_scan(console_t *, int, char **);
void cmd_thrust(console_t *, int, char **);

void cmd_cpu_load(console_t *, int, char **);
void cmd_cpu_start(console_t *, int, char **);
void cmd_cpu_stop(console_t *, int, char **);
void cmd_cpu_reset(console_t *, int, char **);
void cmd_cpu_step(console_t *, int, char **);

char * prompt(EditLine *);
void update_console(console_t *, double);
void process_input(console_t *);
void shutdown_console(console_t *);

#endif

