#include "stdafx.h"
#include "bitboards.h"
#include "moves.h"
#include "node.h"

bool isUnderAttack(tBoard* board, int square)
{
	int turn = board->turn;
	int oppcolor = OPPOSITE_COLOR(turn);
	bool underAttack = false;

	board->turn = oppcolor; // switch sides

	for(int i=0; i<board->last[oppcolor]; i++)
	{
		tPiece piece = board->Pieces[oppcolor][i];

		if( isLegalMove(board, piece.square, square, DONOT_CHECK_FOR_KING_ATTACK))
		{
			underAttack = true;
			break;
		}
	}

	board->turn = turn; // restore

	return underAttack;
}

// whoever has the board's turn can attack the opponent
bool canAttackKing(tBoard* board)
{
	int turn = board->turn;
	tPieceInfo king = findOpponentKing(board);

	for(int i=0; i<board->last[turn]; i++)
	{
		tPiece piece = board->Pieces[turn][i];

		if( isLegalMove(board, piece.square, king.square, DONOT_CHECK_FOR_KING_ATTACK))
			return true;
	}

	return false;
}

// Who ever has the board turn is under check
bool isInCheck(tBoard* board)
{
	int turn = board->turn;
	board->turn = OPPOSITE_COLOR(turn);
	bool inCheck = canAttackKing(board);
	board->turn = turn; // restore turn

	return(inCheck);
}

bool checkInMoveSequence(tNode* node)
{
	for(tNode *checkNode = node; checkNode != 0; checkNode = checkNode->parent)
	{
		if(checkNode->board->inCheck)
			return true;
	}

	return false;
}

bool higherPieceThreatenedInMoveSequence(tNode* node)
{
	for(tNode *checkNode = node; checkNode != 0; checkNode = checkNode->parent)
	{
		if(checkNode->board->higherPieceThreatened)
			return true;
	}

	return false;
}
