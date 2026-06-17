# Melody Key

Welcome to **Melody Key**! This is a lightweight, responsive virtual piano built entirely from scratch in C. 

Whether you want to jam out right from your computer keyboard or dive into the low-level mechanics of audio processing and input handling in C, Melody Key brings a classic instrument straight to your digital workspace.


##  Features

* **Instant Playback:** Highly responsive key-mapping that turns your standard QWERTY keyboard into a musical instrument.
* **Pure C Performance:** No heavy engines or bloated frameworks—just clean, efficient C code showcasing
* **Lightweight & Portable:** Drops right into your terminal or a minimal window with minimal CPU footprint.


##  How to compile

Use:
gcc main.c -o piano -IC:/Raylib/raylib-6.0_win64_mingw-w64/include -LC:/Raylib/raylib-6.0_win64_mingw-w64/lib -lraylib -lopengl32 -lgdi32 -lwinmm -lm
then:
./piano

### Prerequisites
Before compiling, ensure you have a C compiler (like `gcc`) installed. 

