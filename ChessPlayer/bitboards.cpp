//TODO: make board width variable
//TODO: typedef basic types
// Ideas for the future
// 1- Store the moves array as a 64-bit mask, as well as the square ID.
// 2- with every board store the following:
//		- Occupied by Me
//		- Occupied by opponent
//		- Obstacles mask, pre-prepared in the form that can be used directly for ANDING or ORing possible 
//		storing both forms.
// 3- every time you want to generate a move, you lookup the source square, then you keep ANDING
// the destination squares, with "Occupied by Me" until you find one that generates a zero. A capture
// is found by ANDING "Occupied by Opponent" with that same mask. Every time you generate a move,
// make sure you update those masks. 
// 4- There is an idea that there must be a way so that I don't have to keep this masking process every single
// time. but I'll leave that till later.
// 5- Once you know the destination square is clear you then use the "Occupied" (which is basically an OR'ing 
// between "Occupied by Me" and "Occupied by Opponent") along with the "Obstacles) mask to know if there are any
// obstacles in between that prevent the movement. 
// 6- Naturally the last step is not needed in case of the Knight and it will be slightly different in case of pawns
// and will be very specialized in case of castling. Also enpassant and pawn promption needs special handling.
// 7- More thoughts on the moves, the order of squares to check obstacles should grow around the piece, hence the order should be made.
// 8- Search has to be terminated in a direction once we know the piece can't go there. For example if a rook on a1 can't move to a2 because
// on an obstacle, there's no point of continuing to check a3, a4, etc.
// 9- Instead of having several parallel arrays for piece movement, you will probably end up having all in one multidimensional array so that
// you can easily index it by piece type. The indexing itself could be done in a way so that no actual addition or multiplication is performed
// by simple bit operations. However it's unclear that this will affect performance so better make the implementation independent of this
// and experiment with changing it later. Make indexing a multidemensional array a Macro so that it can be modified later.

#include "stdafx.h"

#include <iostream>
#include <string>

#include "bitboards.h"
#include "piece.h"
#include "util.h"

__int8 MovesPawnW [64][MAX_POSSIBLE_PAWN_MOVES];
__int8 MovesPawnB [64][MAX_POSSIBLE_PAWN_MOVES];
__int8 MovesKnight[64][MAX_POSSIBLE_KNIGHT_MOVES];
__int8 MovesBishop[64][DIAGS][MAX_POSSIBLE_BISHOP_MOVES_PER_DIR];
__int8 MovesRook  [64][LINES][MAX_POSSIBLE_ROOK_MOVES_PER_DIR];
__int8 MovesQueen [64][ALLDIRS][MAX_POSSIBLE_QUEEN_MOVES_PER_DIR];
__int8 MovesKing  [64][MAX_POSSIBLE_KING_MOVES];

__int8 AllMoves	  [NUM_PIECE_TYPES][64][64];

__int64 BishopObstacles[64][64];
__int64 RookObstacles[64][64];
__int64 QueenObstacles[64][64];


void GenerateAllPawnMoves()
{
	int row,col, move, moveindex, sq;

	// first init arrays
	for(row=0; row<8; row++)
	{
		for(col=0; col<8; col++)
		{
			sq = row*8+col;
			for(move=0; move<MAX_POSSIBLE_PAWN_MOVES; move++)
			{
				MovesPawnW[sq][move] = MovesPawnB[sq][move] = -1;
			}
		}
	}

	// White pawns (moving upward)
	for(row=1; row<7; row++)
	{
		for(col=0; col<8; col++)
		{
			sq = row*8+col;
			moveindex = 0;

			if(INBOARD(row+1, col-1)) MovesPawnW[sq][moveindex++] = (row+1)*8 + (col-1);	// Capture left
			if(INBOARD(row+1, col)) MovesPawnW[sq][moveindex++] = (row+1)*8 + (col);		// Move one square forward
			if(INBOARD(row+1, col+1)) MovesPawnW[sq][moveindex++] = (row+1)*8 + (col+1);	// Capture right
			if(row==1) // starting position
				MovesPawnW[sq][moveindex++] = (row+2)*8 + col;								// Move two squares if in first rank
		}
	}

	// Black pawns (moveing downward)
	for(row=1; row<7; row++)
	{
		for(col=0; col<8; col++)
		{
			sq = row*8+col;
			moveindex = 0;

			if(INBOARD(row-1, col-1)) MovesPawnB[sq][moveindex++] = (row-1)*8 + (col-1);	// Capture left
			if(INBOARD(row-1, col)) MovesPawnB[sq][moveindex++] = (row-1)*8 + (col);		// Move one square forward
			if(INBOARD(row-1, col+1)) MovesPawnB[sq][moveindex++] = (row-1)*8 + (col+1);	// Capture right
			if(row==6) // starting position
				MovesPawnB[sq][moveindex++] = (row-2)*8 + col;								// Move two squares if in first rank
		}
	}

	// Now summarize in AllMoves[]
	int index;
	for(sq=0; sq<64; sq++)
	{
		index = 0;
		for(moveindex=0; moveindex<MAX_POSSIBLE_PAWN_MOVES; moveindex++)
			if(MovesPawnW[sq][moveindex] != -1)
				AllMoves[PAWNW][sq][index++] = MovesPawnW[sq][moveindex];
	}

	for(sq=0; sq<64; sq++)
	{
		index = 0;
		for(moveindex=0; moveindex<MAX_POSSIBLE_PAWN_MOVES; moveindex++)
			if(MovesPawnB[sq][moveindex] != -1)
				AllMoves[PAWNB][sq][index++] = MovesPawnB[sq][moveindex];
	}
}

