#define SDL_MAIN_HANDLED
#include <stdio.h>
#include "app.h"

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    printf("Program started.\n");

    App app;

    if (!init_app(&app)) {
        printf("init_app failed.\n");
        getchar();
        return 1;
    }

    printf("App initialized.\n");

    run_app(&app);

    printf("App closed.\n");

    destroy_app(&app);

    getchar();
    return 0;
}