#include "gameBoy.h"
#include <stdio.h>

int main(int argc, char** argv)
{
	// Check if correct number of arguments provided
	if (argc != 3)
	{
		printf("Usage: %s <boot_rom_path> <game_rom_path>\n", argv[0]);
		printf("Example: %s ./dmg_boot.gb ./tetris.gb\n", argv[0]);
		return 1;
	}

	const char* bootRomPath = argv[1];
	const char* gameRomPath = argv[2];

	printf("Loading Boot ROM: %s\n", bootRomPath);
	printf("Loading Game ROM: %s\n", gameRomPath);

	GBE* gbe = new GBE(bootRomPath, gameRomPath);

	return 0;
}