void GenerateAllKnightMoves()
{
	int row,col, move, moveindex, sq;

	// first init arrays
	for(row=0; row<8; row++)
	{
		for(col=0; col<8; col++)
		{
			sq = row*8+col;
			for(move=0; move<MAX_POSSIBLE_KNIGHT_MOVES; move++)
			{
				MovesKnight[sq][move] = -1;
			}
		}
	}

	// All possible Knight movements
	for(row=0; row<8; row++)
	{
		for(col=0; col<8; col++)
		{
			sq = row*8+col;
			moveindex=0;

			if(INBOARD(row-2, col-1)) MovesKnight[sq][moveindex++] = (row-2)*8 + (col-1);
			if(INBOARD(row-2, col+1)) MovesKnight[sq][moveindex++] = (row-2)*8 + (col+1);
			if(INBOARD(row-1, col-2)) MovesKnight[sq][moveindex++] = (row-1)*8 + (col-2);
			if(INBOARD(row-1, col+2)) MovesKnight[sq][moveindex++] = (row-1)*8 + (col+2);
			if(INBOARD(row+1, col-2)) MovesKnight[sq][moveindex++] = (row+1)*8 + (col-2);
			if(INBOARD(row+1, col+2)) MovesKnight[sq][moveindex++] = (row+1)*8 + (col+2);
			if(INBOARD(row+2, col-1)) MovesKnight[sq][moveindex++] = (row+2)*8 + (col-1);
			if(INBOARD(row+2, col+1)) MovesKnight[sq][moveindex++] = (row+2)*8 + (col+1);
		}
	}

	// Now summarize in AllMoves[]
	int index;
	for(sq=0; sq<64; sq++)
	{
		index = 0;
		for(moveindex=0; moveindex<MAX_POSSIBLE_KNIGHT_MOVES; moveindex++)
			if(MovesKnight[sq][moveindex] != -1)
				AllMoves[KNIGHT][sq][index++] = MovesKnight[sq][moveindex];
	}
}

void GenerateAllBishopMoves()
{
	int i, row,col, moveindex, sq;

	// first init arrays
	for(sq=0; sq<64; sq++)
	{
		for(i=0; i<DIAGS; i++)
		{
			for(moveindex=0; moveindex<MAX_POSSIBLE_BISHOP_MOVES_PER_DIR; moveindex++)
			{
				MovesBishop[sq][i][moveindex] = -1;
			}
		}
	}

	// All possible diagonal
	for(row=0; row<8; row++)
	{
		for(col=0; col<8; col++)
		{
			sq = row*8+col;

			// first diagonal, bottom left
			for(i=1, moveindex=0; i<8; i++)
			{
				if(INBOARD(row-i, col-i)) MovesBishop[sq][B_BOTTOMLEFT][moveindex++] = (row-i)*8+(col-i);
			}
			// second diagonal, bottom right
			for(i=1, moveindex=0; i<8; i++)
			{
				if(INBOARD(row-i, col+i)) MovesBishop[sq][B_BOTTOMRIGHT][moveindex++] = (row-i)*8+(col+i);
			}
			// third diagonal, top left
			for(i=1, moveindex=0; i<8; i++)
			{
				if(INBOARD(row+i, col-i)) MovesBishop[sq][B_TOPLEFT][moveindex++] = (row+i)*8+(col-i);
			}
			// fourth diagonal, top right
			for(i=1, moveindex=0; i<8; i++)
			{
				if(INBOARD(row+i, col+i)) MovesBishop[sq][B_TOPRIGHT][moveindex++] = (row+i)*8+(col+i);
			}
		}
	}

	// Now summarize in AllMoves[]
	int index = 0;
	for(sq=0; sq<64; sq++)
	{
		index = 0;
		for(int diag=0; diag<DIAGS; diag++)
			for(moveindex=0; moveindex<MAX_POSSIBLE_BISHOP_MOVES_PER_DIR; moveindex++)
				if(MovesBishop[sq][diag][moveindex] != -1)
					AllMoves[BISHOP][sq][index++] = MovesBishop[sq][diag][moveindex];
	}
}

