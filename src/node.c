#include <stdio.h>
#include <stdlib.h>

#include "node.h"

float Node_value(Node *node)
{
    if (node->visits == 0)
        return 0.0f;
    return (float)(node->wins + node->draws) / node->visits;
}

void Node_add_parent(Node *node, Board board)
{
    for (size_t i = 0; i < node->parents.size; i++)
    {
        if (Board_cmp(&node->parents.data[i], &board))
            return;
    }
    if (node->parents.size >= NUM_CELLS)
    {
        printf("Parent size reached NUM_CELLS\n");
        exit(1);
    }
    node->parents.data[node->parents.size++] = board;
}

static ParentsAllocator ParentsAllocator_init()
{
    size_t default_num_vecs = 1 << 20;
    Board *arr = calloc(default_num_vecs * NUM_CELLS, sizeof(Board));
    return (ParentsAllocator){.size = 0, .capacity = default_num_vecs, .arr = arr};
}

static void ParentsAllocator_reset(ParentsAllocator allocator)
{
    memset(allocator.arr, 0, allocator.size * NUM_CELLS * sizeof(Board));
}

static void ParentsAllocator_destroy(ParentsAllocator allocator)
{
    free(allocator.arr);
}

static Parents ParentsAllocator_create_parents(ParentsAllocator *allocator)
{
    if (allocator->size == allocator->capacity)
    {
        printf("ParentsAllocator_create_cells: ran out of memory\n");
        exit(1);
    }
    Board *parents = &allocator->arr[allocator->size * NUM_CELLS];
    allocator->size++;
    return (Parents){.size = 0, .data = parents};
}

NodeMap NodeMap_init()
{
    size_t default_capacity = 1 << 20;
    NodeMapEntry *entries = calloc(default_capacity, sizeof(NodeMapEntry));
    return (NodeMap){
        .allocator = ParentsAllocator_init(),
        .size = 0,
        .capacity = default_capacity,
        .entries = entries,
    };
}

Node *NodeMap_get_or_create(NodeMap *map, Board board)
{
    if (map->size == map->capacity)
    {
        printf("NodeMap_get_or_create: ran out of memory\n");
        exit(1);
    }
    size_t index = Board_hash(&board) & (map->capacity - 1);
    // Linear probing
    for (;;)
    {
        NodeMapEntry entry = map->entries[index];
        if (NodeMapEntry_isempty(entry))
        {
            NodeMapEntry new_entry = (NodeMapEntry){
                .board = board, .node = (Node){.parents = ParentsAllocator_create_parents(&map->allocator)}};
            map->entries[index] = new_entry;
            map->size++;
            return &map->entries[index].node;
        }
        else if (Board_cmp(&board, &entry.board))
            return &map->entries[index].node;
        index++;
        if (index == map->capacity)
            index = 0;
    }
}

void NodeMap_reset(NodeMap *map)
{
    memset(map->entries, 0, map->size * sizeof(NodeMapEntry));
    ParentsAllocator_reset(map->allocator);
}

void NodeMap_destroy(NodeMap *map)
{
    free(map->entries);
    ParentsAllocator_destroy(map->allocator);
}

uint32_t num_parent_visits(BoardCache *boardcache, NodeMap *map, Node *node)
{
    uint32_t num_visits = 0;
    for (size_t i = 0; i < node->parents.size; i++)
    {
        Board parent_board = BoardCache_get_or_create(boardcache, node->parents.data[i]);
        Node *parent_node = NodeMap_get_or_create(map, parent_board);
        num_visits += parent_node->visits;
    }
    return num_visits;
}
