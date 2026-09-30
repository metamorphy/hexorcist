Hex
This month features another round-robin tournament competition, this time with the game of Hex. You might have encountered Hex through one of Marvin Gardner's columns in Scientific American. The game is played on an NxN rhombus of hexagons, with one player attempting to occupy an unbroken chain of hexagons connecting the top and the bottom, and the other player the left with the right. Players alternate occupying hexagons, and the first to complete an unbroken chain is the winner. The prototype for the code you should write is:

Boolean /*legalMove*/ Hex (
 long boardSize, /* number of rows/columns in the game board */
 long oppRow,    /* row where opponent last moved, 0 .. boardSize-1 */
 long oppCol,    /* column where opponent last moved, 0 .. boardSize-1 */
 long *moveRow,  /* return your move - row, 0 .. boardSize-1 */
 long *moveCol,  /* return your move - column, 0 .. boardSize-1 */
 void *privStorage,/* preallocated storage for your use */
 Boolean newGame,/* TRUE if this is your first move in a new game */
 Boolean playFirst /* TRUE if you play first (vertically) */
);


For your first move, Hex will be called with newGame set to TRUE. The size of the board, a number between 8 and 64, inclusive, will be provided in boardSize. If playFirst is TRUE, your objective is to form a vertical connection across the rows of the board, and you make the first move. Otherwise you are playing second and the objective is to form a horizontal connection across the columns. Your code and the code of an opponent will alternate play. Your opponent's move will be provided in (oppRow, oppCol), which will be set to (-1, -1) if you are moving first. When your code is called, you should evaulate your opponents move, calculate your own move, and store it in (*moveRow, *moveCol). If for any reason you believe your opponent has moved into an occupied hexagon, Hex should return a legalMove value of FALSE, otherwise it should return TRUE.

A key to winning at Hex is to form "2-bridges" of hexagons that can be connected regardless of what move your opponent makes. For example, examine this 9x9 board, with the horizontal ("H") player to move:


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


In this example, V can force a win. H can block one of the (row,col) positions (6,2) or (6,5), but not both. If, for example, H occupies (6,2), then V can occupy (6,5), with a guaranteed "2-bridge" connection to (7,3) via (6,4) or (7,4). A similar bridge exists if H occupies (6,5). V is therefore unstoppable from this position.

For some additional strategy, see <http://www.daimi.aau.dk/~tusk/pbmserv/hex/hex.faq.html>, or <http://www.cs.cmu.edu/People/hde/hex/hexfaq>. There is a "proof" that the first player can always win, but the proof is a nonconstructive proof by counterexample that does not divulge the winning strategy.

Your code will be provided with 1MB of preallocated storage pointed to by privStorage. This storage will be preinitialized to zero before your first move and will persist between moves. You should not allocate any additional dynamic storage beyond that provided by privStorage. Small amounts of static storage are permitted.


The Challenge will be scored by a tournament where each entry plays against each other entry twice for each of a number of board sizes, once playing first and once playing second. Another fair tournament schedule might be used if a large number of entries are received. For each game, the winner will earn game points for the victory, minus an amount based on the execution time used to compute the moves, as follows:


game points = 10 - (execution time in seconds)/boardSize


The loser will earn zero points and will not be penalized for the execution time expended in defeat. The player with the most total points for all games in the tournament will be the winner.

This will be a native PowerPC Challenge, using the latest CodeWarrior environment. Solutions may be coded in C, C++, or Pascal. 