void GenerateAllRookMoves()
{
	int i, row,col, moveindex, sq;

	// first init arrays
	for(sq=0; sq<64; sq++)
	{
		for(i=0; i<LINES; i++)
			for(moveindex=0; moveindex<MAX_POSSIBLE_ROOK_MOVES_PER_DIR; moveindex++)
			{
				MovesRook[sq][i][moveindex] = -1;
			}
	}

	// All possible lines
	for(row=0; row<8; row++)
	{
		for(col=0; col<8; col++)
		{
			sq = row*8+col;

			// first the bottom part of the file
			for(i=1, moveindex=0; i<8; i++)
			{
				if(INBOARD(row-i, col)) MovesRook[sq][R_BOTTOM][moveindex++] = (row-i)*8+col;
			}

			// then do the left of the rank
			for(i=1, moveindex=0; i<8; i++)
			{
				if(INBOARD(row, col-i)) MovesRook[sq][R_LEFT][moveindex++] = row*8+(col-i);
			}
			
			// then do the right part of the rank
			for(i=1, moveindex=0; i<8; i++)
			{
				if(INBOARD(row, col+i)) MovesRook[sq][R_RIGHT][moveindex++] = row*8+(col+i);
			}

			// then do the top of the file
			for(i=1, moveindex=0; i<8; i++)
			{
				if(INBOARD(row+i, col)) MovesRook[sq][R_TOP][moveindex++] = (row+i)*8+col;
			}
		} // col
	} // row

	// Now summarize in AllMoves[]
	int index = 0;
	for(sq=0; sq<64; sq++)
	{
		index = 0;
		for(int dir=0; dir<LINES; dir++)
			for(moveindex=0; moveindex<MAX_POSSIBLE_ROOK_MOVES_PER_DIR; moveindex++)
				if(MovesRook[sq][dir][moveindex] != -1)
					AllMoves[ROOK][sq][index++] = MovesRook[sq][dir][moveindex];
	}
}

void GenerateAllQueenMoves()
{
	int i, row,col, moveindex, sq;

	// first init arrays
	for(sq=0; sq<64; sq++)
	{
		for(i=0; i<ALLDIRS; i++)
		{
			for(moveindex=0; moveindex<MAX_POSSIBLE_QUEEN_MOVES_PER_DIR; moveindex++)
			{
				MovesQueen[sq][i][moveindex] = -1;
			}
		}
	}

	for(row=0; row<8; row++)
	{
		for(col=0; col<8; col++)
		{
			sq = row*8+col;
			moveindex=0;

			// First Bishop like moves
			// first diagonal, bottom left
			for(i=1, moveindex=0; i<8; i++)
			{
				if(INBOARD(row-i, col-i)) MovesQueen[sq][Q_BOTTOMLEFT][moveindex++] = (row-i)*8+(col-i);
			}
			// second diagonal, bottom right
			for(i=1, moveindex=0; i<8; i++)
			{
				if(INBOARD(row-i, col+i)) MovesQueen[sq][Q_BOTTOMRIGHT][moveindex++] = (row-i)*8+(col+i);
			}
			// third diagonal, top left
			for(i=1, moveindex=0; i<8; i++)
			{
				if(INBOARD(row+i, col-i)) MovesQueen[sq][Q_TOPLEFT][moveindex++] = (row+i)*8+(col-i);
			}
			// fourth diagonal, top right
			for(i=1, moveindex=0; i<8; i++)
			{
				if(INBOARD(row+i, col+i)) MovesQueen[sq][Q_TOPRIGHT][moveindex++] = (row+i)*8+(col+i);
			}

			// then Rook like moves
			// first the bottom part of the file
			for(i=1, moveindex=0; i<8; i++)
			{
				if(INBOARD(row-i, col)) MovesQueen[sq][Q_BOTTOM][moveindex++] = (row-i)*8+col;
			}

			// then do the left of the rank
			for(i=1, moveindex=0; i<8; i++)
			{
				if(INBOARD(row, col-i)) MovesQueen[sq][Q_LEFT][moveindex++] = row*8+(col-i);
			}
			
			// then do the right part of the rank
			for(i=1, moveindex=0; i<8; i++)
			{
				if(INBOARD(row, col+i)) MovesQueen[sq][Q_RIGHT][moveindex++] = row*8+(col+i);
			}

			// then do the top of the file
			for(i=1, moveindex=0; i<8; i++)
			{
				if(INBOARD(row+i, col)) MovesQueen[sq][Q_TOP][moveindex++] = (row+i)*8+col;
			}
		} // col
	} // row

	// Now summarize in AllMoves[]
	int index = 0;
	for(sq=0; sq<64; sq++)
	{
		index = 0;
		for(int dir=0; dir<ALLDIRS; dir++)
			for(moveindex=0; moveindex<MAX_POSSIBLE_QUEEN_MOVES_PER_DIR; moveindex++)
				if(MovesQueen[sq][dir][moveindex] != -1)
					AllMoves[QUEEN][sq][index++] = MovesQueen[sq][dir][moveindex];
	}
}

