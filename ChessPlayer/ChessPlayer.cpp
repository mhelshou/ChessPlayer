// ChessPlayer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include "params.h"
#include "bitboards.h"
#include "play.h"
#include "uci.h"

#include "util.h"

int _tmain(int argc, _TCHAR* argv[])
{
	setCounterFrequency();

	bitboards_init();
	//bitboards_test();
	//move_test();
	//play_moves();
	/*generateGame();
	exit(0);*/
	//playComputerGame(HUMAN_AGAINST_COMPUTER, WHITE);
	//playComputerGame(HUMAN_AGAINST_COMPUTER, BLACK);
	
	if(!UCI_GAME)
	{
		if(HUMAN_GAME)
			playComputerGame(HUMAN_AGAINST_COMPUTER, HUMAN_COLOR);
		else
			playComputerGame(COMPUTER_AGAINST_COMPUTER, WHITE);
	}
	else
		startUCIGame();

	return 0;
}

