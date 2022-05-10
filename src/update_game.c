//#define DEBUG_IMG
#ifdef DEBUG_IMG
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#endif

#include "board.h"

#define PI 3.14159f

// Assumes a window layout on a 4k screen similar to rubicon_4k_half_screen.png
#define LEFT_X 2044   // The center of the left-most cell
#define RIGHT_X 3713  // The centor of the right-most cell
#define WIDTH 1670    // 1441 - 456 + 1 The width of the board from center to center of extreme cells
#define TOP_Y 306     // The center of the top-most cell
#define BOTTOM_Y 2062 // The center of the bottom-most cell
#define HEIGHT 1757   // 1171 - 137 + 1 The height of the board from center to centor of extreme cells
#define DST 83.5f     // The horizontal and vertical distance between two cell centers

void create_game_config(char *screenshot_path, GameConfig *config)
{
    int width, height, n;
    uint8_t *img = stbi_load(screenshot_path, &width, &height, &n, 3);
    size_t y_stride = width * 3;
    size_t x_stride = 3;

    // For debugging finding cell colors and finding their connections
#ifdef DEBUG_IMG
    size_t data_size = height * width * 3;
    uint8_t *debug_img = (uint8_t *)malloc(data_size);
    memcpy(debug_img, img, data_size);
#endif

    // Counter-clockwise
    float x_offset[8] = {1.0f,  cosf(PI / 4),      0.0f, cosf(3 * PI / 4),
                         -1.0f, cosf(PI + PI / 4), 0.0f, cosf(PI + 3 * PI / 4)};
    float y_offset[8] = {0.0f, sinf(PI / 4),      1.0f,  sinf(3 * PI / 4),
                         0.0f, sinf(PI + PI / 4), -1.0f, sinf(PI + 3 * PI / 4)};

    // Cell indices start from the top, then go downards from left to right
    uint8_t real_cell_idx = 0;
    uint8_t cell_idx = 0;

    float current_y = TOP_Y;
    for (uint8_t depth = 1; depth <= MAX_DEPTH; depth++)
    {
        uint8_t num_virt_cells = virt_cells_for_depth[depth];
        // Middle for num_cells == 1
        float current_x = LEFT_X + WIDTH / 2 - ((num_virt_cells - 1) / 2) * DST;
        for (uint8_t i = 0; i < num_virt_cells; i++)
        {
            bool is_real_cell = false;
#ifdef DEBUG_IMG
            size_t offset = (size_t)current_y * y_stride + (size_t)current_x * x_stride;
#endif
            uint8_t connections = 0;
            for (uint8_t j = 0; j < 8; j++)
            {
                size_t neighbor_y = (size_t)(current_y - y_offset[j] * DST / 2);
                size_t neighbor_x = (size_t)(current_x + x_offset[j] * DST / 2);
                size_t neighbor_offset = neighbor_y * y_stride + neighbor_x * x_stride;
                if (img[neighbor_offset] == 255 && img[neighbor_offset + 1] == 255 && img[neighbor_offset + 2] == 255)
                {
                    is_real_cell = true;
                    connections = connections | 1 << j;
#ifdef DEBUG_IMG
                    debug_img[neighbor_offset] = 255;
                    debug_img[neighbor_offset + 1] = 0;
                    debug_img[neighbor_offset + 2] = 0;
                    debug_img[offset] = 0;
                    debug_img[offset + 1] = 255;
                    debug_img[offset + 2] = 0;
#endif
                }
            }
            if (is_real_cell)
            {
                config->cell_connections[cell_idx] = connections;
                config->all_cells[real_cell_idx] = cell_idx;
                real_cell_idx += 1;
            }
            cell_idx++;
            current_x += DST;
        }
        current_y += DST;
    }

#ifdef DEBUG_IMG
    stbi_write_png("debug_image.png", width, height, 3, debug_img, (int)y_stride);
    free(debug_img);
#endif
    stbi_image_free(img);
}

void update_player_cells(Board *board, const uint8_t *buffer, size_t y_stride)
{
    size_t x_stride = 3;
    uint8_t cell_idx = 0;
    float current_y = TOP_Y;
    for (uint8_t depth = 1; depth <= MAX_DEPTH; depth++)
    {
        uint8_t num_virt_cells = virt_cells_for_depth[depth];
        float current_x = LEFT_X + WIDTH / 2 - ((num_virt_cells - 1) / 2) * DST;
        for (uint8_t i = 0; i < num_virt_cells; i++)
        {
            size_t offset = (size_t)current_y * y_stride + (size_t)current_x * x_stride;
            if (buffer[offset] == 0 && buffer[offset + 1] == 255 && buffer[offset + 2] == 255)
            {
                if (!cell_occupied(board->field[0], cell_idx))
                {
                    occupy_cell(board->field[0], cell_idx);
                    printf("%u changed to ai\n", cell_idx);
                }
            }
            else if (buffer[offset] == 255 && buffer[offset + 1] == 0 && buffer[offset + 2] == 255)
            {
                if (!cell_occupied(board->field[1], cell_idx))
                {
                    occupy_cell(board->field[1], cell_idx);
                    printf("%u changed to player\n", cell_idx);
                }
            }
            cell_idx += 1;
            current_x += DST;
        }
        current_y += DST;
    }
    uint8_t num_player_0_cells = count_occupied_cells(board->field[0]);
    uint8_t num_player_1_cells = count_occupied_cells((board->field[1]));
    if (num_player_0_cells > num_player_1_cells)
    {
        set_player_to_play(board->field[0], 1);
        set_round_to_play(board->field[0], num_player_1_cells % 6 + 1);
    }
    else
    {
        set_player_to_play(board->field[0], 0);
        set_round_to_play(board->field[0], num_player_0_cells % 6 + 1);
    }
}
