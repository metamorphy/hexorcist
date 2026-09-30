
#include "stdafx.h"
// Console.cpp : Defines the entry point for the console application.
//

// main()     THIS FILE NOT PART OF SOLUTION  (See Hex.c instead)
// Copyright Jeff Mallett 1997

//#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
void PrintBoard();

#define MOVES	40
// *************************************************
// **** main
// *************************************************
int main(void)
{
	void  *VStorage, *HStorage;
	// WIN long tmp;
	char *p;
	long i;
	long moveRow, moveCol, x, y;
	Boolean isVert;
	Boolean firstTime = true;

	printf ("Hello World, this is Hex!\n\n");

	srand(time(NULL));	

	VStorage=(void *)malloc(1048576L); // WIN NewPtr(1048576L);
	if (!VStorage) return -1;
	// WIN tmp=GetPtrSize(VStorage);
	
	p = (char *)VStorage;
	for (i=0; i<1048576L; ++i) *(p++) = 0;
		
	HStorage=(void *)malloc(1048576L); // WIN NewPtr(1048576L);
	if (!HStorage) return -1;
	// WIN tmp=GetPtrSize(HStorage);

	p = (char *)HStorage;
	for (i=0; i<1048576L; ++i) *(p++) = 0;

	//Hex (16, -1, -1, &moveRow, &moveCol, VStorage, true, true);
	
	//isVert = true;
	isVert = true;
	for (i=0; i<MOVES; ++i) {
		if (!i && isVert)
			x = y = -1;
		else if (scanf("%ld %ld", &x, &y) != 2)
			break; // End of input
		Hex (16, y, x, &moveRow, &moveCol, VStorage, firstTime, isVert);
	/*
		Hex (16, -1, -1, &moveRow, &moveCol, isVert ? VStorage : HStorage, firstTime, isVert);
		isVert = !isVert;
	*/
		if (moveRow == -1)
			break;
		printf("%ld, %ld\n", moveCol, moveRow);
		PrintBoard();
		firstTime = false;
	}

	return 0;
}
