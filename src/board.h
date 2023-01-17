#pragma once

#include <array>
#include <cstddef>

#include "types.h"

constexpr size_t kNumCells = 238;
constexpr size_t kNumCandidateCells = 242;

struct Cell
{
    // A cell index is a number from 0 to NUM_CANDIDATE_CELLS - 1
    // 4 of these possible values are invalid
    u8 idx;
    inline bool operator==(const Cell &other) const
    {
        return this->idx == other.idx;
    }
};

struct Player
{
    u8 idx;
    inline bool operator==(const Player &other) const
    {
        return this->idx == other.idx;
    }
};

constexpr Player kAI = Player{0};
constexpr Player kPlayer = Player{1};

struct Round
{
    u8 num;
    inline bool operator==(const Round &other) const
    {
        return this->num == other.num;
    }
};

struct Neighbor
{
    u8 idx;
};

struct GameConfig
{
    // Connections follow the unit circle convention, going counter-clockwise
    // 8 potential connections per byte, smallest bit being (1, 0) on the unit circle
    std::array<u8, kNumCandidateCells> cell_connections;
    std::array<Cell, kNumCells> all_cells; // Used to get all the real cell indices
};

struct GameOutcome
{
    bool player_lost;
    bool player_won;
    bool draw;
};

struct Moves
{
    size_t size;
    std::array<Cell, kNumCells> moves;
};

using CellOcc = std::array<u8, kNumCandidateCells>;

class Board
{
  public:
    static const std::array<u8, 23> kVirtCellsForDepth;
    static const std::array<u8, kNumCandidateCells> kDepthForCell;

    Board();
    inline void occupy_cell(Player player, Cell cell);
    inline bool cell_occupied(Player player, Cell cell);
    uint64_t count_occupied_cells(Player player);
    CellOcc get_occupied_cells(Player player);

    inline void set_player_to_play(Player player);
    inline Player player_to_play();
    inline void set_round_to_play(Round to_play);
    inline Round round_to_play();
    static inline Round incr_round(Player new_player, Round round);
    static inline Player incr_player(Player player);

    static Cell get_neighbor(Cell cell, Neighbor neighbor);

    bool operator==(const Board &other)
    {
        bool equal = true;
        for (u8 p_idx = 0; p_idx < 2; p_idx++)
        {
            for (u8 f_idx = 0; f_idx < 4; f_idx++)
                equal = equal && (field_[p_idx][f_idx] == other.field_[p_idx][f_idx]);
        }
        return equal;
    }
    u32 hash();

    Board play_move(Cell cell);
    Moves get_valid_moves(const GameConfig &config);
    bool won(const GameConfig &config, Player player);
    GameOutcome evaluate_outcome(const GameConfig &config);

  private:
    // AI is index 0 and also has the player_to_play and round_to_play bits.
    // bits 0-45 are used for the last cells
    // bit 46 is the player to play
    // bits 47, 48, 49 are round to play
    std::array<std::array<uint64_t, 4>, 2> field_;
};