#include <stdio.h>
#include <png.h>

#include <SDL_image.h>

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#include "src/lib/client.h"
#include "src/lib/entity.h"
#include "src/lib/ui/skybox.h"
#include "src/lib/ui.h"
#include "src/lib/ui/util.h"
#include "src/lib/ui/gfx.h"

static GLuint skybox[16];
static int skybox_id = 0;

void
init_skybox(char *fn)
{
    SDL_Surface *png, *tex;

    if ((png = IMG_Load(fn)) == NULL) {
        fprintf(stderr, "can't load image: %s", SDL_GetError());
        return;
    }

    tex = SDL_CreateRGBSurface(0, png->w, png->h, 24,
            0xff000000,
            0x00ff0000,
            0x0000ff00,
            0x00000000);
    SDL_BlitSurface(png, 0, tex, 0);

    int id = skybox_id++;
    glGenTextures(1, &skybox[id]);
    glBindTexture(GL_TEXTURE_2D, skybox[id]);

    glTexImage2D(GL_TEXTURE_2D, 0, tex->format->BytesPerPixel,
            tex->w, tex->h, 0,
            GL_RGB, GL_UNSIGNED_BYTE, tex->pixels);

    gluBuild2DMipmaps(GL_TEXTURE_2D, tex->format->BytesPerPixel,
            tex->w, tex->h,
            GL_RGB, GL_UNSIGNED_BYTE, tex->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    SDL_FreeSurface(tex);
    SDL_FreeSurface(png);
}

void
render_skybox(gfx_t *gfx, GLuint id)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, skybox[id]);

    glColor4f(1, 1, 1, 1);

    glPushMatrix();
    {
        // Render the front quad
        glColor4f(1, 0, 0, 1); // RED
        glBindTexture(GL_TEXTURE_2D, skybox[id]);
        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.25, 0.33); glVertex3f( 0.5f, -0.5f, -0.5f );
            glTexCoord2f(0.50, 0.33); glVertex3f(-0.5f, -0.5f, -0.5f );
            glTexCoord2f(0.50, 0.66); glVertex3f(-0.5f,  0.5f, -0.5f );
            glTexCoord2f(0.25, 0.66); glVertex3f( 0.5f,  0.5f, -0.5f );
        }
        glEnd();

        // Render the left quad
        glColor4f(1, 0, 1, 1); // PURPLE
        glBindTexture(GL_TEXTURE_2D, skybox[id]);
        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.00, 0.25); glVertex3f( 0.5f, -0.5f,  0.5f );
            glTexCoord2f(0.25, 0.25); glVertex3f( 0.5f, -0.5f, -0.5f );
            glTexCoord2f(0.25, 0.50); glVertex3f( 0.5f,  0.5f, -0.5f );
            glTexCoord2f(0.00, 0.50); glVertex3f( 0.5f,  0.5f,  0.5f );
        }
        glEnd();

        // Render the back quad
        glBindTexture(GL_TEXTURE_2D, skybox[id]);
        glColor4f(1, 1, 0, 1); // YELLOW
        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.75, 0.25); glVertex3f(-0.5f, -0.5f,  0.5f );
            glTexCoord2f(1.00, 0.25); glVertex3f( 0.5f, -0.5f,  0.5f );
            glTexCoord2f(1.00, 0.50); glVertex3f( 0.5f,  0.5f,  0.5f );
            glTexCoord2f(0.75, 0.50); glVertex3f(-0.5f,  0.5f,  0.5f );
        }
        glEnd();

        // Render the right quad
        glColor4f(0.5, 0.5, 0.5, 1); // DARK
        glBindTexture(GL_TEXTURE_2D, skybox[id]);
        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.50, 0.25); glVertex3f(-0.5f, -0.5f, -0.5f );
            glTexCoord2f(0.75, 0.25); glVertex3f(-0.5f, -0.5f,  0.5f );
            glTexCoord2f(0.75, 0.50); glVertex3f(-0.5f,  0.5f,  0.5f );
            glTexCoord2f(0.50, 0.50); glVertex3f(-0.5f,  0.5f, -0.5f );
        }
        glEnd();

        // Render the top quad
        glBindTexture(GL_TEXTURE_2D, skybox[id]);
        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.25, 0.25); glVertex3f(-0.5f,  0.5f, -0.5f );
            glTexCoord2f(0.25, 0.00); glVertex3f(-0.5f,  0.5f,  0.5f );
            glTexCoord2f(0.50, 0.00); glVertex3f( 0.5f,  0.5f,  0.5f );
            glTexCoord2f(0.50, 0.25); glVertex3f( 0.5f,  0.5f, -0.5f );
        }
        glEnd();

        // Render the bottom quad
        glBindTexture(GL_TEXTURE_2D, skybox[id]);
        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.25, 0.50); glVertex3f(-0.5f, -0.5f, -0.5f );
            glTexCoord2f(0.25, 0.75); glVertex3f(-0.5f, -0.5f,  0.5f );
            glTexCoord2f(0.50, 0.75); glVertex3f( 0.5f, -0.5f,  0.5f );
            glTexCoord2f(0.50, 0.50); glVertex3f( 0.5f, -0.5f, -0.5f );
        }
        glEnd();
    }
    glPopMatrix();

    glBindTexture(GL_TEXTURE_2D, 0);
}

