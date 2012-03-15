#include <stdlib.h>

#include <agar/core.h>
#include <agar/gui.h>

#include "src/lib/client.h"
#include "src/lib/entity.h"
#include "src/lib/ui.h"
#include "src/lib/ui/gfx.h"
#include "src/lib/ui/input.h"

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
    int x, y;

    switch (ev->type) {
        case AG_DRIVER_MOUSE_BUTTON_DOWN:
            x = ev->data.button.x;
            y = ev->data.button.y;
            return 1;

        default:
            printf("%i %i\n", ev->data.motion.x, ev->data.motion.y);
            input->ui->gfx->eye.x = ev->data.motion.x;
            input->ui->gfx->eye.y = ev->data.motion.y;
            break;
    }

    return 0;
}

int
handle_keypress(input_t *input, AG_DriverEvent *ev)
{
    vec3f *acc = &input->ui->client->entity->acc;
    float thrust_amt = 0.1;

    switch (ev->type) {
        case AG_DRIVER_KEY_DOWN:
            switch (ev->data.key.ks) {
                case 27:
                    input->ui->client->quit = true;
                    return 1;

                // up
                case 'q':
                    if (input->ui->gfx->drv->kbd->modState == AG_KEYMOD_LMETA) {
                        input->ui->client->quit = true;
                        return 1;
                    }

                    acc->y += thrust_amt;
                    client_send(input->ui->client, P_ENTITY_UPDATE_REQUEST);
                    break;

                // down
                case 'z':
                    acc->y -= thrust_amt;
                    client_send(input->ui->client, P_ENTITY_UPDATE_REQUEST);
                    break;

                // right
                case 'd':
                    acc->x += thrust_amt;
                    client_send(input->ui->client, P_ENTITY_UPDATE_REQUEST);
                    break;

                // left
                case 'a':
                    acc->x -= thrust_amt;
                    client_send(input->ui->client, P_ENTITY_UPDATE_REQUEST);
                    break;

                // forward
                case 'w':
                    acc->z += thrust_amt;
                    client_send(input->ui->client, P_ENTITY_UPDATE_REQUEST);
                    break;

                // back
                case 's':
                    acc->z -= thrust_amt;
                    client_send(input->ui->client, P_ENTITY_UPDATE_REQUEST);
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }

    return 0;
}

