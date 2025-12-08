#include "../headers/campus.h"
#include <algorithm>
#include <cstring>
#include <queue>
#include <vector>

Campus::Campus() : isBackgroundLoaded(false) {
    initializeGrid();

    campusBackground = LoadTexture("assets/top-view.png");
    isBackgroundLoaded = (campusBackground.id != 0);

    addBuilding(Building("Sports Room", {0, 4}, {1, 5}, "Indoor sports"));

    addBuilding(Building("Multipurpose Building", {3, 4}, {6, 13}, "Multi-purpose"));
    addBuilding(Building("Library", {3, 4}, {5, 5}, "Study area"));
    addBuilding(Building("Cafeteria", {4, 7}, {6, 8}, "Food court"));
    addBuilding(Building("Auditorium", {3, 10}, {6, 13}, "Assembly hall"));

    addBuilding(Building("Badar Dhaba", {8, 0}, {10, 4}, "Dining"));
    addBuilding(Building("Masjid", {13, 0}, {14, 3}, "Prayer room"));
    addBuilding(Building("EE Cafeteria", {17, 0}, {20, 1}, "EE dining"));
    addBuilding(Building("Lab 3", {17, 2}, {18, 5}, "Computer lab"));

    addBuilding(Building("Academic Block 1", {9, 8}, {16, 15}, "Classes"));
    addBuilding(Building("Lab 1", {9, 8}, {10, 11}, "Lab inside AB1"));
    addBuilding(Building("R-1", {9, 13}, {11, 15}, "Classroom inside AB1"));
    addBuilding(Building("R-2", {14, 13}, {16, 15}, "Classroom inside AB1"));
    addBuilding(Building("R-3", {12, 8}, {13, 11}, "Classroom inside AB1"));
    addBuilding(Building("One Stop", {15, 8}, {16, 11}, "Registration desk"));

    addBuilding(Building("Academic Block 2", {20, 4}, {23, 19}, "Classes"));
    addBuilding(Building("R-4", {23, 9}, {23, 14}, "Classroom inside AB2"));
    addBuilding(Building("R-5", {20, 16}, {20, 19}, "Classroom inside AB2"));
    addBuilding(Building("R-6", {20, 12}, {21, 13}, "Classroom inside AB2"));
    addBuilding(Building("R-7", {20, 4}, {20, 7}, "Classroom inside AB2"));
    addBuilding(Building("Lab 4", {20, 9}, {21, 10}, "Lab inside AB2"));
    addBuilding(Building("Faculty Office", {22, 4}, {23, 7}, "Faculty inside AB2"));
    addBuilding(Building("Reading Hall", {22, 16}, {23, 19}, "Reading area inside AB2"));
}

Campus::~Campus() {
    if (isBackgroundLoaded) {
        UnloadTexture(campusBackground);
    }
}

void Campus::initializeGrid() {
    int campusLayout[22][24] = {
        {0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,0,0,0,1,0,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,0,0,0,1,0,1,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0},
        {1,1,0,1,1,1,1,0,1,1,1,0,0,0,0,0,0,1,1,0,1,1,1,1},
        {1,1,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,1},
        {0,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,1},
        {1,1,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,1},
        {1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1},
        {1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1},
        {1,1,0,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1},
        {1,1,0,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1},
        {0,0,0,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1},
        {0,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1},
        {1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1},
        {1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1},
        {1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,1,0,0,0,1,1,1,1,1},
        {1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,1},
        {0,0,0,0,0,1,0,0,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1},
        {0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0}
    };
    
    memcpy(grid, campusLayout, sizeof(campusLayout));
}

void Campus::setObstacle(int row, int col, bool isObstacle) {
    if (isValid(row, col)) {
        grid[row][col] = isObstacle ? 1 : 0;
    }
}

bool Campus::isValid(int row, int col) const {
    return row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS;
}

bool Campus::isWalkable(int row, int col) const {
    if (!isValid(row, col)) return false;
    
    // check if inside a walkable standalone building (special Lab 3 case)
    for (const auto& walkableName : walkableBuildingNames) {
        for (const auto& building : buildings) {
            if (building.getName() == walkableName && 
                building.contains({(float)col, (float)row})) {
                return true;
            }
        }
    }
    
    // check if the position is inside Academic Block 1,2 or Multipurpose Buildings but not other buildings
    for(const auto& building : buildings) {
        const std::string& name = building.getName();
        if ((name == "Academic Block 1" || name == "Academic Block 2" || name == "Multipurpose Building") &&
            building.contains({(float)col, (float)row})) { 
            // check if inside other buildings within these blocks like labs and classrooms
            bool insideSubBuilding = false;
            std::string subBuildingName = "";
            for (const auto& subBuilding : buildings) {
                if (subBuilding.getName() != name && 
                    subBuilding.contains({(float)col, (float)row})) {
                    insideSubBuilding = true;
                    subBuildingName = subBuilding.getName();
                    break;
                }
            }
            // If inside a sub-building, check if it's in the walkable list
            if (insideSubBuilding) {
                for (const auto& walkableName : walkableBuildingNames) {
                    if (subBuildingName == walkableName) {
                        return true;
                    }
                }
            }
            return !insideSubBuilding;
        }
    }
    
    // check the grid (0 = walkable path, 1 = wall ya obstacle)
    return grid[row][col] == 0;
}

