#ifndef GAME_OBJECTS_H
#define GAME_OBJECTS_H

#include <SDL.h>
#include <vector>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int BRICK_ROWS = 7;
const int BRICK_COLS = 12;
const int BRICK_WIDTH = SCREEN_WIDTH / BRICK_COLS;
const int BRICK_HEIGHT = 20;
const int PADDLE_WIDTH = 100;
const int PADDLE_HEIGHT = 10;
const int PADDLE_SPEED = 8;
const int BALL_SIZE = 8;
const float BALL_SPEED = 4.0f;
const int MAX_MISSES = 5;
const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;
const int BONUS_SIZE = 10;
const float BONUS_SPEED = 2.0f;
const int STICKY_DURATION = 300;
const int SAFETY_DURATION = 180;

enum BrickType {
    NORMAL = 0,
    UNBREAKABLE,
    SPEED_BONUS,
    HEALTH_BRICK,
    BONUS_BRICK
};

enum BonusType {
    BONUS_NONE = 0,
    BONUS_WIDER_PADDLE,
    BONUS_NARROW_PADDLE,
    BONUS_FASTER_BALL,
    BONUS_SLOWER_BALL,
    BONUS_STICKY_PADDLE,
    BONUS_SAFETY_NET,
    BONUS_EXTRA_BALL
};

struct Brick {
    int x, y;
    int width, height;
    BrickType type;
    int health;
    bool active;
    bool bonusDropped;

    Brick(int x, int y, BrickType t = NORMAL);
    void hit();
    void render(SDL_Renderer* renderer) const;
    SDL_Rect getRect() const { return { x, y, width, height }; }
};

struct Ball {
    float x, y;
    float vx, vy;
    int size;
    bool active;
    bool stuckToPaddle;

    Ball(float x, float y, int size);
    void update();
    void render(SDL_Renderer* renderer) const;
    void handlePaddleCollision(int paddleX, int paddleWidth, bool stickyActive);
    void handleWallCollision();
    void reverseX() { vx = -vx; }
    void reverseY() { vy = -vy; }
    void setPosition(float newX, float newY) { x = newX; y = newY; }
    SDL_Rect getRect() const { return { (int)x, (int)y, size, size }; }
    void modifySpeed(float multiplier) { vx *= multiplier; vy *= multiplier; }
};

struct Bonus {
    int x, y;
    BonusType type;
    bool active;
    float ySpeed;

    Bonus(int x, int y, BonusType t);
    void update();
    void render(SDL_Renderer* renderer) const;
    SDL_Rect getRect() const { return { x, y, BONUS_SIZE, BONUS_SIZE }; }
};

struct Paddle {
    int x;
    int width;
    int originalWidth;

    Paddle();
    void moveLeft();
    void moveRight();
    void updateWidth(int newWidth);
    void resetWidth();
    void render(SDL_Renderer* renderer) const;
    SDL_Rect getRect() const { return { x, SCREEN_HEIGHT - PADDLE_HEIGHT, width, PADDLE_HEIGHT }; }
};

#endif