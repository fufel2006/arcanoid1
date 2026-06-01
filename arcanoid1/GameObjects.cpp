#include "GameObjects.h"
#include <cmath>
#include <algorithm>

Brick::Brick(int x, int y, BrickType t) : x(x), y(y), width(BRICK_WIDTH), height(BRICK_HEIGHT),
type(t), active(true), bonusDropped(false) {
    if (type == HEALTH_BRICK) health = 3;
    else health = 1;
}

void Brick::hit() {
    if (type == UNBREAKABLE) return;
    health--;
    if (health <= 0) active = false;
}

void Brick::render(SDL_Renderer* renderer) const {
    if (!active) return;

    SDL_Color fillColor, borderColor;

    switch (type) {
    case NORMAL:
        fillColor = { 0, 255, 0, 255 };
        borderColor = { 0, 200, 0, 255 };
        break;
    case UNBREAKABLE:
        fillColor = { 80, 80, 80, 255 };
        borderColor = { 120, 120, 120, 255 };
        break;
    case SPEED_BONUS:
        fillColor = { 255, 255, 0, 255 };
        borderColor = { 200, 200, 0, 255 };
        break;
    case HEALTH_BRICK:
        if (health == 3) fillColor = { 255, 0, 0, 255 };
        else if (health == 2) fillColor = { 255, 100, 0, 255 };
        else fillColor = { 255, 150, 0, 255 };
        borderColor = { 200, 0, 0, 255 };
        break;
    case BONUS_BRICK:
        fillColor = { 255, 0, 255, 255 };
        borderColor = { 200, 0, 200, 255 };
        break;
    default:
        fillColor = { 255, 255, 255, 255 };
        borderColor = { 200, 200, 200, 255 };
        break;
    }

    SDL_SetRenderDrawColor(renderer, fillColor.r, fillColor.g, fillColor.b, fillColor.a);
    SDL_Rect rect = getRect();
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderDrawRect(renderer, &rect);

    if (type == HEALTH_BRICK && health > 0) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        int healthBarWidth = (width - 4) * health / 3;
        SDL_Rect healthRect = { x + 2, y + height - 4, healthBarWidth, 2 };
        SDL_RenderFillRect(renderer, &healthRect);
    }
}

Ball::Ball(float x, float y, int size) : x(x), y(y), size(size), active(true), stuckToPaddle(true) {
    vx = 0.0f;
    vy = -BALL_SPEED;
}

void Ball::update() {
    if (!stuckToPaddle) {
        x += vx;
        y += vy;
    }
}

void Ball::render(SDL_Renderer* renderer) const {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect rect = getRect();
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawRect(renderer, &rect);
}

void Ball::handlePaddleCollision(int paddleX, int paddleWidth, bool stickyActive) {
    float hitPos = (x + size / 2 - paddleX) / paddleWidth;
    float angle = (hitPos - 0.5f) * 1.5f;
    float speed = sqrt(vx * vx + vy * vy);
    if (speed < 0.1f) speed = BALL_SPEED;
    vx = sin(angle) * speed;
    vy = -abs(cos(angle) * speed);
    y = SCREEN_HEIGHT - PADDLE_HEIGHT - size;

    if (stickyActive) {
        stuckToPaddle = true;
    }
}

void Ball::handleWallCollision() {
    if (x <= 0 || x + size >= SCREEN_WIDTH) {
        vx = -vx;
        x = std::max(0.0f, std::min(x, (float)(SCREEN_WIDTH - size)));
    }
    if (y <= 0) {
        vy = -vy;
        y = 0;
    }
}

Bonus::Bonus(int x, int y, BonusType t) : x(x), y(y), type(t), active(true), ySpeed(BONUS_SPEED) {}

void Bonus::update() {
    y += ySpeed;
}

void Bonus::render(SDL_Renderer* renderer) const {
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_Rect rect = getRect();
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, 0, 200, 200, 255);
    SDL_RenderDrawRect(renderer, &rect);
}

Paddle::Paddle() {
    x = SCREEN_WIDTH / 2 - PADDLE_WIDTH / 2;
    width = PADDLE_WIDTH;
    originalWidth = PADDLE_WIDTH;
}

void Paddle::moveLeft() {
    x -= PADDLE_SPEED;
    if (x < 0) x = 0;
}

void Paddle::moveRight() {
    x += PADDLE_SPEED;
    if (x + width > SCREEN_WIDTH) x = SCREEN_WIDTH - width;
}

void Paddle::updateWidth(int newWidth) {
    width = newWidth;
    if (x + width > SCREEN_WIDTH) {
        x = SCREEN_WIDTH - width;
    }
}

void Paddle::resetWidth() {
    width = originalWidth;
}

void Paddle::render(SDL_Renderer* renderer) const {
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_Rect rect = getRect();
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, 0, 0, 200, 255);
    SDL_RenderDrawRect(renderer, &rect);
}