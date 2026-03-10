#define SDL_MAIN_HANDLED
#include "include/app.h"

int main(void)
{
    App app;

    if (!init_app(&app)) {
        return 1;
    }

    run_app(&app);
    destroy_app(&app);

    return 0;
}