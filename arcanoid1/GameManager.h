#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "GameObjects.h"
#include <vector>

class GameManager {
private:
    std::vector<Brick> bricks;
    std::vector<Ball> balls;
    std::vector<Bonus> bonuses;
    Paddle paddle;

    int misses;
    bool safetyNetActive;
    bool stickyPaddleActive;
    int stickyFrames;
    int safetyFrames;
    bool running;

public:
    GameManager();

    void init();
    void update();
    void render(SDL_Renderer* renderer);
    bool isRunning() const { return running; }
    void setRunning(bool run) { running = run; }

    void handleInput(const Uint8* keys, bool spacePressed, bool rPressed);
    void resetGame();

private:
    void initBricks();
    void updateBalls();
    void updateBonuses();
    void applyBonus(BonusType type);
    void updateEffects();
    void renderUI(SDL_Renderer* renderer);
};

#endif