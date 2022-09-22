#include <iostream>
#include <fstream>
#include "parser.h"


bool makeGameHeader(GameHeader *gameHeader) {

  std::ifstream game("game.gb", std::ifstream::binary);

  if (!game) {
    return 0;
  }

  game.seekg(0, game.end);

  int file_length = game.tellg();

  if (file_length < 336) {
    return 0;
  }

  game.seekg(256);

  char *buffer;

  for (int i = 0; i < 80; i++) {
    game.read(buffer, 1);
    gameHeader->game_header[i] = *buffer;
  }

  game.close();

  return 1;
}

bool checkNintendoLogo(GameHeader *gameHeader) {

  uint8_t nintendo_logo[48] = {206, 237, 102, 102, 204, 13,  0,   11,  3,   115,
                               0,   131, 0,   12,  0,   13,  0,   8,   17,  31,
                               136, 137, 0,   14,  220, 204, 110, 230, 221, 221,
                               217, 153, 187, 187, 103, 99,  110, 14,  236, 204,
                               221, 220, 153, 159, 187, 185, 51,  62};

  for (int i = 0; i < 48; i++) {
    if (nintendo_logo[i] != (gameHeader->nintendo_logo)[i]) {
      return 0;
    }
  }

  return 1;
}

bool checkHeaderChecksum(GameHeader *gameHeader) {

  uint8_t x = 0;

  for (int i = 52; i <= 76; i++) {
    x = x - gameHeader->game_header[i] - 1;
  }

  if (x != gameHeader->header_checksum[0]) {
    return 0;
  }

  return 1;
}

int main() {

  bool code_runner = 1;

  while (code_runner) {

    code_runner = 0;

    GameHeader game_header;

    bool check = 0;

    check = makeGameHeader(&game_header);
    std::cout << ((check == 1) ? "Game Header successfully created\n"
                               : "Game Header creation failed\n");

    if (check == 0) {
      std::cout << "Terminating Program\n";
      break;
    }

    check = checkNintendoLogo(&game_header);
    std::cout << ((check == 1) ? "Nintendo Logo Matched\n"
                               : "Nintendo Logo Match Failed\n");

    if (check == 0) {
      std::cout << "Terminating Program\n";
      break;
    }

    check = checkHeaderChecksum(&game_header);
    std::cout << ((check == 1) ? "Header Checksum Verified\n"
                               : "Header Checksum Incorrect\n");

    if (check == 0) {
      std::cout << "Terminating Program\n";
      break;
    }
  }
}