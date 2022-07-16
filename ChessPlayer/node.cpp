#include "stdafx.h"
#include <assert.h>
#include <iostream>

#include "params.h"
#include "node.h"
#include "moves.h"
#include "evaluate.h"
#include "attack.h"
#include "util.h"
#include "uci.h"
#include "play.h"

using namespace std;

int max_depth = MIN_SEARCH_DEPTH;

extern __int64 total_positions;

double timeSpentGeneratingNodes=0.0;
double timeSpentCreatingNodes=0.0;
double timeSpentDeletingNodes=0.0;
double timeSpentEvaluating=0.0;
double timeSpentCheckingIsLegal=0.0;
double timeSpentMakingMove=0.0;
double timeSpentCreatingChildren=0.0;
double timeSpentEvaluatingNodes=0.0;

bool debug_print = DEBUG_PRINT;

tNode *createNode(tBoard* board)
{
	tNode *node = new tNode;

	//startCounter(CREATE_NODE_COUNTER);

	if(node!=0)
	{
		node->board = board;
		node->parent = node->next = node->prev = node->children = node->childrenTail = node->bestChildNode = 0;
		node->numChildren=0;
		node->expanded=0;
		node->shortestDepth = 0;
		node->evalChildren = 0;
	}
	else
		cout << "Can not allocate memory for tNode\n";

	//updateCounter(CREATE_NODE_COUNTER, &timeSpentCreatingNodes);

	return node;
}

void generateAllLegalNodes(tNode *node)
{
	tBoard* board = node->board;
	int turn = board->turn;
	tNodeList childrenList;

	//startCounter(GENERATE_NODES_COUNTER);

	// TODO: All the direct manipulation of pieces may not belong here
	for (int i=0; i<board->last[turn]; i++)
	{
		tPiece piece = board->Pieces[turn][i];

		//startCounter(CREATING_CHILD_NODES);
		childrenList = createChildNodes(node, board, piece);
		//updateCounter(CREATING_CHILD_NODES, &timeSpentCreatingChildren);

		if(childrenList.numNodes != 0)
		{
			if(node->children==0)
			{
				node->children = childrenList.head;
				node->childrenTail = childrenList.tail;
				node->numChildren = childrenList.numNodes;
			}
			else
			{
				node->childrenTail->next = childrenList.head;
				childrenList.head->prev = node->childrenTail;
				node->childrenTail = childrenList.tail;
				node->numChildren += childrenList.numNodes;
			}
		}
	}

	total_positions+=node->numChildren;
	node->expanded = 1;

	//updateCounter(GENERATE_NODES_COUNTER, &timeSpentGeneratingNodes);
}

tNodeList createChildNodes(tNode *node, tBoard* board, tPiece piece)
{
	int srcsq, dstsq;
	tNode *head = 0, *tail=0;
	int movetype;
	int turn = board->turn;
	int opp = OPPOSITE_COLOR(turn);
	tNodeList list;
	int numChildren = 0;

	if(piece.type == PAWN) // Pawn moves depends on color so we create two move types (black and white) for the PAWN type
		movetype = turn;
	else
		movetype = piece.type;
	 
	srcsq = piece.square;

	for(int moveindex=0; ; moveindex++)
	{
		dstsq = AllMoves[movetype][srcsq][moveindex];

		if (dstsq == -1)
			break;

		//startCounter(ISLEGAL_COUNTER);
		if(!isLegalMove(board, srcsq, dstsq, CHECK_FOR_KING_ATTACK))
		{
			//updateCounter(ISLEGAL_COUNTER, &timeSpentCheckingIsLegal);
			continue;
		}
		//else
		//	updateCounter(ISLEGAL_COUNTER, &timeSpentCheckingIsLegal);

		tBoard *newBoard = new(tBoard);
		if(newBoard == 0)
			cout << "Can not allocate memory for tBoard\n";

		// Check that a higher piece type is threatened. Sorta works like a null move
		if( ( materialValue[board->boardByPiece[opp][dstsq]] - materialValue[board->boardByPiece[turn][srcsq]]) >= HIGHER_PIECE_THREATENED_THRESHOLD )
			board->higherPieceThreatened = true;

		//startCounter(MAKE_MOVE_COUNTER);
		makeMove(board, newBoard, srcsq, dstsq);
		//updateCounter(ISLEGAL_COUNTER, &timeSpentCheckingIsLegal);
		//updateCounter(MAKE_MOVE_COUNTER, &timeSpentMakingMove);

		//startCounter(CREATE_NODE_COUNTER);
		tNode *pnewNode = createNode(newBoard);
		//updateCounter(ISLEGAL_COUNTER, &timeSpentCheckingIsLegal);
		//updateCounter(CREATE_NODE_COUNTER, &timeSpentCreatingNodes);
		pnewNode->parent = node;
		pnewNode->move.srcsq = srcsq;
		pnewNode->move.dstsq = dstsq;

		if(head == 0)
			head = pnewNode;
		else
			tail->next = pnewNode;

		pnewNode->prev = tail;
		tail = pnewNode;
		numChildren++;
	}

	list.head = head;
	list.tail = tail;
	list.numNodes = numChildren;

	return list;
}

