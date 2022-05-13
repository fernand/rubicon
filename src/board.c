#include "board.h"

#define TOP_GOLD 30  // index the top gold cell
#define BTM_GOLD 211 // index of the bottom gold cell

#define IS_CONNECTION(connections, j) ((connections & 1 << j) != 0)

#define PLAYER_IDX 50
#define ROUND_IDX_START 51

/*
if (depth <= 11)
    return 2 * depth - 1;
else
    return 21 - 2 * (depth - 12);
*/
const uint8_t virt_cells_for_depth[23] = {0,  1,  3,  5,  7,  9,  11, 13, 15, 17, 19, 21,
                                          21, 19, 17, 15, 13, 11, 9,  7,  5,  3,  1};

const uint8_t depth_for_cell[242] = {
    1,  2,  2,  2,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  6,  6,
    6,  6,  6,  6,  6,  6,  6,  6,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
    17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 21, 21, 21, 22,
};

static inline Board occupy_cell(uint64_t *field, uint8_t cell_idx)
{
    div_t quotrem = div((int)cell_idx, 64);
    field[quotrem.quot] = field[quotrem.quot] | (1UL << quotrem.rem);
}

static inline bool cell_occupied(const uint64_t *field, uint8_t cell_idx)
{
    div_t quotrem = div((int)cell_idx, 64);
    return (field[quotrem.quot] >> quotrem.rem) & 1UL == 1;
}

uint64_t count_occupied_cells(const uint64_t *field)
{
    uint64_t num_occupied_cells = 0;
    for (int f_idx = 0; f_idx < 4; f_idx++)
    {
        uint64_t n = field[f_idx];
        uint8_t max_idx = f_idx < 3 ? 64 : PLAYER_IDX;
        for (int i = 0; i < max_idx; i++)
        {
            num_occupied_cells += n & 1UL;
            n >>= 1;
        }
    }
    return num_occupied_cells;
}

uint8_t occupied_cells(const uint64_t *field, uint8_t *cells)
{
    uint8_t cells_size = 0;
    for (uint8_t f_idx = 0; f_idx < 4; f_idx++)
    {
        uint8_t max_idx = f_idx < 3 ? 64 : PLAYER_IDX;
        for (uint8_t i = 0; i < max_idx; i++)
        {
            uint8_t cell_idx = f_idx * 64 + i;
            if (cell_occupied(field, cell_idx))
                cells[cells_size++] = cell_idx;
        }
    }
    return cells_size;
}

static inline Board set_player_to_play(uint64_t *field, uint8_t player_to_play)
{
    field[3] = (field[3] & ~(1UL << PLAYER_IDX)) | ((uint64_t)player_to_play << PLAYER_IDX);
}

static inline uint8_t player_to_play(const uint64_t *field)
{
    return (field[3] >> PLAYER_IDX) & 1UL;
}

static inline Board set_round_to_play(uint64_t *field, uint8_t round_to_play)
{
    field[3] = (field[3] & ~(1UL << ROUND_IDX_START)) | (((uint64_t)round_to_play & 1UL) << ROUND_IDX_START);
    field[3] = (field[3] & ~(1UL << (ROUND_IDX_START + 1))) |
               ((((uint64_t)round_to_play >> 1) & 1UL) << (ROUND_IDX_START + 1));
    field[3] = (field[3] & ~(1UL << (ROUND_IDX_START + 2))) |
               ((((uint64_t)round_to_play >> 2) & 1UL) << (ROUND_IDX_START + 2));
}

static inline uint8_t round_to_play(const uint64_t *field)
{
    return (field[3] >> ROUND_IDX_START) & 7UL;
}

static inline uint8_t incr_round(uint8_t new_player_idx, uint8_t round)
{
    if (new_player_idx == 1)
        return round;
    else if (round < 6)
        return round + 1;
    else
        return 0;
}

