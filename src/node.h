#pragma once

#include "stdbool.h"
#include "stdint.h"

#include "board.h"

typedef struct Parents
{
    size_t size;
    Board *data;
} Parents;

typedef struct Node
{
    Parents parents;
    uint32_t visits;
    uint32_t wins;
    uint32_t losses;
    uint32_t draws;
} Node;

typedef struct ParentsAllocator
{
    size_t size;
    size_t capacity;
    Board *arr;
} ParentsAllocator;

typedef struct NodeMapEntry
{
    Board board;
    Node node;
} NodeMapEntry;

typedef struct NodeMap
{
    ParentsAllocator allocator;
    size_t size;
    size_t capacity;
    NodeMapEntry *entries;
} NodeMap;

static inline bool NodeMapEntry_isempty(NodeMapEntry entry)
{
    return Board_isempty(&entry.board);
}

float Node_value(Node *node);
void Node_add_parent(Node *node, Board board);
NodeMap NodeMap_init();
void NodeMap_reset(NodeMap *nodecache);
Node *NodeMap_get_or_create(NodeMap *map, Board board);
void NodeMap_destroy(NodeMap *map);
uint32_t num_parent_visits(BoardCache *boardcache, NodeMap *map, Node *node);
