#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "board.h"

#define TOP_GOLD 30  // index the top gold cell
#define BTM_GOLD 211 // index of the bottom gold cell

#define IS_CONNECTION(connections, j) ((connections & 1 << j) != 0)

#define FNV_PRIME 16777619UL
#define FNV_SEED 2166136261UL

static inline uint32_t fnv1a(uint8_t b, uint32_t hash)
{
    return (b ^ hash) * FNV_PRIME;
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

static inline uint8_t num_virt_cells_for_depth(uint8_t depth)
{
    if (depth <= 11)
        return 2 * depth - 1;
    else
        return 21 - 2 * (depth - 12);
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

Cell Cell_neighbor(Cell cell, uint8_t neighbor)
{
    switch (neighbor)
    {
    case 0: // right
        return (Cell){.idx = cell.idx + 1, .depth = cell.depth};
    case 1:
        return (Cell){.idx = cell.idx - num_virt_cells_to_top_neigbor(cell.depth) + 1, .depth = cell.depth - 1};
    case 2: // top
        return (Cell){.idx = cell.idx - num_virt_cells_to_top_neigbor(cell.depth), .depth = cell.depth - 1};
    case 3:
        return (Cell){.idx = cell.idx - num_virt_cells_to_top_neigbor(cell.depth) - 1, .depth = cell.depth - 1};
    case 4: // left
        return (Cell){.idx = cell.idx - 1, .depth = cell.depth};
    case 5:
        return (Cell){.idx = cell.idx + num_virt_cells_to_btm_neigbor(cell.depth) - 1, .depth = cell.depth + 1};
    case 6: // bottom
        return (Cell){.idx = cell.idx + num_virt_cells_to_btm_neigbor(cell.depth), .depth = cell.depth + 1};
    case 7:
        return (Cell){cell.idx + num_virt_cells_to_btm_neigbor(cell.depth) + 1, .depth = cell.depth + 1};
    default:
        printf("Error in get_neighbor_idx, cell_idx:%u, cell_depth:%u, neighbor:%u\n", cell.idx, cell.depth, neighbor);
        exit(1);
    }
}

static uint32_t Cells_hash(Cells *cells, uint32_t hash)
{
    for (size_t i = 0; i < cells->size; i++)
        hash = fnv1a(cells->data[i].idx, hash);
    return hash;
}

static inline bool Cell_isempty(Cell cell)
{
    return cell.depth == 0;
}

static inline bool Board_isempty(Board *board)
{
    return board->round_to_play == 0;
}

Board Board_stack_allocate_board(Cell *cell_0, Cell *cell_1)
{
    Cells cells_0 = (Cells){.size = 0, .data = cell_0};
    Cells cells_1 = (Cells){.size = 0, .data = cell_1};
    return (Board){
        .player_cells = {cells_0, cells_1},
        .player_to_play = 1,
        .round_to_play = 1,
    };
}

bool Board_cmp(Board *b1, Board *b2)
{
    if (b1->player_cells[0].size != b2->player_cells[0].size || b1->player_cells[1].size != b2->player_cells[1].size ||
        b1->round_to_play != b2->round_to_play)
        return false;
    bool equal = true;
    for (size_t i = 0; i < b1->player_cells[0].size; i++)
        equal = equal && (b1->player_cells[0].data[i].idx == b2->player_cells[0].data[i].idx);
    for (size_t i = 0; i < b1->player_cells[1].size; i++)
        equal = equal && (b1->player_cells[1].data[i].idx == b2->player_cells[1].data[i].idx);
    return equal;
}

uint32_t Board_hash(Board *board)
{
    uint32_t hash = FNV_SEED;
    hash = Cells_hash(&board->player_cells[0], hash);
    hash = Cells_hash(&board->player_cells[1], hash);
    hash = fnv1a(board->round_to_play, hash);
    return hash;
}

void Cells_append(Cells *cells, Cell cell)
{
    if (cells->size == NUM_CELLS)
    {
        printf("Cells_append: got a Cells vector of size NUM_CELLS\n");
        exit(1);
    }
    cells->data[cells->size++] = cell;
}

bool Cells_isin(Cells cells, Cell cell)
{
    for (size_t i = 0; i < cells.size; i++)
    {
        if (cells.data[i].idx == cell.idx)
            return true;
    }
    return false;
}

static CellsAllocator CellsAllocator_init()
{
    size_t default_num_vecs = 1 << 22;
    Cell *arr = calloc(default_num_vecs * NUM_CELLS, sizeof(Cell));
    return (CellsAllocator){.size = 0, .capacity = default_num_vecs, .arr = arr};
}

static void CellsAllocator_reset(CellsAllocator allocator)
{
    memset(allocator.arr, 0, sizeof(Cell) * NUM_CELLS * allocator.size);
}

static void CellsAllocator_destroy(CellsAllocator allocator)
{
    free(allocator.arr);
}

static Cells CellsAllocator_create_cells(CellsAllocator *allocator)
{
    if (allocator->size == allocator->capacity)
    {
        printf("CellsAllocator_create_cells: ran out of memory\n");
        exit(1);
    }
    Cell *cells = &allocator->arr[allocator->size * NUM_CELLS];
    allocator->size++;
    return (Cells){.size = 0, .data = cells};
}

static void Moves_add_cell(Moves *moves, Cell cell)
{
    for (size_t i = 0; i < moves->size; i++)
    {
        if (cell.idx == moves->moves[i].idx)
            return;
    }
    moves->moves[moves->size++] = cell;
}

static void Moves_add_neighbor_cells(GameConfig *config, Board board, Moves *moves, Cell cell)
{
    uint8_t connections = config->cell_connections[cell.idx];
    for (uint8_t j = 0; j < 8; j++)
    {
        if (IS_CONNECTION(connections, j))
        {
            Cell neighbor = Cell_neighbor(cell, j);
            if (!Cells_isin(board.player_cells[0], neighbor) && !Cells_isin(board.player_cells[1], neighbor))
                Moves_add_cell(moves, neighbor);
        }
    }
}

Moves get_valid_moves(GameConfig *config, Board board)
{
    Moves moves = {0};
    if (board.round_to_play == 1)
    {
        // Any free cell on the board can be played
        for (size_t i = 0; i < NUM_CELLS; i++)
        {
            Cell cell = config->all_cells[i];
            if (!Cells_isin(board.player_cells[0], cell) && !Cells_isin(board.player_cells[1], cell) &&
                cell.idx != TOP_GOLD && cell.idx != BTM_GOLD)
                Moves_add_cell(&moves, cell);
        }
    }
    else
    {
        // For rounds 2+, gold cell neighbors can also be played
        for (size_t i = 0; i < 2; i++)
        {
            Cell gold_cell = config->gold_cells[i];
            Moves_add_neighbor_cells(config, board, &moves, gold_cell);
        }
        size_t player_idx = board.player_to_play;
        for (size_t i = 0; i < board.player_cells[player_idx].size; i++)
        {
            Cell cell = board.player_cells[player_idx].data[i];
            Moves_add_neighbor_cells(config, board, &moves, cell);
        }
    }
    return moves;
}

BoardCache BoardCache_init()
{
    size_t default_capacity = 1 << 20;
    Board *boards = calloc(default_capacity, sizeof(Board));
    return (BoardCache){
        .allocator = CellsAllocator_init(),
        .size = 0,
        .capacity = default_capacity,
        .boards = boards,
    };
}

Board BoardCache_get_or_create(BoardCache *boardcache, Board board)
{
    if (boardcache->size == boardcache->capacity)
    {
        printf("BoardCache_get_or_create: ran out of memory\n");
        exit(1);
    }
    uint32_t index = Board_hash(&board) & (boardcache->capacity - 1);
    // Linear probing
    for (;;)
    {
        Board entry = boardcache->boards[index];
        if (Board_isempty(&entry))
        {
            Board new_entry = (Board){
                .player_cells = {CellsAllocator_create_cells(&boardcache->allocator),
                                 CellsAllocator_create_cells(&boardcache->allocator)},
                .player_to_play = board.player_to_play,
                .round_to_play = board.round_to_play,
            };
            for (size_t player_idx = 0; player_idx < 2; player_idx++)
            {
                Cells cells = board.player_cells[player_idx];
                new_entry.player_cells[player_idx].size = cells.size;
                for (size_t i = 0; i < cells.size; i++)
                    new_entry.player_cells[player_idx].data[i] = cells.data[i];
            }
            boardcache->boards[index] = new_entry;
            boardcache->size++;
            return new_entry;
        }
        else if (Board_cmp(&board, &entry))
            return entry;
        index++;
        if (index == boardcache->capacity)
            index = 0;
    }
}

void BoardCache_reset(BoardCache *boardcache)
{
    memset(boardcache->boards, 0, boardcache->size * sizeof(Board));
    CellsAllocator_reset(boardcache->allocator);
}

void BoardCache_destroy(BoardCache *boardcache)
{
    free(boardcache->boards);
    CellsAllocator_destroy(boardcache->allocator);
}

Board Board_play_move(BoardCache *boardcache, Board board, Cell cell)
{
    Cell cell_0[NUM_CELLS];
    Cell cell_1[NUM_CELLS];
    Board new_board = Board_stack_allocate_board(cell_0, cell_1);
    uint8_t new_player_idx = incr_player(board.player_to_play);
    new_board.player_to_play = new_player_idx;
    new_board.round_to_play = incr_round(new_player_idx, board.round_to_play);
    for (size_t i = 0; i < 2; i++)
    {
        Cells player_cells = board.player_cells[i];
        new_board.player_cells[i].size = player_cells.size;
        for (size_t j = 0; j < player_cells.size; j++)
            Cells_append(&new_board.player_cells[i], player_cells.data[j]);
    }
    Cells_append(&new_board.player_cells[board.player_to_play], cell);
    return BoardCache_get_or_create(boardcache, new_board);
}

bool won(GameConfig *config, Cells cells)
{
    // TODO: reduce the size of this
    Cell stack[NUM_CELLS];
    stack[0] = config->all_cells[TOP_GOLD];
    size_t stack_size = 1;
    Cell visited[NUM_CELLS];
    size_t visited_size = 0;
    while (stack_size > 0)
    {
        Cell cell = stack[stack_size - 1];
        stack_size--;
        visited[visited_size++] = cell;
        uint8_t connections = config->cell_connections[cell.idx];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (IS_CONNECTION(connections, j))
            {
                Cell neighbor = Cell_neighbor(cell, j);
                if (neighbor.idx == BTM_GOLD)
                    return true;
                else if (Cells_isin(cells, neighbor))
                {
                    bool already_visited = false;
                    for (size_t i = 0; i < visited_size; i++)
                    {
                        if (visited[i].idx == neighbor.idx)
                        {
                            already_visited = true;
                            break;
                        }
                    }
                    if (!already_visited)
                        stack[stack_size++] = neighbor;
                }
            }
        }
    }
    return false;
}

GameOutcome evaluate_outcome(GameConfig *config, Board board)
{
    size_t num_occupied_cells = board.player_cells[0].size + board.player_cells[1].size;
    return (GameOutcome){
        .player_lost = won(config, board.player_cells[0]),
        .player_won = won(config, board.player_cells[1]),
        .draw = num_occupied_cells >= NUM_CELLS,
    };
}
