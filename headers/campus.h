#ifndef CAMPUS_H
#define CAMPUS_H

#include <raylib.h>
#include <vector>
#include <queue>
#include <string>
#include "settings.h"
using namespace std;

class Building {
private:
    string name;
    Vector2 gridTopLeft;
    Vector2 gridBottomRight;
    string description;
    bool isVisited;
    int visitOrder;
    
public:
    Building(const string& n, Vector2 topLeft, Vector2 bottomRight, 
             const string& desc = "")
        : name(n), gridTopLeft(topLeft), gridBottomRight(bottomRight),
          description(desc), isVisited(false), visitOrder(-1) {}
    
    const string& getName() const { return name; }
    const string& getDescription() const { return description; }
    Vector2 getGridTopLeft() const { return gridTopLeft; }
    Vector2 getGridBottomRight() const { return gridBottomRight; }
    Vector2 getGridCenter() const {
        return {(gridTopLeft.x + gridBottomRight.x) / 2.0f,
                (gridTopLeft.y + gridBottomRight.y) / 2.0f};
    }
    Vector2 getGridBottomLeft() const { return {gridTopLeft.x, gridBottomRight.y}; }
    Vector2 getGridTopRight() const { return {gridBottomRight.x, gridTopLeft.y}; }
    bool getVisited() const { return isVisited; }
    int getVisitOrder() const { return visitOrder; }
    
    void setVisited(bool visited, int order = -1) { 
        isVisited = visited; 
        visitOrder = order;
    }
    
    bool contains(Vector2 gridPos) const {
        return gridPos.x >= gridTopLeft.x && 
               gridPos.x <= gridBottomRight.x &&
               gridPos.y >= gridTopLeft.y && 
               gridPos.y <= gridBottomRight.y;
    }
    
    Vector2 getSize() const {
        return {gridBottomRight.x - gridTopLeft.x,
                gridBottomRight.y - gridTopLeft.y};
    }
};

class Campus {
private:
    int grid[GRID_ROWS][GRID_COLS];
    vector<Building> buildings;
    vector<Vector2> currentPath;
    Texture2D campusBackground;
    bool isBackgroundLoaded;
    vector<string> walkableBuildingNames;
    
    struct Node {
        Vector2 pos;
        int dist;
        Vector2 parent;
    };
    
    bool isValid(int row, int col) const;
    bool isWalkable(int row, int col) const;

public:
    Campus();
    ~Campus();
    
    void initializeGrid();
    void setObstacle(int row, int col, bool isObstacle);
    bool canWalkOn(Vector2 gridPos) const;

    void addBuilding(const Building& building);
    Building* getBuilding(int index);
    int getBuildingCount() const { return buildings.size(); }
    const vector<Building>& getBuildings() const { return buildings; }

    vector<Vector2> findPath(Vector2 start, Vector2 dest);
    void setCustomPath(Vector2 start, Vector2 dest);
    void clearPath();
    const vector<Vector2>& getCurrentPath() const { return currentPath; }

    void addWalkableBuilding(const string& buildingName);
    void removeWalkableBuilding(const string& buildingName);
    void clearWalkableBuildings();

    void draw() const;
    void drawPath() const;
};

#endif
