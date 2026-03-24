#include <SDL3/SDL_events.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <string>

int makeMove(char board[8][8], int row, int col, bool isPlayer1Turn) {
  char player = isPlayer1Turn ? 'B' : 'W';
  if (board[row][col] != ' ') {
    return -1;
  }
  int directions[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                          {0, 1},   {1, -1}, {1, 0},  {1, 1}};
  bool validMove = false;

  for (auto &dir : directions) {
    int r = row + dir[0];
    int c = col + dir[1];
    bool hasOpponentPiece = false;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Check flip piece for: (%d, %d)",
                r, c);

    while (r >= 0 && r < 8 && c >= 0 && c < 8) {
      if (board[r][c] == ' ') {
        break;
      } else if (board[r][c] != player) {
        hasOpponentPiece = true;
      } else if (board[r][c] == player) {
        if (hasOpponentPiece) {
          validMove = true;
          int flipR = row + dir[0];
          int flipC = col + dir[1];
          while (flipR != r || flipC != c) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                        "Flipping piece at: (%d, %d)", flipR, flipC);
            board[flipR][flipC] = player;
            flipR += dir[0];
            flipC += dir[1];
          }
        } else {
          break;
        }
      }

      r += dir[0];
      c += dir[1];
    }
  }

  if (validMove) {
    board[row][col] = player;
  } else {
    return -1;
  }

  return 0;
}

int checkGameOver(char board[8][8]) {
  int countB = 0;
  int countW = 0;
  bool hasEmpty = false;
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      if (board[i][j] == 'B') {
        countB++;
      } else if (board[i][j] == 'W') {
        countW++;
      } else {
        hasEmpty = true;
      }
    }
  }

  if (!hasEmpty) {
    if (countB > countW) {
      return 1; // Black Wins
    } else if (countW > countB) {
      return 2; // White Wins
    } else {
      return 0; // Draw
    }
  } else {
    if (countB == 0) {
      return 2;
    } else if (countW == 0) {
      return 1;
    } else {
      return -1;
    }
  }

  return -1;
}

int main(int argc, char *argv[]) {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Surface *surface;
  SDL_Texture *texture;

  bool done = false;

  SDL_Color red = {255, 0, 0, SDL_ALPHA_OPAQUE};
  TTF_Font *font = TTF_OpenFont("./njnaruto.ttf", 18);
  SDL_Surface *textSurface =
      TTF_RenderText_Solid(font, "Hello, SDL_ttf!", 10, red);

  SDL_Texture *textTexture =
      SDL_CreateTextureFromSurface(renderer, textSurface);

  char board[8][8] = {{' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
                      {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
                      {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
                      {' ', ' ', ' ', 'W', 'B', ' ', ' ', ' '},
                      {' ', ' ', ' ', 'B', 'W', ' ', ' ', ' '},
                      {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
                      {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
                      {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}};
  std::cout << "Please tell me the name of Player 1 (Black): ";
  std::string player1{"Black"};
  std::cin >> player1;
  std::cout << "Please tell me the name of Player 2 (White): ";
  std::string player2 = "White";
  std::cin >> player2;
  bool isPlayer1Turn = true;
  int attempts{0};

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to Initialize SDL: %s",
                 SDL_GetError());
    return 1;
  }

  if (!TTF_Init()) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to Initialize SDL_ttf: %s",
                 SDL_GetError());
    return 1;
  }

  if (!SDL_CreateWindowAndRenderer("Reversi!!!!!", 400, 420, SDL_WINDOW_VULKAN,
                                   &window, &renderer)) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                 "Failed to create window and rendered: %s", SDL_GetError());
    return 3;
  }

  surface = SDL_LoadBMP("sample.bmp");
  if (!surface) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load Surface Image: %s",
                 SDL_GetError());
    return 2;
  }

  texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (!texture) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                 "Failed to create texture from surface: %s", SDL_GetError());
    return 2;
  }

  SDL_DestroySurface(surface);

  while (!done) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        done = true;
      }
      if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
          int x = event.button.x;
          int y = event.button.y;
          int row = x / 50;
          int col = y / 50;
          SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                      "Mouse CLicked at: (%d, %d) -> Board Position: (%d, %d)",
                      x, y, row, col);
          if (row >= 0 && row < 8 && col >= 0 && col < 8) {
            if (makeMove(board, row, col, isPlayer1Turn) == -1) {
              SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                          "Invalid Move at: (%d, %d)", row, col);
              attempts++;
              if (attempts >= 3) {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                            "Player %s has no valid moves. Skipping turn.",
                            isPlayer1Turn ? player1 : player2);
                attempts = 0;
                isPlayer1Turn = !isPlayer1Turn;
              }
            } else {
              attempts = 0;
              isPlayer1Turn = !isPlayer1Turn;
            }

            // if (board[row][col] == ' ' and isPlayer1Turn) {
            //   board[row][col] = 'B';
            //   isPlayer1Turn = !isPlayer1Turn;
            // } else {
            //   board[row][col] = 'W';
            //   isPlayer1Turn = !isPlayer1Turn;
            // }
          }
        }

        int result{checkGameOver(board)};

        if (result == 1) {
          SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Game Over! %s Wins!",
                      player1.c_str());
          done = true;
        } else if (result == 2) {
          SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Game Over! %s Wins!",
                      player2.c_str());
          done = true;
        } else if (result == 0) {
          SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Game Over! It's a Draw!");
          done = true;
        }
      }
    }

    // Game Logic
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);

    for (int i = 0; i < 8; ++i) {
      for (int j = 0; j < 8; ++j) {
        if (board[i][j] == 'W') {
          SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

        } else if (board[i][j] == 'B') {
          SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        } else {
          // SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
          SDL_SetRenderDrawColor(renderer, 0xBB, 0x8E, 0x51, 0xFF);
        }
        SDL_FRect rect = SDL_FRect{.x = static_cast<float>(i * 50),
                                   .y = static_cast<float>(j * 50),
                                   .w = 48,
                                   .h = 48};
        SDL_RenderFillRect(renderer, &rect);
      }
    }
    // SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

    SDL_RenderDebugTextFormat(
        renderer, 10, 405, "Player Turn: %s Attempt: %d/3",
        isPlayer1Turn ? player1.c_str() : player2.c_str(), attempts + 1);

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    // SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
  }

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();
  return 0;
}
