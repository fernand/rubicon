#include "node.h"

float calculate_value(NodeMap *nodecache, Board *parent_board, Board board)
{
    Node *node = NodeMap_get_or_create(nodecache, board).node;
    Node_add_parent(node, parent_board);
    if (node->visits == 0)
        return INFINITY;
    uint32_t parent_visits = num_parent_visits(nodecache, node);
    // TODO: Not sure why this sometimes happens
    //    if (node->visits > parent_visits)
    //    {
    //        printf("calculate_value: parent visits should be higher than node visits\n");
    //        exit(1);
    //    }
    float exploration_term = sqrtf(2.0f) * sqrtf(logf((float)parent_visits) / (float)node->visits);
    return Node_value(node) + exploration_term;
}

// Can return an empty cell in case no move can be chosen
int choose_move(GameConfig *config, NodeMap *nodecache, Board *board)
{
    float max_value = 0.0f;
    int best_move = -1;
    Moves moves = get_valid_moves(config, board);
    if (moves.size == 0)
    {
        // If you cannot move let's call it a draw
        return -1;
    }
    for (int i = 0; i < moves.size; i++)
    {
        Board child_board = play_move(board, (uint8_t)moves.moves[i]);
        Board *managed_child_board = NodeMap_get_or_create(nodecache, child_board).board;
        float value = calculate_value(nodecache, board, *managed_child_board);
        if (value > max_value)
        {
            max_value = value;
            best_move = moves.moves[i];
        }
    }
    return best_move;
}

void backpropagate(NodeMap *nodecache, Board **game_history, size_t game_size, GameOutcome outcome)
{
    for (size_t i = 0; i < game_size; i++)
    {
        Node *node = NodeMap_get_or_create(nodecache, *game_history[i]).node;
        node->visits++;
        if (outcome.player_lost)
            node->losses++;
        else if (outcome.player_won)
            node->wins++;
        else
            node->draws++;
    }
}

void playout(GameConfig *config, NodeMap *nodecache, Board board)
{
    GameOutcome outcome = {0};
    bool game_over = false;
    size_t game_size = 1;
    Board *game_history[NUM_CELLS];
    Board *current_board = NodeMap_get_or_create(nodecache, board).board;

    game_history[0] = current_board;
    outcome = evaluate_outcome(config, current_board);
    game_over = outcome.player_lost || outcome.player_won || outcome.draw;
    while (!game_over)
    {
        int move = choose_move(config, nodecache, current_board);
        if (move == -1)
            outcome.draw = true;
        else
        {
            Board next_board = play_move(current_board, (uint8_t)move);
            current_board = NodeMap_get_or_create(nodecache, next_board).board;
            game_history[game_size++] = current_board;
            outcome = evaluate_outcome(config, current_board);
        }
        game_over = outcome.player_lost || outcome.player_won || outcome.draw;
    }
    backpropagate(nodecache, game_history, game_size, outcome);
}

int mcts_move(GameConfig *config, NodeMap *nodecache, Board board)
{
    float max_value = 0.0f;
    int best_move = -1;
    Moves moves = get_valid_moves(config, &board);
    if (moves.size == 0)
    {
        printf("choose_move: no valid moves found\n");
        exit(1);
    }
    for (size_t i = 0; i < moves.size; i++)
    {
        Board child_board = play_move(&board, moves.moves[i]);
        Node *child_node = NodeMap_get_or_create(nodecache, child_board).node;
        float value = Node_value(child_node);
        if (value > max_value)
        {
            max_value = value;
            best_move = moves.moves[i];
        }
    }
    uint8_t depth = best_move == -1 ? 0 : depth_for_cell[(uint8_t)best_move];
    printf("Best move: %i, depth:%u, value:%f\n", best_move, depth, max_value);
    return best_move;
}

void playouts(GameConfig *config, NodeMap *nodecache, size_t num_playouts, Board board)
{
    for (size_t i = 0; i < num_playouts; i++)
        playout(config, nodecache, board);
    int best_move = mcts_move(config, nodecache, board);
}
