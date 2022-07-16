#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>

#include "params.h"
#include "play.h"
#include "bitboards.h"
#include "node.h"
#include "moves.h"
#include "evaluate.h"
#include "util.h"
#include "games.h"

using namespace std;

#define ENTERMOVES

__int64 total_positions = 0;
int g_movenum = 0;

tBoard* boardPrev[100];
int undoIndex = 0;
bool gameover = false;
ofstream logfile;
bool print_board_on = false;

string initFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w";

#ifndef INIT_FEN
#define INIT_FEN	"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w"
#endif

void play_moves()
{
	//string initFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w";
	string initFEN = "r1r1kq2/8/8/8/8/8/1N6/R3K2R w";
	string move;
	gameover = false;
	int srcsq, dstsq;
	
	undoIndex = 0;

	char **movearray = game1;
	int index = 0;

	tBoard* board = ParseFEN(initFEN);

	print_board(board);

	while(!gameover)
	{
		bool islegal = false;

		while(!islegal) {
#ifdef ENTERMOVES
			cout << "Enter a move ";
				
			if(board->turn==WHITE)
				cout << "(White) \n";
			else
				cout << "(Black) \n";

			cin >> move;
#else
			move = movearray[index];
#endif
			if(move == "end")
			{
				gameover = true;
				break;
			}

			if(move == "undo")
			{
				board = boardPrev[--undoIndex];
				print_board(board);
				continue;
			}
			
			if(move == "new")
			{
				gameover = false;
				undoIndex = 0;
				index = 0;
				board = ParseFEN(initFEN);

				print_board(board);

				continue;
			}

			srcsq = toupper(move[0])-'A' + (move[1]-'1')*8;
			dstsq = toupper(move[2])-'A' + (move[3]-'1')*8;

			islegal = isLegalMove(board, srcsq, dstsq, CHECK_FOR_KING_ATTACK);
			if(!islegal)
				cout << "Not a legal move. Try again.\n";
			else
				index++;
		}

		if(!gameover)
		{
			tBoard *newBoard = new(tBoard); 
			makeMove(board, newBoard, srcsq, dstsq);
			boardPrev[undoIndex++] = board;
			board = newBoard;

			cout << "move " << (index/2 + 1) << ". = " << move << "\n";
			print_board(board);
		}
	}
}

void move_test()
{
	string position;
	string initFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w";

	cout<<"Enter a FEN Position or init for initial position: \n";
	//cin >> position;
	getline(cin, position);

	if (position=="init")
		position = initFEN;

	tBoard* board = ParseFEN(position);

}

// 1- Create a Start Node
// 2- Generate All Possible Plays
// 3- Choose One At Random
// 4- Go to 2
void generateGame()
{
	//int seed = time(NULL);
	//int seed = 0x567bcc8c;
	//srand(seed);

	string initFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w";

	tBoard* board = ParseFEN(initFEN);

	tNode *node = createNode(board);

	gameover = false;

	while(!gameover)
	{
		generateAllLegalNodes(node);

		if ( (node->numChildren == 0))
		{
			if(node->board->turn == WHITE)
				cout << "{Black Wins} 0-1\n";
			else
				cout << "{White Wins} 1-0\n";

			gameover = true;
		}
		
		else if(/*(g_movenum > 600)*/ ( (node->board->last[WHITE]==1) && (node->board->last[WHITE] == node->board->last[BLACK])))
		{
			cout << "{DRAW} 0-0";
			gameover = true;
		}
		else
		{
			int random = rand() % node->numChildren;	// pick a randome child
			node = getChildByIndex(node, random);

			unsigned char *moveStr = getMoveString(node->move);


			//cout << (g_movenum++/2)+1 << ". " << moveStr << " Played\n\n";
			if((g_movenum)&1)
				cout << moveStr << "\n";
			else
				cout << (g_movenum/2)+1 << ". " << moveStr << "  ";
			g_movenum++;
 
			if(moveStr[0]=='c' && moveStr[1]=='4' && moveStr[2]=='b' && moveStr[3]=='5')
				int stop = 1;
			//print_board(node->board);
		}
	}
}

void outputMove(tNode *node, int movenum);

