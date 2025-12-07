#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <raylib.h>
#include <vector>
#include <queue>
#include <string>
#include "settings.h"
using namespace std;

// make node first 
class MusicNode { 
public:
    string title;
    float duration;
    MusicNode* next;
    MusicNode* prev;
    Music music;
    MusicNode(const string& t, float d, Music m) : title(t), duration(d), music(m), next(nullptr), prev(nullptr) {}
};

class MusicPlayer {
private:
    MusicNode* head;
    MusicNode* tail;
    MusicNode* current;
    int size;
    bool isPlaying;
    Font musicFont;
public:
    MusicPlayer() {
        head = nullptr;
        tail = nullptr;
        current = nullptr;
        size = 0;
        isPlaying = false;
        musicFont = LoadFontEx("assets/Montserrat-SemiBold.ttf", 32, NULL, 0);
        if (musicFont.texture.id == 0) musicFont = GetFontDefault();
    }

    void addSong(string title, float duration, Music music) {
        // music is already loaded in main file
        MusicNode* newNode = new MusicNode(title, duration, music);
        if (!head) {
            head = newNode;
            tail = newNode;
            current = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
            tail->next = head;
            head->prev = tail;
        }
        size++;
    }

    void play() {
        if (current && !isPlaying) {
            PlayMusicStream(current->music);
            isPlaying = true;
        }
    }

    void pause() {
        if (current && isPlaying) {
            PauseMusicStream(current->music);
            isPlaying = false;
        }
    }

    void togglePlayPause() {
        if (isPlaying) {
            pause();
            isPlaying = false;
        } else {
            play();
            isPlaying = true;
        }
    }

    void resume() {
        if (current && !isPlaying) {
            ResumeMusicStream(current->music);
            isPlaying = true;
        }
    }

    void next() {
        if (current && current->next) {
            if (isPlaying) {
                StopMusicStream(current->music);
            }
            current = current->next;
            if (isPlaying) {
                PlayMusicStream(current->music);
            }
        }
    }

    void prev() {
        if (current && current->prev) {
            if (isPlaying) {
                StopMusicStream(current->music);
            }
            current = current->prev;
            if (isPlaying) {
                PlayMusicStream(current->music);
            }
        }
    }

    void update() {
        if (current && isPlaying) {
            UpdateMusicStream(current->music);
        }
    }

    MusicNode* getCurrentTrack() const {
        return current;
    }

    bool getIsPlaying() const {
        return isPlaying;
    }

    int getSize() const {
        return size;
    }

    void draw() const {
        float boxX = 1 * CELL_WIDTH;
        float boxY = 1 * CELL_HEIGHT;
        float boxWidth = 3.0f * CELL_WIDTH;
        float boxHeight = 2.5f * CELL_HEIGHT;

        Color shadow = ColorAlpha(BLACK, 0.18f);
        DrawRectangleRounded({boxX + 3, boxY + 3, boxWidth, boxHeight}, 0.12f, 8, shadow);

        DrawRectangleRounded({boxX, boxY, boxWidth, boxHeight}, 0.12f, 8, WHITE);
        DrawRectangleRoundedLines({boxX, boxY, boxWidth, boxHeight}, 0.12f, 8, Color{180, 180, 180, 255});

        Color accent = Color{90, 140, 255, 255};
        DrawRectangleRounded({boxX, boxY, boxWidth, 0.35f * boxHeight}, 0.12f, 8, accent);

        // paddings
        float padX = boxX + 14;
        float padY = boxY + 12;
        float contentWidth = boxWidth - 28;

        if (current) {
            // title (larger)
            const char* title = current->title.c_str();
            float titleSize = 22.0f;
            Vector2 titleSizeVec = MeasureTextEx(musicFont, title, titleSize, 1);
            DrawTextEx(musicFont, title, {padX + (contentWidth - titleSizeVec.x) / 2, padY}, titleSize, 1, WHITE);

            // status
            const char* statusText = isPlaying ? "Playing" : "Paused";
            float statusSize = 18.0f;
            Vector2 statusSizeVec = MeasureTextEx(musicFont, statusText, statusSize, 1);
            Color statusColor = isPlaying ? Color{60, 200, 120, 255} : Color{240, 170, 70, 255};
            DrawTextEx(musicFont, statusText, {padX + (contentWidth - statusSizeVec.x) / 2, padY + 36}, statusSize, 1, statusColor);

            // time played
            float timePlayed = GetMusicTimePlayed(current->music);
            float totalTime = current->duration;
            char timeStr[64];
            snprintf(timeStr, sizeof(timeStr), "%.0f / %.0f sec", timePlayed, totalTime);
            float timeSize = 18.0f;
            Vector2 timeSizeVec = MeasureTextEx(musicFont, timeStr, timeSize, 1);
            DrawTextEx(musicFont, timeStr, {padX + (contentWidth - timeSizeVec.x) / 2, padY + 64}, timeSize, 1, Color{50, 120, 60, 255});
        } else {
            // centered no track text
            const char* noTrack = "No track loaded";
            float noTrackSize = 18.0f;
            Vector2 ntSizeVec = MeasureTextEx(musicFont, noTrack, noTrackSize, 1);
            DrawTextEx(musicFont, noTrack, {padX + (contentWidth - ntSizeVec.x) / 2, padY + 40}, noTrackSize, 1, DARKGRAY);
        }
    }

    ~MusicPlayer() {
        if (head) {
            MusicNode* node = head;
            do {
                MusicNode* temp = node;
                node = node->next;
                UnloadMusicStream(temp->music);
                delete temp;
            } while (node != head);
        }
        if (musicFont.texture.id != GetFontDefault().texture.id) {
            UnloadFont(musicFont);
        }
    }
};

#endif
