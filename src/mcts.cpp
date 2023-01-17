#include "board.h"
#include "node_map.h"

#include <cmath>
#include <vector>

float calculate_value(NodeMap &map, Board *parent_board, NodeMapEntry entry)
{
    Node *node = entry.node;
    node->add_parent(parent_board);
    if (node->visits == 0)
        return INFINITY;
    u32 parent_visits = map.get_or_create(*parent_board).node->visits;
    float exploration_term = sqrtf(2.0f) * sqrtf(logf((float)parent_visits) / (float)node->visits);
    return node->value() + exploration_term;
}

// Can return an empty cell in case no move can be chosen
int choose_move(const GameConfig &config, NodeMap &map, Board *board)
{
    float max_value = 0.0f;
    int best_move = -1;
    Moves moves = board->get_valid_moves(config);
    if (moves.size == 0)
    {
        // If you cannot move let's call it a draw
        return -1;
    }
    for (size_t i = 0; i < moves.size; i++)
    {
        Board child_board = board->play_move(moves.moves[i]);
        NodeMapEntry entry = map.get_or_create(child_board);
        float value = calculate_value(map, board, entry);
        if (value > max_value)
        {
            max_value = value;
            best_move = (int)moves.moves[i].idx;
        }
    }
    return best_move;
}

using GameHistory = std::vector<Board *>;

void backpropagate(NodeMap &map, const GameHistory &history, GameOutcome outcome)
{
    for (size_t i = 0; i < history.size(); i++)
    {
        Node *node = map.get_or_create(*history[i]).node;
        node->visits++;
        if (outcome.player_lost)
            node->losses++;
        else if (outcome.player_won)
            node->wins++;
        else
            node->draws++;
    }
}

void playout(const GameConfig &config, NodeMap &map, Board board)
{
    GameOutcome outcome{};
    GameHistory history;
    history.reserve(kNumCells);

    Board *current_board = map.get_or_create(board).board;
    history.push_back(current_board);
    outcome = current_board->evaluate_outcome(config);
    bool game_over = outcome.player_lost || outcome.player_won || outcome.draw;
    while (!game_over)
    {
        int move = choose_move(config, map, current_board);
        if (move == -1)
            outcome.draw = true;
        else
        {
            Board next_board = current_board->play_move(Cell{(u8)move});
            current_board = map.get_or_create(next_board).board;
            history.push_back(current_board);
            outcome = current_board->evaluate_outcome(config);
        }
        game_over = outcome.player_lost || outcome.player_won || outcome.draw;
    }
    backpropagate(map, history, outcome);
}

int mcts_move(GameConfig &config, NodeMap &map, Board board)
{
    float max_value = 0.0f;
    int best_move = -1;
    Moves moves = board.get_valid_moves(config);
    if (moves.size == 0)
    {
        printf("choose_move: no valid moves found\n");
        exit(1);
    }
    for (size_t i = 0; i < moves.size; i++)
    {
        Board child_board = board.play_move(moves.moves[i]);
        Node *child_node = map.get_or_create(child_board).node;
        float value = child_node->value();
        if (value > max_value)
        {
            max_value = value;
            best_move = (int)moves.moves[i].idx;
        }
    }
    u8 depth = best_move == -1 ? 0 : Board::kDepthForCell[(u8)best_move];
    u8 num_cells_before_depth = 0;
    for (u8 i = 0; i < depth; i++)
        num_cells_before_depth += Board::kVirtCellsForDepth[i];
    u8 depth_cell_idx = best_move - num_cells_before_depth + 1;
    printf("Best move: depth:%u depth_cell_idx:%u value:%f cell_idx:%u\n", depth, depth_cell_idx, max_value, best_move);
    return best_move;
}

void playouts(GameConfig &config, NodeMap &map, size_t num_playouts, Board board)
{
    for (size_t i = 0; i < num_playouts; i++)
        playout(config, map, board);
    int best_move = mcts_move(config, map, board);
}
