#ifndef NODE_H
#define NODE_H

#include <Windows.h>
#include "bitboards.h"
#include "moves.h"

#define MIN_SEARCH_DEPTH		1
#define MAX_SEARCH_DEPTH		10

#define RANDOMNESS_THRESHOLD	50
#define RANDOM_FACTOR			10

#define CLOSE_SCORE(sc1, sc2)	( abs((sc1) - (sc2)) < RANDOMNESS_THRESHOLD)

#define CREATE_NODE_COUNTER		0
#define GENERATE_NODES_COUNTER  1
#define DELETE_NODES_COUNTER	2
#define EVALUATE_COUNTER		3
#define ISLEGAL_COUNTER			4
#define MAKE_MOVE_COUNTER		5
#define CREATING_CHILD_NODES	6

#define HALT_ON_MOVE(movestr)				{string move=movestr; int src = toupper(move[0])-'A' + (move[1]-'1')*8;	int dst = toupper(move[2])-'A' + (move[3]-'1')*8; \
										if(childNode->move.srcsq == src && childNode->move.dstsq == dst) DebugBreak();}

#define HALT_ON_MOVED(movedepth, movestr)	{string move=movestr; int src = toupper(move[0])-'A' + (move[1]-'1')*8;	int dst = toupper(move[2])-'A' + (move[3]-'1')*8; \
										if(childNode->move.srcsq == src && childNode->move.dstsq == dst && depth==movedepth) DebugBreak();}

typedef struct __tNode
{
	struct __tNode *parent;
	struct __tNode *children;
	struct __tNode *childrenTail;
	struct __tNode *bestChildNode;
	struct __tNode *next;
	struct __tNode *prev;
	__int8 numChildren;
	__int8 expanded;
	__int8 evalChildren;
	__int8 shortestDepth;
	tBoard* board;
	tMove move;
	int score;
} tNode;

typedef struct __tNodeList
{
	tNode *head;
	tNode *tail;
	int numNodes;
} tNodeList;

typedef struct __tResult
{
	int score;
	int depth;
} tResult;

tNode *createNode(tBoard* board);
void generateAllLegalNodes(tNode *node);
tNodeList createChildNodes(tNode *node, tBoard* board, tPiece piece);
tNode *getChildByIndex(tNode *node, int index);
tResult findBestChildScore(tNode* node, int depth, int bestSoFar);
void deleteChildren(tNode *node);
void findBestMove(tNode *node);
void print_move_trail(tNode *node);
void print_move_trail_forward(tNode *node);
void evalChildrenSorted(tNode *node);
void findBestChildScore_Improved(tNode *node, int depth, int bestSoFar);
void insertNodeIntoSortedList(int turn, tNode *node, tNode *nodeList);
void sortChildren(tNode *node);

extern int max_depth;

extern double timeSpentGeneratingNodes;
extern double timeSpentCreatingNodes;
extern double timeSpentDeletingNodes;
extern double timeSpentEvaluatingNodes;
extern double timeSpentCheckingIsLegal;
extern double timeSpentMakingMove;
extern double timeSpentCreatingChildren;

extern bool debug_print;

#endif