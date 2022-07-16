#ifndef MOVES_H
#define MOVES_H

//#include <iostream>
//#include <string>
#include "bitboards.h"
#include "piece.h"

#define CASTLE_RIGHT_MASK_WHITE		0x0000000000000060ULL
#define CASTLE_RIGHT_MASK_BLACK		0x6000000000000000ULL
#define CASTLE_LEFT_MASK_WHITE		0x000000000000000EULL
#define CASTLE_LEFT_MASK_BLACK		0x0E00000000000000ULL

#define DONOT_CHECK_FOR_KING_ATTACK false
#define CHECK_FOR_KING_ATTACK		true

typedef struct __tMove
{
	int srcsq;
	int dstsq;
} tMove;

bool isLegalPawn(tBoard* board, int srcsq, int dstsq);
bool isLegalKnight(tBoard* board, int srcsq, int dstsq);
bool isLegalBishop(tBoard* board, int srcsq, int dstsq);
bool isLegalRook(tBoard* board, int srcsq, int dstsq);
bool isLegalQueen(tBoard* board, int srcsq, int dstsq);
bool isLegalKing(tBoard* board, tPieceInfo king, int srcsq, int dstsq);
bool isLegalMove(tBoard* board, int srcsq, int dstsq, bool checkForKingAttack);
void makeMove(tBoard* board, tBoard* newBoard, int srcSq, int dstSq);
void doCastling(tBoard* board, tBoard* newBoard, int srcSq, int dstSq);
bool canCastle(tBoard* board, tPieceInfo king, int srcsq, int dstsq);
unsigned char* getMoveString(tMove move);

#endif