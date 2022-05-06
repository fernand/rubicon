#include "stdio.h"

#include "update_game.c"
#include "mcts.c"
#include "linux.h"

int main()
{
    struct timespec t1, t2;
    ScreenshotInfo sc_info = get_screenshot_info();
    size_t y_stride = sc_info.width * 3;
    uint8_t *buffer = malloc(sc_info.height * y_stride);
    GameConfig config = {0};
    create_game_config("rubicon_4k.png", &config);
    Cell cell_0[NUM_CELLS];
    Cell cell_1[NUM_CELLS];
    Board main_board = Board_stack_allocate_board(cell_0, cell_1);
    size_t num_playouts = 4000;
    for (;;)
    {
        screenshot(sc_info, buffer);
        update_player_cells(&main_board, buffer, y_stride);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        playouts(&config, num_playouts, main_board);
        clock_gettime(CLOCK_MONOTONIC, &t2);
        float avg_playout_time = (float)clock_diff_us(t1, t2) / num_playouts;
        printf("Average playout time: %f\n", avg_playout_time);
        sleep_ms(1000);
    }
}
