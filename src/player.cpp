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
                textureStill = LoadTexture("assets/person-still.png");
                textureWalking = LoadTexture("assets/person-walking.png");
                rotation = 0.0f;
}

void Player::input() {
    bool moveRight = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);
    bool moveLeft = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
    bool moveDown = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);
    bool moveUp = IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);

    if (moveUp || moveDown) {
        direction.x = 0;
        direction.y = int(moveDown) - int(moveUp);
    } else if (moveLeft || moveRight) {
        direction.x = int(moveRight) - int(moveLeft);
        direction.y = 0;
    } else {
        direction.x = 0;
        direction.y = 0;
    }
}

void Player::move(float dt, const class Campus* campus) {
    if (direction.x != 0 || direction.y != 0) {
        Vector2 newPos = pos;
        newPos.x += direction.x * speed * dt;
        newPos.y += direction.y * speed * dt;
        
        // clamping if out of bounds
        if (newPos.x < 0) newPos.x = 0;
        if (newPos.y < 0) newPos.y = 0;
        if (newPos.x + size > WINDOW_WIDTH) newPos.x = WINDOW_WIDTH - size;
        if (newPos.y + size > WINDOW_HEIGHT) newPos.y = WINDOW_HEIGHT - size;
        
        // collision detection
        bool canMove = true;
        if (campus != nullptr) {
            // check for corners
            Vector2 topLeft = pixelToGrid(newPos);
            Vector2 topRight = pixelToGrid({newPos.x + size, newPos.y});
            Vector2 bottomLeft = pixelToGrid({newPos.x, newPos.y + size});
            Vector2 bottomRight = pixelToGrid({newPos.x + size, newPos.y + size});
            
            if (!campus->canWalkOn(topLeft) || !campus->canWalkOn(topRight) ||
                !campus->canWalkOn(bottomLeft) || !campus->canWalkOn(bottomRight)) {
                canMove = false;
            }
        }
        
        // move if valid
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

void Player::draw() {
    
    // rotation based on movement direction
    if (direction.x != 0 || direction.y != 0) {
        rotation = atan2(direction.y, direction.x) * RAD2DEG + 90.0f;
    }

    Vector2 origin = {size/2, size/2};
    
    if (direction.x != 0 || direction.y != 0) {
        DrawTexturePro(textureWalking, Rectangle{0, 0, (float)textureWalking.width, (float)textureWalking.height}, Rectangle{pos.x + size/2, pos.y + size/2, size, size}, origin, rotation, WHITE);
    } else {
        DrawTexturePro(textureStill, Rectangle{0, 0, (float)textureStill.width, (float)textureStill.height}, Rectangle{pos.x + size/2, pos.y + size/2, size, size}, origin, rotation, WHITE);
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
    return Vector2{(float)floor(pixelPos.x / CELL_WIDTH), (float)floor(pixelPos.y / CELL_HEIGHT)};
}
