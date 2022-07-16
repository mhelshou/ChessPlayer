#ifndef ATTACK_H
#define ATTACK_H

#include "node.h"

bool isUnderAttack(tBoard* board, int square);
bool canAttackKing(tBoard*  board);
bool isInCheck(tBoard* board);
bool checkInMoveSequence(tNode* node);
bool higherPieceThreatenedInMoveSequence(tNode* node);

#endif