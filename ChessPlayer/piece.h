#ifndef PIECE_H
#define PIECE_H

#include "bitboards.h"

tPieceInfo getPieceInfo(tBoard* board, int square);
tPieceInfo findOpponentKing(tBoard* board);

#endif