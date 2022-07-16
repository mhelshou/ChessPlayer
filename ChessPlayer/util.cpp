#include "stdafx.h"
#include <Windows.h>
#include "bitboards.h"
#include "util.h"

tBoard* ParseFEN(string fen)
{
	tBoard* board = new(tBoard);
	int ci=0, sq=0;
	char tomove='w';

	__int64 OccupiedbyWhite=0, OccupiedbyBlack = 0;

	initBoard(board);

	while(fen[ci]!=0)
	{
		if (sq>64)
		{
			cout<<"FEN too long\n";
			break;
		}

		int bbsq = FENTOSQ(sq);

		switch(fen[ci])
		{
		case 'p':
			board->Pieces[BLACK][board->last[BLACK]].square = FENTOSQ(sq);
			board->Pieces[BLACK][board->last[BLACK]].type = PAWN;
			OccupiedbyBlack |= 1ULL<<FENTOSQ(sq);
			board->boardByPiece[BLACK][bbsq] = PAWN;
			board->last[BLACK]++;
			ci++;
			sq++;
			break;
		case 'P':
			board->Pieces[WHITE][board->last[WHITE]].square = FENTOSQ(sq);
			board->Pieces[WHITE][board->last[WHITE]].type = PAWN;
			OccupiedbyWhite |= 1ULL<<FENTOSQ(sq);
			board->boardByPiece[WHITE][bbsq] = PAWN;
			board->last[WHITE]++;
			ci++;
			sq++;
			break;
		case 'n':
			board->Pieces[BLACK][board->last[BLACK]].square = FENTOSQ(sq);
			board->Pieces[BLACK][board->last[BLACK]].type = KNIGHT;
			OccupiedbyBlack |= 1ULL<<FENTOSQ(sq);
			board->boardByPiece[BLACK][bbsq] = KNIGHT;
			board->last[BLACK]++;
			ci++;
			sq++;
			break;
		case 'N':
			board->Pieces[WHITE][board->last[WHITE]].square = FENTOSQ(sq);
			board->Pieces[WHITE][board->last[WHITE]].type = KNIGHT;
			OccupiedbyWhite |= 1ULL<<FENTOSQ(sq);
			board->boardByPiece[WHITE][bbsq] = KNIGHT;
			board->last[WHITE]++;
			ci++;
			sq++;
			break;
		case 'b':
			board->Pieces[BLACK][board->last[BLACK]].square = FENTOSQ(sq);
			board->Pieces[BLACK][board->last[BLACK]].type = BISHOP;
			OccupiedbyBlack |= 1ULL<<FENTOSQ(sq);
			board->boardByPiece[BLACK][bbsq] = BISHOP;
			board->last[BLACK]++;
			ci++;
			sq++;
			break;
		case 'B':
			board->Pieces[WHITE][board->last[WHITE]].square = FENTOSQ(sq);
			board->Pieces[WHITE][board->last[WHITE]].type = BISHOP;
			OccupiedbyWhite |= 1ULL<<FENTOSQ(sq);
			board->boardByPiece[WHITE][bbsq] = BISHOP;
			board->last[WHITE]++;
			ci++;
			sq++;
			break;
		case 'r':
			board->Pieces[BLACK][board->last[BLACK]].square = FENTOSQ(sq);
			board->Pieces[BLACK][board->last[BLACK]].type = ROOK;
			OccupiedbyBlack |= 1ULL<<FENTOSQ(sq);
			board->boardByPiece[BLACK][bbsq] = ROOK;
			board->last[BLACK]++;
			ci++;
			sq++;
			break;
		case 'R':
			board->Pieces[WHITE][board->last[WHITE]].square = FENTOSQ(sq);
			board->Pieces[WHITE][board->last[WHITE]].type = ROOK;
			OccupiedbyWhite |= 1ULL<<FENTOSQ(sq);
			board->boardByPiece[WHITE][bbsq] = ROOK;
			board->last[WHITE]++;
			ci++;
			sq++;
			break;
		case 'q':
			board->Pieces[BLACK][board->last[BLACK]].square = FENTOSQ(sq);
			board->Pieces[BLACK][board->last[BLACK]].type = QUEEN;
			OccupiedbyBlack |= 1ULL<<FENTOSQ(sq);
			board->boardByPiece[BLACK][bbsq] = QUEEN;
			board->last[BLACK]++;
			ci++;
			sq++;
			break;
		case 'Q':
			board->Pieces[WHITE][board->last[WHITE]].square = FENTOSQ(sq);
			board->Pieces[WHITE][board->last[WHITE]].type = QUEEN;
			OccupiedbyWhite |= 1ULL<<FENTOSQ(sq);
			board->boardByPiece[WHITE][bbsq] = QUEEN;
			board->last[WHITE]++;
			ci++;
			sq++;
			break;
		case 'k':
			board->Pieces[BLACK][board->last[BLACK]].square = FENTOSQ(sq);
			board->Pieces[BLACK][board->last[BLACK]].type = KING;
			OccupiedbyBlack |= 1ULL<<FENTOSQ(sq);
			board->boardByPiece[BLACK][bbsq] = KING;
			board->last[BLACK]++;
			ci++;
			sq++;
			break;
		case 'K':
			board->Pieces[WHITE][board->last[WHITE]].square = FENTOSQ(sq);
			board->Pieces[WHITE][board->last[WHITE]].type = KING;
			OccupiedbyWhite |= 1ULL<<FENTOSQ(sq);
			board->boardByPiece[WHITE][bbsq] = KING;
			board->last[WHITE]++;
			ci++;
			sq++;
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
			sq += (fen[ci]-'0');
			ci++;
			break;
		case '/':
			if ((sq%8) != 0)
			{
				cout<<"Not enough letters in FEN line\n";
				goto end;
			}
			ci++;
			break;
		case ' ':
			if(sq!=64)
			{
				cout<<"Not enough letters in FEN line\n";
				goto end;
			}
			// skip white space
			while(fen[ci]!='w' && fen[ci]!= 'b')
				ci++;
			tomove=fen[ci];

			// skip everything else after
			goto end;
		default:
			cout<<"Illegal character in FEN\n";
			//exit loop
			goto end;			
		}
	}

end:

	board->Occupiedby[WHITE] = OccupiedbyWhite;
	board->Occupiedby[BLACK] = OccupiedbyBlack;

	if(tomove == 'w')
	{
		board->turn = WHITE;
	}
	else
	{
		board->turn = BLACK;
	}

	return board;
}

double PCFreq = 0.0;
__int64 internalCounter[10];

void setCounterFrequency()
{
    LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li))
	cout << "QueryPerformanceFrequency failed!\n";

    PCFreq = double(li.QuadPart)/1000.0;
}

double getPCFrequency()
{
	return PCFreq;
}

void startCounter(int counterId)
{
    LARGE_INTEGER li;

	QueryPerformanceCounter(&li);
    internalCounter[counterId] = li.QuadPart;
}

void updateCounter(int counterId, double *countVar)
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    *countVar += double(li.QuadPart-internalCounter[counterId])/PCFreq;
}
