#pragma once

#include "board.h"

#define PARENTS_MAX_SIZE NUM_CELLS / 4

typedef struct Node
{
    size_t parents_size;
    Board *parents[PARENTS_MAX_SIZE];
    uint32_t visits;
    uint32_t wins;
    uint32_t losses;
    uint32_t draws;
} Node;

typedef struct BoardAllocator
{
    size_t size;
    size_t capacity;
    Board *boards;
} BoardAllocator;

typedef struct NodeAllocator
{
    size_t size;
    size_t capacity;
    Node *nodes;
} NodeAllocator;

typedef struct NodeMapEntry
{
    Board *board;
    Node *node;
} NodeMapEntry;

typedef struct NodeMap
{
    BoardAllocator board_allocator;
    NodeAllocator node_allocator;
    size_t size;
    size_t capacity;
    NodeMapEntry *entries;
} NodeMap;
