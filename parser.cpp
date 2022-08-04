#include <iostream>
#include <fstream>
#include "parser.h"

bool makeGameHeader ( GameHeader *gameHeader )
{

	std::ifstream game ("game.gb", std::ifstream::binary) ;

	if ( !game ) { return 0 ; }

	game.seekg ( 0 , game.end ) ;

	int file_length = game.tellg() ;

	if ( file_length < 336 ) { return 0 ; }

	game.seekg ( 256 ) ;

	char *buffer ;

	for ( int i = 0 ; i < 80 ; i++ )
	{
		game.read ( buffer , 1 ) ;
		gameHeader -> game_header[i] = *buffer ;
	}

    game.close();

	return 1 ;

}