#include <err.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

#include <SDL/SDL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <agar/core.h>

#include "src/lib/gfx.h"

void
init_gfx(gfx_t **gfx)
{
    int w = 800;
    int h = 600;

    *gfx = calloc(1, sizeof(gfx_t));

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        err(EX_CONFIG, "%s", SDL_GetError());

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    const SDL_VideoInfo *info;
    if ((info = SDL_GetVideoInfo()) == NULL)
        err(EX_CONFIG, "%s", SDL_GetError());

    int flags = SDL_HWSURFACE | SDL_OPENGL;
    if (SDL_SetVideoMode(w, h, info->vfmt->BitsPerPixel, flags) == NULL)
        err(EX_CONFIG, "%s", SDL_GetError());
}

void
handle_input(gfx_t *gfx)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                gfx->quit = true;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        gfx->quit = true;
                        break;

                    case SDLK_q:
                        if (event.key.keysym.mod & KMOD_META)
                            gfx->quit = true;
                        break;

                    default:
                        break;
            }
        }
    }
}

void
update_gfx(gfx_t *gfx, double dt)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBegin(GL_LINES);
    glVertex2f(0.0, 0.0);
    glVertex2f(0.0, 0.9);
    glVertex2f(0.0, 0.9);
    glVertex2f(0.9, 0.9);
    glVertex2f(0.6, 0.3);
    glVertex2f(0.1, 0.2);
    glEnd();
    SDL_GL_SwapBuffers();
}

