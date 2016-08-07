// Hexorcist -- Hex/Polygon playing engine
// Copyright Jeff Mallett 1997
#include "stdafx.h"

/*
            0 1 2 3 4 5 6 7 8
          0  . . . . . . . . .
           1  . . . . . . . V .
            2  . . V . . . . V .
             3  . . . V . . . . .
              4  . . . . . . V . .
               5  . . V H H H V . .
                6  . . . . . . H . .
                 7  . H . V . H . . .
                  8  . H . . . . . . .
*/

// WIN #include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef short GROUP;
typedef struct SConnections {
	GROUP Strong[3];
	GROUP Weak[6];
} SConnections;


// *************************************************
// **** function prototypes
// *************************************************
void PlaceCounter(Boolean isVertical, long row, long col);
Boolean Pour(long waterLevel, Boolean isVertical);
long GetNextWaterLevel(long currentLevel);
void Flood(Boolean isVertical);
void InitializeEmptyGraph();
long GetSaddlePoints();
unsigned long *GetBestSaddle(Boolean isVertical);
unsigned long *PlayAnyMove();
void SpiralInfluence(long *hex);
void InfluenceGraph(unsigned long **p, long *graph);
void InfluenceGraphs();
long IsSaddlePoint(long dir, long k, long *wasUp, long *pG);

void ClearBonuses();
void NearPlayBonus(Boolean isVertical, long row, long col);
unsigned long *AdjustMove(unsigned long *move);
void NoConnections(SConnections *connects);
void AddWeakConnection(SConnections *connects, GROUP group);
void AddStrongConnection(SConnections *connects, GROUP group);
long CountStrongConnections(SConnections *connects);
long CountWeakConnections(SConnections *connects);
void RemoveStrongConnection(SConnections *connects, GROUP group);
Boolean HasStrongConnection(SConnections *connects, GROUP group);
Boolean HasWeakConnection(SConnections *connects, GROUP group);
Boolean ExistsOnStrongList(GROUP a, GROUP b);
void AddToStrongList(GROUP a, GROUP b);
void Merge(GROUP a, GROUP b);
unsigned long *ConnectionTactics(Boolean isVertical, Boolean friendly);
Boolean /*legalMove*/ Hex (
  long boardSize,     /* number of rows/columns in the game board */
  long oppRow,        /* row where opponent last moved, 0 .. boardSize-1 */
  long oppCol,        /* column where opponent last moved, 0 .. boardSize-1 */
  long *moveRow,      /* return your move - row, 0 .. boardSize-1 */
  long *moveCol,      /* return your move - column, 0 .. boardSize-1 */
  void *privStorage,  /* preallocated storage for your use */
  Boolean newGame,    /* TRUE if this is your first move in a new game */
  Boolean playFirst   /* TRUE if you play first (vertically) */
);


// *************************************************
// **** macros/constants
// *************************************************
#define INFINITY			999999999L

#define H							0x00010000 // Is V man here?
#define V							0x00020000 // Is H man here?
#define BORDER_BIT		0x00040000 // Is this a border square?
#define MAXIMUM				0x10000000

#define NULL_GROUP				9999

#define BONUS_NEAR					1
#define BONUS_WEAK_THEM			1
#define BONUS_WEAK_US				2
#define BONUS_LINKING				7
#define BONUS_EDGE_LINKING	4

#define NW						-gRealBoardSize
#define SE						gRealBoardSize
#define W							-1
#define E							1
#define NE						(1 - gRealBoardSize)
#define SW            (gRealBoardSize - 1)

#define POTENTIAL_VALUE(x)	gDividedBy[x]

#define SCALE								1000L
#define BIG_RAISE						SCALE
#define RAISE								(SCALE / 2)

#define INSIDE_EMPTY_BITS		(BORDER_BIT | V | H)
#define NOT_IS_EMPTY(q)			(*(q) & INSIDE_EMPTY_BITS)
#define IS_EMPTY(q)					!NOT_IS_EMPTY(q)

#define ON_TOP_EDGE(q)			(*((q) + NW) & BORDER_BIT)
#define ON_LEFT_EDGE(q)			(*((q) +  W) & BORDER_BIT)
#define ON_BOTTOM_EDGE(q)		(*((q) + SE) & BORDER_BIT)
#define ON_RIGHT_EDGE(q)		(*((q) +  E) & BORDER_BIT)

// *************************************************
// **** static variables
// *************************************************
static long gDirs[7];
static long gBoardSize;
static long gRealBoardSize; // Length of a side (includes borders)
static long gStrongListSize;
static long gHexes, gOnHexes;

static unsigned long *gBoard, *gBoardStart; // Board
static unsigned long *gRaised, *gRaisedStart;

static unsigned long **gVPieces, **gHPieces;
static unsigned long **gSaddles;
static unsigned long **gTemp;

static long *gVGraph, *gVGraphStart;
static long *gHGraph, *gHGraphStart;
static long *gGraph, *gGraphStart;
static long *gEmptyGraph, *gEmptyGraphStart;
static long *gTempGraph, *gTempGraphStart;
static long *gBonus, *gBonusStart;
static long *gStrongList;
static long *gDividedBy;

