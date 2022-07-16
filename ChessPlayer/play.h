#ifndef PLAY_H
#define PLAY_H

#include <iostream>
#include <fstream>
#include "node.h"

#define PARSE_MOVES					0
#define ENTER_MOVES					1
#define RANDOM_GAME					2
#define COMPUTER_AGAINST_COMPUTER	3
#define HUMAN_AGAINST_COMPUTER		4

#ifndef INIT_FEN
#define INIT_FEN	"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w"
#endif

void play_moves();
void generateGame();
void playComputerGame(int gameMode, int colorToPlay);
tNode* enterMove(tBoard* board);

using namespace std;

extern __int64 total_positions;
extern int g_movenum;
extern ofstream logfile;
extern bool print_board_on;

#endif