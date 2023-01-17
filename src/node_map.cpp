#include "node_map.h"

constexpr size_t kNumBoards = (size_t)(1.3f * (float)(1UL << 25));
constexpr size_t kMapCapacity = 1UL << 25;

NodeMap::NodeMap() : board_memory_(), node_memory_(), size_(0), capacity_(kMapCapacity), entries_(kMapCapacity)
{
    board_memory_.reserve(kNumBoards);
    node_memory_.reserve(kNumBoards);
    printf("Board memory: %f MB\n", (float)kNumBoards * sizeof(Board) / 1e6);
    printf("Node memory: %f MB\n", (float)kNumBoards * sizeof(Node) / 1e6);
    printf("NodeMap memory: %f MB\n", (float)kMapCapacity * sizeof(NodeMapEntry) / 1e6);
}

Board *NodeMap::allocate_board(const Board &board)
{
    if (board_memory_.size() == board_memory_.capacity())
    {
        printf("Ran out of board memory\n");
        abort();
    }
    board_memory_.emplace_back(board);
    return &board_memory_[board_memory_.size() - 1];
}

Node *NodeMap::allocate_node()
{
    if (node_memory_.size() == node_memory_.capacity())
    {
        printf("Ran out of node memory\n");
        abort();
    }
    node_memory_.emplace_back(Node{});
    return &node_memory_[node_memory_.size() - 1];
}

NodeMapEntry NodeMap::get_or_create(Board board)
{
    if (size_ == capacity_)
    {
        printf("NodeMap: ran out of memory\n");
        abort();
    }
    size_t index = board.hash() & (capacity_ - 1);
    // Linear probing
    for (;;)
    {
        NodeMapEntry entry = entries_[index];
        if (entry.board == nullptr)
        {
            Board *managed_board = allocate_board(board);
            *managed_board = board;
            Node *managed_node = allocate_node();
            NodeMapEntry new_entry = NodeMapEntry{.board = managed_board, .node = managed_node};
            entries_[index] = new_entry;
            size_++;
            return new_entry;
        }
        else if (board == *entry.board)
            return entries_[index];
        index++;
        if (index == capacity_)
            index = 0;
    }
}

u32 NodeMap::num_parent_visits(Node *node)
{
    u32 num_visits = 0;
    for (size_t i = 0; i < node->parents_size; i++)
    {
        Board *parent_board = node->parents[i];
        Node *parent_node = get_or_create(*parent_board).node;
        num_visits += parent_node->visits;
    }
    return num_visits;
}