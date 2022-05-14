#include "node.h"

#define FNV_PRIME 16777619UL
#define FNV_SEED 2166136261UL

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

void Node_add_parent(Node *node, Board *board)
{
    for (size_t i = 0; i < node->parents_size; i++)
    {
        if (board_cmp(node->parents[i], board))
            return;
    }
    if (node->parents_size >= PARENTS_MAX_SIZE)
    {
        printf("Parent size reached PARENTS_MAX_SIZE\n");
        exit(1);
    }
    node->parents[node->parents_size++] = board;
}

BoardAllocator BoardAllocator_Init()
{
    size_t num_boards = (size_t)(1.0f * (1 << 24));
    printf("Size of BoardAllocator: %f MB\n", (float)num_boards * sizeof(Board) / 1e6);
    Board *boards = calloc(num_boards, sizeof(Board));
    if (boards == NULL)
    {
        printf("Could not allocate BoardAllocator\n");
        exit(1);
    }
    return (BoardAllocator){.size = 0, .capacity = num_boards, .boards = boards};
}

void BoardAllocator_reset(BoardAllocator *allocator)
{
    memset(allocator->boards, 0, allocator->size * sizeof(Board));
    allocator->size = 0;
}

void BoardAllocator_destroy(BoardAllocator *allocator)
{
    free(allocator->boards);
}

Board *BoardAllocator_create_board(BoardAllocator *allocator)
{
    if (allocator->size == allocator->capacity)
    {
        printf("BoardAllocator_create_node: ran out of memory\n");
        exit(1);
    }
    Board *board = &allocator->boards[allocator->size];
    allocator->size++;
    return board;
}

NodeAllocator NodeAllocator_Init()
{
    size_t num_nodes = (size_t)(1.0f * (float)(1 << 24));
    printf("Size of NodeAllocator: %f MB\n", (float)num_nodes * sizeof(Node) / 1e6);
    Node *nodes = calloc(num_nodes, sizeof(Node));
    if (nodes == NULL)
    {
        printf("Could not allocate NodeAllocator\n");
        exit(1);
    }
    return (NodeAllocator){.size = 0, .capacity = num_nodes, .nodes = nodes};
}

void NodeAllocator_reset(NodeAllocator *allocator)
{
    memset(allocator->nodes, 0, allocator->size * sizeof(Node));
    allocator->size = 0;
}

void NodeAllocator_destroy(NodeAllocator *allocator)
{
    free(allocator->nodes);
}

Node *NodeAllocator_create_node(NodeAllocator *allocator)
{
    if (allocator->size == allocator->capacity)
    {
        printf("NodeAllocator_create_node: ran out of memory\n");
        exit(1);
    }
    Node *node = &allocator->nodes[allocator->size];
    allocator->size++;
    return node;
}

NodeMap NodeMap_init()
{
    size_t capacity = 1 << 25;
    printf("Size of NodeMap: %f MB\n", (float)capacity * sizeof(NodeMapEntry) / 1e6);
    NodeMapEntry *entries = calloc(capacity, sizeof(NodeMapEntry));
    if (entries == NULL)
    {
        printf("Could not allocate NodeMap\n");
        exit(1);
    }
    return (NodeMap){
        .board_allocator = BoardAllocator_Init(),
        .node_allocator = NodeAllocator_Init(),
        .size = 0,
        .capacity = capacity,
        .entries = entries,
    };
}

uint32_t board_hash(Board *board)
{
    uint32_t hash = FNV_SEED;
    for (uint8_t p_idx = 0; p_idx < 2; p_idx++)
    {
        for (uint8_t f_idx = 0; f_idx < 4; f_idx++)
        {
            uint8_t *bytes = (uint8_t *)&board->field[p_idx][f_idx];
            for (uint8_t b_idx = 0; b_idx < sizeof(uint64_t); b_idx++)
                hash = fnv1a(bytes[b_idx], hash);
        }
    }
    return hash;
}

NodeMapEntry NodeMap_get_or_create(NodeMap *map, Board board)
{
    if (map->size == map->capacity)
    {
        printf("NodeMap_get_or_create: ran out of memory\n");
        exit(1);
    }
    size_t index = board_hash(&board) & (map->capacity - 1);
    // Linear probing
    for (;;)
    {
        NodeMapEntry entry = map->entries[index];
        if (entry.board == NULL)
        {
            Board *managed_board = BoardAllocator_create_board(&map->board_allocator);
            *managed_board = board;
            Node *managed_node = NodeAllocator_create_node(&map->node_allocator);
            NodeMapEntry new_entry = (NodeMapEntry){.board = managed_board, .node = managed_node};
            map->entries[index] = new_entry;
            map->size++;
            return new_entry;
        }
        else if (board_cmp(&board, entry.board))
            return map->entries[index];
        index++;
        if (index == map->capacity)
            index = 0;
    }
}

void NodeMap_reset(NodeMap *map)
{
    memset(map->entries, 0, map->capacity * sizeof(NodeMapEntry));
    map->size = 0;
    BoardAllocator_reset(&map->board_allocator);
    NodeAllocator_reset(&map->node_allocator);
}

void NodeMap_destroy(NodeMap *map)
{
    free(map->entries);
    BoardAllocator_destroy(&map->board_allocator);
    NodeAllocator_destroy(&map->node_allocator);
}

uint32_t num_parent_visits(NodeMap *map, Node *node)
{
    uint32_t num_visits = 0;
    for (size_t i = 0; i < node->parents_size; i++)
    {
        Board *parent_board = node->parents[i];
        Node *parent_node = NodeMap_get_or_create(map, *parent_board).node;
        num_visits += parent_node->visits;
    }
    return num_visits;
}
