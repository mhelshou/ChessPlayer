#include "stdafx.h"
#include <math.h>
#include <iostream>

#include "moves.h"
#include "bitboards.h"
#include "piece.h"
#include "attack.h"
#include "node.h"
#include "evaluate.h"
#include "util.h"

using namespace std;

__int64 CASTLE_RIGHT_MASK[] = {CASTLE_RIGHT_MASK_WHITE, CASTLE_RIGHT_MASK_BLACK};
__int64 CASTLE_LEFT_MASK[]  = {CASTLE_LEFT_MASK_WHITE, CASTLE_LEFT_MASK_BLACK};

int developmentFactor[] = {0, PAWN_DEVELOPMENT_FACTOR, KNIGHT_DEVELOPMENT_FACTOR, BISHOP_DEVELOPMENT_FACTOR, ROOK_DEVELOPMENT_FACTOR, QUEEN_DEVELOPMENT_FACTOR, KING_DEVELOPMENT_FACTOR};
int developmentFactorCastling[] = {0, 0, 0, 0,  ROOK_DEVELOPMENT_FACTOR_CASTLING, 0, KING_DEVELOPMENT_FACTOR_CASTLING};

bool isLegalPawn(tBoard* board, int srcsq, int dstsq)
{
	int srcRow = srcsq / 8;
	int dstRow = dstsq / 8;
	int srcCol = srcsq - srcRow*8;
	int dstCol = dstsq - dstRow*8;
	int turn = board->turn;
	int opp  = OPPOSITE_COLOR(turn);
	// TODO: king safety

	// forward move. Destination must have no piece
	if (srcCol == dstCol)
	{
		if(((board->Occupiedby[opp]&(1ULL<<dstsq))!=0))
			return false;

		if(board->turn == WHITE)
		{
			// one move forward
			if((dstRow - srcRow) == 1)
				return true;
			// two moves forward on first row and nothing exists in the intermediate square
			else if(((dstRow - srcRow) == 2) && (srcRow == 1) && (((board->Occupiedby[turn] | board->Occupiedby[opp])&(1ULL<<(srcsq+8))) == 0) )
				return true;
			else
				return false;
		}
		else
		{
			// one move forward
			if((dstRow - srcRow) == -1)
				return true;
			// two moves forward on first row and nothing exists in the intermediate square
			else if(((dstRow - srcRow) == -2) && (srcRow == 6) && (((board->Occupiedby[turn] | board->Occupiedby[opp])&(1ULL<<(srcsq-8))) == 0) )
				return true;
			else
				return false;
		}
	}
	// Are we moving more than row forward (backward for black) ?
	else if( (dstRow - srcRow) != ((turn == WHITE)?1:-1) )
		return false;
	// Are we trying to capture more than one column away?
	else if( ((dstCol - srcCol) != 1) && ((dstCol - srcCol) != -1) )
		return false;
	else if ( (board->pawnJumpSq != -1) && ((dstsq - board->pawnJumpSq) == (board->turn==WHITE?8:-8) ) ) // en passant is legal
		return true;
	else
		if(((board->Occupiedby[opp]&(1ULL<<dstsq))==0))
			return false;
	
	return true;
}

bool isLegalKnight(tBoard* board, int srcsq, int dstsq)
{
	int i=0;

	for(i=0; i<MAX_POSSIBLE_KNIGHT_MOVES; i++)
		if(MovesKnight[srcsq][i] == dstsq)
			return true;

	return false;
}

bool isLegalBishop(tBoard* board, int srcsq, int dstsq)
{
	int turn = board->turn;
	int opp  = OPPOSITE_COLOR(turn);

	if( ((board->Occupiedby[turn] | board->Occupiedby[opp]) & BishopObstacles[srcsq][dstsq]) != 0 )
		return false;
	else
		return true;
}

bool isLegalRook(tBoard* board, int srcsq, int dstsq)
{
	int turn = board->turn;
	int opp  = OPPOSITE_COLOR(turn);

	if( ((board->Occupiedby[turn] | board->Occupiedby[opp]) & RookObstacles[srcsq][dstsq]) != 0 )
		return false;
	else
		return true;
}