void GenerateAllKingMoves()
{
	int row,col, move, moveindex, sq;

	// first init arrays
	for(row=0; row<8; row++)
	{
		for(col=0; col<8; col++)
		{
			sq = row*8+col;
			for(move=0; move<MAX_POSSIBLE_KING_MOVES; move++)
			{
				MovesKing[sq][move] = -1;
			}
		}
	}

	// All possible King movements
	for(row=0; row<8; row++)
	{
		for(col=0; col<8; col++)
		{
			sq = row*8+col;
			moveindex=0;

			if( (sq == 4) || (sq == 60) ) // King castling
			{
				MovesKing[sq][moveindex++] = sq+2; // castle right
				MovesKing[sq][moveindex++] = sq-2; // castle left
			}

			if(INBOARD(row-1, col-1)) MovesKing[sq][moveindex++] = (row-1)*8 + (col-1);
			if(INBOARD(row-1, col)) MovesKing[sq][moveindex++] = (row-1)*8 + (col);
			if(INBOARD(row-1, col+1)) MovesKing[sq][moveindex++] = (row-1)*8 + (col+1);
			if(INBOARD(row, col-1)) MovesKing[sq][moveindex++] = (row)*8 + (col-1);
			if(INBOARD(row, col+1)) MovesKing[sq][moveindex++] = (row)*8 + (col+1);
			if(INBOARD(row+1, col-1)) MovesKing[sq][moveindex++] = (row+1)*8 + (col-1);
			if(INBOARD(row+1, col)) MovesKing[sq][moveindex++] = (row+1)*8 + (col);
			if(INBOARD(row+1, col+1)) MovesKing[sq][moveindex++] = (row+1)*8 + (col+1);
		}
	}

	// Now summarize in AllMoves[]
	int index = 0;
	for(sq=0; sq<64; sq++)
	{
		index = 0;
		for(moveindex=0; moveindex<MAX_POSSIBLE_KING_MOVES; moveindex++)
			if(MovesKing[sq][moveindex] != -1)
				AllMoves[KING][sq][index++] = MovesKing[sq][moveindex];
	}
}

void GenerateAllMoves()
{
	int piece, srcsq, moveindex;

	// All Moves is an easier indexed move list. It may be all we need. Not sure what was the thinking separating diagonals and directions
	for(piece=0; piece<NUM_PIECE_TYPES; piece++)
		for(srcsq=0; srcsq<64; srcsq++)
			for(moveindex=0; moveindex<64; moveindex++)
				AllMoves[piece][srcsq][moveindex] = -1;

	// this handles move generation on a very basic level. Two special cases are handled elsewhere, King's castling and pawn capture.
	GenerateAllPawnMoves();
	GenerateAllKnightMoves();
	GenerateAllBishopMoves();
	GenerateAllRookMoves();
	GenerateAllQueenMoves();
	GenerateAllKingMoves();
}

