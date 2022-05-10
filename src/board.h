#pragma once

#define MAX_DEPTH 22
#define NUM_CELLS 238
#define NUM_CANDITATE_CELLS 242

typedef struct Board
{
    // A cell index is a number from 0 to NUM_CANDIDATE_CELLS - 1
    // 4 of these possible values are invalid

    // AI is index 0 and also has the player_to_play and round_to_play bits.
    // bits 0-45 are used for the last cells
    // bit 46 is the player to play
    // bits 47, 48, 49 are round to play
    uint64_t field[2][4];
} Board;

typedef struct GameConfig
{
    // Connections follow the unit circle convention, going counter-clockwise
    // 8 potential connections per byte, smallest bit being (1, 0) on the unit circle
    uint8_t cell_connections[NUM_CANDITATE_CELLS];
    uint8_t all_cells[NUM_CELLS]; // Used to get all the real cell indices
} GameConfig;

typedef struct Moves
{
    uint8_t size;
    int moves[NUM_CELLS];
} Moves;

typedef struct GameOutcome
{
    bool player_lost;
    bool player_won;
    bool draw;
} GameOutcome;