static GROUP *gGroups, *gGroupsStart;

static Boolean *gSubmerged, *gSubmergedStart;
static Boolean *gToDo, *gToDoStart;

static SConnections *gConnects;

// *************************************************
// **** Hex
// *************************************************
Boolean /*legalMove*/ Hex (
  long boardSize,     /* number of rows/columns in the game board */
  long oppRow,        /* row where opponent last moved, 0 .. boardSize-1 */
  long oppCol,        /* column where opponent last moved, 0 .. boardSize-1 */
  long *moveRow,      /* return your move - row, 0 .. boardSize-1 */
  long *moveCol,      /* return your move - column, 0 .. boardSize-1 */
  void *privStorage,  /* preallocated storage for your use */
  Boolean newGame,    /* TRUE if this is your first move in a new game */
  Boolean playFirst   /* TRUE if you play first (vertically) */
)
{
	unsigned long *move;

	if (newGame) {
		long i, j, k;
		unsigned long *b;
		
		gBoardSize = boardSize;
		gRealBoardSize = 2 * boardSize;
		gOnHexes = gBoardSize * gBoardSize;
		gHexes = gRealBoardSize * gRealBoardSize;
		
		gBoard = (unsigned long *)privStorage;
		gRaised = gBoard + gHexes;
		gVGraph = (long *)(gRaised + gHexes);
		gHGraph = gVGraph + gHexes;
		gGraph = gHGraph + gHexes;
		gEmptyGraph = gGraph + gHexes;
		gTempGraph = gEmptyGraph + gHexes;
		gVPieces = (unsigned long **)(gTempGraph + gHexes);
		gHPieces = (gVPieces + gOnHexes);
		gSaddles = (gHPieces + gOnHexes);	
		gGroups = (GROUP *)(gSaddles + gOnHexes);
		gStrongList = (long *)(gGroups + gHexes);
		gBonus = gStrongList + gHexes;
		gToDo = (Boolean *)(gBonus + gHexes);
		gConnects = (SConnections *)(gToDo + gHexes);
		gDividedBy = (long *)(gConnects + gHexes);

		gTemp = (unsigned long **)gTempGraph;
		gSubmerged = gToDo;

		k = (gRealBoardSize + 1) * gBoardSize/2;
		gBoardStart = gBoard + k;
		gRaisedStart = gRaised + k;
		gVGraphStart = gVGraph + k;
		gHGraphStart = gHGraph + k;
		gGraphStart = gGraph + k;
		gEmptyGraphStart = gEmptyGraph + k;
		gTempGraphStart = gTempGraph + k;
		gSubmergedStart = gSubmerged + k;
		gGroupsStart = gGroups + k;
		gBonusStart = gBonus + k;
		gToDoStart = gToDo + k;
	
		gDirs[0] = E;
		gDirs[1] = SE;
		gDirs[2] = SW;
		gDirs[3] = W;
		gDirs[4] = NW;
		gDirs[5] = NE;
		gDirs[6] = 0;
	
		gDividedBy[0] = 9999;
		for (i=1; i<gBoardSize; ++i)
			gDividedBy[i] = SCALE / i;
			
		b = gBoard;
		for (i=gRealBoardSize * boardSize/2; i; --i)
			*(b++) = BORDER_BIT;
		for (i=boardSize; i; --i) {
			for (j=boardSize/2; j; --j)
				*(b++) = BORDER_BIT;
			for (j=boardSize; j; --j)
				*(b++) = 0;
			for (j=boardSize/2; j; --j)
				*(b++) = BORDER_BIT;
		}			
		for (i=gRealBoardSize * boardSize/2; i; --i)
			*(b++) = BORDER_BIT;
		
		*gVPieces = *gHPieces = NULL;
		
		InitializeEmptyGraph();
		
		if (oppRow == -1) {
			// First move of game: Move in center
			*moveRow = *moveCol = (boardSize-1)/2;
			PlaceCounter(playFirst, *moveRow, *moveCol);
			return true;
		}
	}
	
	ClearBonuses();

	if (oppRow != -1) {
		PlaceCounter(!playFirst, oppRow, oppCol);
		NearPlayBonus(!playFirst, oppRow, oppCol);
	}
	
	move = ConnectionTactics(playFirst, true);
	
	if (!move)
		move = ConnectionTactics(!playFirst, false);
	
	if (!move) {
		InfluenceGraphs();
		
		if (GetSaddlePoints()) {
			move = AdjustMove(GetBestSaddle(playFirst));
		} else {
			move = PlayAnyMove();
		}
	}
	
	{
		long k = move - gBoardStart;
		*moveRow = k / gRealBoardSize;
		*moveCol = k - *moveRow * gRealBoardSize;
	}

	PlaceCounter(playFirst, *moveRow, *moveCol);
	return true;
}

// *************************************************
// **** PlaceCounter
// *************************************************
void PlaceCounter(Boolean isVertical, long row, long col)
{
		unsigned long **p, *where;
		
		where = gBoardStart + (row * gRealBoardSize + col);
		*where = isVertical ? V : H;
		
		for (p = isVertical ? gVPieces : gHPieces; *p; ++p)
			;
		*p = where;
		*(++p) = NULL;
}

