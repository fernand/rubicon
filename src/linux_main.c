#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "board.c"
#include "update_game.c"
#include "node.c"
#include "mcts.c"
#include "linux.c"

int main()
{
    struct timespec t1, t2;
    ScreenshotInfo sc_info = get_screenshot_info();
    size_t y_stride = sc_info.width * 3;
    uint8_t *buffer = malloc(sc_info.height * y_stride);
    GameConfig config = {0};
    create_game_config("rubicon_4k_half_screen.png", &config);
    NodeMap nodecache = NodeMap_init();
    Board main_board = {0};
    set_round_to_play(main_board.field[0], 1);
    set_player_to_play(main_board.field[0], 1);
    occupy_cell(main_board.field[0], 131);
    size_t num_playouts = 3000;

    clock_gettime(CLOCK_MONOTONIC, &t1);
    playouts(&config, &nodecache, num_playouts, main_board);
    clock_gettime(CLOCK_MONOTONIC, &t2);
    float avg_playout_time = (float)clock_diff_us(t1, t2) / (float)num_playouts;
    printf("Average playout time: %f\n", avg_playout_time);

    for (;;)
    {
        screenshot(sc_info, buffer);
        update_player_cells(&main_board, buffer, y_stride);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        playouts(&config, &nodecache, num_playouts, main_board);
        clock_gettime(CLOCK_MONOTONIC, &t2);
        float avg_playout_time = (float)clock_diff_us(t1, t2) / num_playouts;
        printf("Average playout time: %f\n", avg_playout_time);
        NodeMap_reset(&nodecache);
    }
    NodeMap_destroy(&nodecache);
}
