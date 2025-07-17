#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE          20
#define WORLD_BLOCK_WIDTH   80
#define WORLD_BLOCK_HEIGHT  50

#define AIR     (BlockType){ 0, WHITE, 0.0f, true }
#define STONE   (BlockType){ 1, GRAY, 0.2f, false }
#define DIRT    (BlockType){ 2, BROWN, 0.0f, false }
#define GRASS   (BlockType){ 3, GREEN, 0.1f, false }

typedef struct BlockVector2 {
    int x;
    int y;
} BlockVector2;

typedef struct BlockType {
    int id;
    Color color;
    float blastResist;
    bool isAir;
} BlockType;

typedef struct Block {
    Rectangle rect;
    BlockType type;
    float x;
    float y;
} Block;

typedef struct World {
    Block** blocks;
    int width;
    int height;
} World;

Block** initBlocks(){
    const int worldSize = WORLD_BLOCK_WIDTH * WORLD_BLOCK_HEIGHT;
    const int halfWorldHeight = WORLD_BLOCK_HEIGHT / 2;
    const int grassLine = halfWorldHeight + 1;
    const int dirtLayer = grassLine + 2;
    Block** blocks = (Block**)malloc((worldSize) * sizeof(Block*));
    if (!blocks){
        return NULL; // Allocation error 1
    }
    float rectX;
    float rectY;
    for (int x = 0; x < WORLD_BLOCK_WIDTH; x++){
        blocks[x] = (Block*)malloc(WORLD_BLOCK_HEIGHT * sizeof(Block));
        if (!blocks[x]){ // Allocation error 2
            for (int i = 0; i < x; i++){
                free(blocks[i]);
            }
            free(blocks);
            return NULL;
        }
        rectX = x * BLOCK_SIZE;
        for (int y = 0; y < WORLD_BLOCK_HEIGHT; y++){
            rectY = y * BLOCK_SIZE;
            blocks[x][y].x = x;
            blocks[x][y].y = y;
            blocks[x][y].rect = (Rectangle){ rectX, rectY, BLOCK_SIZE, BLOCK_SIZE };
            if (y <= halfWorldHeight){
                blocks[x][y].type = AIR;
            } else if (y == grassLine){
                blocks[x][y].type = GRASS;
            } else if (y <= dirtLayer){
                blocks[x][y].type =  DIRT;
            } else {
                blocks[x][y].type = STONE;
            }
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

void drawWorld(World* world){
    if (!(world->blocks)){
        return;
    }
    for (int x = 0; x < world->width; x++){
        for (int y = 0; y < world->height; y++){
            Block* block = &(world->blocks[x][y]);
            DrawRectangleRec(block->rect, block->type.color);
        }
    }
}

BlockVector2 getBlock(Vector2 pos){
    int blockX = ((int)(pos.x / BLOCK_SIZE));
    int blockY = ((int)(pos.y / BLOCK_SIZE));
    return (BlockVector2){ blockX, blockY };
}

void changeBlock(World* world, BlockVector2 blockPos, BlockType newType){
    Block* block = &(world->blocks[blockPos.x][blockPos.y]);
    block->type = newType;
}

void drawBlockOutline(BlockVector2 blockPos){
    int blockX = blockPos.x * BLOCK_SIZE;
    int blockY = blockPos.y * BLOCK_SIZE;
    DrawRectangleLines(blockX, blockY, BLOCK_SIZE, BLOCK_SIZE, BLACK);
}

void game(void){
    // Game init
    const int screenWidth = WORLD_BLOCK_WIDTH * BLOCK_SIZE;
    const int screenHeight = WORLD_BLOCK_HEIGHT * BLOCK_SIZE;
    InitWindow(screenWidth, screenHeight, "Game");

    // init world array
    World world = {
        initBlocks(),
        WORLD_BLOCK_WIDTH,
        WORLD_BLOCK_HEIGHT
    };

    SetTargetFPS(60);
    printf("Game Init Successful\n");
    // Update
    while (!WindowShouldClose()){
        // user input
        
        // Render
        BeginDrawing();
        ClearBackground(RAYWHITE);
        drawWorld(&world);
        BlockVector2 mouseTarget = getBlock(GetMousePosition());
        drawBlockOutline(mouseTarget);
        if (IsKeyPressed(KEY_D)){
            changeBlock(&world, mouseTarget, AIR);
        }
        if (IsKeyPressed(KEY_B)){
            changeBlock(&world, mouseTarget, STONE);
        }
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