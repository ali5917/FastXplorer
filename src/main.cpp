#include <raylib.h>
#include "ball.h"
#include "../headers/settings.h"

int main()
{
    const Color darkGreen = {20, 160, 133, 255};

    Ball ball;

    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "FastXplorer");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        ball.Update();

        BeginDrawing();
            ClearBackground(darkGreen);
            ball.Draw();
        EndDrawing();
    }

    CloseWindow();
}
