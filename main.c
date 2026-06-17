/* ============================================================
   MelodyKey - Virtual Piano
   Language : C
   Library  : Raylib
   Features : 13 keys (8 white, 5 black, C4-C5)
              Mouse click input
              Keyboard key input (A S D F G H J K / W E T Y U)
              Real-time sine wave audio generation
              Color feedback on key press
              Stable 60 FPS game loop
   ============================================================ */

#include "raylib.h"
#include <math.h>
#include <stdlib.h>

/* ---------------- Configuration constants ---------------- */
#define NUM_WHITE_KEYS   8
#define NUM_BLACK_KEYS   5
#define WHITE_KEY_WIDTH  80
#define WHITE_KEY_HEIGHT 300
#define BLACK_KEY_WIDTH  50
#define BLACK_KEY_HEIGHT 180
#define SCREEN_WIDTH     800
#define SCREEN_HEIGHT    450
#define SAMPLE_RATE      44100
#define MAX_SAMPLES      (SAMPLE_RATE * 2)   /* 2 seconds of audio per note */

/* ---------------- Data structure ---------------- */
/* Each piano key (white or black) is represented by this struct */
typedef struct PianoKey {
    int   keyboardButton;  /* which laptop key triggers this note (KEY_A, KEY_S...) */
    float frequency;       /* frequency of the note in Hz (261.63 = C4, etc.)        */
    bool  isPressed;       /* true while the key is currently being pressed          */
    bool  wasPressed;      /* state from the previous frame (used to detect a NEW press) */
    bool  isBlackKey;      /* true = black key, false = white key                    */
    int   xPosition;       /* x coordinate where this key is drawn on screen         */
    Sound sound;           /* pre-generated sine wave sound for this note            */
} PianoKey;

/* ----------------------------------------------------------
   Generates a sine wave tone for a given frequency.
   An exponential decay envelope is applied so the note fades
   out naturally instead of sounding like a flat buzzer.
   ---------------------------------------------------------- */
Sound GenerateToneSound(float frequency) {
    int sampleCount = MAX_SAMPLES;
    short *samples = (short *)malloc(sampleCount * sizeof(short));

    for (int i = 0; i < sampleCount; i++) {
        float t = (float)i / SAMPLE_RATE;
        float envelope = expf(-2.0f * t);                       /* decay over time   */
        float value = sinf(2.0f * PI * frequency * t) * envelope; /* sine wave samples */
        samples[i] = (short)(value * 32000);
    }

    Wave wave   = { 0 };
    wave.frameCount = sampleCount;
    wave.sampleRate = SAMPLE_RATE;
    wave.sampleSize = 16;
    wave.channels   = 1;
    wave.data       = samples;

    Sound sound = LoadSoundFromWave(wave);
    free(samples);  /* LoadSoundFromWave copies the data internally, safe to free */
    return sound;
}

