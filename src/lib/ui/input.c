#include <stdlib.h>

#include <agar/core.h>
#include <agar/gui.h>

#include "src/lib/client.h"
#include "src/lib/entity.h"
#include "src/lib/ui.h"
#include "src/lib/ui/gfx.h"
#include "src/lib/ui/input.h"

/*
bool *keys = client->ui->input->keys;
float thrust_amt = 0.1;
vec3f *acc = &client->entity->acc;

if (keys['w'])
    acc->z += thrust_amt;

if (keys['s'])
    acc->z -= thrust_amt;

if (keys['a'])
    acc->x -= thrust_amt;

if (keys['d'])
    acc->x += thrust_amt;

if (keys['q'])
    acc->y += thrust_amt;

if (keys['z'])
    acc->y -= thrust_amt;

if (keys[' '])
    acc->x = acc->y = acc->z = 0.0;
*/

input_t *
init_input(ui_t *ui)
{
    input_t *input = calloc(1, sizeof(input_t));
    input->ui = ui;
    return input;
}

int
handle_mouse(input_t *input, AG_DriverEvent *ev)
{
    if (!input->ui || !input->ui->gfx)
        return 0;

    int x, y;
    struct ag_mouse *mouse = input->ui->gfx->drv->mouse;

    switch (ev->type) {
        case AG_DRIVER_MOUSE_BUTTON_DOWN:
            x = ev->data.button.x;
            y = ev->data.button.y;
            return 1;

        default:
            //if (mouse->x == 0)
            //    SDL_WarpMouse(input->ui->gfx->w, mouse->y);

            //if (mouse->y == 0)
            //    SDL_WarpMouse(mouse->x, input->ui->gfx->h);

            input->ui->gfx->tgt->x += mouse->xRel;
            input->ui->gfx->tgt->y += mouse->yRel;
            break;
    }

    return 0;
}

int
handle_keypress(input_t *input, AG_DriverEvent *ev)
{
    switch (ev->type) {
        case AG_DRIVER_KEY_DOWN:
            input->keys[ev->data.key.ks] = true;

            switch (ev->data.key.ks) {
                case 27:
                    input->ui->client->quit = true;
                    return 1;

                default:
                    break;
            }
            break;

        case AG_DRIVER_KEY_UP:
            input->keys[ev->data.key.ks] = false;
            break;

        default:
            break;
    }

    return 0;
}

