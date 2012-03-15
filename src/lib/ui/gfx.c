#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>

#include <png.h>

#include "src/lib/client.h"
#include "src/lib/entity.h"
#include "src/lib/ui.h"
#include "src/lib/ui/gfx.h"
#include "src/lib/ui/input.h"

bool show_ui = true;
bool bg_focused;

int wireframe = 0;
static vec3f _pos, _vel, _acc;

GLuint textures[10];

gfx_t *
init_gfx(ui_t *ui)
{
    gfx_t *gfx = calloc(1, sizeof(gfx_t));
    gfx->ui = ui;

    gfx->w = 1024;
    gfx->h = 768;

    if (AG_InitCore("hello computer", 0) == -1) {
        fprintf(stderr, "%s", AG_GetError());
        return NULL;
    }

    if (false) {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            fprintf(stderr, "%s", SDL_GetError());
            return NULL;
        }

        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        const SDL_VideoInfo *info;
        if ((info = SDL_GetVideoInfo()) == NULL) {
            fprintf(stderr, "%s", SDL_GetError());
            return NULL;
        }

        SDL_Surface *surf;
        int flags = SDL_HWSURFACE | SDL_OPENGL | SDL_RESIZABLE;
        if ((surf = SDL_SetVideoMode(gfx->w, gfx->h, info->vfmt->BitsPerPixel, flags)) == NULL) {
            fprintf(stderr, "%s", SDL_GetError());
            return NULL;
        }

        flags = AG_VIDEO_HWSURFACE | AG_VIDEO_OPENGL | AG_VIDEO_RESIZABLE;
        if (AG_InitVideoSDL(surf, flags) == -1) {
            fprintf(stderr, "%s", AG_GetError());
            return NULL;
        }
    } else {
        // | AG_VIDEO_NOFRAME;
        int flags = AG_VIDEO_HWSURFACE | AG_VIDEO_DOUBLEBUF | AG_VIDEO_OPENGL | AG_VIDEO_RESIZABLE;
        if ((AG_InitVideo(gfx->w, gfx->h, 24, flags) == -1)) {
            fprintf(stderr, "%s", AG_GetError());
            return NULL;
        }
    }

    if ((gfx->drv = AGDRIVER(agDriverSw)) == NULL) {
        fprintf(stderr, "%s", AG_GetError());
        return NULL;
    }

    init_gfx_ui(gfx);

    return gfx;
}

void
init_gfx_ui(gfx_t *gfx)
{
    init_gfx_menu(gfx);
    init_gfx_universe(gfx);
}

void
init_gfx_menu(gfx_t *gfx)
{
    gfx->menu = AG_MenuNewGlobal(0);
    AG_MenuItem *item;

    item = AG_MenuNode(gfx->menu->root, "Net", NULL);
    AG_MenuAction(item, "Connect", NULL, handle_net_connect_button, NULL);

    item = AG_MenuNode(gfx->menu->root, "System", NULL);
    AG_MenuAction(item, "Quit", NULL, handle_system_quit_button, NULL);
}

void
init_gfx_universe()
{
    glGenTextures(1, &textures[0]);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    png_t *png;

    if (!load_png(&png, "space.png")) {
        fprintf(stderr, "can't load skybox\n");
        return;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, png->data);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, png->data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void
render_universe(gfx_t *gfx)
{
    float x = 0, y = 0, z = 0;
    float width = 30, height = 30, length = 30;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[0]);

    // Center the skybox
    x = x - width  / 2;
    y = y - height / 2;
    z = z - length / 2;
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y,         z);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x,         y + height, z);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x,         y,         z);

    glEnd();
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x,         y,         z + length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x,         y + height, z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z + length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y,         z + length);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glBegin(GL_QUADS);

    glTexCoord2f(1.0f, 0.0f); glVertex3f(x,         y,         z);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x,         y,         z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y,         z + length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y,         z);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y + height, z);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y + height, z + length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x,         y + height,   z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x,         y + height,   z);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x,         y + height,   z);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x,         y + height,   z + length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x,         y,         z + length);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x,         y,         z);

    glEnd();
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y,         z);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y,         z + length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height,   z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height,   z);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void
init_gfx_ship_ui(gfx_t *gfx)
{
    init_gfx_ship(gfx);
    init_gfx_ship_status_window(gfx);
}

void
init_gfx_ship()
{
    // gluCylinder(gluNewQuadric(), 2, 1, 2, 4, 4);
}

