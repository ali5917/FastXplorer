#include "raylib.h"
#include "rlgl.h"
#include "../headers/settings.h"
#include "../headers/player.h"
#include "../headers/campus.h"
#include "../headers/introScreen.h"
#include "../headers/musicPlayer.h"
#include "../headers/schedule.h"
#include <iostream>
#include <string>
using namespace std;

class FastXplorerSystem {
public:
    enum GameState {
        INTRO_SCREEN,
        MAIN_GAME,
        PATH_SELECT,
        REGISTRATION,
        TIMETABLE_VIEW,
        MANUAL,
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
    MusicPlayer musicPlayer;
    Scheduler scheduler;
    Texture2D registrationBackground;
    Texture2D manualBg;
    string previousSlotBuilding;
    
    // simple notification
    bool hasNotification = false;
    string notificationText;
    float notificationTimer = 0.0f;

    void showNotification(const string& msg, float duration = 2.5f) {
        hasNotification = true;
        notificationText = msg;
        notificationTimer = duration;
    }

    void updateNotification(float dt) {
        if (!hasNotification) return;
        notificationTimer -= dt;
        if (notificationTimer <= 0.0f) {
            hasNotification = false;
            notificationText.clear();
        }
    }

    void drawNotification() {
        if (!hasNotification) return;
        int pad = 12;
        int fontSize = 20;
        int textWidth = MeasureText(notificationText.c_str(), fontSize);
        int boxW = textWidth + pad * 2;
        int boxH = fontSize + pad * 2;
        int x = WINDOW_WIDTH - boxW - 20;
        int y = 20;
        DrawRectangleRounded({(float)x, (float)y, (float)boxW, (float)boxH}, 0.2f, 8, Fade(BLACK, 0.65f));
        DrawText(notificationText.c_str(), x + pad, y + pad, fontSize, RAYWHITE);
    }

    void refreshWalkablesForCurrentSlot() {
        // ALL OF THIS TO PREVENT TRAPPING INSIDE THE BUILDING 
        vector<string> newWalkables;
        newWalkables.push_back("One Stop");
        
        TimeSlot* slot = scheduler.getCurrentSlotInfo();
        string currentBuilding = (slot && slot->course) ? slot->building : "";
        
        // Keep previous building walkable only if it's different from current
        if (!previousSlotBuilding.empty() && previousSlotBuilding != currentBuilding) {
            newWalkables.push_back(previousSlotBuilding);
        }
        
        if (!currentBuilding.empty()) {
            newWalkables.push_back(currentBuilding);
            showNotification("Now walkable: " + currentBuilding);
        }
        
        // Now replace all at once
        campus.clearWalkableBuildings();
        for (const auto& name : newWalkables) {
            campus.addWalkableBuilding(name);
        }
    }

    void loadMusicTracks() {
        // hardcoded tracks smh, I couln't find dynamic way to list files
        // pair syntax filename, title
        vector<pair<string, string>> tracks = {
            {"Asmoo.mp3", "Asmoo"},
            {"ComfortablePath.mp3", "Comfortable Path"},
            {"Destiny.mp3", "Destiny"},
            {"FiyyaHubbun.mp3", "Fiyya Hubbun"},
            {"Frozen.mp3", "Frozen"},
            {"LostInDreams.mp3", "Lost In Dreams"},
            {"OnMyWay.mp3", "On My Way"}
        };
        
        for (const auto& track : tracks) {
            string filePath = "assets/music/" + track.first;
            Music music = LoadMusicStream(filePath.c_str());
            if (IsMusicValid(music)) {
                float duration = GetMusicTimeLength(music);
                musicPlayer.addSong(track.second, duration, music);
            } else {
                TraceLog(LOG_WARNING, "Failed to load music: %s", filePath.c_str());
            }
        }
    }
    
public:
    FastXplorerSystem() : state(INTRO_SCREEN), player(nullptr), selectedStartBuilding(-1), selectedEndBuilding(-1), currentDestCell({-1, -1}), previousSlotBuilding("") {
        // font - Montserrat everywhere
        gameFont = LoadFontEx("assets/Montserrat-SemiBold.ttf", FONT_SIZE, NULL, 0);
        if (gameFont.texture.id == 0) {
            gameFont = GetFontDefault();
            TraceLog(LOG_WARNING, "Failed to load Montserrat font");
        }
        
        player = new Player({16, 20}, PLAYER_SPEED);
        campus.initializeGrid();

        loadMusicTracks();
        
        // registration background
        registrationBackground = LoadTexture("assets/Registration Desk.png");
        if (registrationBackground.id == 0) {
            TraceLog(LOG_WARNING, "Failed to load registration background");
        }
        
        // make One Stop walkable for registration
        campus.addWalkableBuilding("One Stop");

        if (!scheduler.initialize("assets/courses.txt")) {
            TraceLog(LOG_WARNING, "Failed to initialize scheduler");
        }
        
        scheduler.loadFromCSV("assets/schedule_save.csv");
        
        // make current slot building walkable if year is registered
        if (scheduler.getSelectedYear() > 0) {
            refreshWalkablesForCurrentSlot();
            showNotification("Schedule loaded: Year " + to_string(scheduler.getSelectedYear()));
        }

        manualBg = LoadTexture("assets/manual.png");
        
    }