bool isLegalQueen(tBoard* board, int srcsq, int dstsq)
{
	int turn = board->turn;
	int opp  = OPPOSITE_COLOR(turn);

	if( ((board->Occupiedby[turn] | board->Occupiedby[opp]) & QueenObstacles[srcsq][dstsq]) != 0 )
		return false;
	else
		return true;
}

bool isLegalKing(tBoard* board, tPieceInfo king, int srcsq, int dstsq)
{
	int srcRow = srcsq / 8;
	int dstRow = dstsq / 8;
	int srcCol = srcsq - srcRow*8;
	int dstCol = dstsq - dstRow*8;

	if( (abs(srcRow - dstRow) <= 1) && (abs(srcCol - dstCol) <= 1) )
		return true;
	else if (abs(dstsq - srcsq) == 2)
	{
		return canCastle(board, king, srcsq, dstsq);
	}
	else
		return false;
}

// A move is legal if:
// 1- The king is not threatened after making the move
// 2- The distination square doesn't have a piece of same color
// 3- The piece has no obstacles (if not a Knight)
// 4- The destination square is either empty or has a piece of a different color (except for pawns moving forward it has to be empty)

bool isLegalMove(tBoard* board, int srcsq, int dstsq, bool checkForKingAttack)
{
	tBoard newBoard;

	tPieceInfo pieceInfo;
	int turn = board->turn;
	int opp  = OPPOSITE_COLOR(turn);
	bool isLegal = false;
	bool res = true;

	//startCounter(ISLEGAL_COUNTER);

	// First check we're not standstill
	if (srcsq == dstsq)
		res = false;

	else if (srcsq < 0 || srcsq > 63 || dstsq < 0 || dstsq > 63)
		res =  false;

	// do we have a piece there?
	else if ( (board->Occupiedby[turn]&(1ULL<<srcsq)) == 0)
		res = false;

	// does the destination have a piece with our color?
	else if ( (board->Occupiedby[turn]&(1ULL<<dstsq)) != 0)
		res =  false;

	else
	{
		pieceInfo = getPieceInfo(board, srcsq); // TODO: don't we have this information already? maybe just pass it to the function

		switch (pieceInfo.type)
		{
		case PAWN:
			isLegal = isLegalPawn(board, srcsq, dstsq);
			break;
		case KNIGHT:
			isLegal = isLegalKnight(board, srcsq, dstsq);
			break;
		case BISHOP:
			isLegal = isLegalBishop(board, srcsq, dstsq);
			break;
		case ROOK:
			isLegal = isLegalRook(board, srcsq, dstsq);
			break;
		case QUEEN:
			isLegal = isLegalQueen(board, srcsq, dstsq);
			break;
		case KING:
			isLegal = isLegalKing(board, pieceInfo, srcsq, dstsq);
			break;
		default:
			isLegal = false;
		}

		res = isLegal;

		if(isLegal && checkForKingAttack)
		{
			// Now we need to see if the king is attacked after making the move
			makeMove(board, &newBoard, srcsq, dstsq);

			if(canAttackKing(&newBoard))
				res = false;
		}
	}

	//updateCounter(ISLEGAL_COUNTER, &timeSpentCheckingIsLegal);

	return res;
}

// We can castle if:
// 1- King hasn't moved
// 2- Rook hasn't moved
// 3- King and Rook are on the first rank (from player's perspective)
// 4- King is not under check
// 5- No pieces exist between the King and Rook
// 6- King movement squares are not under attack