void playComputerGame(int gameMode, int colorToPlay)
{
	if(RANDOMIZE_EVERY_RUN)
	{
		int seed = (int)time(NULL);
		//int seed = 0x567bcc8c;
		srand(seed);

		cout << "{ seed = " << seed << " }\n";
	}

	double elapsed;
	int millisStart, millisEnd;

	if(debug_print)
		logfile.open("move_log.txt");

//	initFEN = "rnb1k2r/ppppqppp/5n2/2b1p3/P1B5/4PQ2/NPPP1PPP/R1B1K1NR b";
	initFEN = INIT_FEN;

	tBoard* board = ParseFEN(initFEN);

	print_board(board);

	tNode *node = createNode(board);

	gameover = false;
	undoIndex = 0;

	while(!gameover)
	{
		total_positions = 0;

		if( (gameMode == HUMAN_AGAINST_COMPUTER) && (board->turn == colorToPlay) )
		{
			node = enterMove(node->board);
			node->numChildren = 1; // just get around later check
		}
		else
		{
			if(gameMode == HUMAN_AGAINST_COMPUTER)
				cout << "{Pondering...}\n";

			millisStart = GetTickCount();

			//findBestChildScore_Improved(node, 0, (node->board->turn==WHITE)?MAX_POSITIVE_SCORE:MIN_NEGATIVE_SCORE);
			findBestMove(node); //<<<<<

			millisEnd = GetTickCount();
			elapsed = double(millisEnd-millisStart)/1000.0;
		}

		if( (node->move.srcsq != -1) && (node->move.dstsq != -1) )
			outputMove(node, g_movenum++);

		if(g_movenum==10)
			debug_print = 0;
		if(g_movenum==13)
			debug_print = 0;

		//cout << "{total positions = " << total_positions << "} ";
//		cout << "{total positions = " << total_positions << "} ";
		//cout << "Move time = " << elapsed << "seconds \n";
		//cout << "{" << elapsed << " s} \n";


		if(!node)
		{
			cout << "No legal move can be found \n";
			break;
		}

		if (node->numChildren == 0)
		{
			if (node->score != 0)
			{
				if(node->board->turn == WHITE)
				{
					cout << "{Black Wins} 0-1\n";
					gameover = true;
				}
				else
				{
					cout << "{White Wins} 1-0\n";
					gameover = true;
				}
			}
			else
			{
				cout << "{Stalemate. Game is Drawn} 0-0\n";
				gameover = true;
			}
		}
		
		else if(( (node->board->last[WHITE]==1) && (node->board->last[WHITE] == node->board->last[BLACK])))
		{
			cout << "{DRAW} 0-0";
			gameover = true;
		}

		else if ((g_movenum > (MAX_MOVES*2)) )
		{
			cout << "{Game lasted too long. DRAW} 0-0";
			gameover = true;
		}

		else
		{		
			if(( (gameMode == HUMAN_AGAINST_COMPUTER) && (board->turn != colorToPlay) ) || (gameMode != HUMAN_AGAINST_COMPUTER))
			{
				cout << "{" << total_positions << " ";
				if(elapsed < 60)
					cout << "(" << elapsed << " s) ";
				else
					cout << "(" << elapsed/60 << " min) ";

				cout << double(total_positions)/elapsed << " nodes/s ";
				cout << " }";
			}

			//cout << "\nTime Spent Creating Nodes = " << timeSpentCreatingNodes << " ms\n";
			//cout << "\nTime Spent Generating Nodes = " << timeSpentGeneratingNodes << " ms\n";
			//cout << "\nTime Spent Deleting Nodes = " << timeSpentDeletingNodes << " ms\n";
			//cout << "\nTime Spent Checking if legal = " << timeSpentCheckingIsLegal << " ms\n";
			//cout << "\nTime Spent Making Moves = " << timeSpentMakingMove << " ms\n";
			//cout << "\nTime Spent Creating Children = " << timeSpentCreatingChildren << " ms\n";
			//cout << "\nTime Spent Evaluating = " << timeSpentEvaluatingNodes << " ms\n";

			timeSpentCreatingNodes = timeSpentDeletingNodes = timeSpentGeneratingNodes = timeSpentCheckingIsLegal = timeSpentMakingMove = timeSpentCreatingChildren = timeSpentEvaluatingNodes = 0.0;

			//cout << "max_depth = " << max_depth;
			max_depth = MIN_SEARCH_DEPTH;
			
			if( !(g_movenum & 1))
				cout << "\n";
			
			if(gameMode == HUMAN_AGAINST_COMPUTER)
				print_board(node->board);

			// TODO: There is probably a better way to do this.
			board = node->board;
			//if(node->numChildren != 0)
			//	deleteChildren(node);
			delete node;
			node = createNode(board);
		}
	}
}

tNode* enterMove(tBoard* board)
{
	bool islegal = false;
	string move;
	int srcsq, dstsq;

	while(!islegal) {
		cout << "Enter a move ";
				
		if(board->turn==WHITE)
			cout << "(White) \n";
		else
			cout << "(Black) \n";

		cin >> move;

		if(move == "end")
		{
			gameover = true;
			break;
		}

		if(move == "undo")
		{
			board = boardPrev[--undoIndex];
			print_board(board);
			continue;
		}
			
		if(move == "new")
		{
			gameover = false;
			undoIndex = 0;
			board = ParseFEN(initFEN);

			print_board(board);

			continue;
		}

		srcsq = toupper(move[0])-'A' + (move[1]-'1')*8;
		dstsq = toupper(move[2])-'A' + (move[3]-'1')*8;

		islegal = isLegalMove(board, srcsq, dstsq, CHECK_FOR_KING_ATTACK);
		if(!islegal)
			cout << "Not a legal move. Try again.\n";
	}

	tBoard* newBoard = new(tBoard);
	makeMove(board, newBoard, srcsq, dstsq);
	boardPrev[undoIndex++] = board;
	board = newBoard;
	tNode *node = createNode(board);
	node->move.srcsq = srcsq;
	node->move.dstsq = dstsq;

	return node;
}