void
init_gfx_ship_status_window(gfx_t *gfx)
{
    AG_Window *win = AG_WindowNew(AG_WINDOW_DIALOG);
    AG_WindowSetCaption(win, "Status");
    AG_WindowSetPosition(win, AG_WINDOW_BR, 1);
    AG_WindowSetGeometry(win, 0, 0, 200, 200);

    vec3f *pos, *vel, *acc;

    entity_t *e;

    if ((e= gfx->ui->client->entity) != NULL)
        pos = &e->pos, vel = &e->vel, acc = &e->acc;
    else
        pos = &_pos, vel = &_vel, acc = &_acc;

    char *hint;
    char *fmt = "x:%f y:%f z:%f";
    asprintf(&hint, "x:%f y:%f z:%f", 0.0f, 0.0f, 0.0f);

    AG_Label *l;

    l = AG_LabelNew(win, AG_LABEL_FRAME, "Thrust");
    l = AG_LabelNewPolled(win, 0, fmt, &acc->x, &acc->y, &acc->z);
    AG_LabelSizeHint(l, 1, hint);

    l = AG_LabelNew(win, AG_LABEL_FRAME, "Velocity");
    l = AG_LabelNewPolled(win, 0, fmt, &vel->x, &vel->y, &vel->z);
    AG_LabelSizeHint(l, 1, hint);

    l = AG_LabelNew(win, AG_LABEL_FRAME, "Position");
    l = AG_LabelNewPolled(win, 0, fmt, &pos->x, &pos->y, &pos->z);
    AG_LabelSizeHint(l, 1, hint);

    AG_WindowShow(win);
}

void
update_gfx(gfx_t *gfx, double dt)
{
    /*
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, gfx->w, gfx->h);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90, 1, 0.1, 100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -50.0f);

    //gluLookAt(0, 0, 0, 0, 0, 0, 1, 0, 0);
    //glTranslatef(-gfx->eye.x, gfx->eye.y, 0);

    glBegin(GL_QUADS);
    glVertex3f(4.0f, 0.0f, 20.0f);
    glVertex3f(4.0f, 4.0f, 20.0f);
    glVertex3f(0.0f, 4.0f, 20.0f);
    glVertex3f(0.0f, 0.0f, 20.0f);
    glEnd();

    glLoadIdentity();
    */

    AG_Window *win;
    AG_LockVFS(gfx->drv);
    AG_BeginRendering(gfx->drv);

    AG_FOREACH_WINDOW(win, gfx->drv) {
        AG_ObjectLock(win);
        AG_WindowDraw(win);
        AG_ObjectUnlock(win);
    }

    AG_EndRendering(gfx->drv);
    AG_UnlockVFS(gfx->drv);

    AG_DriverEvent ev;
    while (AG_PendingEvents(gfx->drv) > 0)
        if (AG_GetNextEvent(gfx->drv, &ev))
            handle_event(gfx, &ev);
}

void
shutdown_gfx(gfx_t *gfx)
{
    AG_Destroy();
}

bool
load_png(png_t **pngp, char *fn)
{
    FILE *fp;

    if ((fp = fopen(fn, "r")) == NULL) {
        fprintf(stderr, "can't open %s", fn);
        return false;
    }

    int check = 8;
    unsigned char header[check];
    fread(header, check, 1, fp);

    if (png_sig_cmp(header, 0, check) == 1) {
        fprintf(stderr, "%s not a png file\n", fn);
        return false;
    }

    png_struct *png;
    if ((png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL) {
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

    png_read_png(png, png_info, PNG_TRANSFORM_IDENTITY, NULL);

    int width = png_get_image_width(png, png_info);
    int height = png_get_image_height(png, png_info);
    int bpp = png_get_bit_depth(png, png_info);

    *pngp = malloc(sizeof(png_t));
    (*pngp)->data = malloc(width * height * bpp);
    png_set_rows(png, png_info, (*pngp)->data);

    png_destroy_read_struct(&png, &png_info, NULL);

    fclose(fp);
    return true;
}

void
handle_resize(gfx_t *gfx, AG_DriverEvent *ev)
{
}

int
handle_event(gfx_t *gfx, AG_DriverEvent *ev)
{
    int rv = 0;
    int x, y;

    switch (ev->type) {
        case AG_DRIVER_MOUSE_BUTTON_DOWN:
            x = ev->data.button.x;
            y = ev->data.button.y;
            return AG_ProcessEvent(gfx->drv, ev); // TODO

            if (show_ui && AG_WindowFocusAtPos(AGDRIVER_SW(gfx->drv), x, y)) {
                if (bg_focused)
                    bg_focused = 0;
                return AG_ProcessEvent(gfx->drv, ev);
            } else {
                if (bg_focused) {
                    return handle_mouse(gfx->ui->input, ev);
                } else {
                    bg_focused = 1;
                }
            }
            break;

        case AG_DRIVER_VIDEORESIZE:
            handle_resize(gfx, ev);
            return AG_ProcessEvent(gfx->drv, ev);

        case AG_DRIVER_KEY_DOWN:
            if (handle_keypress(gfx->ui->input, ev))
                return 1;
            break;

        case AG_DRIVER_CLOSE:
            gfx->ui->client->quit = true;
            return 1;

        default:
            handle_mouse(gfx->ui->input, ev);

            if (!bg_focused && show_ui) {
                return AG_ProcessEvent(gfx->drv, ev);
            } else {
                if (handle_mouse(gfx->ui->input, ev) != 1) {
                    return AG_ProcessEvent(gfx->drv, ev);
                } else {
                    return 1;
                }
            }
            break;
    }

    return 0;
}

void
handle_net_connect_button(AG_Event *event)
{
    printf("hi\n");
}

void
handle_system_quit_button(AG_Event *event)
{
    //gfx_t *gfx = AG_PTR(1);
    //gfx->ui->client->quit = true;
}
