#include "../headers/player.h"
#include "../headers/campus.h"
#include <cmath>

Player::Player(Vector2 startGridPos, float s) 
        : pos(gridToPixel(startGridPos)),
            gridPos(startGridPos),
            direction({0, 0}),
            speed(s),
            color(BLUE),
            score(INITIAL_SCORE),
            size(PLAYER_SIZE) {
}

void Player::input() {
    direction.x = int(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) - 
                  int(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A));
    direction.y = int(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) - 
                  int(IsKeyDown(KEY_UP) || IsKeyDown(KEY_W));
    direction = normalized(direction);
}

void Player::move(float dt, const class Campus* campus) {
    if (direction.x != 0 || direction.y != 0) {
        // Calculate new position
        Vector2 newPos = pos;
        newPos.x += direction.x * speed * dt;
        newPos.y += direction.y * speed * dt;
        
        // Clamp to screen bounds
        if (newPos.x < 0) newPos.x = 0;
        if (newPos.y < 0) newPos.y = 0;
        if (newPos.x + size > WINDOW_WIDTH) newPos.x = WINDOW_WIDTH - size;
        if (newPos.y + size > WINDOW_HEIGHT) newPos.y = WINDOW_HEIGHT - size;
        
        // Check collision with obstacles if campus provided
        bool canMove = true;
        if (campus != nullptr) {
            // Check all corners of the player bounding box
            Vector2 topLeft = pixelToGrid(newPos);
            Vector2 topRight = pixelToGrid({newPos.x + size, newPos.y});
            Vector2 bottomLeft = pixelToGrid({newPos.x, newPos.y + size});
            Vector2 bottomRight = pixelToGrid({newPos.x + size, newPos.y + size});
            
            if (!campus->canWalkOn(topLeft) || !campus->canWalkOn(topRight) ||
                !campus->canWalkOn(bottomLeft) || !campus->canWalkOn(bottomRight)) {
                canMove = false;
            }
        }
        
        // Only update position if movement is valid
        if (canMove) {
            pos = newPos;
            gridPos = pixelToGrid(pos);
        }
    }
}

void Player::update(float dt, const class Campus* campus) {
    input();
    move(dt, campus);
}

void Player::draw() const {
    // Draw player as a circle
    DrawCircleV(getCenter(), size/2, color);
    
    // Draw direction indicator
    if (direction.x != 0 || direction.y != 0) {
        Vector2 center = getCenter();
        Vector2 dirEnd = {center.x + direction.x * size/2, 
                         center.y + direction.y * size/2};
        DrawLineEx(center, dirEnd, 3, WHITE);
    }
}

void Player::setGridPos(Vector2 gPos) {
    gridPos = gPos;
    pos = gridToPixel(gPos);
}

Vector2 Player::gridToPixel(Vector2 gridPos) {
    return Vector2{gridPos.x * CELL_WIDTH, gridPos.y * CELL_HEIGHT};
}

Vector2 Player::pixelToGrid(Vector2 pixelPos) {
    return Vector2{std::floor(pixelPos.x / CELL_WIDTH), std::floor(pixelPos.y / CELL_HEIGHT)};
}