// *************************************************
// **** GetSaddlePoints
// *************************************************
// Finds saddle points and puts them in gSaddles
// Returns number found
long GetSaddlePoints()
{
	register long j, i;
	register unsigned long *p;
	unsigned long **s;
	long count;
	
	count = 0;	
	s = gSaddles;
	p = gBoardStart;
	for (i=0; i<gBoardSize; ++i) {
		for (j=0; j<gBoardSize; ++j) {
			if (IS_EMPTY(p) && (*p & MAXIMUM)) {
				++count;
				*(s++) = p;
			}
			p += E;
		}
		p += gBoardSize;
	}
	*s = 0;
	
	return count;
}

// *************************************************
// **** GetBestSaddle
// *************************************************
unsigned long *GetBestSaddle(Boolean isVertical)
{
	register unsigned long **s;
	register long score;
	long offset, d, bestScore;
	unsigned long *best;
	
	if (*gSaddles && !*(gSaddles + 1))
		return *gSaddles; // only one saddle
		
	// #1 Flood method: Use highest flooded saddle
	Flood(isVertical);
	bestScore = -INFINITY;
	for (s=gSaddles; *s; ++s) {
		offset = *s - gBoard;
		if (gSubmerged[offset]) {
			score = gGraph[offset] + gBonus[offset];
			if (score > bestScore) {
				bestScore = score;
				best = *s;
			}
		}
	}
	
	// #2: Use saddle closest to zero
	if (bestScore == -INFINITY) {
		for (s=gSaddles; *s; ++s) {
			d = *s - gBoardStart;
			score = *(gVGraphStart + d) - *(gHGraphStart + d);
			if (score < 0)
				score = -score;
			if (score < bestScore) {
				bestScore = score;
				best = *s;
			}
		}
	}
	
	return best; //### best may not be defined here
}

// *************************************************
// **** PlayAnyMove
// *************************************************
// I hope it never gets here!
// Play empty closest to zero (evenly contested)
unsigned long *PlayAnyMove()
{
	register unsigned long *p;
	register long *g;
	long x, y, bestHeight, m;
	unsigned long *best;
	
	bestHeight = 9999999L;
	p = gBoardStart;
	g = gGraphStart;
	y = gBoardSize;
	do {
		x = gBoardSize;
		do {
			if (IS_EMPTY(p)) {
				m = *g;
				if (m < 0)
					m = -m;
				if (m < bestHeight) {
					best = p;
					bestHeight = m;
				}
			}
			p += E;
			g += E;
		} while (--x);
		p += gBoardSize;
		g += gBoardSize;
	} while (--y);		
	
	return best;
}

// *************************************************
// **** SpiralInfluence
// *************************************************
#define SETVAL 		*hex += potential
void SpiralInfluence(long *hex)
{
	register long i;
	long *d, distance, stop;
	long potential;
	
	*hex = 9999999L;
	stop = gBoardSize / 2;
	distance = 1;
	do {
		potential = POTENTIAL_VALUE(distance);
		hex += NW;
		for (d = gDirs; *d; ++d) {
			i = 0;
			do {
				hex += *d;
				SETVAL;
			} while (++i < distance);
		}
	} while (++distance < stop);
}


// *************************************************
// **** Pour
// *************************************************
// Returns true if floods from one end to the other
Boolean Pour(long waterLevel, Boolean isVertical)
{
	long *g;
	long x, y, oldLeaks, delta;
	Boolean *s;
	long leaks = 0;
	Boolean flood = FALSE;
	
	// For each hex along low edge, make underwater if under <= level
	delta = isVertical ? E : SE;
	g = gGraphStart;
	s = gSubmergedStart;
	for (x=gBoardSize; x; --x) {
		if (!*s && *g <= waterLevel) {
				++leaks;
				*s = TRUE;
		}
		g += delta;
		s += delta;
	}
	
	// While seepage, check each hex for leaks
	do {
		oldLeaks = leaks;
		g = gGraphStart;
		s = gSubmergedStart;
		y = gBoardSize;
		do {
			x = gBoardSize;
			do {
				if (!*s && *g <= waterLevel) {
					if (*(s+NW) || *(s+SE) || *(s+W) || *(s+E) || *(s+NE) || *(s+SW)) {
						++leaks;
						*s = TRUE;
						if (isVertical) {
							if (y == 1)
								flood = TRUE;
						} else {
							if (x == 1)
								flood = TRUE;
						}
					}
				}
				g += E;
				s += E;
			} while (--x);
			g += gBoardSize;
			s += gBoardSize;
		} while (--y);		
	} while (leaks != oldLeaks);
		   
	return flood;
}

// *************************************************
// **** GetNextWaterLevel
// *************************************************
long GetNextWaterLevel(long currentLevel)
{
	long *g;
	long x, y;
	long nextLevel = 99999L;
	
	g = gGraphStart;
	y = gBoardSize;
	do {
		x = gBoardSize;
		do {
			if (*g > currentLevel && *g < nextLevel) {
				nextLevel = *g;
			}
			g += E;
		} while (--x);
		g += gBoardSize;
	} while (--y);	
	
	return nextLevel;		
}

