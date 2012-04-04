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

#include "src/lib/client.h"
#include "src/lib/server.h"
#include "src/lib/world.h"
#include "src/lib/entity.h"
#include "src/lib/ui.h"
#include "src/lib/ui/gfx.h"
#include "src/lib/ui/input.h"
#include "src/lib/ui/util.h"
#include "src/lib/ui/skybox.h"
#include "src/lib/ui/starfield.h"

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
    gfx->eye = calloc(1, sizeof(vec3f));
    gfx->tgt = calloc(1, sizeof(vec3f));

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
        if ((surf = SDL_SetVideoMode(gfx->w, gfx->h, info->vfmt->BitsPerPixel,
                        flags)) == NULL) {
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
        int flags = AG_VIDEO_HWSURFACE | AG_VIDEO_DOUBLEBUF | AG_VIDEO_OPENGL
            | AG_VIDEO_RESIZABLE;
        if ((AG_InitVideo(gfx->w, gfx->h, 24, flags) == -1)) {
            fprintf(stderr, "%s", AG_GetError());
            return NULL;
        }
    }

    if ((gfx->drv = AGDRIVER(agDriverSw)) == NULL) {
        fprintf(stderr, "%s", AG_GetError());
        return NULL;
    }

    SDL_WM_GrabInput(SDL_GRAB_ON);
    //SDL_ShowCursor(0);

    init_gfx_ui(gfx);

    return gfx;
}

void
init_gfx_ui(gfx_t *this)
{
    this->mouse = this->drv->mouse;
    init_gfx_menu(this);
    //init_skybox("data/space.png");
    init_skybox("data/spheremapgalaxyasteroid.png");
    //init_skybox("data/SkyBox-Clouds-Few-Dawn.png");
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
init_gfx_ship_ui(gfx_t *gfx, entity_t *entity)
{
    init_gfx_ship(gfx, entity);
    init_gfx_ship_status_window(gfx, entity);
}

void
init_gfx_ship(gfx_t *gfx, entity_t *entity)
{
    init_starfield(gfx);
    // gluCylinder(gluNewQuadric(), 2, 1, 2, 4, 4);
}

void
init_gfx_ship_status_window(gfx_t *gfx, entity_t *entity)
{
    AG_Window *win = AG_WindowNew(AG_WINDOW_DIALOG);
    AG_WindowSetCaption(win, "Status");
    AG_WindowSetPosition(win, AG_WINDOW_BR, 1);
    AG_WindowSetGeometry(win, 0, 0, 200, 200);

    vec3f *pos = entity->pos;
    vec3f *vel = entity->vel;
    vec3f *acc = entity->acc;
    vec3f *ypr = entity->ypr;

    char *hint;
    char *fmt = "x:%lf y:%lf z:%lf";
    asprintf(&hint, "x:%lf y:%lf z:%lf", 0.0f, 0.0f, 0.0f);

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

    l = AG_LabelNew(win, AG_LABEL_FRAME, "YPR");
    l = AG_LabelNewPolled(win, 0, fmt, &ypr->x, &ypr->y, &ypr->z);
    AG_LabelSizeHint(l, 1, hint);

    AG_WindowShow(win);
}

void
update_gfx(gfx_t *gfx, double dt)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    AG_BeginRendering(gfx->drv);
    {
        glPushMatrix();
        {
            glViewport(0, 0, gfx->w, gfx->h);

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluPerspective(45, gfx->w / gfx->h, 0.1, 1000);

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            {
                glLoadIdentity();

                orient_eye(gfx);
                render_skybox(gfx, 0);
                render_starfield(gfx, dt);
            }
            glPopMatrix();
        }
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        AG_LockVFS(gfx->drv);

        AG_Window *win;
        AG_FOREACH_WINDOW(win, gfx->drv) {
            AG_ObjectLock(win);
            AG_WindowDraw(win);
            AG_ObjectUnlock(win);
        }

        AG_UnlockVFS(gfx->drv);
    }

    AG_EndRendering(gfx->drv);

    AG_DriverEvent ev;
    while (AG_PendingEvents(gfx->drv) > 0)
        if (AG_GetNextEvent(gfx->drv, &ev))
            handle_event(gfx, &ev);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void
orient_eye(gfx_t *gfx)
{
    //vec3f *vel = gfx->ui->client->entity->vel;

    //glRotatef(vel->x, 0, 1, 0);
    //glRotatef(vel->y, 1, 0, 0);

    vec3f *ypr = gfx->ui->client->entity->ypr;

    glRotatef(gfx->mouse->y, 1, 0, 0);
    glRotatef(gfx->mouse->x, 0, 1, 0);

    glRotatef(ypr->z, 0, 0, 1);
    glRotatef(ypr->y, 1, 0, 0);
    glRotatef(ypr->x, 0, 1, 0);
}

void
shutdown_gfx(gfx_t *gfx)
{
    AG_Destroy();
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
        case AG_DRIVER_KEY_UP:
            if (handle_keypress(gfx->ui->input, ev))
                return 1;
            break;

        case AG_DRIVER_CLOSE:
            gfx->ui->client->quit = true;
            return 1;

        case AG_DRIVER_MOUSE_MOTION:
            return handle_mouse(gfx->ui->input, ev);

        default:
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

