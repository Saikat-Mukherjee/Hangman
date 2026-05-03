/*
 * src/winmain.c — Windows WinMain entry point.
 *
 * SDL3 3.x no longer ships libSDL3main.a in the MinGW package, so we provide
 * the WinMain → main bridge ourselves.  This file is only compiled on Windows.
 */
#ifdef _WIN32

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <windows.h>
#include <stdlib.h>   /* __argc, __argv */

extern int main(int argc, char *argv[]);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nShowCmd)
{
    (void)hInstance; (void)hPrevInstance;
    (void)lpCmdLine; (void)nShowCmd;
    return main(__argc, __argv);
}

#endif /* _WIN32 */