// *************************************************
// **** Flood
// *************************************************
// Finds lowest path from one side to the other
void Flood(Boolean isVertical)
{
	long i, waterLevel, delta;
	Boolean *s;
	long *g;
	
	// Reverse numbers if !isVertical
	if (!isVertical) {
		long *g = gGraph;
		for (i = gHexes; i; --i) {
			*g = -*g;
			++g;
		}
	}
	
	// Clear gSubmerged
	s = gSubmerged;
	for (i = gHexes; i; --i)
		*(s++) = FALSE;
	
	// Find highest safe starting value for waterLevel:
	//   the height of the lowest hex along low edge
	waterLevel = 99999L;
	delta = isVertical ? E : SE;
	g = gGraphStart;
	for (i=gBoardSize; i; --i) {
		if (*g < waterLevel) {
				waterLevel = *g;
		}
		g += delta;
	}

	// Pour in water from one side
	waterLevel = GetNextWaterLevel(waterLevel);
	while (!Pour(waterLevel, isVertical)) {
		waterLevel = GetNextWaterLevel(waterLevel);
	}
}

// *************************************************
// **** InitializeEmptyGraph
// *************************************************
void InitializeEmptyGraph()
{
	register long i, *g, *t;
	long distance, stop;
	
	// Initialize borders
	g = gEmptyGraph;
	t = gTempGraph;
	
	i = gOnHexes;	
	while (i--)
		*(g++) = *(t++) = -SCALE;
		
	i = gBoardSize * gRealBoardSize;
	while (i--)
		*(g++) = *(t++) = SCALE;
		
	i = gOnHexes;
	while (i--)
		*(g++) = *(t++) = -SCALE;

	// Initialize border corners
	*(gEmptyGraphStart + gBoardSize + NW) = 0;
	*(gTempGraphStart  + gBoardSize + NW) = 0;
	*(gEmptyGraphStart + gBoardSize * gRealBoardSize + W) = 0;
	*(gTempGraphStart  + gBoardSize * gRealBoardSize + W) = 0;
	
	
	stop = gBoardSize / 2 + 1;

	// Approximation for influence of left and right edges (positive)
	g = gEmptyGraphStart;
	
	distance = 1;
	do {
		for (i=gBoardSize; i; --i) {
			*g = POTENTIAL_VALUE(distance);
			g += SE;
		}
		g += 1 + (NW * gBoardSize);
	} while (++distance < stop);
	
	do {
		--distance;
		for (i=gBoardSize; i; --i) {
			*g = POTENTIAL_VALUE(distance);
			g += SE;
		}
		g += 1 + (NW * gBoardSize);
	} while (distance != 1);	


	// Approximation for influence of top and bottom edges (negative)
	g = gEmptyGraphStart;
	
	distance = 1;
	do {
		for (i=gBoardSize; i; --i) {
			*(g++) -= POTENTIAL_VALUE(distance);
		}
		g += gBoardSize;
	} while (++distance < stop);
	
	do {
		--distance;
		for (i=gBoardSize; i; --i) {
			*(g++) -= POTENTIAL_VALUE(distance);
		}
		g += gBoardSize;
	} while (distance != 1);
	
}

// *************************************************
// **** InfluenceGraph
// *************************************************
void InfluenceGraph(unsigned long **p, long *graph)
{
	register long i, *g;

	// Initialize
	g = graph;
	i = gOnHexes;
	while (i--)
		*(g++) = -INFINITY + 1; // very large negative number
	i = gBoardSize * gRealBoardSize;
	while (i--)
		*(g++) = INFINITY - 1; // very large number
	i = gOnHexes;
	while (i--)
		*(g++) = -INFINITY + 1; // very large negative number
	
	// Influence of borders
	{
		long distance, stop;
		stop = gBoardSize / 2 + 1;

		if (graph == gVGraph) {
			g = gVGraphStart;
			distance = 1;
			do {
				for (i=gBoardSize; i; --i) {
					*(g++) = 0; //POTENTIAL_VALUE(distance);
				}
				g += gBoardSize;
			} while (++distance < stop);
			do {
				--distance;
				for (i=gBoardSize; i; --i) {
					*(g++) = 0; //POTENTIAL_VALUE(distance);
				}
				g += gBoardSize;
			} while (distance != 1);	
			
		} else {
			g = gHGraphStart;
			distance = 1;
			do {
				for (i=gBoardSize; i; --i) {
					*g = 0; //POTENTIAL_VALUE(distance);
					g += SE;
				}
				g += 1 + (NW * gBoardSize);
			} while (++distance < stop);
			do {
				--distance;
				for (i=gBoardSize; i; --i) {
					*g = 0; //POTENTIAL_VALUE(distance);
					g += SE;
				}
				g += 1 + (NW * gBoardSize);
			} while (distance != 1);	
		}
	}
	
	// Influence of pieces
	while (*p) {
		SpiralInfluence(graph + (*p - gBoard));
		++p;
	}
}


