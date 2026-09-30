void PrintBoard();

// *************************************************
// **** PrintBoard
// *************************************************
void PrintBoard()
{
	int i, j;
	unsigned long *p;
	long offset, height;

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
			else if (*p & HREALMAXIMUM) {
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
				printf("`");
#else
			else
				printf(".");
#endif
			printf("  ");
			p += E;
		}
		printf(" %d\n", i%100);
		p += gBoardSize;
	}
	for (j=0; j<=gBoardSize+1; ++j)
		printf("  ");
	for (j=0; j<gBoardSize; ++j)
		printf("%d  ", j%10);
	printf("\n");
}
