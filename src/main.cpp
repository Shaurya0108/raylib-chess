/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/
#include "game/MultiplayerManager.hpp"

#include "raylib.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

int main(int argc, char* argv[]) {
    // Check if running as server
    bool isServer = argc > 1 && std::string(argv[1]) == "--server";
    
    // Initialize window
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(1280, 800, isServer ? "Game Server" : "Game Client");
    
    // Initialize multiplayer
    MultiplayerManager multiplayer(isServer);
    if (!multiplayer.initialize()) {
        DrawText("Failed to initialize network!", 200, 200, 20, RED);
        return -1;
    }

    while (!WindowShouldClose()) {
        // Update
        multiplayer.update();
        
        // Your existing game update code...

        // Drawing
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Your existing game drawing code...
        
        // Draw multiplayer UI
        multiplayer.draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