bool Campus::canWalkOn(Vector2 gridPos) const {
    return isWalkable((int)gridPos.y, (int)gridPos.x);
}

void Campus::addBuilding(const Building& building) {
    buildings.push_back(building);
}

Building* Campus::getBuilding(int index) {
    if (index >= 0 && index < static_cast<int>(buildings.size())) {
        return &buildings[index];
    }
    return nullptr;
}

std::vector<Vector2> Campus::findPath(Vector2 start, Vector2 dest) {
    std::vector<Vector2> path;

    int startX = (int)start.x;
    int startY = (int)start.y;
    int destX = (int)dest.x;
    int destY = (int)dest.y;
    
    // check if start and dest are valid and walkable
    if (!isWalkable(startY, startX) || !isWalkable(destY, destX)) {
        return path; // returns empty path
    }
    
    
    bool visited[GRID_ROWS][GRID_COLS] = {false};
    
    Vector2 parent[GRID_ROWS][GRID_COLS];
    for (int i = 0; i < GRID_ROWS; i++) {
        for (int j = 0; j < GRID_COLS; j++) {
            parent[i][j] = {-1, -1};
        }
    }
    
    std::queue<Vector2> q;
    q.push(start);
    visited[startY][startX] = true;
    
    // direction vectors: right, left, down, up
    int dx[] = {1, -1, 0, 0};
    int dy[] = {0, 0, 1, -1};
    
    bool found = false;
    
    while (!q.empty() && !found) {
        Vector2 current = q.front();
        q.pop();
        
        int curX = (int)current.x;
        int curY = (int)current.y;
        
        if (curX == destX && curY == destY) {
            found = true;
            break;
        }
        
        // explore neighbors
        for (int i = 0; i < 4; i++) {
            int newX = curX + dx[i];
            int newY = curY + dy[i];
            
            if (isWalkable(newY, newX) && !visited[newY][newX]) {
                visited[newY][newX] = true;
                parent[newY][newX] = current;
                q.push({(float)newX, (float)newY});
            }
        }
    }
    
    // reconstruct path
    if (found) {
        Vector2 current = dest;
        while ((int)current.x != startX || (int)current.y != startY) {
            path.push_back(current);
            current = parent[(int)current.y][(int)current.x];
        }
        path.push_back(start);
        std::reverse(path.begin(), path.end());
    }
    
    return path;
}

void Campus::setCustomPath(Vector2 start, Vector2 dest) {
    currentPath = findPath(start, dest);
}

void Campus::clearPath() {
    currentPath.clear();
}

void Campus::addWalkableBuilding(const string& buildingName) {
    // Check if already in list
    for (const auto& name : walkableBuildingNames) {
        if (name == buildingName) return;
    }
    walkableBuildingNames.push_back(buildingName);
}

void Campus::removeWalkableBuilding(const string& buildingName) {
    walkableBuildingNames.erase(
        std::remove(walkableBuildingNames.begin(), walkableBuildingNames.end(), buildingName),
        walkableBuildingNames.end()
    );
}

void Campus::clearWalkableBuildings() {
    walkableBuildingNames.clear();
}

void Campus::draw() const {
    if (isBackgroundLoaded) {
        DrawTexture(campusBackground, 0, 0, WHITE);
    } else {
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, LIGHTGRAY);
    }
}

void Campus::drawPath() const {
    if (currentPath.empty()) return;

    for (size_t i = 0; i < currentPath.size() - 1; i++) {
        Vector2 start = {currentPath[i].x * CELL_WIDTH + CELL_WIDTH/2, 
                        currentPath[i].y * CELL_HEIGHT + CELL_HEIGHT/2};
        Vector2 end = {currentPath[i+1].x * CELL_WIDTH + CELL_WIDTH/2, 
                      currentPath[i+1].y * CELL_HEIGHT + CELL_HEIGHT/2};
        DrawLineEx(start, end, 3, YELLOW);
    }

    // for (const auto& cell : currentPath) {
    //     Rectangle pathCell = {cell.x * CELL_WIDTH, cell.y * CELL_HEIGHT, 
    //                          CELL_WIDTH, CELL_HEIGHT};
    //     DrawRectangleLinesEx(pathCell, 2, YELLOW);
    // }
    Vector2 cell = currentPath.front();
    Rectangle pathCell = {cell.x * CELL_WIDTH, cell.y * CELL_HEIGHT,CELL_WIDTH, CELL_HEIGHT};
    DrawRectangleLinesEx(pathCell, 2, YELLOW);
    cell = currentPath.back();
    pathCell = {cell.x * CELL_WIDTH, cell.y * CELL_HEIGHT,CELL_WIDTH, CELL_HEIGHT};
    DrawRectangleLinesEx(pathCell, 2,GREEN);
}