static inline uint8_t incr_player(uint8_t player_idx)
{
    if (player_idx == 1)
        return 0;
    else
        return 1;
}

static inline uint8_t num_virt_cells_to_top_neigbor(uint8_t depth)
{
    if (depth <= 11)
    {
        uint8_t num_virt_cells_for_depth = 2 * depth - 1;
        return num_virt_cells_for_depth - 1;
    }
    else if (depth == 12)
        return 21;
    else
    {
        uint8_t num_virt_cells_for_depth = 21 - 2 * (depth - 12);
        return num_virt_cells_for_depth + 1;
    }
}

static inline uint8_t num_virt_cells_to_btm_neigbor(uint8_t depth)
{
    if (depth <= 10)
    {
        uint8_t num_virt_cells_for_depth = 2 * depth - 1;
        return num_virt_cells_for_depth + 1;
    }
    else if (depth == 11)
        return 21;
    else
    {
        uint8_t num_virt_cells_for_depth = 21 - 2 * (depth - 12);
        return num_virt_cells_for_depth - 1;
    }
}

uint8_t neighbor(uint8_t cell_idx, uint8_t neighbor)
{
    switch (neighbor)
    {
    case 0: // right
        return cell_idx + 1;
    case 1:
        return cell_idx - num_virt_cells_to_top_neigbor(depth_for_cell[cell_idx]) + 1;
    case 2: // top
        return cell_idx - num_virt_cells_to_top_neigbor(depth_for_cell[cell_idx]);
    case 3:
        return cell_idx - num_virt_cells_to_top_neigbor(depth_for_cell[cell_idx]) - 1;
    case 4: // left
        return cell_idx - 1;
    case 5:
        return cell_idx + num_virt_cells_to_btm_neigbor(depth_for_cell[cell_idx]) - 1;
    case 6: // bottom
        return cell_idx + num_virt_cells_to_btm_neigbor(depth_for_cell[cell_idx]);
    case 7:
        return cell_idx + num_virt_cells_to_btm_neigbor(depth_for_cell[cell_idx]) + 1;
    default:
        printf("Error in get_neighbor_idx, cell_idx:%u, neighbor:%u\n", cell_idx, neighbor);
        exit(1);
    }
}

bool cells_isin(const uint8_t *cells, size_t cells_size, uint8_t cell_idx)
{
    for (size_t i = 0; i < cells_size; i++)
    {
        if (cells[i] == cell_idx)
            return true;
    }
    return false;
}

bool board_cmp(Board b1, Board b2)
{
    bool equal = true;
    for (uint8_t p_idx = 0; p_idx < 2; p_idx++)
    {
        for (uint8_t f_idx = 0; f_idx < 4; f_idx++)
            equal = equal && (b1.field[p_idx][f_idx] == b2.field[p_idx][f_idx]);
    }
    return equal;
}

Board play_move(Board board, uint8_t cell_idx)
{
    Board new_board = board;
    uint8_t player_to_play_idx = player_to_play(board.field[0]);
    uint8_t new_player_idx = incr_player(player_to_play_idx);
    set_player_to_play(new_board.field[0], new_player_idx);
    set_round_to_play(new_board.field[0], incr_round(new_player_idx, round_to_play(board.field[0])));
    occupy_cell(new_board.field[player_to_play_idx], cell_idx);
    return new_board;
}

static void add_move(Moves *moves, uint8_t cell_idx)
{
    for (size_t i = 0; i < moves->size; i++)
    {
        if (cell_idx == moves->moves[i])
            return;
    }
    moves->moves[moves->size++] = cell_idx;
}

