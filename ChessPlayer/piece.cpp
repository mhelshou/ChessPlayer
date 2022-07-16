#include "stdafx.h"
#include <assert.h>
#include "bitboards.h"

tPieceInfo getPieceInfo(tBoard* board, int square)
{
	int i, color;
	tPieceInfo tPI;
	bool found = false;

	for(color = WHITE; (color <= BLACK) && !found; color++)
		for(i=0; i<board->last[color]; i++)
			if (board->Pieces[color][i].square == square)
			{
				tPI.color = color;
				tPI.index = i;
				tPI.square = square;
				tPI.type = board->Pieces[color][i].type;
				tPI.moved = board->Pieces[color][i].moved;
	
				found = true;
				break;
			}

	if(!found)
	{
		tPI.index = -1;
		tPI.color = -1;
		tPI.type  = -1;
	}

	return tPI;
}

tPieceInfo findOpponentKing(tBoard* board)
{
	int i, oppcolor;
	tPieceInfo tPI;
	bool found = false;

	oppcolor = OPPOSITE_COLOR(board->turn);

	for(i=0; i<board->last[oppcolor]; i++)
		if (board->Pieces[oppcolor][i].type == KING)
		{
			tPI.color = oppcolor;
			tPI.index = i;
			tPI.square = board->Pieces[oppcolor][i].square;
			tPI.type = KING;
			tPI.moved = board->Pieces[oppcolor][i].moved;
	
			found = true;
			break;
		}

	if(!found)
		assert(0);

	return tPI;
}

tPieceInfo findKing(tBoard* board, int color)
{
	int i;
	tPieceInfo tPI;
	bool found = false;

	for(i=0; i<board->last[color]; i++)
		if (board->Pieces[color][i].type == KING)
		{
			tPI.color = color;
			tPI.index = i;
			tPI.square = board->Pieces[color][i].square;
			tPI.type = KING;
			tPI.moved = board->Pieces[color][i].moved;
	
			found = true;
			break;
		}

	if(!found)
		assert(0);

	return tPI;
}