tNode *getChildByIndex(tNode *node, int index)
{
	tNode *np = node->children;

	if(index >= node->numChildren)
		return 0;

	for(int i=0; i<index; i++)
		np = np->next;

	return np;
}
int maxxdepth = 0;
tResult findBestChildScore(tNode *node, int depth, int bestSoFar)
{
	tNode *bestChildNode=0;
	int bestChildScore, shortestDepth = 1000;
	tResult res;

	if(node->board->turn == WHITE)
		bestChildScore = MIN_NEGATIVE_SCORE;
	else
		bestChildScore = MAX_POSITIVE_SCORE;

	assert(node);
	assert(depth <= MAX_SEARCH_DEPTH);

	if (depth > max_depth)
		max_depth = depth;

	// Get all legal moves.
	if(node->expanded == 0)
		generateAllLegalNodes(node);

	if ( (depth >= MAX_SEARCH_DEPTH) || ((depth >= MIN_SEARCH_DEPTH) && !node->board->isCapture && !node->board->inCheck /*!checkInMoveSequence(node) /*&& !higherPieceThreatenedInMoveSequence(node)*/ ) ) // if there is a capture, check, or higher piece threatened, continue until there is no more 
	{
		if (debug_print) logfile << "{ ";
		if (debug_print) print_move_trail(node);

		bestChildScore = evaluate(node);
		deleteChildren(node);

		shortestDepth = depth;

		if (debug_print) logfile << " total score = " << bestChildScore << " }\n";

		if (uci_go)
		{
			cout << "info multipv 1 depth " << depth << " score cp " << bestChildScore/10 << " pv ";
			print_move_trail(node);
			cout << "\n";
		}

		//if(depth > maxxdepth)
		//{
		//	maxxdepth = depth;
		//	cout << "max depth = " << depth << "total positions = " << total_positions << " \n";
		//	print_move_trail(node);
		//	cout << "\n";
		//}
	}
	else
	{
		if(node->numChildren == 0)
		{
			shortestDepth = depth;

			if (debug_print) logfile << "\n{ ";
			if (debug_print) print_move_trail(node);
			if(isInCheck(node->board))
			{
				if (debug_print) logfile << " mate detected color = " << node->board->turn << " }\n";
				if(node->board->turn == WHITE)
					bestChildScore = WHITE_MATED_OR_STALE; // our turn but we have no moves
				else
					bestChildScore = BLACK_MATED_OR_STALE; // not our turn but opponent has no moves: good for us
			}
			else
				bestChildScore = 0;
		}
		else
		{
			depth ++;

			//evalChildrenSorted(node);
			int cn = 0;

			tNode *childNode;

			//for(childNode = node->children; (childNode != 0) && (cn<5); childNode = childNode->next, cn++)
			for(tNode *childNode = node->children; childNode != 0; childNode = childNode->next)
			{
				res = findBestChildScore(childNode, depth, bestChildScore);
		
				// add some randomization if score is equal
				//if(abs(res.score - bestChildScore) < RANDOMNESS_THRESHOLD)  // randomly add or subtract a 1
				//	res.score += (2*(rand()%2)-1)*RANDOM_FACTOR;

				if(node->board->turn == WHITE)
				{
					// if they are equal with one has a better depth -> choose the shorter depth
					// if they are not close or not equal depth only choose the move that is clearly higher
					// what if they are equal depth and equal score? to get around this, always make the min and max possible lower or higher than the value of mate
					// Update: Removed CLOSE_SCORE because the alpha beta pruning check strictly is < and < not CLOSE_SCORE. This means that if one node is 100 and another is 120
					// the alpha beta check can bail out because 120>100 but higher in the tree, the algorithm can choose the 120 move because it's CLOSE enough, however since we bailed
					// out, there could have been a 9000 score in the 120 node and we shouldn't have chosen it. So either remove the CLOSE_SCORE from the checks below or add CLOSE_SCORE
					// to the alpha beta bail out condition. I will start by removing it from the checks below with the danger that it may be easier to get repeated moves
					//if( (CLOSE_SCORE(res.score, bestChildScore) && (res.depth < shortestDepth)) || (res.score > bestChildScore))
					if( ((res.score == bestChildScore) && (res.depth < shortestDepth)) || (res.score > bestChildScore))
					{
						bestChildScore = res.score;
						shortestDepth = res.depth;
					}
				}
				else
				{
					//if( (CLOSE_SCORE(res.score, bestChildScore) && (res.depth < shortestDepth)) || (res.score < bestChildScore))
					if( ( (res.score == bestChildScore) && (res.depth < shortestDepth)) || (res.score < bestChildScore))
					{
						bestChildScore = res.score;
						shortestDepth = res.depth;
					}
				}

				// Now you find who is the best one, free the memory
				deleteChildren(childNode);

				if(node->board->turn == WHITE)
				{
					if(bestChildScore > bestSoFar)
					{
						// We're bailing out early but to sort and eval the nodes we created all children ahead of time so now we have to delete them all
						for(tNode *deleteNode = childNode->next; deleteNode != 0; deleteNode = deleteNode->next)
							deleteChildren(deleteNode);
						break;
					}
				}
				else
				{
					if(bestChildScore < bestSoFar)
					{
						// We're bailing out early but to sort and eval the nodes we created all children ahead of time so now we have to delete them all
						for(tNode *deleteNode = childNode->next; deleteNode != 0; deleteNode = deleteNode->next)
							deleteChildren(deleteNode);
						break;
					}
				}
			}

			//for(tNode *deleteNode = childNode; deleteNode != 0; deleteNode = deleteNode->next)
			//	deleteChildren(deleteNode);

		}
	}

	res.score = bestChildScore;
	res.depth = shortestDepth;

	return res;
}