bool canCastle(tBoard* board, tPieceInfo king, int srcsq, int dstsq)
{
	int turn = board->turn;
	int opp  = OPPOSITE_COLOR(turn);
	tPieceInfo rook;

	if(abs(srcsq-dstsq) != 2)
		return false;
	if( (king.type != KING) || (king.color != turn) )
		return false;

	if(dstsq > srcsq)
		rook = getPieceInfo(board, dstsq + 1);
	else
		rook = getPieceInfo(board, dstsq - 2);

	if(king.moved || rook.moved)
		return false;

	if( (rook.type != ROOK) || (rook.color != turn) )
		return false;

	int kingRow = GETROW(king.square);
	int rookRow = GETROW(rook.square);

	if( (turn == WHITE) && ( (kingRow != 0) || (rookRow != 0) ) )
		return false;
	if( (turn == BLACK) && ( (kingRow != 7) || (rookRow != 7) ) )
		return false;

	if(isUnderAttack(board, king.square))
		return false;

	__int64 occupiedMask = 	board->Occupiedby[turn] | board->Occupiedby[opp];

	if(dstsq > srcsq)
	{ // Castle right
		if( (CASTLE_RIGHT_MASK[turn] & occupiedMask) != 0 )
			return false;

		if(isUnderAttack(board, king.square+1))
			return false;
		if(isUnderAttack(board, king.square+2))
			return false;
	}
	else
	{ // Castle Left
		if( (CASTLE_LEFT_MASK[turn] & occupiedMask) != 0 )
			return false;

		if(isUnderAttack(board, king.square-1))
			return false;
		if(isUnderAttack(board, king.square-2))
			return false;
	}

	return true;
}

void makeMove(tBoard* board, tBoard* newBoard, int srcSq, int dstSq)
{
	tPieceInfo srcPiece, dstPiece;
	int turn = board->turn;
	int opp  = OPPOSITE_COLOR(turn);
	int srcRow = GETROW(srcSq);
	int dstRow = GETROW(dstSq);
	int srcCol = GETCOL(srcSq);
	int dstCol = GETCOL(dstSq);

	srcPiece = getPieceInfo(board, srcSq);

	if( (srcPiece.type == KING) && (abs(dstSq - srcSq) == 2) )
	{
		doCastling(board, newBoard, srcSq, dstSq);
	}
	else
	{
		*newBoard = *board;

		newBoard->Occupiedby[turn] &= ~(1ULL<<srcSq);	// Remove from source square
		newBoard->Occupiedby[turn] |= (1ULL<<dstSq);		// Add to destination square

		newBoard->boardByPiece[turn][srcSq] = 0;
		newBoard->boardByPiece[turn][dstSq] = srcPiece.type;

		newBoard->Pieces[board->turn][srcPiece.index].square = dstSq;

		newBoard->isCapture = false;
		newBoard->inCheck = false;

		if( (srcPiece.type == PAWN) && (abs(srcRow - dstRow) == 1) && (abs(srcCol - dstCol) == 1) && ISEMPTY(board, dstSq) ) // todo: we're probably have this info already from checking legality
		{
			// Pawn capture, it must en passant
			// Capture happened
			dstSq = TOSQUARE(srcRow, dstCol);
		}

		if( (newBoard->Occupiedby[opp] & (1ULL<<dstSq)) != 0)
		{
			// Capture happened
			dstPiece = getPieceInfo(board, dstSq);
			newBoard->Occupiedby[opp] &= ~(1ULL<<dstSq);	// Remove from destination square (capture)
			newBoard->boardByPiece[turn][dstSq] = 0;
			newBoard->Pieces[1-board->turn][dstPiece.index] = newBoard->Pieces[1-board->turn][board->last[1-board->turn]-1]; // replace the captured piece with the last accounted for piece
			newBoard->last[1-board->turn]--;
			newBoard->isCapture = true;
		}

		if(srcPiece.type == KING)
			newBoard->kingMoved[turn] = true;
		if( (srcPiece.type == ROOK) && (srcPiece.moved == false))	// If a Rook hasn't moved before increment the rook moved count (used for evaluation)
			newBoard->rooksMoved[turn]++;

		if(!srcPiece.moved)
		{
			newBoard->Pieces[board->turn][srcPiece.index].moved = true;
			newBoard->development[turn] += developmentFactor[srcPiece.type];
		}

		// Pawn promotion
		// TODO: Always promoting to a Queen for now
		if( (srcPiece.type == PAWN) && (turn == WHITE) && (dstRow == 7) )
			newBoard->Pieces[board->turn][srcPiece.index].type = QUEEN;

		if( (srcPiece.type == PAWN) && (turn == BLACK) && (dstRow == 0) )
			newBoard->Pieces[board->turn][srcPiece.index].type = QUEEN;
	}

	if ( (srcPiece.type == PAWN) && (abs(srcRow - dstRow) == 2) )
		newBoard->pawnJumpSq = dstSq;	// pawn jump 2 squares. Needed for en passant
	else
		newBoard->pawnJumpSq = -1;

	newBoard->higherPieceThreatened = false;	// start out as false, only when you generate children of this board can you tell if this will be true;
	newBoard->turn = 1 - board->turn;

	if(isInCheck(newBoard))
		newBoard->inCheck = true;
}

