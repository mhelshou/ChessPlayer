#ifndef EVALUATE_H
#define EVALUATE_H

#include "bitboards.h"
#include "node.h"

#define MIN_NEGATIVE_SCORE	-100000
#define MAX_POSITIVE_SCORE	100000
#define WHITE_MATED_OR_STALE (MIN_NEGATIVE_SCORE+500)
#define BLACK_MATED_OR_STALE (MAX_POSITIVE_SCORE-500)

#define IS_WORSE_SCORE_THAN(turn, bestChildScore, bestSoFar) (turn==WHITE?(bestChildScore<bestSoFar):(bestChildScore>bestSoFar))
#define IS_BETTER_SCORE_THAN(turn, bestChildScore, bestSoFar) (turn==WHITE?(bestChildScore>bestSoFar):(bestChildScore<bestSoFar))
#define IS_WORSE_SCORE_THAN_OR_EQUAL(turn, bestChildScore, bestSoFar) (turn==WHITE?(bestChildScore<=bestSoFar):(bestChildScore>=bestSoFar))
#define IS_BETTER_SCORE_THAN_OR_EQUAL(turn, bestChildScore, bestSoFar) (turn==WHITE?(bestChildScore>=bestSoFar):(bestChildScore<=bestSoFar))
					
#define CENTER_MASK			0x0000001818000000ULL

// Mobility
#define MOBILITY_FACTOR			10

// Center
#define CENTER_CONTROL_FACTOR	50

// Castling
#define CASTLING_MOVENUM_THRESHOLD	12
#define LATE_CASTLING_PENALTY		200
#define LOSE_CASTLING_RIGHT_PENALTY	200

// Development
#define	PAWN_DEVELOPMENT_FACTOR		50
#define KNIGHT_DEVELOPMENT_FACTOR	300
#define BISHOP_DEVELOPMENT_FACTOR	300
#define ROOK_DEVELOPMENT_FACTOR		100
#define QUEEN_DEVELOPMENT_FACTOR	-100
#define KING_DEVELOPMENT_FACTOR		-300

#define ROOK_DEVELOPMENT_FACTOR_CASTLING	300
#define KING_DEVELOPMENT_FACTOR_CASTLING	300

#define HIGHER_PIECE_THREATENED_THRESHOLD	2000

int evaluate(tNode *node);
int countMaterial(tBoard* board, int color);

extern int materialValue[];

#endif