void findBestMove(tNode *node)
{
	int depth = 0;
	tNode *tied_best[64];
	int tie_index = -1;

	tNode *bestChildNode=0;
	int bestChildScore, shortestDepth = 1000;
	tResult res;

	if(node->board->turn == WHITE)
		bestChildScore = MIN_NEGATIVE_SCORE;
	else
		bestChildScore = MAX_POSITIVE_SCORE;

	assert(node);

	generateAllLegalNodes(node);

	if(node->numChildren == 0)
	{
		if(isInCheck(node->board))
			bestChildScore = (node->board->turn == WHITE)?BLACK_MATED_OR_STALE:WHITE_MATED_OR_STALE;
		else
			bestChildScore = 0;
	}
	else
	{
		//evalChildrenSorted(node);

		int cn=0;

		tNode *childNode;
		for(childNode = node->children; childNode != 0; childNode = childNode->next)
		//for(childNode = node->children; (childNode != 0) && (cn<5); childNode = childNode->next, cn++)
		{
			//HALT_ON_MOVE("c6e5");
			res = findBestChildScore(childNode, depth, bestChildScore);

			// add some randomization if score is equal
			//if(abs(score - bestChildScore) < RANDOMNESS_THRESHOLD) // randomly add or subtract a 1
			//	score += (2*(rand()%2)-1)*RANDOM_FACTOR;

			if( (res.depth==shortestDepth) && (res.score == bestChildScore) )
			{
				tie_index++;
				tied_best[tie_index] = childNode;
			}
			if(node->board->turn == WHITE)
			{
				if( ((res.score == bestChildScore) && (res.depth < shortestDepth)) || (res.score > bestChildScore))
				{
					bestChildScore = res.score;
					shortestDepth = res.depth;
					bestChildNode = childNode;
					tie_index=0;
					tied_best[0] = childNode;
				}
			}
			else
			{
				if( ((res.score == bestChildScore) && (res.depth < shortestDepth)) || (res.score < bestChildScore))
				{
					bestChildScore = res.score;
					shortestDepth = res.depth;
					bestChildNode = childNode;
					tie_index=0;
					tied_best[0] = childNode;
				}
			}

			// Now you find who is the best one, free the memory
			deleteChildren(childNode);
		}
		
		//for(tNode *deleteNode = childNode; deleteNode != 0; deleteNode = deleteNode->next)
			//deleteChildren(deleteNode);
	}

	node->score = bestChildScore;
	if(bestChildNode)
	{
		if(tie_index > 0)
		{
			int random_index = rand()%(tie_index+1);
			bestChildNode = tied_best[random_index];
		}

		node->move = bestChildNode->move;
		node->board = bestChildNode->board;
	}
	else
		node->move.dstsq = node->move.srcsq = -1;
}

