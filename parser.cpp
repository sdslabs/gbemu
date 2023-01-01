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
	
    for (int i = 0; i < 48; i++)
    {
        if (original_nintendo_logo[i] != gameHeader->fetchNintendoLogo(i)) // call fetchNintendoLogo function
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

std::string fetchTitle(GameHeader *gameHeader) // fetch game_title from game_header
{

    std::string game_title = "";

    for (int i = 0; i < 16; i++)
    {
        uint8_t a_char = gameHeader->fetchTitle(i);

        if (!a_char)
        {
            break;
        }

        game_title += a_char;
    }

    return game_title;
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

        std::cout << "Starting Game " << fetchTitle(&game_header) << "\n";
    }
}