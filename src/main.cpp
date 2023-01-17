#include "pch.h"

#include "board.cpp"
#include "node_map.cpp"
#include "mcts.cpp"
#include "update_game.cpp"
#include "linux.cpp"

// int UIMain()
//{
//     if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
//     {
//         printf("Error: %s\n", SDL_GetError());
//         return -1;
//     }
//
//     SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
//     SDL_Window *window = SDL_CreateWindow("Rubicon", 0, 0, 3840 / 2, 2860, window_flags);
//     SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
//
//     IMGUI_CHECKVERSION();
//     ImGui::CreateContext();
//     ImGuiIO &io = ImGui::GetIO();
//     ImFont *font = io.Fonts->AddFontFromFileTTF("/home/fernand/.fonts/PragmataPro.ttf", 32, nullptr, nullptr);
//     IM_ASSERT(font != nullptr);
//     ImGui::StyleColorsDark();
//
//     ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
//     ImGui_ImplSDLRenderer_Init(renderer);
//
//     bool show_window = true;
//     ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
//
//     bool done = false;
//     while (!done)
//     {
//         SDL_Event event;
//         while (SDL_PollEvent(&event))
//         {
//             ImGui_ImplSDL2_ProcessEvent(&event);
//             if (event.type == SDL_QUIT)
//                 done = true;
//             if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
//                 event.window.windowID == SDL_GetWindowID(window))
//                 done = true;
//         }
//
//         ImGui_ImplSDLRenderer_NewFrame();
//         ImGui_ImplSDL2_NewFrame();
//         ImGui::NewFrame();
//
//         if (show_window)
//         {
//             ImGui::Begin("Another Window",
//                          &show_window); // Pass a pointer to our bool variable (the window will have a closing
//                                         // button that will clear the bool when clicked)
//             ImGui::Text("Hello from another window!");
//             if (ImGui::Button("Close Me"))
//                 show_window = false;
//             ImGui::End();
//         }
//
//         ImGui::Render();
//         SDL_SetRenderDrawColor(renderer, (u8)(clear_color.x * 255), (u8)(clear_color.y * 255),
//                                (u8)(clear_color.z * 255), 255);
//         SDL_RenderClear(renderer);
//         ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
//         SDL_RenderPresent(renderer);
//     }
//
//     ImGui_ImplSDLRenderer_Shutdown();
//     ImGui_ImplSDL2_Shutdown();
//     ImGui::DestroyContext();
//
//     SDL_DestroyWindow(window);
//     SDL_Quit();
//
//     return 0;
// }

int main()
{
    ScreenshotInfo sc_info = get_screenshot_info();
    size_t y_stride = sc_info.width * 3;
    u8 *buffer = new u8[sc_info.height * y_stride];
    GameConfig config{};
    create_game_config("rubicon_4k_half_screen.png", &config);
    NodeMap map{};
    Board main_board{};
    size_t num_playouts = 100;

    auto t1 = std::chrono::steady_clock::now();
    playouts(config, map, num_playouts, main_board);
    auto t2 = std::chrono::steady_clock::now();
    std::cout << "Average playout time: "
              << (float)(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / (float)num_playouts
              << std::endl;

    //        screenshot(sc_info, buffer);
    //        update_player_cells(&main_board, buffer, y_stride);
}