#pragma once

#include <array>
#include <cstdint>
#include <cstdio>
#include <vector>

#include "board.h"

constexpr size_t kParentsMaxSize = kNumCells / 4;

struct Node
{
    u8 parents_size;
    std::array<Board *, kParentsMaxSize> parents;
    u32 visits;
    u32 wins;
    u32 losses;
    u32 draws;

    void add_parent(Board *board)
    {
        for (size_t i = 0; i < this->parents_size; i++)
        {
            if (*this->parents[i] == *board)
                return;
        }
        if (this->parents_size >= kParentsMaxSize)
        {
            printf("Parent size reached kParentsMaxSize\n");
            abort();
        }
        this->parents[this->parents_size++] = board;
    }

    float value() const
    {
        if (this->visits == 0)
            return 0;
        return (float)this->wins / (float)this->visits;
    }
};

struct NodeMapEntry
{
    Board *board;
    Node *node;
};

class NodeMap
{
  public:
    NodeMap();
    ~NodeMap() = default;
    NodeMapEntry get_or_create(Board board);
    u32 num_parent_visits(Node *node);

  private:
    std::vector<Board> board_memory_;
    std::vector<Node> node_memory_;
    size_t size_;
    size_t capacity_;
    std::vector<NodeMapEntry> entries_;
    Board *allocate_board(const Board &board);
    Node *allocate_node();
};