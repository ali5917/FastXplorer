#ifndef CAMPUS_H
#define CAMPUS_H

#include <raylib.h>
#include <vector>
#include <queue>
#include <string>
#include "settings.h"

class Building {
private:
    std::string name;
    Vector2 gridTopLeft;
    Vector2 gridBottomRight;
    std::string description;
    bool isVisited;
    int visitOrder;
    
public:
    Building(const std::string& n, Vector2 topLeft, Vector2 bottomRight, 
             const std::string& desc = "")
        : name(n), gridTopLeft(topLeft), gridBottomRight(bottomRight),
          description(desc), isVisited(false), visitOrder(-1) {}
    
    const std::string& getName() const { return name; }
    const std::string& getDescription() const { return description; }
    Vector2 getGridTopLeft() const { return gridTopLeft; }
    Vector2 getGridBottomRight() const { return gridBottomRight; }
    Vector2 getGridCenter() const {
        return {(gridTopLeft.x + gridBottomRight.x) / 2.0f,
                (gridTopLeft.y + gridBottomRight.y) / 2.0f};
    }
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
    std::vector<Building> buildings;
    std::vector<Vector2> currentPath;
    int startBuildingIndex;
    int destBuildingIndex;
    Texture2D campusBackground;
    bool isBackgroundLoaded;
    
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

    std::vector<Vector2> findPath(Vector2 start, Vector2 dest);
    void selectRandomStartAndDest();
    Building* getStartBuilding();
    Building* getDestBuilding();
    const std::vector<Vector2>& getCurrentPath() const { return currentPath; }

    void draw() const;
    void drawPath() const;
};

#endif
