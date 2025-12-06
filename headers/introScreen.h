#ifndef INTROSCREEN_H
#define INTROSCREEN_H
#include <raylib.h>

class IntroScreen {
private:
    Texture2D introImage;
    bool isLoaded;

public:
    IntroScreen() : isLoaded(false) {
        introImage = LoadTexture("assets/IntroScreen.png");
        isLoaded = (introImage.id != 0);
    }

    ~IntroScreen() {
        if (isLoaded) {
            UnloadTexture(introImage);
        }
    }

    void draw() const {
        if (isLoaded) {
            DrawTexture(introImage, 0, 0, WHITE);
        }
    }
};
#endif