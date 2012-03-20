#include <stdio.h>

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#include "src/lib/ui/skybox.h"
#include "src/lib/ui/util.h"

GLuint skybox[16];
GLuint skybox_id = 0;

void
init_skybox(char *fn)
{
    GLuint id = skybox_id++;
    glGenTextures(1, &skybox[id]);
    glBindTexture(GL_TEXTURE_2D, skybox[id]);
    png_t *png;

    if (!load_png(&png, fn)) {
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
render_skybox(GLuint id)
{
    float x = 0, y = 0, z = 0;
    float width = 30, height = 30, length = 30;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, skybox[id]);

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
    glBindTexture(GL_TEXTURE_2D, skybox[id]);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x,         y,         z + length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x,         y + height, z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z + length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y,         z + length);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, skybox[id]);
    glBegin(GL_QUADS);

    glTexCoord2f(1.0f, 0.0f); glVertex3f(x,         y,         z);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x,         y,         z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y,         z + length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y,         z);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, skybox[id]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y + height, z);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y + height, z + length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x,         y + height,   z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x,         y + height,   z);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, skybox[id]);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x,         y + height,   z);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x,         y + height,   z + length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x,         y,         z + length);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x,         y,         z);

    glEnd();
    glBindTexture(GL_TEXTURE_2D, skybox[id]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y,         z);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y,         z + length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height,   z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height,   z);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

