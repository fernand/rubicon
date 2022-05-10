#include "node.h"

#define FNV_PRIME 16777619UL
#define FNV_SEED 2166136261UL

#define PARENTS_MAX_SIZE NUM_CELLS / 4

static inline uint32_t fnv1a(uint8_t b, uint32_t hash)
{
    return (b ^ hash) * FNV_PRIME;
}

float Node_value(Node *node)
{
    if (node->visits == 0)
        return 0.0f;
    return (float)node->wins / node->visits;
}

void Node_add_parent(Node *node, Board board)
{
    for (size_t i = 0; i < node->parents.size; i++)
    {
        if (board_cmp(node->parents.data[i], board))
            return;
    }
    if (node->parents.size >= PARENTS_MAX_SIZE)
    {
        printf("Parent size reached PARENTS_MAX_SIZE\n");
        exit(1);
    }
    node->parents.data[node->parents.size++] = board;
}

ParentsAllocator ParentsAllocator_init()
{
    size_t num_vecs = (size_t)(1.0f * (float)(1 << 22));
    printf("Size of ParentsAllocator: %f MB\n", (float)num_vecs * PARENTS_MAX_SIZE * sizeof(Board) / 1e6);
    Board *arr = calloc(num_vecs * PARENTS_MAX_SIZE, sizeof(Board));
    if (arr == NULL)
    {
        printf("Could not allocate ParentsAllocator\n");
        exit(1);
    }
    return (ParentsAllocator){.size = 0, .capacity = num_vecs, .arr = arr};
}

void ParentsAllocator_reset(ParentsAllocator *allocator)
{
    memset(allocator->arr, 0, allocator->size * PARENTS_MAX_SIZE * sizeof(Board));
    allocator->size = 0;
}

void ParentsAllocator_destroy(ParentsAllocator *allocator)
{
    free(allocator->arr);
}

Parents ParentsAllocator_create_parents(ParentsAllocator *allocator)
{
    if (allocator->size == allocator->capacity)
    {
        printf("ParentsAllocator_create_cells: ran out of memory\n");
        exit(1);
    }
    Board *parents = &allocator->arr[allocator->size * PARENTS_MAX_SIZE];
    allocator->size++;
    return (Parents){.size = 0, .data = parents};
}

NodeMap NodeMap_init()
{
    size_t capacity = 1 << 24;
    printf("Size of NodeMap: %f MB\n", (float)capacity * sizeof(NodeMapEntry) / 1e6);
    NodeMapEntry *entries = calloc(capacity, sizeof(NodeMapEntry));
    if (entries == NULL)
    {
        printf("Could not allocate NodeMap\n");
        exit(1);
    }
    return (NodeMap){
        .allocator = ParentsAllocator_init(),
        .size = 0,
        .capacity = capacity,
        .entries = entries,
    };
}

uint32_t board_hash(Board board)
{
    uint32_t hash = FNV_SEED;
    for (uint8_t p_idx = 0; p_idx < 2; p_idx++)
    {
        for (uint8_t f_idx = 0; f_idx < 4; f_idx++)
        {
            uint8_t *bytes = (uint8_t *)&board.field[p_idx][f_idx];
            for (uint8_t b_idx = 0; b_idx < sizeof(uint64_t); b_idx++)
                hash = fnv1a(bytes[b_idx], hash);
        }
    }
    return hash;
}

Node *NodeMap_get_or_create(NodeMap *map, Board board)
{
    if (map->size == map->capacity)
    {
        printf("NodeMap_get_or_create: ran out of memory\n");
        exit(1);
    }
    size_t index = board_hash(board) & (map->capacity - 1);
    // Linear probing
    for (;;)
    {
        NodeMapEntry entry = map->entries[index];
        if (entry.board.field[0][3] == 0)
        {
            NodeMapEntry new_entry = (NodeMapEntry){
                .board = board, .node = (Node){.parents = ParentsAllocator_create_parents(&map->allocator)}};
            map->entries[index] = new_entry;
            map->size++;
            return &map->entries[index].node;
        }
        else if (board_cmp(board, entry.board))
            return &map->entries[index].node;
        index++;
        if (index == map->capacity)
            index = 0;
    }
}

void NodeMap_reset(NodeMap *map)
{
    memset(map->entries, 0, map->capacity * sizeof(NodeMapEntry));
    map->size = 0;
    ParentsAllocator_reset(&map->allocator);
}

void NodeMap_destroy(NodeMap *map)
{
    free(map->entries);
    ParentsAllocator_destroy(&map->allocator);
}

uint32_t num_parent_visits(NodeMap *map, Node *node)
{
    uint32_t num_visits = 0;
    for (size_t i = 0; i < node->parents.size; i++)
    {
        Board parent_board = node->parents.data[i];
        Node *parent_node = NodeMap_get_or_create(map, parent_board);
        num_visits += parent_node->visits;
    }
    return num_visits;
}