// Will castle but doesn't do any checks
// These are assumed to have been done in checking legal moves phase
void doCastling(tBoard* board, tBoard* newBoard, int srcSq, int dstSq)
{
	tPieceInfo king, rook;
	int rookDstSq;
	int turn = board->turn;
	int opp  = OPPOSITE_COLOR(turn);

	*newBoard = *board;

	king = getPieceInfo(board, srcSq);

	if(dstSq > srcSq)
	{	// Castle right
		rook = getPieceInfo(board, dstSq+1);
		rookDstSq = dstSq-1; // to the left of the king
	}
	else // Castle left
	{
		rook = getPieceInfo(board, dstSq-2);
		rookDstSq = dstSq+1;		// to the right of the king
	}

	// TODO: make sure that indeed king and rook exist on the right squares

	newBoard->Occupiedby[turn] &= ~(1ULL<<king.square);	// Remove king from source square
	newBoard->Occupiedby[turn] |= (1ULL<<dstSq);			// Add to destination square

	newBoard->boardByPiece[turn][king.square] = 0;
	newBoard->boardByPiece[turn][dstSq] = KING;

	newBoard->Occupiedby[turn] &= ~(1ULL<<rook.square);	// Remove rook from source square
	newBoard->Occupiedby[turn] |= (1ULL<<rookDstSq);		// Add to destination square

	newBoard->boardByPiece[turn][rook.square] = 0;
	newBoard->boardByPiece[turn][rookDstSq] = ROOK;

	newBoard->Pieces[board->turn][king.index].square = dstSq;
	newBoard->Pieces[board->turn][rook.index].square = rookDstSq;

	if(!newBoard->kingMoved[turn])
	{
		newBoard->Pieces[board->turn][king.index].moved = true;
		newBoard->kingMoved[turn] = true;
		newBoard->development[turn] += developmentFactorCastling[king.type];
	}

	if( rook.moved == false)	// If a Rook hasn't moved before increment the rook moved count (used for evaluation)
	{
		newBoard->Pieces[board->turn][rook.index].moved = true;
		newBoard->rooksMoved[turn]++;
		newBoard->development[turn] += developmentFactorCastling[rook.type];
	}

	newBoard->isCastled[board->turn] = true;
}

unsigned char* getMoveString(tMove move)
{
	int srcsq=move.srcsq, dstsq=move.dstsq;
	int srcRow = srcsq / 8;
	int dstRow = dstsq / 8;
	int srcCol = srcsq - srcRow*8;
	int dstCol = dstsq - dstRow*8;

	unsigned char *str = new unsigned char[5];
	
	str[0] = 'a' + srcCol;
	str[1] = '1' + srcRow;
	str[2] = 'a' + dstCol;
	str[3] = '1' + dstRow;
	str[4] = 0;

	return str;
}

void outputMove(tNode *node, int movenum)
{
	unsigned char *moveStr = getMoveString(node->move);
	
	//cout << (movenum++/2)+1 << ". " << moveStr << " Played\n\n";

	if((movenum)&1)
		cout << " ... " << moveStr << "  ";
	else
		cout << (movenum/2)+1 << ". " << moveStr << "  ";
 
}