// *************************************************
// **** IsSaddlePoint
// *************************************************
long IsSaddlePoint(long dir, long k, long *wasUp, long *pG)
{
	long q, isUp;
	long *p;
	
	p = pG + dir;
	q = *p;
	if ( *(gBoardStart + (p - gGraphStart)) & BORDER_BIT ) {
		q = 9999999L;
		if (p < gGraphStart || p > gGraphStart + (gRealBoardSize * gBoardSize))
			q = -9999999L;
	} else if (k == q) {
		p += dir;
		if ( !(*(gBoardStart + (p - gGraphStart)) & BORDER_BIT) )
			q = *p;
	}
	if (k != q) {
		isUp = (k < q);
		if (isUp != *wasUp) {
			*wasUp = isUp;
			return 1;
		}
	}
	return 0;
}

// *************************************************
// **** InfluenceGraphs
// *************************************************
void InfluenceGraphs()
{
	// Set gVGraph and gHGraph
	InfluenceGraph(gVPieces, gVGraph);
	InfluenceGraph(gHPieces, gHGraph);

	{ // Set gGraph
		register long *pG, *pV, *pH, *pE;
		register unsigned long *pR;
		long x, y;

		pG = gGraphStart;
		pE = gEmptyGraphStart;
		pV = gVGraphStart;
		pH = gHGraphStart;
		pR = gRaised;
		
		y = gBoardSize;
		do {
			x = gBoardSize;
			do {
				*pG = *pE + *pH - *pV;
				if (*pR) {
					if (*pR & (H >> 2)) {
						*pG += BIG_RAISE;
					} else if (*pR & H) {
						*pG += RAISE;
					}
					if (*pR & (V >> 2)) {
						*pG -= BIG_RAISE;
					} else if (*pR & V) {
						*pG -= RAISE;
					}
				}
				pG += E;
				pE += E;
				pV += E;
				pH += E;
				pR += E;
			} while (--x);
			pG += gBoardSize;
			pE += gBoardSize;
			pV += gBoardSize;
			pH += gBoardSize;
			pR += gBoardSize;
		} while (--y);
	}
	
	{ // Clear maxima on gBoard
		long i;
		unsigned long *b;
		
		i = gHexes;
		b = gBoard;
		while (i--) {
			*b &= ~MAXIMUM;
			++b;
		}
	}
	
	{
		register long *pG, *pV, *pH, *pE;
		long x, y, k;

		// Mark maxima
		pG = gGraphStart;
		pE = gEmptyGraphStart;
		pV = gVGraphStart;
		pH = gHGraphStart;
		y = gBoardSize;
		do {
			x = gBoardSize;
			do {
				{
					long *d, count, wasUp;
					
					count = 0;
					k = *pG;
					wasUp = 2;
					for (d = gDirs; *d; ++d)
						count += IsSaddlePoint(*d, k, &wasUp, pG);
					if (count >= 4) {
						*(gBoardStart + (pG - gGraphStart)) |= MAXIMUM;
					}
				}
				pG += E;
				pE += E;
				pV += E;
				pH += E;
			} while (--x);
			pG += gBoardSize;
			pE += gBoardSize;
			pV += gBoardSize;
			pH += gBoardSize;
		} while (--y);
	}
}

// *************************************************
// **** ClearBonuses
// *************************************************
void ClearBonuses()
{	
	register long *p, i;
	
	p = gBonus;
	for (i=gHexes; i; --i)
		*(p++) = 0;
}

// *************************************************
// **** NearPlayBonus
// *************************************************
void NearPlayBonus(Boolean isVertical, long row, long col)
{
		unsigned long *where, *q;
		
		where = gBoardStart + (row * gRealBoardSize + col);
		
		if (isVertical) {
			q = where + NW;
			if (IS_EMPTY(q))
				gBonus[q - gBoard] += BONUS_NEAR;
			q = where + SE;
			if (IS_EMPTY(q))
				gBonus[q - gBoard] += BONUS_NEAR;
				
		} else {
			q = where + W;
			if (IS_EMPTY(q))
				gBonus[q - gBoard] += BONUS_NEAR;
			q = where + E;
			if (IS_EMPTY(q))
				gBonus[q - gBoard] += BONUS_NEAR;
		}
}

// *************************************************
// **** AdjustMove
// *************************************************
// Adjust move by 1 hex if indicated by bonuses
unsigned long * AdjustMove(unsigned long *move)
{
	long *d;
	unsigned long *best, *q, *r;
	long q_offset, r_offset;
	long bestBonus, bonus, m1, m2, height;
	
	height = gGraph[move - gBoard];
	best = move;
	bestBonus = gBonus[move - gBoard] + 1;
	
	for (d = gDirs; *d; ++d) {
		q = move + *d;
		
		if (IS_EMPTY(q)) {
			q_offset = q - gBoard;
			bonus = gBonus[q_offset];
			
			if (bonus >= bestBonus) {
				// Opposite pulls it too
				r = move - *d;
				r_offset = r - gBoard;				
				if (IS_EMPTY(r))
					bonus -= gBonus[r_offset] / 2;
					
				if (bonus > bestBonus) {
					// New best
					best = q;
					bestBonus = bonus;
					
				} else if (bonus == bestBonus && best != move) {
					// Toss-up: use the one closest to the original height
					m1 = gGraph[q_offset]      - height;
					m2 = gGraph[best - gBoard] - height;
					if (m1 < 0)
						m1 = -m1;
					if (m2 < 0)
						m2 = -m2;
					if (m1 < m2)
						best = q;
				}
			}
		}
	}
	
	return best;
}