void GenerateBishopObstacles()
{
	int src, dst, prev, diags, i;
	__int64 mask;

	for(src=0; src<64; src++)
		for(dst=0; dst<64; dst++)
			BishopObstacles[src][dst] = -1; // Maybe needed to change to some other value (possibly zero) depending on the algorithm

	for(src=0; src<64; src++)
	{
		for(diags=0; diags<DIAGS; diags++)
		{
			for(i=0, dst=0, prev=0; dst!=-1; i++)
			{
				dst = MovesBishop[src][diags][i];

				if(dst!=-1) {
					if(prev==0)
						mask=0;		// the first square in the array is the most adjacent to the current square so there are not obstacles
					else
						mask |= 1ULL<<(prev);
					BishopObstacles[src][dst] = mask;
					prev = dst;
				}
			}
		}
	}
}

void GenerateRookObstacles()
{
	int src, dst, prev, lines, i;
	__int64 mask;

	for(src=0; src<64; src++)
		for(dst=0; dst<64; dst++)
			RookObstacles[src][dst] = -1; // Maybe needed to change to some other value (possibly zero) depending on the algorithm

	for(src=0; src<64; src++)
	{
		for(lines=0; lines<LINES; lines++)
		{
			for(i=0, dst=0, prev=0; dst!=-1; i++)
			{
				dst = MovesRook[src][lines][i];

				if(dst!=-1) {
					if(prev==0)
						mask=0;		// the first square in the array is the most adjacent to the current square so there are not obstacles
					else
						mask |= 1ULL<<(prev);
					RookObstacles[src][dst] = mask;
					prev = dst;
				}
			}
		}
	}
}

void GenerateQueenObstacles()
{
	int src, dst, prev, dirs, i;
	__int64 mask;

	for(src=0; src<64; src++)
		for(dst=0; dst<64; dst++)
			QueenObstacles[src][dst] = -1; // Maybe needed to change to some other value (possibly zero) depending on the algorithm

	for(src=0; src<64; src++)
	{
		for(dirs=0; dirs<ALLDIRS; dirs++)
		{
			for(i=0, dst=0, prev=0; dst!=-1; i++)
			{
				dst = MovesQueen[src][dirs][i];

				if(dst!=-1) {
					if(prev==0)
						mask=0;		// the first square in the array is the most adjacent to the current square so there are not obstacles
					else
						mask |= 1ULL<<(prev);
					QueenObstacles[src][dst] = mask;
					prev = dst;
				}
			}
		}
	}
}

void GenerateObstacles()
{
	GenerateBishopObstacles();
	GenerateRookObstacles();
	GenerateQueenObstacles();
}

void bitboards_init()
{
	GenerateAllMoves();
	GenerateObstacles();
}

void initBoard(tBoard* board)
{
	board->turn = WHITE;

	for(int color=0; color<COLORS; color++)
		for(int piece=0; piece<MAXPIECES; piece++) {
			board->Pieces[color][piece].square = -1;
			board->Pieces[color][piece].type = -1;
			board->Pieces[color][piece].moved = false;
		}
	
	for(int color=0; color<COLORS; color++)
		for(int sq=0; sq<64; sq++)
			board->boardByPiece[color][sq] = 0;

	board->Occupiedby[WHITE] = board->Occupiedby[BLACK] = 0;
	board->last[BLACK] = board->last[WHITE] = 0;

	board->inCheck = false;
	board->isCapture = false;
	board->higherPieceThreatened = false;

	board->isCastled[WHITE] = board->isCastled[BLACK] = false; // todo: initialized from FEN
	board->kingMoved[WHITE] = board->kingMoved[BLACK] = false;
	board->rooksMoved[WHITE] = board->rooksMoved[BLACK] = 0;
	board->development[WHITE] = board->development[BLACK] = 0;
}

