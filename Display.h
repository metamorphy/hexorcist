void PrintBoard();

//#define FULL_GRAPH

// *************************************************
// **** PrintBoard
// *************************************************
void PrintBoard()
{
	int i, j;
	unsigned long *p;
	long offset, height;

	p = gBoardStart;
	printf("\n  ");
	for (j=0; j<gBoardSize; ++j)
		printf("%d ", j%10);
	printf("\n");
	for (i=0; i<gBoardSize; ++i) {
		for (j=0; j<i; ++j)
			printf(" ");
		printf("%2d ", i);
		for (j=0; j<gBoardSize; ++j) {
			offset = p - gBoardStart;
			
			// ### *(gGraphStart + offset); 
			height = *(gEmptyGraphStart + offset) + *(gHGraphStart + offset) - *(gVGraphStart + offset);
			if (*p & BORDER_BIT)
				printf("?");
			else if (*p & V)
				printf("V");
			else if (*p & H)
				printf("H");
#ifdef FULL_GRAPH
			else if (*p & MAXIMUM)
				printf("#");
/*			else if (*p & HREALMAXIMUM) {
				if (*p & VREALMAXIMUM)
					printf("#");
				else if (*p & VMAXIMUM)
					printf("­");
				else
					printf("=");
			} else if (*p & VREALMAXIMUM) {
				if (*p & HMAXIMUM)
					printf("¹");
				else
					printf("\\");
			} else if (*p & HMAXIMUM) {
				if (*p & VMAXIMUM)
					printf("+");
				else
					printf("-");
			} else if (*p & VMAXIMUM)
				printf("`"); */
#endif
			else
				printf(".");
			if (j != gBoardSize-1)
				printf(" ");
			p += E;
		}
		printf(" %d\n", i%10);
		p += gBoardSize;
	}
	for (j=0; j<=gBoardSize+2; ++j)
		printf(" ");
	for (j=0; j<gBoardSize; ++j)
		printf("%d ", j%10);
	printf("\n");
}

#ifdef HEX_DEBUG

// *************************************************
// **** PrintBonuses
// *************************************************
void PrintBonuses()
{
	int i, j;
	unsigned long *p;

	p = gBoardStart;
	printf("\n   ");
	for (j=0; j<gBoardSize; ++j)
		printf("%2d ", j%100);
	printf("\n");
	for (i=0; i<gBoardSize; ++i) {
		for (j=0; j<i; ++j)
			printf("  ");
		printf("%2d   ", i%100);
		for (j=0; j<gBoardSize; ++j) {
			if (*p & V)
				printf(" V");
			else if (*p & H)
				printf(" H");
/*
			else if (CountStrongConnections(&gConnects[p - gBoard]))
				printf("S%d", CountStrongConnections(&gConnects[p - gBoard]));
			else if (CountWeakConnections(&gConnects[p - gBoard]))
				printf("W%d", CountWeakConnections(&gConnects[p - gBoard]));
*/
			else if (!gBonus[p - gBoard])
				printf(" -");
			else
				printf("%2ld", gBonus[p - gBoard]);
			printf(" ");
			p += E;
		}
		printf("  %d\n", i%100);
		p += gBoardSize;
	}
	for (j=0; j<=gBoardSize; ++j)
		printf("  ");
	printf("     ");
	for (j=0; j<gBoardSize; ++j)
		printf("%d  ", j%10);
	printf("\n");
}

// *************************************************
// **** PrintConnections
// *************************************************
void PrintConnections()
{
	int i, j;
	GROUP *g;

	g = gGroupsStart;
	printf("\n   ");
	for (j=0; j<gBoardSize; ++j)
		printf("%2d ", j%100);
	printf("\n");
	for (i=0; i<gBoardSize; ++i) {
		for (j=0; j<i; ++j)
			printf("  ");
		printf("%2d   ", i%100);
		for (j=0; j<gBoardSize; ++j) {
			if (*g == NULL_GROUP)
				printf("-");
			else
				printf("%d", *g);
			printf("  ");
			g += E;
		}
		printf(" %d\n", i%100);
		g += gBoardSize;
	}
	for (j=0; j<=gBoardSize+1; ++j)
		printf("  ");
	for (j=0; j<gBoardSize; ++j)
		printf("%d  ", j%10);
	printf("\n");
}

// *************************************************
// **** PrintFlooded
// *************************************************
void PrintFlooded()
{
	int i, j;
	unsigned long *p;
	Boolean *b;

	p = gBoardStart;
	b = gSubmergedStart;
	printf("\n   ");
	for (j=0; j<gBoardSize; ++j)
		printf("%2d ", j%100);
	printf("\n");
	for (i=0; i<gBoardSize; ++i) {
		for (j=0; j<i; ++j)
			printf("  ");
		printf("%2d   ", i%100);
		for (j=0; j<gBoardSize; ++j) {
			if (*p & BORDER_BIT)
				printf("?");
			else if (*p & V)
				printf("V");
			else if (*p & H)
				printf("H");
			else if (*b)
				printf("w");
			else
				printf("-");
			printf("  ");
			p += E;
			b += E;
		}
		printf(" %d\n", i%100);
		p += gBoardSize;
		b += gBoardSize;
	}
	for (j=0; j<=gBoardSize+1; ++j)
		printf("  ");
	for (j=0; j<gBoardSize; ++j)
		printf("%d  ", j%10);
	printf("\n");
}

// *************************************************
// **** PrintNumericBoard
// *************************************************
void PrintNumericBoard()
{
	int i, j;
	unsigned long *p;
	long offset, height;

	p = gBoardStart;
	printf("\n  ");
	for (j=0; j<gBoardSize; ++j)
		printf("   "); //"%d  ", j%10);
	printf("\n");
	for (i=0; i<gBoardSize; ++i) {
		for (j=0; j<i; ++j)
			printf("  ");
		printf("     ");//"%2d   ", i%100);
		for (j=0; j<gBoardSize; ++j) {
			offset = p - gBoardStart;
			
			// ### *(gGraphStart + offset); 
			height = *(gEmptyGraphStart + offset) + *(gHGraphStart + offset) - *(gVGraphStart + offset);
			if (*p & BORDER_BIT)
				printf("  ? ");
			else if (*p & V)
				printf(" VVV");
			else if (*p & H)
				printf(" HHH");
			else if (*p & MAXIMUM)
				printf("#%3ld", height/(SCALE/50));
			else
				printf("%4ld", height/(SCALE/50));
			printf(" ");
			p += E;
		}
		printf("\n");
		p += gBoardSize;
	}
	printf("\n");
}

#endif // HEX_DEBUG
