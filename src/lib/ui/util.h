#ifndef __src_lib_ui_util_h
#define __src_lib_ui_util_h

#include <stdbool.h>

typedef struct png_st {
    int w, h;
    unsigned char **data;
} png_t;

bool load_png(png_t **, char *);

#endif

