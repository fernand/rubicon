#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "board.c"
#include "update_game.c"
#include "node.c"
#include "mcts.c"
#include "linux.c"

SDL_Texture *GetAreaTexture(SDL_Rect rect, SDL_Renderer *renderer, SDL_Texture *source)
{
    SDL_Texture *result =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, rect.w, rect.h);
    SDL_SetRenderTarget(renderer, result);
    SDL_RenderCopy(renderer, source, &rect, NULL);
    SDL_SetRenderTarget(renderer, NULL);
    return result;
}

int main()
{
    SDL_Window *win = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *img = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return 1;

    win = SDL_CreateWindow("Rubicon", 0, 0, 3840 / 2, 2860, 0);
    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    int w, h;
    img = IMG_LoadTexture(renderer, "rubicon_4k_half_screen.png");
    SDL_QueryTexture(img, NULL, NULL, &w, &h);
    SDL_Rect cropr;
    cropr.x = 50 + w / 2;
    cropr.y = 250;
    cropr.w = w / 2 - 50 - 50;
    cropr.h = h - 250 - 30;
    SDL_Texture *cropped_tex = GetAreaTexture(cropr, renderer, img);

    SDL_Rect texr = (SDL_Rect){.x = 0, .y = 0, .h = h - 250 - 30, .w = w / 2 - 50 - 50};
    while (1)
    {
        SDL_Event e;
        if (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                break;
            else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)
                break;
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, cropped_tex, NULL, &texr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(img);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);

    return 0;
    //    struct timespec t1, t2;
    //    ScreenshotInfo sc_info = get_screenshot_info();
    //    size_t y_stride = sc_info.width * 3;
    //    uint8_t *buffer = malloc(sc_info.height * y_stride);
    //    GameConfig config = {0};
    //    create_game_config("rubicon_4k_half_screen.png", &config);
    //    NodeMap nodecache = NodeMap_init();
    //    Board main_board = {0};
    //    set_round_to_play(main_board.field[0], 1);
    //    set_player_to_play(main_board.field[0], 1);
    //    occupy_cell(main_board.field[0], 131);
    //    size_t num_playouts = 2000;
    //
    //    clock_gettime(CLOCK_MONOTONIC, &t1);
    //    playouts(&config, &nodecache, num_playouts, main_board);
    //    clock_gettime(CLOCK_MONOTONIC, &t2);
    //    float avg_playout_time = (float)clock_diff_us(t1, t2) / (float)num_playouts;
    //    printf("Average playout time: %f\n", avg_playout_time);

    //    for (;;)
    //    {
    //        screenshot(sc_info, buffer);
    //        update_player_cells(&main_board, buffer, y_stride);
    //        clock_gettime(CLOCK_MONOTONIC, &t1);
    //        playouts(&config, &nodecache, num_playouts, main_board);
    //        clock_gettime(CLOCK_MONOTONIC, &t2);
    //        float avg_playout_time = (float)clock_diff_us(t1, t2) / num_playouts;
    //        printf("Average playout time: %f\n", avg_playout_time);
    //        NodeMap_reset(&nodecache);
    //    }
    //    NodeMap_destroy(&nodecache);
}