// *************************************************
// **** NoConnections
// *************************************************
void NoConnections(SConnections *connects)
{	
	connects->Strong[0] =
		connects->Strong[1] =
		connects->Strong[2] =
		connects->Weak[0] =
		connects->Weak[1] =
		connects->Weak[2] =
		connects->Weak[3] =
		connects->Weak[4] =
		connects->Weak[5] = NULL_GROUP;
}

// *************************************************
// **** AddStrongConnection
// *************************************************
// Add strong connection to group if it doesn't exist
void AddStrongConnection(SConnections *connects, GROUP group)
{
	GROUP *s;
	
	for (s = connects->Strong; *s != NULL_GROUP; ++s)
		if (*s == group)
			return;

	*s = group;
}

// *************************************************
// **** AddWeakConnection
// *************************************************
// Add weak connection to group if it doesn't exist
void AddWeakConnection(SConnections *connects, GROUP group)
{
	register GROUP *s;
	
	for (s = connects->Weak; *s != NULL_GROUP; ++s)
		if (*s == group)
			return;

	*s = group;
}

// *************************************************
// **** CountStrongConnections
// *************************************************
// Returns number of strong connections
long CountStrongConnections(SConnections *connects)
{
	register GROUP *s;
	register long count = 0;

	for (s = connects->Strong; *s != NULL_GROUP; ++s)
		++count;
		
	return count;
}

// *************************************************
// **** CountWeakConnections
// *************************************************
// Returns number of strong connections
long CountWeakConnections(SConnections *connects)
{
	register GROUP *s;
	register long count = 0;

	for (s = connects->Weak; *s != NULL_GROUP; ++s)
		++count;
		
	return count;
}

// *************************************************
// **** RemoveStrongConnection
// *************************************************
// Removes strong connection
void RemoveStrongConnection(SConnections *connects, GROUP group)
{
	if (connects->Strong[0] == group) {
		connects->Strong[0] = connects->Strong[1];
		connects->Strong[1] = connects->Strong[2];
		connects->Strong[2] = NULL_GROUP;
		
	} else if (connects->Strong[1] == group) {
		connects->Strong[1] = connects->Strong[2];
		connects->Strong[2] = NULL_GROUP;
		
	} else if (connects->Strong[2] == group) {
		connects->Strong[2] = NULL_GROUP;
	}
}

// *************************************************
// **** HasStrongConnection
// *************************************************
Boolean HasStrongConnection(SConnections *connects, GROUP group)
{
	return connects->Strong[0] == group ||
				 connects->Strong[1] == group ||
				 connects->Strong[2] == group;
}

// *************************************************
// **** HasWeakConnection
// *************************************************
Boolean HasWeakConnection(SConnections *connects, GROUP group)
{
	return connects->Weak[0] == group ||
				 connects->Weak[1] == group ||
				 connects->Weak[2] == group ||
				 connects->Weak[3] == group ||
				 connects->Weak[4] == group ||
				 connects->Weak[5] == group;
}

// *************************************************
// **** ExistsOnStrongList
// *************************************************
Boolean ExistsOnStrongList(GROUP a, GROUP b)
{
	register long *p, i;
	
	// Make a < b
	if (b < a) {
		GROUP temp;
		temp = a; a = b; b = temp;
	}
	
	p = gStrongList;
	for (i=gStrongListSize; i; --i) {
		if (*p == a && *(p+1) == b)
			return TRUE;
		p += 2;
	}
	return FALSE;
}

// *************************************************
// **** AddToStrongList
// *************************************************
void AddToStrongList(GROUP a, GROUP b)
{
	long *p;
	
	// Make a < b
	if (b < a) {
		GROUP temp;
		temp = a; a = b; b = temp;
	}
	
	p = &gStrongList[gStrongListSize++ * 2];
	*p = a;
	*(++p) = b;
}

// *************************************************
// **** Merge
// *************************************************
void Merge(GROUP a, GROUP b)
{
	long offset;
	SConnections *c;
	
	// Make a < b
	if (b < a) {
		GROUP temp;
		temp = a; a = b; b = temp;
	}

	// b ---> a
	
	// Change groups, Change empties' Strong[]
	{
		unsigned long *q;
		long x, y;
		
		q = gBoardStart;
		y = gBoardSize;
		do {
			x = gBoardSize;
			do {
				offset = q - gBoard;
				if (gGroups[offset] == b)
					gGroups[offset] = a;
				if (IS_EMPTY(q)) {
					c = &gConnects[offset];
					if (HasStrongConnection(c, b)) {
						RemoveStrongConnection(c, b);
						AddStrongConnection(c, a);
					}
				}
				q += E;
			} while (--x);
			q += gBoardSize;
		} while (--y);
	}
	
	// Change in gStrongList
	{
		long i;
		long *p;
	
		p = gStrongList;
		for (i=gStrongListSize; --i; i) {
			if (*p == b)
				*p = a;
			if (*(p+1) == b)
				*(p+1) = a;
			if (*p == *(p+1)) {
				--gStrongListSize;
				*p = gStrongList[gStrongListSize * 2];
				*(p+1) = gStrongList[gStrongListSize * 2 + 1];
			}
			p += 2;
		}
	}
}	
	
