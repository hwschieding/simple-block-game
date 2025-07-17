#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE      20

typedef struct Block {
    Rectangle rect;
    Color color;
    int x;
    int y;
} Block;

typedef struct World {
    Block** blocks;
    int width;
    int height;
} World;

Block** initBlocks(int width, int height, int screenWidth){

    const float blocksWidth = width * BLOCK_SIZE;
    const float blockStartX = (screenWidth / 2.0f) - (blocksWidth / 2.0f); // screen center - half world width
    const float blockStartY = 500.0f;

    const int totalBlocks = width * height;

    Block** blocks = (Block**)malloc(totalBlocks * sizeof(Block*));
    if (!blocks){
        return NULL; // Allocation error
    }
    float xPos;
    float yPos;
    for(int x = 0; x < width; x++){
        blocks[x] = (Block*)malloc(height * sizeof(Block)); // column
        if (!blocks[x]){ // Allocation error 2
            for (int i = 0; i < x; i++){
                free(blocks[i]);
            }
            free(blocks);
            return NULL;
        }
        for (int y = 0; y < height; y++){
            xPos = blockStartX + (x * BLOCK_SIZE);
            yPos = blockStartY - (y * BLOCK_SIZE);
            blocks[x][y].rect = (Rectangle){ xPos, yPos, BLOCK_SIZE, BLOCK_SIZE };
            blocks[x][y].color = (Color){
                GetRandomValue(100, 200),
                GetRandomValue(100, 200),
                GetRandomValue(100, 200),
                255
            };
            blocks[x][y].x = x;
            blocks[x][y].y = y;
        }
    }
    return blocks;

}

void freeBlocks(Block** arr, int width){
    for (int i = 0; i < width; i++){
        free(arr[i]);
    }
    free(arr);
}

void updateWorldParameter(World* world, int* parameter, int screenWidth, int change){
    if (!(world->blocks)){
        printf("Failed blockmap check\n");
        return;
    }

    const int newParameter = *parameter + change;
    if (newParameter <= 0){
        printf("Failed parameter check\n");
        return;
    }
    freeBlocks(world->blocks, world->width);
    *parameter = newParameter;
    world->blocks = initBlocks(world->width, world->height, screenWidth);
}

void drawBlocks(World* world){
    if (!(world->blocks)){
        return;
    }
    for (int x = 0; x < world->width; x++){
        for (int y = 0; y < world->height; y++){
            Block* block = &(world->blocks[x][y]);
            DrawRectangleRec(block->rect, block->color);
        }
    }
}

void game(){
    // Game init
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Game");

    int worldWidth = 15;
    int worldHeight = 10;

    // init world array
    World world = {
        initBlocks(worldWidth, worldHeight, screenWidth),
        worldWidth,
        worldHeight
    };

    SetTargetFPS(60);
    printf("Game Init Successful\n");
    // Update
    while (!WindowShouldClose()){
        // user input
        if (IsKeyPressed(KEY_RIGHT)){
            updateWorldParameter(&world, &(world.width), screenWidth, 1);
        }
        else if (IsKeyPressed(KEY_LEFT)){
            updateWorldParameter(&world, &(world.width), screenWidth, -1);
        }
        else if (IsKeyPressed(KEY_UP)){
            updateWorldParameter(&world, &(world.height), screenWidth, 1);
        }
        else if (IsKeyPressed(KEY_DOWN)){
            updateWorldParameter(&world, &(world.height), screenWidth, -1);
        }
        // Render
        BeginDrawing();
        ClearBackground(RAYWHITE);
        drawBlocks(&world);
        // debug text
        DrawText(TextFormat("world height (x): %d", world.width), 10, 10, 20, BLACK);
        DrawText(TextFormat("world width (y): %d", world.height), 10, 40, 20, BLACK);
        DrawFPS(screenWidth - 100, 10);
        EndDrawing();
    }
    // deinitialize
    if (world.blocks){
        freeBlocks(world.blocks, world.width);
    }
    CloseWindow();

}

int main(void){
    game();
    return 0;
}