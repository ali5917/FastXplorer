#include <raylib.h>
#include "../headers/settings.h"
#include "../headers/player.h"
#include "../headers/campus.h"
#include "../headers/introScreen.h"
#include <string>

class FastXplorerSystem {
public:
    enum GameState {
        INTRO_SCREEN,
        MAIN_GAME,
        PAUSED,
        GAME_OVER
    };

private:
    GameState state;
    Player* player;
    IntroScreen introScreen;
    Campus campus;
    Font gameFont;
    
public:
    FastXplorerSystem() : state(INTRO_SCREEN), player(nullptr) {
        // Load font
        gameFont = LoadFontEx("C:/raylib/raylib/examples/text/resources/fonts/pixantiqua.ttf", FONT_SIZE, NULL, 0);
        if (gameFont.texture.id == 0) {
            gameFont = GetFontDefault();
        }
        
        player = new Player({2, 2}, PLAYER_SPEED);
        campus.initializeGrid();
        campus.selectRandomStartAndDest();
    }

    ~FastXplorerSystem() {
        delete player;
        if (gameFont.texture.id != GetFontDefault().texture.id) {
            UnloadFont(gameFont);
        }
        CloseWindow();
    }

    void runGame() {
        while (!WindowShouldClose()) {
            update();
            draw();
        }
    }

    void update() {
        float dt = GetFrameTime();
        
        if(state == INTRO_SCREEN) {
            if (IsKeyPressed(KEY_F)) {
                state = MAIN_GAME;
            }
        } else if (state == MAIN_GAME) {
            // Player update with collision detection
            player->update(dt, &campus);
            
            // Generate new path if R is pressed
            if (IsKeyPressed(KEY_R)) {
                campus.selectRandomStartAndDest();
            }

            // Pause game
            if (IsKeyPressed(KEY_P)) {
                state = PAUSED;
            }
            
        } else if (state == PAUSED) {
            // Unpause
            if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_SPACE)) {
                state = MAIN_GAME;
            }
        }
        
    }

    void draw() {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        if (state == MAIN_GAME) {
            // Draw campus grid and buildings
            campus.draw();
            
            // Draw pathfinding visualization
            campus.drawPath();
            
            // Draw player
            player->draw();
            
            // Draw UI elements
            drawUI();
            
            
        } else if (state == PAUSED) {
            // Draw paused overlay
            DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.5f));
            const char* pauseText = "PAUSED";
            int textWidth = MeasureText(pauseText, 60);
            DrawText(pauseText, WINDOW_WIDTH/2 - textWidth/2, WINDOW_HEIGHT/2 - 30, 60, WHITE);
            
            const char* resumeText = "Press P to resume";
            int resumeWidth = MeasureText(resumeText, 20);
            DrawText(resumeText, WINDOW_WIDTH/2 - resumeWidth/2, WINDOW_HEIGHT/2 + 40, 20, LIGHTGRAY);
        } else if (state == INTRO_SCREEN) {
            introScreen.draw();
        }
        
        EndDrawing();
    }
    
    void drawUI() {
        
        // Building navigation info
        Building* start = campus.getStartBuilding();
        Building* dest = campus.getDestBuilding();
        
        if (start && dest) {
            DrawText("Navigation:", 20, 100, 20, DARKGREEN);
            const char* startText = TextFormat("From: %s", start->getName().c_str());
            const char* destText = TextFormat("To: %s", dest->getName().c_str());
            DrawText(startText, 20, 125, 18, DARKGRAY);
            DrawText(destText, 20, 145, 18, DARKGRAY);
        }
        
    }
};

int main() {
    // Set fullscreen mode (from CapTale)
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "FastXplorer - Campus Navigation");
    SetTargetFPS(60);

    FastXplorerSystem game;
    game.runGame();

    return 0;
}