// *************************************************
// **** ConnectionTactics
// *************************************************
unsigned long * ConnectionTactics(Boolean isVertical, Boolean friendly)
{
	register long *d;
	register long i, x, new_offset;
	register unsigned long *q;
	register unsigned long **p;
	long y, j, offset, count, delta;
	GROUP *g, group, tempList[30];
	Boolean *t, found;
	SConnections *c;
	unsigned long side = isVertical ? V : H;
	unsigned long **pieces = isVertical ? gVPieces : gHPieces;
	
	// Initialize for both calls
	if (friendly) {
		q = gRaised;
		i = gHexes;	
		do {
			*(q++) = 0L;
		} while (--i);
	}
	
	// Assign group NULL_GROUP to all hexes
	g = gGroups;
	i = gHexes;	
	do {
		*(g++) = NULL_GROUP;
	} while (--i);
	
	// Assign group 0 to one edge
	delta = isVertical ? E : SE;
	j = isVertical ? NW : W;
	g = gGroupsStart;
	i = gBoardSize;
	do {
		*g = *(g + j) = 0;
		g += delta;
	} while (--i);
	
	// Assign group 1 to other edge
	g = gGroupsStart + (gBoardSize - 1) * (isVertical ? gRealBoardSize : 1);
	i = gBoardSize;
	do {
		*g = *(g - j) = 1;
		g += delta;
	} while (--i);

	// Assign unique group numbers to each piece
	// Initialize gToDo
	group = 2;
	for (p = pieces; *p; ++p) {
		offset = *p - gBoard;
		if ( (isVertical && ON_TOP_EDGE(*p)) ||
				 (!isVertical && ON_LEFT_EDGE(*p)) ) {
			gGroups[offset] = 0;
		} else if ( (isVertical && ON_BOTTOM_EDGE(*p)) ||
								(!isVertical && ON_RIGHT_EDGE(*p)) ) {
			gGroups[offset] = 1;
		} else {
			gGroups[offset] = group++;
		}
		gToDo[offset] = TRUE;
	}
	
	// MERGE STRONGLY-CONNECTED GROUPS
	// Find all contiguous groups
	// Merge to lowest group numbers
	do {
		found = FALSE;
		for (p = pieces; *p; ++p) {
			offset = *p - gBoard;
			if (gToDo[offset]) {
				for (d = gDirs; *d; ++d) {
					new_offset = (*p + *d) - gBoard;
					if (gGroups[new_offset] > gGroups[offset]) {
						gGroups[new_offset] = gGroups[offset];
						if (gBoard[new_offset] & side) {
							gToDo[new_offset] = TRUE;
							found = TRUE; // found one we'll have to re-look at
						}
					}
				}
				gToDo[offset] = FALSE;
			}
		}
	} while (found);
	
	// FIND STRONG CONNECTIONS FOR EMPTIES
	// For each empty, find all strong connections (adjacent piece)
	// Also, if weakly connects two groups then note this in gRaised[]
	q = gBoardStart;
	y = gBoardSize;
	do {
		x = gBoardSize;
		do {
			if (IS_EMPTY(q)) {
				offset = q - gBoard;
				c = &gConnects[offset];
				NoConnections(c);
				if (gGroups[offset] < NULL_GROUP) {
					for (d = gDirs; *d; ++d) {
						new_offset = (q + *d) - gBoard;
						if ( (gBoard[new_offset] & side) ||
								 ((gBoard[new_offset] & BORDER_BIT) && gGroups[new_offset] <= 1) ) {
							AddStrongConnection(c, gGroups[new_offset]);
						}
					}
					if (c->Strong[1] != NULL_GROUP) {
						gRaised[offset] &= side;
					}
				}
			}
			q += E;
		} while (--x);
		q += gBoardSize;
	} while (--y);

	// Update gRaised[] for 2-way stretches
	q = gRaisedStart;
	y = gBoardSize;
	do {
		x = gBoardSize;
		do {
			if (*q & side) {
				for (d = gDirs; *d; ++d) {
					if (*(q + *d) & side) {
						*q &= (side >> 2);
					}
				}
			}
			q += E;
		} while (--x);
		q += gBoardSize;
	} while (--y);

	// MERGE WEAKLY CONNECTED GROUPS
	// Wherever two groups are adjacent to 2+ empties that are strongly-connected
	//   to both groups, then the groups are weakly connected: merge the two groups
	//   (if merging groups 0 and 1 then immediately play a merging empty)
	gStrongListSize = 0;
	q = gBoardStart;
	y = gBoardSize;
	do {
		x = gBoardSize;
		do {
			if (IS_EMPTY(q)) {
				offset = q - gBoard;
				j = gConnects[offset].Strong[1];
				if (j != NULL_GROUP) {
					i = gConnects[offset].Strong[0];
					if (ExistsOnStrongList(i, j)) {
						if (i + j == 1) {
							return q; // Strong connection between groups 0 and 1
						}
						Merge(i, j);
					} else {
						AddToStrongList(i, j);
					}
				}
			}
			q += E;
		} while (--x);
		q += gBoardSize;
	} while (--y);
	
	// FIND WEAK CONNECTIONS FOR EMPTIES
	// For each empty that is
	//   (a) adjacent to 2+ empties that are strongly-connected to group x, and
	//   (b) is not itself strongly-connected to group x
	// make that empty weakly-connected to x
	q = gBoardStart;
	y = gBoardSize;
	do {
		x = gBoardSize;
		do {
			if (IS_EMPTY(q)) {
				count = 0;
				offset = q - gBoard;
				c = &gConnects[offset];
				for (d = gDirs; *d; ++d) {
					if (IS_EMPTY(q + *d)) {
						new_offset = (q + *d) - gBoard;
						for (i=0; i<3; i++) {
							if (gConnects[new_offset].Strong[i] == NULL_GROUP)
								break;
							if (!HasStrongConnection(c, gConnects[new_offset].Strong[i])) {
								tempList[count++] = gConnects[new_offset].Strong[i]; // Add group to list
							}
						}
					}
				}
				if (count > 1) {
					for (i=0; i<count; ++i) {
						for (j=i+1; j<count; ++j) {
							if (tempList[i] == tempList[j]) { // Found repeated element
								AddWeakConnection(c, tempList[i]);
								// ### Could also Raise/Lower connection hexes
							}
						}
					}
				}
			}
			q += E;
		} while (--x);
		q += gBoardSize;
	} while (--y);
	
	// Play any empty that is strongly-connected to 0 and weakly-connected to 1
	//    or vice versa
	q = gBoardStart;
	y = gBoardSize;
	do {
		x = gBoardSize;
		do {
			if (IS_EMPTY(q)) {
				offset = q - gBoard;
				c = &gConnects[offset];
				if (HasStrongConnection(c, 0)) {
					if (HasWeakConnection(c, 1)) {
						return q; // Create a 2-way stretch between 0 and 1
					}
				}
				if (HasStrongConnection(c, 1)) {
					if (HasWeakConnection(c, 0)) {
						return q; // Create a 2-way stretch between 0 and 1
					}
				}
			}
			q += E;
		} while (--x);
		q += gBoardSize;
	} while (--y);
	
	// Find weak connections that connect groups 0 and 1
	// If friendly, play any found in order to connect up
	q = gBoardStart;
	count = 0;
	y = gBoardSize;
	do {
		x = gBoardSize;
		do {
			if (IS_EMPTY(q)) {
				offset = q - gBoard;
				if (HasWeakConnection(&gConnects[offset], 0) &&
						HasWeakConnection(&gConnects[offset], 1)) {
					if (friendly)
						return q; // Create 2-way stretches between both 0 and 1 groups
					gTemp[count++] = q;
				}
			}
			q += E;
		} while (--x);
		q += gBoardSize;
	} while (--y);
	
	// If not friendly and weak connections between groups 0 and 1 were found,
	//   play on the weak connection, or
	//   on an empty adjacent hex in order to block 2 weak connections simultaneously
	if (count) {
		if (count == 1)
			return gTemp[0]; // Create 2-way stretches between both 0 and 1 groups
		// See if any common empty hexes adjacent
		t = gToDoStart;
		for (i=gBoardSize * gRealBoardSize; i; --i)
			*(t++) = FALSE;
		for (i=0; i<count; ++i) {
			for (d = gDirs; *d; ++d) {
				q = gTemp[i] + *d;
				if (IS_EMPTY(q)) {
					offset = gBoard - q;
					if (gToDo[offset]) {
						// Two weak-connect hex have an adjacent hex in common: block there
						return q;
					}
					gToDo[offset] = TRUE;
				}
			}
		}
		// We've lost... block one
		return gTemp[0];
	}
	
	// LINKING BONUSES
	// For each empty, add bonus to that empty for
	//   (1) each weak connection
	//   (2) linking up multiple groups
	q = gBoardStart;
	y = gBoardSize;
	do {
		x = gBoardSize;
		do {
			if (IS_EMPTY(q)) {
				offset = q - gBoard;
				c = &gConnects[offset];
				j = CountStrongConnections(c) + CountWeakConnections(c);
				
				if (j == 1) {
					if (c->Weak[0] != NULL_GROUP)
						gBonus[offset] += friendly ? BONUS_WEAK_US : BONUS_WEAK_THEM;
						
				} else if (j > 1) {
					// This empty connects up 2 or more groups
					gBonus[offset] += BONUS_LINKING * (j-1);
					if (HasStrongConnection(c, 0) ||
							HasStrongConnection(c, 1) ||
							HasWeakConnection(c, 0) ||
							HasWeakConnection(c, 1)) {
						// Links group up with edge
						gBonus[offset] += BONUS_EDGE_LINKING;
					}
				}
			}
			q += E;
		} while (--x);
		q += gBoardSize;
	} while (--y);
	
	return NULL; // No immediate move
}


#include "Display.h"

