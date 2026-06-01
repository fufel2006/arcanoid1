#include "GameManager.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>

GameManager::GameManager() : misses(0), safetyNetActive(false),
stickyPaddleActive(false), stickyFrames(0), safetyFrames(0), running(true) {
    srand(time(nullptr));
}

void GameManager::init() {
    initBricks();
    balls.clear();
    balls.push_back(Ball(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 50, BALL_SIZE));
    misses = 0;
    safetyNetActive = false;
    stickyPaddleActive = false;
    paddle = Paddle();
}

void GameManager::initBricks() {
    bricks.clear();

    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            int x = col * BRICK_WIDTH;
            int y = row * BRICK_HEIGHT + 50;

            BrickType type = NORMAL;

            if (row == 0) {
                type = UNBREAKABLE;
            }
            else if ((col == 0 || col == BRICK_COLS - 1) && row < 3) {
                type = UNBREAKABLE;
            }
            else if (row == 2 && (col == 2 || col == BRICK_COLS - 3)) {
                type = UNBREAKABLE;
            }
            else if (row == 4 && (col == 4 || col == BRICK_COLS - 5)) {
                type = UNBREAKABLE;
            }
            else if (row == 1 && rand() % 3 == 0) {
                type = SPEED_BONUS;
            }
            else if (row == 3 && rand() % 4 == 0) {
                type = HEALTH_BRICK;
            }
            else if (row == 5 && rand() % 5 == 0) {
                type = BONUS_BRICK;
            }

            bricks.push_back(Brick(x, y, type));
        }
    }
}

void GameManager::update() {
    updateEffects();
    updateBalls();
    updateBonuses();

    bool allBricksDestroyed = true;
    for (const auto& brick : bricks) {
        if (brick.active && brick.type != UNBREAKABLE) {
            allBricksDestroyed = false;
            break;
        }
    }

    if (allBricksDestroyed) {
        running = false;
    }
}

void GameManager::updateEffects() {
    if (stickyPaddleActive) {
        stickyFrames++;
        if (stickyFrames > STICKY_DURATION) {
            stickyPaddleActive = false;
            stickyFrames = 0;
        }
    }

    if (safetyNetActive) {
        safetyFrames++;
        if (safetyFrames > SAFETY_DURATION) {
            safetyNetActive = false;
            safetyFrames = 0;
        }
    }
}

void GameManager::updateBalls() {
    for (int i = 0; i < balls.size(); i++) {
        if (!balls[i].active) continue;

        if (balls[i].stuckToPaddle) {
            balls[i].x = paddle.x + paddle.width / 2;
        }

        balls[i].update();
        balls[i].handleWallCollision();

        SDL_Rect ballRect = balls[i].getRect();
        SDL_Rect paddleRect = paddle.getRect();

        if (SDL_HasIntersection(&ballRect, &paddleRect)) {
            balls[i].handlePaddleCollision(paddle.x, paddle.width, stickyPaddleActive);
        }

        for (int j = 0; j < bricks.size(); j++) {
            if (!bricks[j].active) continue;

            SDL_Rect brickRect = bricks[j].getRect();
            if (SDL_HasIntersection(&ballRect, &brickRect)) {
                bricks[j].hit();

                if (bricks[j].type == SPEED_BONUS && bricks[j].active) {
                    for (auto& b : balls) {
                        b.modifySpeed(1.2f);
                    }
                }
                else if (bricks[j].type == BONUS_BRICK && !bricks[j].bonusDropped && !bricks[j].active) {
                    bricks[j].bonusDropped = true;
                    BonusType bonus = (BonusType)(rand() % 7 + 1);
                    bonuses.push_back(Bonus(bricks[j].x + bricks[j].width / 2, bricks[j].y + bricks[j].height, bonus));
                }

                float overlapLeft = (ballRect.x + ballRect.w) - brickRect.x;
                float overlapRight = (brickRect.x + brickRect.w) - ballRect.x;
                float overlapTop = (ballRect.y + ballRect.h) - brickRect.y;
                float overlapBottom = (brickRect.y + brickRect.h) - ballRect.y;

                bool fromLeft = overlapLeft < overlapRight;
                bool fromTop = overlapTop < overlapBottom;

                if (fromLeft && fromTop) {
                    if (overlapLeft < overlapTop) balls[i].reverseX();
                    else balls[i].reverseY();
                }
                else if (!fromLeft && fromTop) {
                    if (overlapRight < overlapTop) balls[i].reverseX();
                    else balls[i].reverseY();
                }
                else if (fromLeft && !fromTop) {
                    if (overlapLeft < overlapBottom) balls[i].reverseX();
                    else balls[i].reverseY();
                }
                else {
                    if (overlapRight < overlapBottom) balls[i].reverseX();
                    else balls[i].reverseY();
                }
                break;
            }
        }

        if (balls[i].y + balls[i].size >= SCREEN_HEIGHT) {
            if (safetyNetActive) {
                safetyNetActive = false;
                balls[i].y = SCREEN_HEIGHT - balls[i].size - 1;
                balls[i].vy = -balls[i].vy;
            }
            else {
                balls[i].active = false;
                misses++;

                if (misses >= MAX_MISSES) {
                    running = false;
                }
            }
        }
    }

    balls.erase(std::remove_if(balls.begin(), balls.end(),
        [](const Ball& b) { return !b.active; }), balls.end());

    if (balls.empty()) {
        balls.push_back(Ball(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 50, BALL_SIZE));
        balls[0].stuckToPaddle = true;
    }
}