int main(void) {

    /* ---------------- Step 1: Initialize window + audio ---------------- */
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "MelodyKey - Virtual Piano");
    InitAudioDevice();
    SetTargetFPS(60);

    /* ---------------- Step 2: Define white keys (C4 to C5) ---------------- */
    /* Keyboard mapping: A S D F G H J K */
    PianoKey whiteKeys[NUM_WHITE_KEYS] = {
        { KEY_A, 261.63f, false, false, false, 0 },  /* C4 */
        { KEY_S, 293.66f, false, false, false, 0 },  /* D4 */
        { KEY_D, 329.63f, false, false, false, 0 },  /* E4 */
        { KEY_F, 349.23f, false, false, false, 0 },  /* F4 */
        { KEY_G, 392.00f, false, false, false, 0 },  /* G4 */
        { KEY_H, 440.00f, false, false, false, 0 },  /* A4 */
        { KEY_J, 493.88f, false, false, false, 0 },  /* B4 */
        { KEY_K, 523.25f, false, false, false, 0 }   /* C5 */
    };

    /* ---------------- Step 3: Define black keys (sharps) ---------------- */
    /* Keyboard mapping: W E T Y U  (no black key between E-F and B-C, like a real piano) */
    PianoKey blackKeys[NUM_BLACK_KEYS] = {
        { KEY_W, 277.18f, false, false, true, 0 },   /* C#4 */
        { KEY_E, 311.13f, false, false, true, 0 },   /* D#4 */
        { KEY_T, 369.99f, false, false, true, 0 },   /* F#4 */
        { KEY_Y, 415.30f, false, false, true, 0 },   /* G#4 */
        { KEY_U, 466.16f, false, false, true, 0 }    /* A#4 */
    };

    /* ---------------- Step 4: Pre-generate sound for every key ---------------- */
    /* Doing this once up front avoids any lag/delay when a key is pressed during play */
    for (int i = 0; i < NUM_WHITE_KEYS; i++) {
        whiteKeys[i].sound = GenerateToneSound(whiteKeys[i].frequency);
    }
    for (int i = 0; i < NUM_BLACK_KEYS; i++) {
        blackKeys[i].sound = GenerateToneSound(blackKeys[i].frequency);
    }

    /* ---------------- Step 5: Calculate key screen positions ---------------- */
    int startX = (SCREEN_WIDTH - (NUM_WHITE_KEYS * WHITE_KEY_WIDTH)) / 2;
    for (int i = 0; i < NUM_WHITE_KEYS; i++) {
        whiteKeys[i].xPosition = startX + (i * WHITE_KEY_WIDTH);
    }

    /* Each black key sits centered on the boundary AFTER the given white key index */
    int blackKeyWhiteIndex[NUM_BLACK_KEYS] = {0, 1, 3, 4, 5};
    for (int i = 0; i < NUM_BLACK_KEYS; i++) {
        int wIndex = blackKeyWhiteIndex[i];
        blackKeys[i].xPosition = whiteKeys[wIndex].xPosition + WHITE_KEY_WIDTH - (BLACK_KEY_WIDTH / 2);
    }

    int keyboardY = (SCREEN_HEIGHT - WHITE_KEY_HEIGHT) / 2;

    /* ============================================================
       MAIN GAME LOOP - runs continuously at 60 FPS until window closes
       ============================================================ */
    while (!WindowShouldClose()) {

        Vector2 mousePos  = GetMousePosition();
        bool    mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);

        /* ---- Update black keys first (they are drawn on top, so check them first) ---- */
        for (int i = 0; i < NUM_BLACK_KEYS; i++) {
            Rectangle keyRect = { (float)blackKeys[i].xPosition, (float)keyboardY,
                                   (float)BLACK_KEY_WIDTH, (float)BLACK_KEY_HEIGHT };
            bool mouseOver       = CheckCollisionPointRec(mousePos, keyRect);
            bool keyboardPressed = IsKeyDown(blackKeys[i].keyboardButton);

            blackKeys[i].wasPressed = blackKeys[i].isPressed;
            blackKeys[i].isPressed  = keyboardPressed || (mouseOver && mouseDown);

            /* Play the note only on the exact frame it becomes pressed,
               not every frame while held down */
            if (blackKeys[i].isPressed && !blackKeys[i].wasPressed) {
                PlaySound(blackKeys[i].sound);
            }
        }

        /* ---- Update white keys, ignoring areas covered by black keys ---- */
        for (int i = 0; i < NUM_WHITE_KEYS; i++) {
            Rectangle keyRect = { (float)whiteKeys[i].xPosition, (float)keyboardY,
                                   (float)WHITE_KEY_WIDTH, (float)WHITE_KEY_HEIGHT };
            bool mouseOver = CheckCollisionPointRec(mousePos, keyRect);

            /* Check whether the mouse is actually over a black key sitting on top */
            bool overBlackKey = false;
            for (int j = 0; j < NUM_BLACK_KEYS; j++) {
                Rectangle blackRect = { (float)blackKeys[j].xPosition, (float)keyboardY,
                                         (float)BLACK_KEY_WIDTH, (float)BLACK_KEY_HEIGHT };
                if (CheckCollisionPointRec(mousePos, blackRect)) {
                    overBlackKey = true;
                    break;
                }
            }

            bool keyboardPressed = IsKeyDown(whiteKeys[i].keyboardButton);

            whiteKeys[i].wasPressed = whiteKeys[i].isPressed;
            whiteKeys[i].isPressed  = keyboardPressed || (mouseOver && !overBlackKey && mouseDown);

            if (whiteKeys[i].isPressed && !whiteKeys[i].wasPressed) {
                PlaySound(whiteKeys[i].sound);
            }
        }

        /* ---------------- Draw everything ---------------- */
        BeginDrawing();
        ClearBackground(RAYWHITE);

        /* White keys drawn first (background layer) */
        for (int i = 0; i < NUM_WHITE_KEYS; i++) {
            Color keyColor = whiteKeys[i].isPressed ? SKYBLUE : WHITE;
            DrawRectangle(whiteKeys[i].xPosition, keyboardY, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT, keyColor);
            DrawRectangleLines(whiteKeys[i].xPosition, keyboardY, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT, BLACK);
        }

        /* Black keys drawn on top (foreground layer) */
        for (int i = 0; i < NUM_BLACK_KEYS; i++) {
            Color keyColor = blackKeys[i].isPressed ? PURPLE : BLACK;
            DrawRectangle(blackKeys[i].xPosition, keyboardY, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, keyColor);
        }

        DrawText("MelodyKey - Virtual Piano", 10, 10, 20, DARKGRAY);
        DrawText("Keys: A S D F G H J K (white)   W E T Y U (black)   ESC to quit", 10, 35, 16, GRAY);

        EndDrawing();
    }

    /* ---------------- Cleanup ---------------- */
    for (int i = 0; i < NUM_WHITE_KEYS; i++) UnloadSound(whiteKeys[i].sound);
    for (int i = 0; i < NUM_BLACK_KEYS; i++) UnloadSound(blackKeys[i].sound);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}