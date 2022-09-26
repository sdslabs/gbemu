#include <fstream>
#include <iostream>
#include "parser.h"

bool makeGameHeader(GameHeader *gameHeader) // read Game Header values from cartridge and write it in game_header object
{

    std::ifstream game("game.gb", std::ifstream::binary);

    if (!game) // return 0 if game file is absent
    {
        return 0;
    }

    game.seekg(0, game.end);

    int file_length = game.tellg();

    if (file_length < 336) // return 0 if complete game header is not present
    {
        return 0;
    }

    game.seekg(256, game.beg);

    char buffer[80];

    game.read(buffer, 80);

    for (int i = 0; i < 80; i++)
    {
        gameHeader->updateGameHeader(i, buffer[i]); // call updateGameHeader function of GameHeader class
    }

    game.close();

    return 1;
}

bool checkNintendoLogo(GameHeader *gameHeader) // match Nintendo logo in the cartridge to the actual one
{

    uint8_t nintendo_logo[48] = {206, 237, 102, 102, 204, 13, 0, 11, 3, 115,
                                 0, 131, 0, 12, 0, 13, 0, 8, 17, 31,
                                 136, 137, 0, 14, 220, 204, 110, 230, 221, 221,
                                 217, 153, 187, 187, 103, 99, 110, 14, 236, 204,
                                 221, 220, 153, 159, 187, 185, 51, 62};

    for (int i = 0; i < 48; i++)
    {
        if (nintendo_logo[i] != gameHeader->fetchNintendoLogo(i)) // call fetchNintendoLogo function
        {
            return 0;
        }
    }

    return 1;
}

bool checkHeaderChecksum(GameHeader *gameHeader) // calculate and verify header checksum
{

    uint8_t x = 0;

    for (int i = 52; i <= 76; i++)
    {
        x = x - gameHeader->fetchGameHeader(i) - 1; // call fetchGameHeader function
    }

    if (x != gameHeader->fetchHeaderChecksum(0)) // call fetchHeaderChecksum function
    {
        return 0;
    }

    return 1;
}

int main()
{

    bool code_runner = 1;

    while (code_runner) // driver code
    {

        code_runner = 0;

        GameHeader game_header;

        bool check = 0;

        check = makeGameHeader(&game_header);
        std::cout << ((check == 1) ? "Game Header successfully created\n"
                                   : "Game Header creation failed\n");

        if (check == 0)
        {
            std::cout << "Terminating Program\n";
            break;
        }

        check = checkNintendoLogo(&game_header);
        std::cout << ((check == 1) ? "Nintendo Logo Matched\n"
                                   : "Nintendo Logo Match Failed\n");

        if (check == 0)
        {
            std::cout << "Terminating Program\n";
            break;
        }

        check = checkHeaderChecksum(&game_header);
        std::cout << ((check == 1) ? "Header Checksum Verified\n"
                                   : "Header Checksum Incorrect\n");

        if (check == 0)
        {
            std::cout << "Terminating Program\n";
            break;
        }
    }
}