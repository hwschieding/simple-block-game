#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define BLOCK_SIZE          20
#define WORLD_BLOCK_WIDTH   80
#define WORLD_BLOCK_HEIGHT  50

#define AIR     (BlockType){ 0, WHITE, 0.0f, true }
#define STONE   (BlockType){ 1, GRAY, 0.3f, false }
#define DIRT    (BlockType){ 2, BROWN, 0.1f, false }
#define GRASS   (BlockType){ 3, GREEN, 0.2f, false }

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
    bool inExplosionQueue;
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
    const int dirtLayer = grassLine + 10;
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
            blocks[x][y].inExplosionQueue = false;
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

BlockVector2 getBlockPos(Vector2 pos){
    int blockX = ((int)(pos.x / BLOCK_SIZE));
    int blockY = ((int)(pos.y / BLOCK_SIZE));
    return (BlockVector2){ blockX, blockY };
}

BlockType* getBlockType(World* world, BlockVector2 blockpos){
    if (!world->blocks){
        return NULL;
    }
    Block* block = &(world->blocks[blockpos.x][blockpos.y]);
    return &(block->type);
}

bool isInBounds(BlockVector2 blockpos){
    if (blockpos.x >= 0 && blockpos.x < WORLD_BLOCK_WIDTH && blockpos.y >= 0 && blockpos.y < WORLD_BLOCK_HEIGHT){
        return true;
    }
    // printf("Position out of bounds: %d (x), %d (y)\n", blockpos.x, blockpos.y);
    return false;
}

bool isBlock(World* world, BlockVector2 blockPos, int type){
    if (!world->blocks){
        return false;
    }
    BlockType* checkType = getBlockType(world, blockPos);
    if (checkType->id == type){
        return true;
    }
    return false;
}

bool isBlockNot(World* world, BlockVector2 blockPos, int type){
    if (!world->blocks){
        return false;
    }
    BlockType* checkType = getBlockType(world, blockPos);
    if (checkType->id != type){
        return true;
    }
    return false;
}

bool checkAdjacent(World* world, BlockVector2 blockpos, bool (*validate)(World*, BlockVector2, int), int type){
    if (!world->blocks){
        return false;
    }
    for (int i = -1; i < 2; i += 2){
        BlockVector2 checkPosX = (BlockVector2){ blockpos.x + i, blockpos.y };
        BlockVector2 checkPosY = (BlockVector2){ blockpos.x, blockpos.y + i };
        if (isInBounds(checkPosX) && validate(world, checkPosX, type)){
            return true;
        }
        if (isInBounds(checkPosY) && validate(world, checkPosY, type)){
            return true;
        }
    }
    return false;
}

void changeBlock(World* world, BlockVector2 blockPos, BlockType newType){
    if (!world->blocks){
        return;
    }
    Block* block = &(world->blocks[blockPos.x][blockPos.y]);
    block->type = newType;
}

void drawBlockOutline(BlockVector2 blockPos){
    int blockX = blockPos.x * BLOCK_SIZE;
    int blockY = blockPos.y * BLOCK_SIZE;
    DrawRectangleLines(blockX, blockY, BLOCK_SIZE, BLOCK_SIZE, BLACK);
}

void explosion(World* world, BlockVector2 origin, float power, const int rays){
    // setup variables
    const double angleChange = (PI * 2) / rays;
    const float powerFallOff = 0.2f;
    const int travel = (int)ceil(power / powerFallOff);
    double angle, angleCos, angleSin;
    float xChange, yChange, screenPosX, screenPosY;
    float powerRemaining;
    BlockVector2 newBlockPos;
    Block** affectedBlocks = (Block**)malloc((rays * travel) * sizeof(Block*));
    if (!affectedBlocks){
        return; // allocation error
    }
    int arrIndex = 0;
    // cast rays radially around centerpoint
    for (int ray = 0; ray < rays; ray++){
        angle = angleChange * ray;
        angleCos = cos(angle);
        angleSin = sin(angle);
        powerRemaining = power - powerFallOff;
        // step along direction of ray
        for (int length = 0; length < travel; length++){
            xChange = angleCos * length;
            yChange = angleSin * length;
            newBlockPos = (BlockVector2){ (int)(origin.x + 0.5f + xChange), (int)(origin.y + 0.5f + yChange) };
            if (!isInBounds(newBlockPos)){ // ensure detected block is within game bounds
                break;
            }
            // reduce explosion power based on block resistance
            powerRemaining -= (getBlockType(world, newBlockPos)->blastResist + powerFallOff);
            if (powerRemaining <= 0.0f){ // ensure explosion has enough power to continue
                break;
            }
            Block* newBlock = &(world->blocks[newBlockPos.x][newBlockPos.y]);
            if (!newBlock->inExplosionQueue && !newBlock->type.isAir){ // ensure block is not air or already triggered
                newBlock->inExplosionQueue = true;
                affectedBlocks[arrIndex] = newBlock;
                arrIndex += 1;
            }
            
        }
        
    }
    // Preform explosion/replace all detected blocks with air
    // printf("Destroying blocks (%d affected)...\n", arrIndex);
    for(int i = 0; i < arrIndex; i++){
        affectedBlocks[i]->type = AIR;
        affectedBlocks[i]->inExplosionQueue = false;
    }
    free(affectedBlocks);

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
        BlockVector2 mouseTarget = getBlockPos(GetMousePosition());
        // user input
        if (IsKeyPressed(KEY_D) && checkAdjacent(&world, mouseTarget, &isBlock, 0)){ // Destroy
            changeBlock(&world, mouseTarget, AIR);
        }
        if (IsKeyPressed(KEY_B) && getBlockType(&world, mouseTarget)->isAir && checkAdjacent(&world, mouseTarget, &isBlockNot, 0)){ // Build
            changeBlock(&world, mouseTarget, STONE);
        }
        // Render
        BeginDrawing();
        ClearBackground(RAYWHITE);
        drawWorld(&world);
        if(IsKeyPressed(KEY_R)) explosion(&world, mouseTarget, 2.0f, 40);
        drawBlockOutline(mouseTarget);
        // debug text
        DrawText(TextFormat("world width (x): %d", world.width), 10, 10, 20, BLACK);
        DrawText(TextFormat("world height (y): %d", world.height), 10, 40, 20, BLACK);
        DrawText(TextFormat("Cursor block position: %d (x), %d (y)", mouseTarget.x, mouseTarget.y), 10, 70, 20, BLACK);
        DrawText("Press 'B' to build", 400, 10, 20, GRAY);
        DrawText("Press 'D' to destroy", 400, 40, 20, GRAY);
        DrawText("Press 'R' to spawn explosion", 400, 70, 20, GRAY);
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