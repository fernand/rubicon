#pragma once

#include "stdbool.h"
#include "stdint.h"

#include "board.h"

typedef struct Parents
{
    size_t size;
    Board **data;
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
    Board **arr;
} ParentsAllocator;

typedef struct NodeMapEntry
{
    Board *board;
    Node node;
} NodeMapEntry;

typedef struct NodeMap
{
    ParentsAllocator allocator;
    size_t size;
    size_t capacity;
    NodeMapEntry *entries;
} NodeMap;
