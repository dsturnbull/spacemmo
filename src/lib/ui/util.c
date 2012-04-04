#include <png.h>
#include <stdbool.h>
#include <stdlib.h>

#include "src/lib/ui/util.h"

bool
load_png(png_t **pngp, char *fn)
{
    FILE *fp;

    if ((fp = fopen(fn, "r")) == NULL) {
        fprintf(stderr, "can't open %s\n", fn);
        return false;
    }

    int check = 8;
    unsigned char header[check];
    fread(header, 1, check, fp);

    if (png_sig_cmp(header, 0, check) != 0) {
        fprintf(stderr, "%s not a png file\n", fn);
        return false;
    }

    png_struct *png;
    if ((png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))
            == NULL) {
        fprintf(stderr, "can't allocate png *\n");
        return false;
    }

    png_info *png_info;
    if ((png_info = png_create_info_struct(png)) == NULL) {
        png_destroy_read_struct(&png, NULL, NULL);
        fprintf(stderr, "can't allocate png_info *\n");
        return false;
    }

    png_init_io(png, fp);
    png_set_sig_bytes(png, 8);
    png_set_expand(png);

    png_read_png(png, png_info, PNG_TRANSFORM_EXPAND, NULL);

    *pngp = malloc(sizeof(png_t));
    (*pngp)->w   = png_get_image_width(png, png_info);
    (*pngp)->h   = png_get_image_height(png, png_info);
    (*pngp)->bpp = png_get_bit_depth(png, png_info);

    (*pngp)->data = png_malloc(png, (*pngp)->h * png_sizeof(png_bytep));
    for (int i = 0; i < (*pngp)->h; i++) {
        (*pngp)->data[i] = png_malloc(png, (*pngp)->w * (*pngp)->bpp);
    }

    png_set_rows(png, png_info, (*pngp)->data);
    png_destroy_read_struct(&png, &png_info, NULL);
    png_read_end(png, NULL);

    fclose(fp);
    return true;
}