///////////////////////////////////////////
// Test
void print_legal_moves(string move)
{
	int i, piece, row, col, irow, icol, sq, nextmove, dir, idx;
	int board[64], output[64];

	piece = toupper(move[0]);
	col = toupper(move[1]) - 'A';
	row = toupper(move[2]) - '1';
	sq = row*8+col;

	for (irow=0; irow<8; irow++)
		for(icol=0; icol<8; icol++)
			board[irow*8+icol] = output[irow*8+icol] = -1;

	board[sq] = piece;

	if(piece=='P' || piece=='Z' || piece=='N' || piece=='K')
	{
		for(i=0, nextmove=0; nextmove!=-1; i++)
		{
			if(piece=='P') // white pawn
				nextmove = MovesPawnW[sq][i];
			else if (piece=='Z') // black pawn
				nextmove = MovesPawnB[sq][i];
			else if (piece=='N') // knight
				nextmove = MovesKnight[sq][i];
			else if (piece=='K') // king
				nextmove = MovesKing[sq][i];

			if (nextmove != -1) {
				board[nextmove] = '*';
				output[i] = nextmove;
			}
		}
	}
	else
	{
		idx=0;
		for(dir=0; dir<8; dir++)
		{
			if(dir>3 && piece!='Q')
				break;
			for(i=0, nextmove=0; nextmove!=-1; i++)
			{
				if (piece=='B') // bishop
					nextmove = MovesBishop[sq][dir][i];
				else if (piece=='R') // rook
					nextmove = MovesRook[sq][dir][i];
				else if (piece=='Q') // queen
					nextmove = MovesQueen[sq][dir][i];

				if (nextmove != -1) {
					board[nextmove] = '*';
					output[idx++] = nextmove;
				}
			}
		}
	}

	// Now that everything is filled, print the board
	cout << " ";
	for(col=0; col<8; col++)
		cout << "_ ";
	cout << "\n";

	for(row=0; row<8; row++) {
		cout << '|';
		for(col=0; col<8; col++) {
			sq = (7-row)*8+col;
			if(board[sq] != -1)
				cout << char(board[sq]) << '|';
			else
				cout << " |";
		}
		cout << "\n";
	}

	cout << " ";
	for(col=0; col<8; col++)
		cout << char(196) << " ";
	cout << "\n";

	// Now output the coordinates
	nextmove = 0;
	for(i=0; nextmove != -1; i++) {
		nextmove = output[i];
		if(nextmove != -1)
		{
			row = nextmove/8;
			col = nextmove%8;
			cout << char(col+'a') << char(row+'1');
			if (output[i+1] != -1)
				cout <<", ";
			else
				cout << "\n";
		}
	}
}

void print_board(tBoard* board)
{
	int row, col, sq;
	tPieceInfo tPI;
	char pch;

	cout << "\n\n\n\n";
	cout << "     a   b   c   d   e   f   g   h  \n ";
	for(row=7; row>=0; row--) // rows drawn top to bottom
	{
		cout << "    --------------------------------\n ";
		for(col=0; col<8; col++)
		{
			if(col==0)
				cout << (row+1) << "  ";

			cout << "| ";

			sq = TOSQUARE(row, col);
			tPI = getPieceInfo(board, sq);

			switch (tPI.type)
			{
			case PAWN:
				pch = 'p';
				break;
			case KNIGHT:
				pch = 'n'; 
				break;
			case BISHOP:
				pch = 'b'; 
				break;
			case ROOK:
				pch = 'r'; 
				break;
			case QUEEN:
				pch = 'q'; 
				break;
			case KING:
				pch = 'k'; 
				break;
			default:
				pch = ' '; 
				break;
			}

			if(tPI.color == WHITE)
				pch = toupper(pch);

			cout << pch << " ";
		}

		cout << "|\n";
	}

	cout << "    --------------------------------\n ";
	cout << "     a   b   c   d   e   f   g   h  \n ";
	cout << "\n\n\n\n";
	if(board->turn == WHITE)
		cout << "White to play\n";
	else
		cout << "Black to play\n";

	//// Now that everything is filled, print the board
	//cout << " ";
	//for(col=0; col<8; col++)
	//	cout << "_ ";
	//cout << "\n";

	//for(row=0; row<8; row++) {
	//	cout << '|';
	//	for(col=0; col<8; col++) {
	//		sq = (7-row)*8+col;
	//		if(board[sq] != -1)
	//			cout << char(board[sq]) << '|';
	//		else
	//			cout << " |";
	//	}
	//	cout << "\n";
	//}

	//cout << " ";
	//for(col=0; col<8; col++)
	//	cout << char(196) << " ";
	//cout << "\n";

	//// Now output the coordinates
	//nextmove = 0;
	//for(i=0; nextmove != -1; i++) {
	//	nextmove = output[i];
	//	if(nextmove != -1)
	//	{
	//		row = nextmove/8;
	//		col = nextmove%8;
	//		cout << char(col+'a') << char(row+'1');
	//		if (output[i+1] != -1)
	//			cout <<", ";
	//		else
	//			cout << "\n";
	//	}
	//}
}

void bitboards_test()
{
	string move;

	while(1)
	{
		cout<<"What is the move: ";
		cin >> move;

		if (move == "exit")
			return;
		else
			print_legal_moves(move);
	}
}