    ~FastXplorerSystem() {
        delete player;
        if (gameFont.texture.id != GetFontDefault().texture.id) {
            UnloadFont(gameFont);
        }
        if (registrationBackground.id != 0) {
            UnloadTexture(registrationBackground);
        }
        if (manualBg.id != 0) {
            UnloadTexture(manualBg);
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
        updateNotification(dt);
        
        if(state == INTRO_SCREEN) {
            if (IsKeyPressed(KEY_F)) {
                state = MAIN_GAME;
            }
        } else if (state == MAIN_GAME) {
            // collision detection
            player->update(dt, &campus);

            // check if player has exited the previous slot building (& the adjacent nodes asw to avoid trapping)
            Vector2 playerCell = player->getGridPos();
            if (!previousSlotBuilding.empty()) {
                // Check if current slot is in the same building as previous
                TimeSlot* currentSlot = scheduler.getCurrentSlotInfo();
                string currentBuilding = (currentSlot && currentSlot->course) ? currentSlot->building : "";
                bool sameBuilding = (previousSlotBuilding == currentBuilding);
                
                bool stillNearPrevious = false;
                for (int i = 0; i < campus.getBuildingCount(); i++) {
                    Building* b = campus.getBuilding(i);
                    if (b && b->getName() == previousSlotBuilding) {
                        Vector2 topLeft = b->getGridTopLeft();
                        Vector2 bottomRight = b->getGridBottomRight();
                        
                        // checking if the player is inside or adjacent to building (1 cell)
                        if (playerCell.x >= topLeft.x - 1 && playerCell.x <= bottomRight.x + 1 &&
                            playerCell.y >= topLeft.y - 1 && playerCell.y <= bottomRight.y + 1) {
                            stillNearPrevious = true;
                            break;
                        }
                    }
                }
                
                // If player is far from building (not inside or adjacent) AND current slot is in different building, remove walkability
                if (!stillNearPrevious && !sameBuilding) {
                    campus.removeWalkableBuilding(previousSlotBuilding);
                    showNotification("Exited " + previousSlotBuilding);
                    previousSlotBuilding.clear();
                }
            }

            // check if player is at One Stop
            Building* oneStop = nullptr;
            for (int i = 0; i < campus.getBuildingCount(); i++) {
                Building* b = campus.getBuilding(i);
                if (b && b->getName() == "One Stop" && b->contains(playerCell)) {
                    oneStop = b;
                    break;
                }
            }
            
            if (oneStop && IsKeyPressed(KEY_ENTER)) {
                state = REGISTRATION;
            }

            // check if player is at current slot's building for attendance
            if (scheduler.getSelectedYear() > 0) {
                TimeSlot* currentSlot = scheduler.getCurrentSlotInfo();
                if (currentSlot && currentSlot->course) {
                    
                    Building* slotBuilding = nullptr;
                    for (int i = 0; i < campus.getBuildingCount(); i++) {
                        Building* b = campus.getBuilding(i);
                        if (b && b->getName() == currentSlot->building && b->contains(playerCell)) {
                            slotBuilding = b;
                            break;
                        }
                    }
                    
                    // mark attendance it pressed K
                    if (slotBuilding && IsKeyPressed(KEY_K)) {
                        // store current as previous
                        previousSlotBuilding = currentSlot->building;
                        
                        scheduler.advanceSlot();
                        scheduler.saveToCSV("assets/schedule_save.csv");
                        refreshWalkablesForCurrentSlot();
                        
                        TimeSlot* nextSlot = scheduler.getCurrentSlotInfo();
                        if (nextSlot && nextSlot->course) {
                            showNotification("Attendance marked! Next: " + nextSlot->building);
                        } else {
                            showNotification("All classes completed!");
                            previousSlotBuilding.clear();
                        }
                    }
                }
            }

            // clear the path and target when reached destination
            if (selectedEndBuilding >= 0 && currentDestCell.x >= 0 && currentDestCell.y >= 0) {
                if ((int)playerCell.x == (int)currentDestCell.x && (int)playerCell.y == (int)currentDestCell.y) {
                    campus.clearPath();
                    selectedEndBuilding = -1;
                    currentDestCell = {-1, -1};
                }
            }
            
            // path selection mode
            if (IsKeyPressed(KEY_Q)) {
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

            // music controls
            if (IsKeyPressed(KEY_P)) {
                musicPlayer.prev();
            }
            if (IsKeyPressed(KEY_N)) {
                musicPlayer.next();
            }
            if (IsKeyPressed(KEY_SPACE)) {
                if (musicPlayer.getIsPlaying()) {
                    musicPlayer.pause();
                } else {
                    musicPlayer.play();
                }
            }

            musicPlayer.update();
            
            // open timetable view
            if (scheduler.getSelectedYear() > 0 && IsKeyPressed(KEY_T)) {
                state = TIMETABLE_VIEW;
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mouse = GetMousePosition();

                int cellX = mouse.x / CELL_WIDTH;
                int cellY = mouse.y / CELL_HEIGHT;

                if ((cellY == 20 || cellY == 21) && cellX == 0) {
                    state = MANUAL;
                }
            }
        } else if (state == MANUAL) {
            if (IsKeyPressed(KEY_F)) {
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
            
        } else if (state == REGISTRATION) {
            
            int yearSelected = -1;
            if (IsKeyPressed(KEY_ONE)) yearSelected = 1;
            else if (IsKeyPressed(KEY_TWO)) yearSelected = 2;
            else if (IsKeyPressed(KEY_THREE)) yearSelected = 3;
            else if (IsKeyPressed(KEY_FOUR)) yearSelected = 4;
            
            if (yearSelected > 0) {
                // Register the year and assign courses
                if (scheduler.registerYear(yearSelected)) {
                    scheduler.saveToCSV("assets/schedule_save.csv");
                    refreshWalkablesForCurrentSlot();
                    showNotification("Registered Year " + to_string(yearSelected));
                } else {
                    showNotification("Registration failed");
                }
                state = MAIN_GAME;
            }
            
            if (IsKeyPressed(KEY_R)) {
                state = MAIN_GAME;
            }
        }
        else if (state == TIMETABLE_VIEW) {
            // regenerate schedule
            if (IsKeyPressed(KEY_G)) {
                scheduler.regenerateSchedule();
                scheduler.saveToCSV("assets/schedule_save.csv");
                refreshWalkablesForCurrentSlot();
                showNotification("Timetable regenerated");
            }
            // close timetable
            if (IsKeyPressed(KEY_T) || IsKeyPressed(KEY_ESCAPE)) {
                state = MAIN_GAME;
            }
        }
        
    }

    void draw() {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        if (state == MAIN_GAME) {
            campus.draw();
            campus.drawPath();
            musicPlayer.draw();
            player->draw();
            
            // Draw current slot
            if (scheduler.getSelectedYear() > 0) {
                TimeSlot* slot = scheduler.getCurrentSlotInfo();
                if (slot && slot->course) {
                    string hudTitle = "Slot " + to_string(scheduler.getCurrentSlot()) + " (Year " + to_string(scheduler.getSelectedYear()) + ")";
                    string hudLine1 = slot->course->name;
                    string hudLine2 = slot->building + " @ " + to_string(slot->startHour) + ":00";
                    int fontSize = 20;
                    int width = MeasureText(hudTitle.c_str(), fontSize);
                    width = max(width, MeasureText(hudLine1.c_str(), fontSize));
                    width = max(width, MeasureText(hudLine2.c_str(), fontSize));
                    width += 24;
                    int height = fontSize * 3 + 28;
                    int x = WINDOW_WIDTH - width - 20;
                    int y = 80;
                    DrawRectangleRounded({(float)x, (float)y, (float)width, (float)height}, 0.2f, 8, Fade(BLACK, 0.55f));
                    DrawText(hudTitle.c_str(), x + 12, y + 8, fontSize, RAYWHITE);
                    DrawText(hudLine1.c_str(), x + 12, y + 8 + fontSize, fontSize, LIGHTGRAY);
                    DrawText(hudLine2.c_str(), x + 12, y + 8 + fontSize * 2, fontSize, SKYBLUE);
                }
            }
            
            // Show "Press K to mark attendance" prompt when in correct building
            if (scheduler.getSelectedYear() > 0) {
                TimeSlot* slot = scheduler.getCurrentSlotInfo();
                if (slot && slot->course) {
                    Vector2 playerCell = player->getGridPos();
                    Building* slotBuilding = nullptr;
                    for (int i = 0; i < campus.getBuildingCount(); i++) {
                        Building* b = campus.getBuilding(i);
                        if (b && b->getName() == slot->building && b->contains(playerCell)) {
                            slotBuilding = b;
                            break;
                        }
                    }
                    
                    if (slotBuilding) {
                        const char* prompt = "Press K to mark attendance";
                        int promptWidth = MeasureText(prompt, 28);
                        int promptX = WINDOW_WIDTH / 2 - promptWidth / 2;
                        int promptY = WINDOW_HEIGHT - 100;
                        DrawRectangleRounded({(float)(promptX - 20), (float)(promptY - 10), (float)(promptWidth + 40), 50.0f}, 0.2f, 8, Fade(GREEN, 0.8f));
                        DrawText(prompt, promptX, promptY, 28, WHITE);
                    }
                }
            }
            drawNotification();
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
            const char* title = "SELECT DESTINATION FOR SHORTEST PATH";
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
            drawNotification();
        } else if (state == TIMETABLE_VIEW) {
            scheduler.draw(gameFont);
            drawNotification();
        } else if (state == REGISTRATION) {
            // Draw registration background
            if (registrationBackground.id != 0) {
                DrawTexture(registrationBackground, 0, 0, WHITE);
            } else {
                ClearBackground(RAYWHITE);
            }
            drawNotification();
        } else if (state == INTRO_SCREEN) {
            introScreen.draw();
        } else if (state == MANUAL) {
            if (manualBg.id != 0) {
                DrawTexture(manualBg, 0, 0, WHITE);
            } else {
                ClearBackground(RAYWHITE);
            }
        }
        
        EndDrawing();
    }
};

int main() {
    // Window setup: create windowed first, then toggle fullscreen for safer viewport sizing
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "FastXplorer - Campus Navigation");
    ToggleFullscreen();

    // Force viewport to reported render size and warn on mismatch
    int renderW = GetRenderWidth();
    int renderH = GetRenderHeight();
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    rlViewport(0, 0, renderW, renderH);
    if (renderW != screenW || renderH != screenH) {
        TraceLog(LOG_WARNING, "Render size (%d x %d) != screen size (%d x %d). GPU/driver may be scaling.", renderW, renderH, screenW, screenH);
    }

    // Initialize audio device for music playback
    InitAudioDevice();

    SetTargetFPS(60);
    
    FastXplorerSystem game;
    game.runGame();

    CloseAudioDevice();
    return 0;
}