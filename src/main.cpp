#include <raylib.h>
#include "rlgl.h"
#include "../headers/settings.h"
#include "../headers/player.h"
#include "../headers/campus.h"
#include "../headers/introScreen.h"
#include <string>
#include <bits/stdc++.h>
using namespace std;

class FastXplorerSystem {
public:
    enum GameState {
        INTRO_SCREEN,
        MAIN_GAME,
        PAUSED,
        PATH_SELECT,
        GAME_OVER
    };

private:
    GameState state;
    Player* player;
    IntroScreen introScreen;
    Campus campus;
    Font gameFont;
    int selectedStartBuilding;
    int selectedEndBuilding;
    Vector2 currentDestCell;
    
public:
    FastXplorerSystem() : state(INTRO_SCREEN), player(nullptr), selectedStartBuilding(-1), selectedEndBuilding(-1), currentDestCell({-1, -1}) {
        // font
        gameFont = LoadFontEx("C:/raylib/raylib/examples/text/resources/fonts/pixantiqua.ttf", FONT_SIZE, NULL, 0);
        if (gameFont.texture.id == 0) {
            gameFont = GetFontDefault();
        }
        
        player = new Player({16, 20}, PLAYER_SPEED);
        campus.initializeGrid();
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
            // collision detection
            player->update(dt, &campus);

            // clear the path and target when reached destination
            if (selectedEndBuilding >= 0 && currentDestCell.x >= 0 && currentDestCell.y >= 0) {
                Vector2 playerCell = player->getGridPos();
                if ((int)playerCell.x == (int)currentDestCell.x && (int)playerCell.y == (int)currentDestCell.y) {
                    campus.clearPath();
                    selectedEndBuilding = -1;
                    currentDestCell = {-1, -1};
                }
            }
            
            // path selection mode
            if (IsKeyPressed(KEY_S)) {
                state = PATH_SELECT;
                selectedStartBuilding = -1;
                selectedEndBuilding = -1;
            }

            // clear path when not needed
            if (IsKeyPressed(KEY_R)) {
                campus.clearPath();
                selectedEndBuilding = -1;
                currentDestCell = {-1, -1};
            }

            // find best path to selected building upon every update
            if (selectedEndBuilding >= 0) {
                Vector2 startPos = player->getGridPos();
                Building* end = campus.getBuilding(selectedEndBuilding);
                if (end) {
                    Vector2 endPos = end->getGridTopLeft();
                    bool foundEnd = false;
                    float minDistance = 999999.0f;

                    Vector2 topLeft = end->getGridTopLeft();
                    Vector2 bottomRight = end->getGridBottomRight();

                    auto considerCandidate = [&](Vector2 candidate) {
                        if (!campus.canWalkOn(candidate)) return;
                        float dx = candidate.x - startPos.x;
                        float dy = candidate.y - startPos.y;
                        float dist = dx * dx + dy * dy;
                        if (dist < minDistance) {
                            endPos = candidate;
                            minDistance = dist;
                            foundEnd = true;
                        }
                    };

                    for (int col = (int)topLeft.x - 1; col <= (int)bottomRight.x + 1; col++) {
                        considerCandidate({(float)col, topLeft.y - 1});
                    }
                    for (int col = (int)topLeft.x - 1; col <= (int)bottomRight.x + 1; col++) {
                        considerCandidate({(float)col, bottomRight.y + 1});
                    }
                    for (int row = (int)topLeft.y - 1; row <= (int)bottomRight.y + 1; row++) {
                        considerCandidate({topLeft.x - 1, (float)row});
                    }
                    for (int row = (int)topLeft.y - 1; row <= (int)bottomRight.y + 1; row++) {
                        considerCandidate({bottomRight.x + 1, (float)row});
                    }

                    if (foundEnd) {
                        campus.setCustomPath(startPos, endPos);
                        currentDestCell = endPos;
                    } else {
                        campus.clearPath();
                        currentDestCell = {-1, -1};
                    }
                } else {
                    campus.clearPath();
                    selectedEndBuilding = -1;
                    currentDestCell = {-1, -1};
                }
            }
            // pause the game here
            if (IsKeyPressed(KEY_P)) {
                state = PAUSED;
            }
            
        } else if (state == PAUSED) {
            // Unpause
            if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_SPACE)) {
                state = MAIN_GAME;
            }
        } else if (state == PATH_SELECT) {
            // Handle mouse clicks for destination building selection
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mousePos = GetMousePosition();
                Vector2 gridPos = Player::pixelToGrid(mousePos);
                
                // Find which building was clicked (skip large block buildings)
                int buildingIndex = -1;
                for (int i = 0; i < campus.getBuildingCount(); i++) {
                    Building* b = campus.getBuilding(i);
                    if (b && b->contains(gridPos)) {
                        // Skip large block buildings - only select buildings inside them
                        const string& name = b->getName();
                        if (name != "Multipurpose Building" && 
                            name != "Academic Block 1" && 
                            name != "Academic Block 2") {
                            buildingIndex = i;
                            break;
                        }
                    }
                }
                
                if (buildingIndex >= 0) {
                    selectedEndBuilding = buildingIndex;
                }
            }

            // Clear any existing path with R while in selection
            if (IsKeyPressed(KEY_R)) {
                campus.clearPath();
                selectedEndBuilding = -1;
                currentDestCell = {-1, -1};
            }
            
            // If destination building selected, calculate path from player position and return to main game
            if (selectedEndBuilding >= 0) {
                Vector2 startPos = player->getGridPos();
                Building* end = campus.getBuilding(selectedEndBuilding);
                
                if (end) {
                    // Find nearest walkable cell adjacent to destination building
                    Vector2 endPos = end->getGridTopLeft();
                    bool foundEnd = false;
                    float minDistance = 999999.0f;
                    
                    Vector2 topLeft = end->getGridTopLeft();
                    Vector2 bottomRight = end->getGridBottomRight();
                    
                    // Check all adjacent cells around the building and find the nearest one to player
                    // Top edge
                    for (int col = (int)topLeft.x - 1; col <= (int)bottomRight.x + 1; col++) {
                        Vector2 candidate = {(float)col, topLeft.y - 1};
                        if (campus.canWalkOn(candidate)) {
                            float dx = candidate.x - startPos.x;
                            float dy = candidate.y - startPos.y;
                            float dist = dx * dx + dy * dy; // squared distance
                            if (dist < minDistance) {
                                endPos = candidate;
                                minDistance = dist;
                                foundEnd = true;
                            }
                        }
                    }
                    
                    // Bottom edge
                    for (int col = (int)topLeft.x - 1; col <= (int)bottomRight.x + 1; col++) {
                        Vector2 candidate = {(float)col, bottomRight.y + 1};
                        if (campus.canWalkOn(candidate)) {
                            float dx = candidate.x - startPos.x;
                            float dy = candidate.y - startPos.y;
                            float dist = dx * dx + dy * dy;
                            if (dist < minDistance) {
                                endPos = candidate;
                                minDistance = dist;
                                foundEnd = true;
                            }
                        }
                    }
                    
                    // Left edge
                    for (int row = (int)topLeft.y - 1; row <= (int)bottomRight.y + 1; row++) {
                        Vector2 candidate = {topLeft.x - 1, (float)row};
                        if (campus.canWalkOn(candidate)) {
                            float dx = candidate.x - startPos.x;
                            float dy = candidate.y - startPos.y;
                            float dist = dx * dx + dy * dy;
                            if (dist < minDistance) {
                                endPos = candidate;
                                minDistance = dist;
                                foundEnd = true;
                            }
                        }
                    }
                    
                    // Right edge
                    for (int row = (int)topLeft.y - 1; row <= (int)bottomRight.y + 1; row++) {
                        Vector2 candidate = {bottomRight.x + 1, (float)row};
                        if (campus.canWalkOn(candidate)) {
                            float dx = candidate.x - startPos.x;
                            float dy = candidate.y - startPos.y;
                            float dist = dx * dx + dy * dy;
                            if (dist < minDistance) {
                                endPos = candidate;
                                minDistance = dist;
                                foundEnd = true;
                            }
                        }
                    }
                    
                    // Set the path in campus and return to main game (only if valid end point found)
                    if (foundEnd) {
                        campus.setCustomPath(startPos, endPos);
                        currentDestCell = endPos;
                    }
                    state = MAIN_GAME;
                }
            }
            
            // Cancel path selection with R
            if (IsKeyPressed(KEY_R)) {
                state = MAIN_GAME;
                selectedStartBuilding = -1;
                selectedEndBuilding = -1;
            }
        }
        
    }

    void draw() {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        if (state == MAIN_GAME) {
            campus.draw();
            campus.drawPath(); // draw current path if any
            player->draw();
        } else if (state == PAUSED) {
            // draw paused state
            DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.5f));
            const char* pauseText = "PAUSED";
            int textWidth = MeasureText(pauseText, 60);
            DrawText(pauseText, WINDOW_WIDTH/2 - textWidth/2, WINDOW_HEIGHT/2 - 30, 60, WHITE);
            const char* resumeText = "Press P to resume";
            int resumeWidth = MeasureText(resumeText, 20);
            DrawText(resumeText, WINDOW_WIDTH/2 - resumeWidth/2, WINDOW_HEIGHT/2 + 40, 20, LIGHTGRAY);
        } else if (state == PATH_SELECT) {
            campus.draw();
            // check hoverrrr so cool lol
            Vector2 mousePos = GetMousePosition();
            Vector2 gridPos = Player::pixelToGrid(mousePos);
            int hoveredBuilding = -1;
            for (int i = 0; i < campus.getBuildingCount(); i++) {
                Building* b = campus.getBuilding(i);
                if (b) {
                    const string& name = b->getName();
                    // Skip large block buildings cus u can select buildings inside them
                    if (name == "Multipurpose Building" || 
                        name == "Academic Block 1" || 
                        name == "Academic Block 2") {
                        continue;
                    }
                    
                    Vector2 topLeft = b->getGridTopLeft();
                    Vector2 bottomRight = b->getGridBottomRight();
                    
                    // Convert to pixel coordinates
                    float pixelX = topLeft.x * CELL_WIDTH;
                    float pixelY = topLeft.y * CELL_HEIGHT;
                    float pixelWidth = (bottomRight.x - topLeft.x + 1) * CELL_WIDTH;
                    float pixelHeight = (bottomRight.y - topLeft.y + 1) * CELL_HEIGHT;
                    
                    // check if mouse is hovering over this building
                    bool isHovered = b->contains(gridPos);
                    if (isHovered) {
                        hoveredBuilding = i;
                    }
                    
                    // Draw building rectangle with highlight if hovered
                    Color buildingColor = isHovered ? Fade(YELLOW, 0.6f) : Fade(GREEN, 0.4f);
                    DrawRectangle(pixelX, pixelY, pixelWidth, pixelHeight, buildingColor);
                    DrawRectangleLines(pixelX, pixelY, pixelWidth, pixelHeight, isHovered ? YELLOW : GREEN);
                }
            }
            
            // instructions 
            const char* title = "SELECT SHORTEST PATH";
            int titleWidth = MeasureText(title, 40);
            DrawText(title, WINDOW_WIDTH/2 - titleWidth/2, 50, 40, DARKGREEN);
            
            if (selectedEndBuilding < 0) {
                const char* instruction = "Click a building to select DESTINATION";
                int instrWidth = MeasureText(instruction, 24);
                DrawText(instruction, WINDOW_WIDTH/2 - instrWidth/2, 120, 24, GREEN);
            }
            
            const char* cancelText = "Press R to cancel";
            int cancelWidth = MeasureText(cancelText, 16);
            DrawText(cancelText, WINDOW_WIDTH/2 - cancelWidth/2, WINDOW_HEIGHT - 40, 16, LIGHTGRAY);
        } else if (state == INTRO_SCREEN) {
            introScreen.draw();
        }
        
        EndDrawing();
    }
    

};

int main()
{
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "FastXplorer - Campus Navigation");

    ToggleFullscreen();   // safer than startup fullscreen
    SetTargetFPS(60);

    int renderW = GetRenderWidth();
    int renderH = GetRenderHeight();
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    // Force sync without rlViewport
    SetWindowSize(renderW, renderH);

    if (renderW != screenW || renderH != screenH)
    {
        TraceLog(LOG_WARNING,
            "Render size (%d x %d) != screen size (%d x %d). GPU/driver scaling active.",
            renderW, renderH, screenW, screenH);
    }

    FastXplorerSystem game;
    game.runGame();

    return 0;
}

