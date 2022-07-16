#ifndef BIT_BOARDS_H
#define BIT_BOARDS_H

// Max possible moves rounded to nearest higher power of 2 (assuming this improves memory allocation performance)
// TODO: align directive
#define MAX_POSSIBLE_PAWN_MOVES 8
#define MAX_POSSIBLE_KNIGHT_MOVES 16
#define MAX_POSSIBLE_BISHOP_MOVES_PER_DIR 8
#define MAX_POSSIBLE_ROOK_MOVES_PER_DIR 8
#define MAX_POSSIBLE_QUEEN_MOVES_PER_DIR 8
#define MAX_POSSIBLE_KING_MOVES 16

#define DIAGS	4	// 0 = bottom left, 1 = bottom right, 2 = top left, 3 = top right
#define LINES	4   // 0 = bottom, 1 = left, 2 = right, 3 = top
#define ALLDIRS 8	// 0 = bottom left, 1 = bottom, 2 = bottom right, 3 = left, 4 = right, 5 = top left, 6 = top, 7 = top right

//Bishop Diagonals
#define B_BOTTOMLEFT  0
#define B_BOTTOMRIGHT 1
#define B_TOPLEFT     2
#define B_TOPRIGHT    3
// Rook lines
#define R_BOTTOM      0
#define R_LEFT        1
#define R_RIGHT       2
#define R_TOP         3

//Queen all directions
#define Q_BOTTOMLEFT  0
#define Q_BOTTOM      1
#define Q_BOTTOMRIGHT 2
#define Q_LEFT        3
#define Q_RIGHT       4
#define Q_TOPLEFT     5
#define Q_TOP         6
#define Q_TOPRIGHT    7

#define COLORS					2
#define WHITE					0
#define BLACK					1
#define INVALID_COLOR	10		0
#define OPPOSITE_COLOR(color)	(1 - (color))
#define OPPOSITE(color)			OPPOSITE_COLOR(color)

#define PAWNW					0
#define PAWNB					1

#define PAWN					1
#define KNIGHT					2
#define BISHOP					3
#define ROOK					4
#define QUEEN					5
#define KING					6

#define NUM_PIECE_TYPES			7

#define MAXPIECES		32

#define TOSQUARE(row, col)	(8*(row)+(col))
#define GETROW(square)		((square)/8)
#define GETCOL(square)		((square)%8)
#define SQ2MASK(sq)			(1ULL<<(sq))
#define INBOARD(row, col)	(((row)>=0) && ((row)<8) && ((col)>=0) && ((col)<8))
#define ISEMPTY(board, sq)	(( (1ULL<<(sq)) & (board->Occupiedby[WHITE] | board->Occupiedby[BLACK])) == 0)

typedef struct __Piece
{
	__int8 square;
	__int8 type;
	bool moved;
} tPiece;

typedef struct __PieceInfo
{
	__int8 color;
	__int8 index;
	__int8 square;
	__int8 type;
	bool moved;
} tPieceInfo;

typedef struct __Board
{
	int		turn;
	tPiece	Pieces[COLORS][MAXPIECES];
	__int8	last[2];
	__int64 Occupiedby[COLORS];
	__int8	boardByPiece[COLORS][64]; 
	bool	isCapture;
	bool	inCheck;
	bool	higherPieceThreatened;

	bool	isCastled[COLORS];
	int		pawnJumpSq;
	bool	kingMoved[COLORS];
	int		rooksMoved[COLORS];
	int		development[COLORS];
} tBoard;

// Prototypes
void bitboards_init();
void initBoard(tBoard* board);
void bitboards_test();
void move_test();
void print_board(tBoard* board);

// Externs
extern __int8 MovesPawnW [64][MAX_POSSIBLE_PAWN_MOVES];
extern __int8 MovesPawnB [64][MAX_POSSIBLE_PAWN_MOVES];
extern __int8 MovesKnight[64][MAX_POSSIBLE_KNIGHT_MOVES];
extern __int8 MovesBishop[64][DIAGS][MAX_POSSIBLE_BISHOP_MOVES_PER_DIR];
extern __int8 MovesRook  [64][LINES][MAX_POSSIBLE_ROOK_MOVES_PER_DIR];
extern __int8 MovesQueen [64][ALLDIRS][MAX_POSSIBLE_QUEEN_MOVES_PER_DIR];
extern __int8 MovesKing  [64][MAX_POSSIBLE_KING_MOVES];

extern __int8 AllMoves	  [NUM_PIECE_TYPES][64][64];

extern __int64 BishopObstacles[64][64];
extern __int64 RookObstacles[64][64];
extern __int64 QueenObstacles[64][64];

#endif