#include <cstdlib>

#include "board.h"

static constexpr Cell kTopGold = Cell{30};  // index the top gold cell
static constexpr Cell kBtmGold = Cell{211}; // index of the bottom gold cell

#define IS_CONNECTION(connections, j) ((connections & 1 << j) != 0)

static constexpr uint64_t kPlayerIdx = 50;
static constexpr uint64_t kRoundIdxStart = kPlayerIdx + 1;

/*
if (depth <= 11)
    return 2 * depth - 1;
else
    return 21 - 2 * (depth - 12);
*/
const std::array<u8, 23> Board::kVirtCellsForDepth = {0,  1,  3,  5,  7,  9,  11, 13, 15, 17, 19, 21,
                                                      21, 19, 17, 15, 13, 11, 9,  7,  5,  3,  1};

const std::array<u8, kNumCandidateCells> Board::kDepthForCell = {
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

Board::Board() : field_{}
{
    set_round_to_play(Round{1});
    set_player_to_play(kPlayer);
    occupy_cell(kAI, Cell{131});
}

inline void Board::occupy_cell(Player player, Cell cell)
{
    div_t quotrem = div((int)cell.idx, 64);
    field_[player.idx][quotrem.quot] = field_[player.idx][quotrem.quot] | (1UL << quotrem.rem);
}

inline bool Board::cell_occupied(Player player, Cell cell)
{
    div_t quotrem = div((int)cell.idx, 64);
    return ((field_[player.idx][quotrem.quot] >> quotrem.rem) & 1UL) == 1;
}

uint64_t Board::count_occupied_cells(Player player)
{
    uint64_t num_occupied_cells = 0;
    for (int f_idx = 0; f_idx < 4; f_idx++)
    {
        uint64_t n = field_[player.idx][f_idx];
        u8 max_idx = f_idx < 3 ? 64 : kPlayerIdx;
        for (int i = 0; i < max_idx; i++)
        {
            num_occupied_cells += n & 1UL;
            n >>= 1;
        }
    }
    return num_occupied_cells;
}

CellOcc Board::get_occupied_cells(Player player)
{
    CellOcc cells{};
    for (u8 f_idx = 0; f_idx < 4; f_idx++)
    {
        u8 max_idx = f_idx < 3 ? 64 : kPlayerIdx;
        for (u8 i = 0; i < max_idx; i++)
        {
            u8 cell_idx = f_idx * 64 + i;
            if (cell_occupied(player, Cell{cell_idx}))
                cells[cell_idx] = true;
        }
    }
    return cells;
}

inline void Board::set_player_to_play(Player player)
{
    field_[0][3] = (field_[0][3] & ~(1UL << kPlayerIdx)) | ((uint64_t)player.idx << kPlayerIdx);
}

inline Player Board::player_to_play()
{
    uint64_t player_idx = (field_[0][3] >> kPlayerIdx) & 1UL;
    return Player{(u8)player_idx};
}

inline void Board::set_round_to_play(Round to_play)
{
    field_[0][3] = (field_[0][3] & ~(1UL << kRoundIdxStart)) | (((uint64_t)to_play.num & 1UL) << kRoundIdxStart);
    field_[0][3] = (field_[0][3] & ~(1UL << (kRoundIdxStart + 1))) |
                   ((((uint64_t)to_play.num >> 1) & 1UL) << (kRoundIdxStart + 1));
    field_[0][3] = (field_[0][3] & ~(1UL << (kRoundIdxStart + 2))) |
                   ((((uint64_t)to_play.num >> 2) & 1UL) << (kRoundIdxStart + 2));
}

inline Round Board::round_to_play()
{
    uint64_t round_num = (field_[0][3] >> kRoundIdxStart) & 7UL;
    return Round{(u8)round_num};
}

inline Round Board::incr_round(Player new_player, Round round)
{
    if (new_player.idx == 1)
        return round;
    else if (round.num < 6)
        return Round{(u8)(round.num + 1U)};
    else
        return Round{0};
}

inline Player Board::incr_player(Player player)
{
    if (player.idx == 1)
        return Player{0};
    else
        return Player{1};
}

static inline u8 num_virt_cells_to_top_neighbor(u8 depth)
{
    if (depth <= 11)
    {
        u8 num_virt_cells_for_depth = 2 * depth - 1;
        return num_virt_cells_for_depth - 1;
    }
    else if (depth == 12)
        return 21;
    else
    {
        u8 num_virt_cells_for_depth = 21 - 2 * (depth - 12);
        return num_virt_cells_for_depth + 1;
    }
}
static inline u8 num_virt_cells_to_btm_neighbor(u8 depth)
{
    if (depth <= 10)
    {
        u8 num_virt_cells_for_depth = 2 * depth - 1;
        return num_virt_cells_for_depth + 1;
    }
    else if (depth == 11)
        return 21;
    else
    {
        u8 num_virt_cells_for_depth = 21 - 2 * (depth - 12);
        return num_virt_cells_for_depth - 1;
    }
}
Cell Board::get_neighbor(Cell cell, Neighbor neighbor)
{
    switch (neighbor.idx)
    {
    case 0: // right
        return Cell{(u8)(cell.idx + 1U)};
    case 1:
        return Cell{(u8)(cell.idx - num_virt_cells_to_top_neighbor(kDepthForCell[cell.idx]) + 1)};
    case 2: // top
        return Cell{(u8)(cell.idx - num_virt_cells_to_top_neighbor(kDepthForCell[cell.idx]))};
    case 3:
        return Cell{(u8)(cell.idx - num_virt_cells_to_top_neighbor(kDepthForCell[cell.idx]) - 1)};
    case 4: // left
        return Cell{(u8)(cell.idx - 1)};
    case 5:
        return Cell{(u8)(cell.idx + num_virt_cells_to_btm_neighbor(kDepthForCell[cell.idx]) - 1)};
    case 6: // bottom
        return Cell{(u8)(cell.idx + num_virt_cells_to_btm_neighbor(kDepthForCell[cell.idx]))};
    case 7:
        return Cell{(u8)(cell.idx + num_virt_cells_to_btm_neighbor(kDepthForCell[cell.idx]) + 1)};
    }
}

Board Board::play_move(Cell cell)
{
    Board new_board{};
    Player to_play = player_to_play();
    Player new_player = incr_player(to_play);
    new_board.occupy_cell(to_play, cell);
    new_board.set_player_to_play(new_player);
    new_board.set_round_to_play(incr_round(new_player, round_to_play()));
    return new_board;
}

static void add_move(Moves &moves, Cell cell)
{
    for (size_t i = 0; i < moves.size; i++)
    {
        if (cell == moves.moves[i])
            return;
    }

    moves.moves[moves.size++] = cell;
}

static void add_neighbor_cells(const GameConfig &config, const CellOcc &ai_cells, const CellOcc &player_cells,
                               Moves &moves, Cell cell)
{
    u8 connections = config.cell_connections[cell.idx];
    for (u8 j = 0; j < 8; j++)
    {
        if (IS_CONNECTION(connections, j))
        {
            Cell neighbor = Board::get_neighbor(cell, Neighbor{j});
            if (ai_cells[neighbor.idx] == 0 && player_cells[neighbor.idx] == 0)
                add_move(moves, neighbor);
        }
    }
}

Moves Board::get_valid_moves(const GameConfig &config)
{
    Moves moves{};
    CellOcc ai_cells = get_occupied_cells(kAI);
    CellOcc player_cells = get_occupied_cells(kPlayer);
    if (round_to_play() == Round{1})
    {
        // Any free cell on the board can be played
        for (size_t i = 0; i < kNumCells; i++)
        {
            Cell cell = config.all_cells[i];
            if (ai_cells[cell.idx] == 0 && player_cells[cell.idx] == 0 && cell != kTopGold && cell != kBtmGold)
                add_move(moves, cell);
        }
    }
    else
    {
        // For rounds 2+, gold cell neighbors can also be played
        add_neighbor_cells(config, ai_cells, player_cells, moves, kTopGold);
        add_neighbor_cells(config, ai_cells, player_cells, moves, kBtmGold);
        Player player = player_to_play();
        CellOcc cells;
        if (player == kAI)
            cells = ai_cells;
        else
            cells = player_cells;
        for (u8 i = 0; i < kNumCandidateCells; i++)
        {
            if (cells[i] == 1)
                add_neighbor_cells(config, ai_cells, player_cells, moves, Cell{i});
        }
    }
    return moves;
}

// TODO: Make my own stack vector class.
bool Board::won(const GameConfig &config, Player player)
{
    CellOcc cells = get_occupied_cells(player);
    std::array<Cell, kNumCells> stack;
    stack[0] = kTopGold;
    u8 stack_size = 1;
    std::array<Cell, kNumCells> visited;
    u8 visited_size = 0;
    while (stack_size > 0)
    {
        Cell cell = stack[stack_size - 1];
        stack_size--;
        visited[visited_size++] = cell;
        u8 connections = config.cell_connections[cell.idx];
        for (u8 j = 0; j < 8; j++)
        {
            if (IS_CONNECTION(connections, j))
            {
                Cell neighbor = get_neighbor(cell, Neighbor{j});
                if (neighbor == kBtmGold)
                    return true;
                else if (cells[neighbor.idx])
                {
                    bool already_visited = false;
                    for (u8 i = 0; i < visited_size; i++)
                    {
                        if (visited[i] == neighbor)
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

GameOutcome Board::evaluate_outcome(const GameConfig &config)
{
    // Two occupied gold cells
    uint64_t num_occupied_cells = 2;
    num_occupied_cells += count_occupied_cells(kAI);
    num_occupied_cells += count_occupied_cells(kPlayer);
    return GameOutcome{
        .player_lost = won(config, kAI),
        .player_won = won(config, kPlayer),
        .draw = num_occupied_cells >= kNumCells,
    };
}

static constexpr u32 kFNVPrime = 16777619;
static constexpr u32 kFNVSeed = 2166136261;

inline u32 fnv1a(u8 b, u32 hash)
{
    return (b ^ hash) * kFNVPrime;
}

u32 Board::hash()
{
    u32 hash = kFNVSeed;
    for (u8 p_idx = 0; p_idx < 2; p_idx++)
    {
        for (u8 f_idx = 0; f_idx < 4; f_idx++)
        {
            u8 *bytes = (u8 *)&field_[p_idx][f_idx];
            for (u8 b_idx = 0; b_idx < (u8)sizeof(uint64_t); b_idx++)
                hash = fnv1a(bytes[b_idx], hash);
        }
    }
    return hash;
}