#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "node.h"

float calculate_value(BoardCache *boardcache, NodeMap *nodecache, Board parent_board, Board board)
{
    Node *node = NodeMap_get_or_create(nodecache, board);
    Node_add_parent(node, parent_board);
    if (node->visits == 0)
        return INFINITY;
    uint32_t parent_visits = num_parent_visits(boardcache, nodecache, board, node);
    if (node->visits > parent_visits)
    {
        printf("calculate_value: parent visits should be higher than node visits\n");
        exit(1);
    }
    float exploration_term = sqrtf(2.0f) * sqrtf(logf((float)parent_visits) / node->visits);
    return Node_value(node) + exploration_term;
}

Cell choose_move(GameConfig *config, BoardCache *boardcache, NodeMap *nodecache, Board board)
{
    float max_value = 0.0f;
    Cell best_move = {0};
    Moves moves = get_valid_moves(config, board);
    if (moves.size == 0)
    {
        printf("choose_move: no valid moves found\n");
        exit(1);
    }
    for (size_t i = 0; i < moves.size; i++)
    {
        Board child_board = Board_play_move(boardcache, board, moves.moves[i]);
        float value = calculate_value(boardcache, nodecache, board, child_board);
        if (value > max_value)
        {
            max_value = value;
            best_move = moves.moves[i];
        }
    }
    return best_move;
}

void backpropagate(NodeMap *nodecache, Board *game_history, size_t game_size, GameOutcome outcome)
{
    for (size_t i = 0; i < game_size; i++)
    {
        Node *node = NodeMap_get_or_create(nodecache, game_history[i]);
        node->visits++;
        if (outcome.player_lost)
            node->losses++;
        else if (outcome.player_won)
            node->wins++;
        else
            node->draws++;
    }
}

void playout(GameConfig *config, BoardCache *boardcache, NodeMap *nodecache, Board board)
{
    GameOutcome outcome = {0};
    bool game_over = false;
    size_t game_size = 1;
    Board game_history[NUM_CELLS];
    Board current_board = board;

    game_history[0] = current_board;
    outcome = evaluate_outcome(config, current_board);
    game_over = outcome.player_lost || outcome.player_won || outcome.draw;
    while (!game_over)
    {
        Cell move = choose_move(config, boardcache, nodecache, current_board);
        Board next_board = Board_play_move(boardcache, current_board, move);
        game_history[game_size++] = next_board;
        current_board = next_board;
        outcome = evaluate_outcome(config, current_board);
        game_over = outcome.player_lost || outcome.player_won || outcome.draw;
    }
    backpropagate(nodecache, game_history, game_size, outcome);
}

Cell mcts_move(GameConfig *config, BoardCache *boardcache, NodeMap *nodecache, Board board)
{
    float max_value = 0.0f;
    Cell best_move = {0};
    Moves moves = get_valid_moves(config, board);
    if (moves.size == 0)
    {
        printf("choose_move: no valid moves found\n");
        exit(1);
    }
    for (size_t i = 0; i < moves.size; i++)
    {
        Board child_board = Board_play_move(boardcache, board, moves.moves[i]);
        Node *node = NodeMap_get_or_create(nodecache, board);
        float value = Node_value(node);
        if (value > max_value)
        {
            max_value = value;
            best_move = moves.moves[i];
        }
    }
    printf("Best move: %u, depth:%u, value:%f\n", best_move.idx, best_move.depth, max_value);
    return best_move;
}

void playouts(GameConfig *config, size_t num_playouts, Board board)
{
    // TODO: don't reallocate stuff for every playout
    // Just memset the memory
    BoardCache boardcache = BoardCache_init();
    NodeMap nodecache = NodeMap_init();
    Board managed_board = BoardCache_get_or_create(&boardcache, board);
    for (size_t i = 0; i < num_playouts; i++)
        playout(config, &boardcache, &nodecache, managed_board);
    Cell best_move = mcts_move(config, &boardcache, &nodecache, managed_board);
    BoardCache_destroy(boardcache);
    NodeMap_destroy(nodecache);
}