static void add_neighbor_cells(GameConfig *config, uint8_t *ai_cells, uint8_t ai_cells_size, uint8_t *player_cells,
                               uint8_t player_cells_size, Moves *moves, uint8_t cell_idx)
{
    uint8_t connections = config->cell_connections[cell_idx];
    for (uint8_t j = 0; j < 8; j++)
    {
        if (IS_CONNECTION(connections, j))
        {
            uint8_t neighbor_idx = neighbor(cell_idx, j);
            if (!cells_isin(ai_cells, ai_cells_size, neighbor_idx) &&
                !cells_isin(player_cells, player_cells_size, neighbor_idx))
                add_move(moves, neighbor_idx);
        }
    }
}

Moves get_valid_moves(GameConfig *config, Board board)
{
    Moves moves = {0};
    uint8_t ai_cells[NUM_CELLS / 2];
    uint8_t ai_cells_size = occupied_cells(board.field[0], ai_cells);
    uint8_t player_cells[NUM_CELLS / 2];
    uint8_t player_cells_size = occupied_cells(board.field[1], player_cells);
    if (round_to_play(board.field[0]) == 1)
    {
        // Any free cell on the board can be played
        for (size_t i = 0; i < NUM_CELLS; i++)
        {
            uint8_t cell_idx = config->all_cells[i];
            if (!cells_isin(ai_cells, ai_cells_size, cell_idx) &&
                !cells_isin(player_cells, player_cells_size, cell_idx) && cell_idx != TOP_GOLD && cell_idx != BTM_GOLD)
                add_move(&moves, cell_idx);
        }
    }
    else
    {
        // For rounds 2+, gold cell neighbors can also be played
        add_neighbor_cells(config, ai_cells, ai_cells_size, player_cells, player_cells_size, &moves, TOP_GOLD);
        add_neighbor_cells(config, ai_cells, ai_cells_size, player_cells, player_cells_size, &moves, BTM_GOLD);
        uint8_t player_idx = player_to_play(board.field[0]);
        uint8_t *cells;
        uint8_t cells_size;
        if (player_idx == 0)
        {
            cells = ai_cells;
            cells_size = ai_cells_size;
        }
        else
        {
            cells = player_cells;
            cells_size = player_cells_size;
        }
        for (uint8_t i = 0; i < cells_size; i++)
            add_neighbor_cells(config, ai_cells, ai_cells_size, player_cells, player_cells_size, &moves, cells[i]);
    }
    return moves;
}

bool won(GameConfig *config, uint64_t *field)
{
    uint8_t cells[NUM_CELLS];
    uint8_t cells_size = occupied_cells(field, cells);
    uint8_t stack[NUM_CELLS];
    stack[0] = config->all_cells[TOP_GOLD];
    uint8_t stack_size = 1;
    uint8_t visited[NUM_CELLS];
    uint8_t visited_size = 0;
    while (stack_size > 0)
    {
        uint8_t cell_idx = stack[stack_size - 1];
        stack_size--;
        visited[visited_size++] = cell_idx;
        uint8_t connections = config->cell_connections[cell_idx];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (IS_CONNECTION(connections, j))
            {
                uint8_t neighbor_idx = neighbor(cell_idx, j);
                if (neighbor_idx == BTM_GOLD)
                    return true;
                else if (cells_isin(cells, cells_size, neighbor_idx))
                {
                    bool already_visited = false;
                    for (uint8_t i = 0; i < visited_size; i++)
                    {
                        if (visited[i] == neighbor_idx)
                        {
                            already_visited = true;
                            break;
                        }
                    }
                    if (!already_visited)
                        stack[stack_size++] = neighbor_idx;
                }
            }
        }
    }
    return false;
}

GameOutcome evaluate_outcome(GameConfig *config, Board board)
{
    // Two occupied gold cells
    uint64_t num_occupied_cells = 2;
    for (int p_idx = 0; p_idx < 2; p_idx++)
    {
        num_occupied_cells += count_occupied_cells(board.field[p_idx]);
    }
    return (GameOutcome){
        .player_lost = won(config, board.field[0]),
        .player_won = won(config, board.field[1]),
        .draw = num_occupied_cells >= NUM_CELLS,
    };
}
