#include "stdafx.h"
#include "windows.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>

#include "params.h"
#include "node.h"
#include "moves.h"
#include "evaluate.h"
#include "play.h"
#include "util.h"
#include "version.h"

using namespace std;

int uci_go = 0;

void outMsg(string s)
{
	ofstream outfile;

	outfile.open("ucilog.txt", ios::app);
	outfile << s << "\n";
	outfile.close();	
}

tBoard *parseUCIPosition(string pos_str)
{
	int curpos = 0;
	string newString, nextMove, lastMove;
	int srcsq, dstsq;

	tBoard *board, newBoard;

	curpos = pos_str.find("startpos");

	if (curpos != string::npos)
	{
		board = ParseFEN(INIT_FEN);

		curpos = pos_str.find("moves");

		if(curpos != string::npos)
		{
			newString = pos_str.substr(curpos);

			std::istringstream moves(newString);

			moves >> nextMove; // discard the word "moves"
			while(1)
			{
				moves >> nextMove;
				if( (lastMove == nextMove) || (nextMove == "print") )
					break;
				srcsq = toupper(nextMove[0])-'A' + (nextMove[1]-'1')*8;
				dstsq = toupper(nextMove[2])-'A' + (nextMove[3]-'1')*8;
				makeMove(board, &newBoard, srcsq, dstsq); // todo: check the moves are legal before doing them
				*board = newBoard;
				lastMove = nextMove;
			}
		}

		if(pos_str.find("print") != string::npos)
			print_board(board);

	}

	return board;
}

void startUCIGame()
{
	string input;
	bool gameover = false;
	int movenum = 0;
	double elapsed;
	int millisStart, millisEnd;
	ofstream outfile;
	outfile.open("ucilog.txt", ios::app);

	cout << "Whimpy Master\n";
	cout << "Version " << version << "\n";
	cout << "by Mohammed ELShoukry\n";

	tBoard* board = ParseFEN(INIT_FEN_START);
	tNode *node = createNode(board);

	if(RANDOMIZE_EVERY_RUN)
	{
		int seed = (int)time(NULL);
		//int seed = 0x567bcc8c;
		srand(seed);

		cout << "info string seed = " << seed << "\n";
	}
	else
		srand(1);

	while(!gameover)
	{
		getline(cin, input);
		//cout << "info string " << input << "\n";
		outMsg(input);

		if (input == "uci")
		{
			cout << "id name Whimpy Master\n";
			cout << "id author Mohammed ElShoukry\n";

			cout << "uciok\n";
		}
		else if( input == "ucinewgame")
		{
			if(!RANDOMIZE_EVERY_RUN)
				srand(1);
		}
		else if (input == "isready")
			cout << "readyok\n";
		else if (input.find("setoption") == 0)
		{
		}
		else if (input.find("go") == 0)
		{
			uci_go = 0;

			millisStart = GetTickCount();

			findBestMove(node); //<<<<<
			//findBestChildScore_Improved(node, 0, (node->board->turn==WHITE)?MAX_POSITIVE_SCORE:MIN_NEGATIVE_SCORE);

			millisEnd = GetTickCount();
			elapsed = double(millisEnd-millisStart)/1000.0;

			if(input.find("infinite") == string::npos)
				cout << "bestmove " << getMoveString(node->move) << "\n";

			if(print_board_on)
				print_board(node->board);
		}
		else if (input.find("position") == 0)
		{
			board = parseUCIPosition(input);
			node = createNode(board);

			if(print_board_on)
				print_board(board);
		}
		else if (input == "stop")
		{
			if( (node->move.srcsq != -1) && (node->move.dstsq != -1) )
				cout << "bestmove " << getMoveString(node->move) << "\n";
		}
		else if (input == "quit")
		{
			exit(0);
		}
		else if (input == "logon")
		{
			debug_print = 1;
			cout << "Log is On\n";
			logfile.open("move_log.txt");
		}
		else if (input == "logoff")
		{
			debug_print = 0;
			cout << "Log is Off\n";
			logfile.close();				
		}
		else if (input == "print_board_on")
		{
			cout << "ASCII Board On\n";
			print_board_on = true;

			print_board(board);
		}
		else if (input == "print_board_off")
		{
			cout << "ASCII Board Off\n";
			print_board_on = false;
		}
		else
		{
			outMsg("???");
		}
	}
}
