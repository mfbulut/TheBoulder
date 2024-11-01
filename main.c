#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <string.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define SLIDER_WIDTH 300
#define SLIDER_HEIGHT 25
#define BUTTON_ROUND 0.1f

Font font24x;
Font font48x;
Font font96x;
#define GAME_TITLE "THE BOULDER"
#define FONT_SIZE_TITLE 96
#define FONT_SIZE_NORMAL 48

#define F_BLACK  ((Color){ 26 , 20 , 23 , 255})
#define F_BROWN  ((Color){ 103, 65 , 58 , 255})
#define F_RED    ((Color){ 175, 75 , 59 , 255})
#define F_ORANGE ((Color){ 193, 133, 73 , 255})
#define F_WHITE  ((Color){ 217, 203, 174, 255})
#define F_GREEN  ((Color){ 141, 179, 94 , 255})
#define F_GRAY   ((Color){ 124, 131, 129, 255})
#define F_YELLOW ((Color){ 217, 203, 73 , 255})

#include "utils.h"
Settings settings;
#include "game.h"

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    InitAudioDevice();
    SetTargetFPS(60);

    #if defined(PLATFORM_WEB)
    font24x = LoadFontEx("bin/resources/Monocraft.ttf", 24, NULL, 0);
    font48x = LoadFontEx("bin/resources/Monocraft.ttf", 48, NULL, 0);
    font96x = LoadFontEx("bin/resources/Monocraft.ttf", 96, NULL, 0);
    #else
    font24x = LoadFontEx("resources/Monocraft.ttf", 24, NULL, 0);
    font48x = LoadFontEx("resources/Monocraft.ttf", 24, NULL, 0);
    font96x = LoadFontEx("resources/Monocraft.ttf", 96, NULL, 0);
    #endif

    settings = LoadSettings();

    typedef enum { MAIN_MENU, SETTINGS_MENU, GAME } MenuState;
    MenuState currentState = MAIN_MENU;

    InitGame();

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(F_BLACK);

        switch (currentState) {
            case MAIN_MENU: {
                // Draw centered title
                float titleWidth = MeasureTextCustom(GAME_TITLE, FONT_SIZE_TITLE);
                DrawTextCustom(GAME_TITLE, SCREEN_WIDTH/2 - titleWidth/2, 50, FONT_SIZE_TITLE, F_WHITE);

                if (GuiButton((Rectangle){ SCREEN_WIDTH/2 - 150, 200, 300, 80 }, "PLAY")) {
                    currentState = GAME;
                }

                if (GuiButton((Rectangle){ SCREEN_WIDTH/2 - 150, 300, 300, 80 }, "SETTINGS")) {
                    currentState = SETTINGS_MENU;
                }
            } break;

            case SETTINGS_MENU: {
                float titleWidth = MeasureTextCustom("SETTINGS", FONT_SIZE_TITLE);
                DrawTextCustom("SETTINGS", SCREEN_WIDTH/2 - titleWidth/2, 10, FONT_SIZE_TITLE, F_WHITE);

                Rectangle volumeSlider = { SCREEN_WIDTH/2 - SLIDER_WIDTH/2, 180, SLIDER_WIDTH, SLIDER_HEIGHT };
                Rectangle musicSlider  = { SCREEN_WIDTH/2 - SLIDER_WIDTH/2, 270, SLIDER_WIDTH, SLIDER_HEIGHT };

                settings.volume = DrawSlider(volumeSlider, settings.volume, "Volume");
                settings.music = DrawSlider(musicSlider, settings.music, "Music");

                if (GuiButton((Rectangle){ SCREEN_WIDTH/2 - 150, 350, 300, 80 }, "BACK")) {
                    currentState = MAIN_MENU;
                    SaveSettings(settings);
                }
            } break;

            case GAME: {
                UpdateGame();
            } break;


        }

        EndDrawing();
    }

    UnloadFont(font24x);
    UnloadFont(font48x);
    UnloadFont(font96x);

    CloseWindow();
}