void findBestChildScore_Improved(tNode *node, int depth, int bestSoFar)
{
	tNode *bestChildNode=0, *childNode;
	int bestChildScore, bestChildScoreDescending, shortestDepth = 1000;
	int turn = node->board->turn;

	if(node->board->turn == WHITE)
		bestChildScore = bestChildScoreDescending = MIN_NEGATIVE_SCORE;
	else
		bestChildScore = bestChildScoreDescending = MAX_POSITIVE_SCORE;

	assert(node);
	//assert(depth <= MAX_SEARCH_DEPTH);

	//if (depth > max_depth)
	//{
	//	cout << "{ total_positions = " << total_positions << " max_depth = " << max_depth << "}\n";
	//	max_depth = depth;
	//}

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	if(node->expanded == 0)
	{
		generateAllLegalNodes(node);
		evalChildrenSorted(node);
	}

	if ( /*(depth >= max_depth) || */(depth >= MAX_SEARCH_DEPTH) )
	{
		bestChildNode = 0;
		bestChildScore = node->score;
		shortestDepth = depth;
	}
	else
	{
		if(node->evalChildren == 0)
		{
			evalChildrenSorted(node);
		}

		if(node->numChildren == 0)
		{
			shortestDepth = depth;

			if (debug_print) logfile << "\n{ ";
			if (debug_print) print_move_trail(node);
			if(isInCheck(node->board))
			{
				if (debug_print) logfile << " mate detected color = " << node->board->turn << " }\n";
				if(node->board->turn == WHITE)
					bestChildScore = WHITE_MATED_OR_STALE; // our turn but we have no moves
				else
					bestChildScore = BLACK_MATED_OR_STALE; // not our turn but opponent has no moves: good for us
			}
			else
				bestChildScore = 0;
		}
		else
		{
			childNode = node->children;
			bestChildScore = childNode->score;
			bestChildNode = childNode;

			while( childNode && IS_WORSE_SCORE_THAN_OR_EQUAL(turn, bestChildScore, bestSoFar) )
			{
				/*if(bestChildScore == childNode->score)
					break;*/

				findBestChildScore_Improved(childNode, depth+1, bestChildScore);

				if(depth == 0)
				{
					max_depth++;
					cout << "{ total_positions = " << total_positions << " max_depth = " << max_depth << "}\n";
					print_move_trail_forward(childNode);
				}

				if( ((childNode->score == bestChildScore) && (childNode->shortestDepth < shortestDepth)) || IS_BETTER_SCORE_THAN(turn, childNode->score, bestChildScore))
				{
					if(depth == 0)
					{
						max_depth++;
						cout << "{ total_positions = " << total_positions << " max_depth = " << max_depth << "}\n";
						print_move_trail_forward(childNode);
					}
					bestChildNode = childNode;
					bestChildScore = childNode->score;
					childNode = childNode->next;
					//if(depth != 0) // this seems like a klugde. Keep it until we have a general model of operation
					//	break;
				}
				//else if(childNode->score == bestChildScore)
				//{
				//	if(depth == 0)
				//		max_depth++;
				//}
				else
				{
					insertNodeIntoSortedList(turn, childNode, node->children);
					bestChildNode = node->children; // pick first item
					bestChildScore = bestChildNode->score;
					childNode = node->children;
				}

				// 
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	//	if (debug_print) logfile << " total score = " << bestChildScore << " }\n";

	//	if (uci_go)
	//	{
	//		cout << "info multipv 1 depth " << depth << " score cp " << bestChildScore/10 << " pv ";
	//		print_move_trail(node);
	//		cout << "\n";
	//	}

	//	//cout << "total positions = " << total_positions << " \n";
	//}

	node->score = bestChildScore;
	if(bestChildNode)
	{
		node->bestChildNode = bestChildNode;
		node->shortestDepth = bestChildNode->shortestDepth;
	}
	else
	{
		node->bestChildNode = 0;
		node->shortestDepth = depth;
	}
}

void deleteChildren(tNode *node)
{
	tNode *nextChild;

	//startCounter(DELETE_NODES_COUNTER);

	if(node->numChildren != 0)
	{
		for(tNode *childNode = node->children;;)
		{
			nextChild = childNode->next;
			//deleteChildren(childNode);
			delete(childNode->board);
			delete(childNode);

			if(!nextChild)
				break;
			else
				childNode = nextChild;
		}
	}

	node->numChildren = 0;
	node->children = node->childrenTail = 0;

	//updateCounter(DELETE_NODES_COUNTER, &timeSpentDeletingNodes);
}

void print_move_trail(tNode *node)
{
	tMove moves[100];
	unsigned char *moveStr;

	int index = 0;

	while(node->parent != 0)
	{
		moves[index++] = node->move;
		node = node->parent;
	}

	for (int i=(index - 1); i>=0; i--)
	{
		moveStr = getMoveString(moves[i]);
		//logfile << " " << moveStr << " ";
		cout << " " << moveStr << " ";
	}
}

void print_move_trail_forward(tNode *node)
{
	unsigned char *moveStr;

	int index = 0;

	for(tNode *pn = node; pn != 0; pn = pn->bestChildNode)
	{
		moveStr = getMoveString(pn->move);
		cout << " " << moveStr << " ";
		//logfile << " " << moveStr << " ";
	}
	cout << "\n";
}

void evalChildrenSorted(tNode *node)
{
	int score;

	// First collect the scores
	for(tNode *childNode = node->children; childNode != 0; childNode = childNode->next)
	{
		if(childNode->expanded == 0)
			generateAllLegalNodes(childNode); // generate legal moves only to use in certain assesments (mobility, etc)

		score = evaluate(childNode);
	}

	sortChildren(node);
}

void sortChildren(tNode *node)
{
	tNode *head, *newList=0, *newListTail, *bestNode;
	int bestScore;

	if(node->board->turn == WHITE)
		bestScore = MIN_NEGATIVE_SCORE;
	else
		bestScore = MAX_POSITIVE_SCORE;

	head = node->children;

	while(head!=0)
	{
		bestScore = head->score;

		// Now sort from most favorable to least favorable
		for(tNode *childNode = head; childNode != 0; childNode = childNode->next)
		{
			if(node->board->turn == WHITE)
			{
				if(childNode->score >= bestScore)
				{
					bestNode = childNode;
					bestScore = bestNode->score;
				}
			}
			else
			{
				if(childNode->score <= bestScore)
				{
					bestNode = childNode;
					bestScore = bestNode->score;
				}
			}
		}

		if(bestNode == head)
			head = head->next;

		// First remove it from the original list
		if(bestNode->prev)
			bestNode->prev->next = bestNode->next;
		if(bestNode->next)
			bestNode->next->prev = bestNode->prev;

		// Then add it to the new list
		if(!newList)
		{
			newList = bestNode;
		}
		else
		{
			newListTail->next = bestNode;
			bestNode->prev = newListTail; 
		}

		newListTail = bestNode;
	}

	node->children = newList;
	node->childrenTail = newListTail;
	newListTail->next = 0;
	newList->prev = 0;

	node->evalChildren = 1;
}

void insertNodeIntoSortedList(int turn, tNode *node, tNode *nodeList)
{
	int nodeScore = node->score;
	tNode *parent = nodeList->parent;
	tNode *childNode;

	assert(nodeList != 0);
	assert(nodeList == node); // the node we're removing should always be the first node in the sorted list

	// Make the next item head of the list
	nodeList = nodeList->next;

	if(!nodeList)
		return;

	nodeList->prev = 0;

	// Now sort from most favorable to least favorable
	for(childNode = nodeList; childNode != 0; childNode = childNode->next)
	{
		if(IS_BETTER_SCORE_THAN_OR_EQUAL(turn, nodeScore, childNode->score))	// insert ahead of the best node
		{
			node->next = childNode;
			node->prev = childNode->prev;
			if(childNode->prev)
				childNode->prev->next = node;
			childNode->prev = node;

			break;
		}
	}

	if(childNode == 0)	// empty list or we are the worst
	{
		parent->childrenTail->next = node;
		node->next = 0;
		node->prev = parent->childrenTail;
		parent->childrenTail = node;
	}

	// Since the item we're readding to the sorted list is always the first item, clip it off the beginning

	if(!node->prev)
		parent->children = node; // adjust the children list
	else
		parent->children = nodeList;
}