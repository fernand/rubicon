#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_DEPTH 22
#define NUM_CELLS 238
#define NUM_CANDITATE_CELLS 242

typedef struct Cell
{
    // A cell index is a number from 0 to NUM_CANDIDATE_CELLS - 1
    // 4 of these possible values are invalid
    uint8_t idx;
    // Depth is necessary for inferring cell neighbor indices
    // Also useful to check whether higher level structs are empty (zero-initialized)
    uint8_t depth;
} Cell;

typedef struct Cells
{
    size_t size;
    // Memory centrally managed by CellsAllocator
    Cell *data;
} Cells;

typedef struct Board
{
    Cells player_cells[2];
    uint8_t player_to_play; // 0 or 1, 0 is the AI who starts
    uint8_t round_to_play;  // 1 to 6
} Board;

typedef struct CellsAllocator
{
    size_t size;
    size_t capacity;
    Cell *arr;
} CellsAllocator;

typedef struct GameConfig
{
    // Connections follow the unit circle convention, going counter-clockwise
    // 8 potential connections per byte, smallest bit being (1, 0) on the unit circle
    uint8_t cell_connections[NUM_CANDITATE_CELLS];
    Cell all_cells[NUM_CELLS]; // Used to get all the real cell indices
    Cell gold_cells[2];        // Index 0 is the top one, index 1 is the bottom one
} GameConfig;

typedef struct Moves
{
    uint8_t size;
    Cell moves[NUM_CELLS];
} Moves;

typedef struct BoardCache
{
    CellsAllocator allocator;
    size_t size;
    size_t capacity;
    Board *boards;
} BoardCache;

typedef struct GameOutcome
{
    bool player_lost;
    bool player_won;
    bool draw;
} GameOutcome;
