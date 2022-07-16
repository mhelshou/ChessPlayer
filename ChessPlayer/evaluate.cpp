#include "stdafx.h"
#include <iostream>
#include "params.h"
#include "evaluate.h"
#include "play.h"
#include "node.h"
#include "util.h"

using namespace std;

int materialValue[] = { 0, 1000, 3000, 3300, 5000, 9000, 1000000};

int evaluateMaterial(tBoard* board)
{
	int W_Material, B_Material;

	W_Material = countMaterial(board, WHITE);

	B_Material = countMaterial(board, BLACK);

	return(W_Material - B_Material);
}

int evaluateMobility(tNode *node)
{
	int score;
	//tBoard board = node->board;

	//board->turn = OPPOSITE_COLOR(board->turn);
	//tNode *node2 = createNode(board);
	//generateAllLegalNodes(node2);

	//if(node->board->turn == WHITE)
	//	score = (node->numChildren - node2->numChildren) * MOBILITY_FACTOR;
	//else
	//	score = (node2->numChildren - node->numChildren) * MOBILITY_FACTOR;

	if (node->parent)
	{
		if(node->board->turn == WHITE)
			score = (node->numChildren - node->parent->numChildren) * MOBILITY_FACTOR;
		else
			score = (node->parent->numChildren - node->numChildren) * MOBILITY_FACTOR;
	}
	else
		return 0;

	//deleteChildren(node);
	//deleteChildren(node2);
	//delete node2;

	return(score);
}

int evaluateKingSafety(tNode *node)
{
	// For now this only adds a penalty if we are losing castling rights 
	// We are losing castling rights if king has moved or if both rooks have moved.
	int score = 0;

	if(!node->board->isCastled[WHITE])
		score -= LATE_CASTLING_PENALTY;
	if(!node->board->isCastled[BLACK])
		score += LATE_CASTLING_PENALTY;
	if(node->board->kingMoved[WHITE] || (node->board->rooksMoved[WHITE]==2) )
		score -= LOSE_CASTLING_RIGHT_PENALTY;
	if(node->board->kingMoved[BLACK] || (node->board->rooksMoved[BLACK]==2) )
		score += LOSE_CASTLING_RIGHT_PENALTY;

	return score;
}

int evaluateCenterControl(tNode *node)
{
	int score = 0;

	for(tNode *childNode = node->children; childNode != 0; childNode = childNode->next)
	{
		if ( ((1ULL<<childNode->move.dstsq) & CENTER_MASK) != 0)
			score ++;
	}

	if(node->board->turn == WHITE)
		score *= CENTER_CONTROL_FACTOR;
	else
		score *= -CENTER_CONTROL_FACTOR;

	return score;
}

int evaluateDevelopment(tNode *node)
{
	return(node->board->development[WHITE] - node->board->development[BLACK]);
}

int evaluate(tNode *node)
{
	int score = 0;
	int material, mobility, king_safety, development;
	//int centerctl;

	//startCounter(GENERATE_NODES_COUNTER);
	//updateCounter(GENERATE_NODES_COUNTER, &timeSpentGeneratingNodes);
	//startCounter(EVALUATE_COUNTER);
	
	if (debug_print) logfile << " color = " << node->board->turn;
	if(node->numChildren == 0) // mate
	{
		if (debug_print) logfile << " mate or stalemate detected ";
		if(node->board->turn == WHITE)
			score = WHITE_MATED_OR_STALE;
		else
			score = BLACK_MATED_OR_STALE;
	}
	else
	{
		material = evaluateMaterial(node->board);
		score += material;
		if (debug_print) logfile << " material = " << material;

		mobility = evaluateMobility(node);
		score += mobility;
		if (debug_print) logfile << " mobility = " << mobility;

		king_safety = evaluateKingSafety(node);
		score += king_safety;
		if (debug_print) logfile << " king safety = " << king_safety;

		development = evaluateDevelopment(node);
		score += development;
		if (debug_print) logfile << " development = " << development;

		//centerctl = evaluateCenterControl(node, colorToPlay);
		//score += centerctl;
		//if (debug_print) cout << " center = " << centerctl;
	}

	node->score = score;

	//updateCounter(EVALUATE_COUNTER, &timeSpentEvaluatingNodes);

	return score;
}

int countMaterial(tBoard* board, int color)
{
	int count = 0;

	for(int i=0; i<board->last[color]; i++)
	{
		tPiece piece = board->Pieces[color][i];

		if(piece.type == KING)
			continue;

		count += materialValue[piece.type];
	}

	return count;
}