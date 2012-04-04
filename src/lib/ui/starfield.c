#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#include "src/lib/client.h"
#include "src/lib/entity.h"
#include "src/lib/ui.h"
#include "src/lib/ui/gfx.h"
#include "src/lib/ui/starfield.h"

void
init_starfield(gfx_t *gfx)
{
for (int x = 0; x < STAR_GRID_SIZE; x++)
    for (int y = 0; y < STAR_GRID_SIZE; y++) {
        starfield.star_grid[x][y] = calloc(1, sizeof(vec3f));
        starfield.star_grid[x][y]->x = (float)(rand() & 0xFF) * 2;
        starfield.star_grid[x][y]->y = (float)(rand() & 0xFF) * 2;
    }
}

void
render_starfield(gfx_t *gfx, double dt)
{
    glPushMatrix();
    {
        glTranslatef(-256.0f, 256.0f, -256.0f);

        for (int x = 0; x < STAR_GRID_SIZE; x++) {
            for (int y = 0; y < STAR_GRID_SIZE; y++) {
                glPushMatrix();
                {
                    vec3f *loc = starfield.star_grid[x][y];
                    if ((rand() & 0xFFF) < 128) {
                        starfield.star_pos = 0;
                    }
                    starfield.star_pos += 0.1;

                    vec3f *vel = gfx->ui->client->entity->vel;

                    glTranslatef(loc->x, -loc->y, loc->z);

                    glColor3f(1.0f, 1.0f, 1.0f);

                    glPushMatrix();
                    {
                        glBegin(GL_TRIANGLES);
                        glVertex3f(0.0f, 0.0f, 0.0f);
                        glVertex3f(0.0f, 1.0f, 0.0f);
                        glVertex3f(1.0f, 1.0f, 0.0f);
                        glEnd();
                    }
                    glPopMatrix();

                    glBegin(GL_LINES);
                    glVertex3f(0.0f, 0.0f, 0.0f);
                    glVertex3f(
                           vel->x / 10// + gfx->star_pos
                        , -vel->y / 10// + gfx->star_pos
                        ,  vel->z / 10// + gfx->star_pos
                    );
                    glEnd();

                }
                glPopMatrix();

            }

        }

    }
    glPopMatrix();
}

