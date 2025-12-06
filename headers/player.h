#ifndef PLAYER_H
#define PLAYER_H

#include <raylib.h>
#include "settings.h"

inline Vector2 normalized(Vector2 v) {
    if (v.x != 0 && v.y != 0) {
        if (v.x < 0) v.x = -1;
        else if (v.x > 0) v.x = 1;

        if (v.y < 0) v.y = -1;
        else if (v.y > 0) v.y = 1;
    }
    return v;
}

class Player {
private:
    Vector2 pos;
    Vector2 gridPos;
    Vector2 direction;
    float speed;
    Color color;
    int score;
    float size;

public:
    Player(Vector2 startGridPos = {1, 1}, float s = PLAYER_SPEED);

    void input();
    
    void move(float dt, const class Campus* campus = nullptr);
    void update(float dt, const class Campus* campus = nullptr);
    
    void draw() const;
    
    Vector2 getPos() const { return pos; }
    Vector2 getGridPos() const { return gridPos; }
    Vector2 getCenter() const { return Vector2{pos.x + size/2, pos.y + size/2}; }
    int getScore() const { return score; }
    
    void setGridPos(Vector2 gPos);
    void setPos(Vector2 p) { pos = p; }
    void addScore(int points) { score += points; }
    
    static Vector2 gridToPixel(Vector2 gridPos);
    static Vector2 pixelToGrid(Vector2 pixelPos);
};

#endif