void GameManager::updateBonuses() {
    for (int i = 0; i < bonuses.size(); i++) {
        bonuses[i].update();

        if (bonuses[i].y + BONUS_SIZE >= SCREEN_HEIGHT) {
            bonuses[i].active = false;
            continue;
        }

        SDL_Rect bonusRect = bonuses[i].getRect();
        SDL_Rect paddleRect = paddle.getRect();

        if (SDL_HasIntersection(&bonusRect, &paddleRect)) {
            applyBonus(bonuses[i].type);
            bonuses[i].active = false;
        }
    }

    bonuses.erase(std::remove_if(bonuses.begin(), bonuses.end(),
        [](const Bonus& b) { return !b.active; }), bonuses.end());
}

void GameManager::applyBonus(BonusType type) {
    switch (type) {
    case BONUS_WIDER_PADDLE:
        paddle.updateWidth(std::min(paddle.width + 40, SCREEN_WIDTH - 50));
        break;
    case BONUS_NARROW_PADDLE:
        paddle.updateWidth(std::max(paddle.width - 30, 40));
        break;
    case BONUS_FASTER_BALL:
        for (auto& b : balls) {
            b.modifySpeed(1.5f);
        }
        break;
    case BONUS_SLOWER_BALL:
        for (auto& b : balls) {
            b.modifySpeed(0.7f);
        }
        break;
    case BONUS_STICKY_PADDLE:
        stickyPaddleActive = true;
        stickyFrames = 0;
        break;
    case BONUS_SAFETY_NET:
        safetyNetActive = true;
        safetyFrames = 0;
        break;
    case BONUS_EXTRA_BALL:
        if (!balls.empty()) {
            Ball newBall(balls[0].x, balls[0].y, BALL_SIZE);
            newBall.vx = (rand() % 2 == 0) ? 2.0f : -2.0f;
            newBall.vy = -3.0f;
            newBall.stuckToPaddle = false;
            balls.push_back(newBall);
        }
        break;
    default:
        break;
    }
}

void GameManager::handleInput(const Uint8* keys, bool spacePressed, bool rPressed) {
    if (keys[SDL_SCANCODE_LEFT]) {
        paddle.moveLeft();
        if (!balls.empty() && balls[0].stuckToPaddle) {
            balls[0].x = paddle.x + paddle.width / 2;
        }
    }
    if (keys[SDL_SCANCODE_RIGHT]) {
        paddle.moveRight();
        if (!balls.empty() && balls[0].stuckToPaddle) {
            balls[0].x = paddle.x + paddle.width / 2;
        }
    }

    if (spacePressed && !balls.empty() && balls[0].stuckToPaddle) {
        balls[0].stuckToPaddle = false;
    }

    if (rPressed) {
        resetGame();
    }
}

void GameManager::resetGame() {
    init();
}

void GameManager::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (const auto& brick : bricks) {
        brick.render(renderer);
    }

    for (const auto& ball : balls) {
        ball.render(renderer);
    }

    for (const auto& bonus : bonuses) {
        bonus.render(renderer);
    }

    paddle.render(renderer);

    renderUI(renderer);

    SDL_RenderPresent(renderer);
}

void GameManager::renderUI(SDL_Renderer* renderer) {
    for (int i = 0; i < MAX_MISSES; i++) {
        int x = 10 + i * 35;
        int y = SCREEN_HEIGHT - 50;
        int size = 25;

        SDL_Rect lifeRect = { x, y, size, size };

        if (i < misses) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &lifeRect);
            SDL_SetRenderDrawColor(renderer, 50, 0, 0, 255);
            SDL_RenderFillRect(renderer, &lifeRect);
        }
        else {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &lifeRect);
            SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &lifeRect);
        }
    }

    if (safetyNetActive) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        SDL_Rect safetyRect = { SCREEN_WIDTH - 120, SCREEN_HEIGHT - 40, 110, 20 };
        SDL_RenderFillRect(renderer, &safetyRect);
        SDL_SetRenderDrawColor(renderer, 0, 200, 200, 255);
        SDL_RenderDrawRect(renderer, &safetyRect);
    }

    if (stickyPaddleActive) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_Rect stickyRect = { SCREEN_WIDTH - 120, SCREEN_HEIGHT - 70, 110, 20 };
        SDL_RenderFillRect(renderer, &stickyRect);
        SDL_SetRenderDrawColor(renderer, 200, 200, 0, 255);
        SDL_RenderDrawRect(renderer, &stickyRect);
    }
}