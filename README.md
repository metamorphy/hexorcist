# Hexorcist

An *n*×*n* [Hex](https://en.wikipedia.org/wiki/Hex_(board_game)) playing engine written in C in 1997 by Jeff Mallett.

```text
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
```

Hex is played on a rhombus of hexagons. V tries to link the top and bottom edges, H the left and right. In this 9×9 position from the contest's problem statement, V can force a win.

It was submitted to the [MacTech Magazine](https://en.wikipedia.org/wiki/MacTech) "Programmer's Challenge" contest for March 1997 (v.13 Issue 3) and took second place, narrowly losing to Gregory Cooper's entry 80.08 to 79.98 points. The contest asked for a player that implemented a fixed `Hex()` entry point, used only 1MB of host-provided storage, and played on boards from 8×8 up to 64×64. Each win scored:
```
10 - (execution time in seconds)/boardSize
```
points.

Instead of a game-tree search, Hexorcist evaluates the board as terrain. Each hex gets a height from the influence of the edges and the pieces on the board. The engine then "pours water" in from one edge to find the lowest path across and picks moves at the saddle points along it. Connection tactics sit on top of this: stones are merged into groups, strong (adjacent) and weak (two-bridge) connections are tracked, and bonuses nudge the move toward hexes that link groups or block the opponent's links.

The engine is [`Hex.cpp`](Hex.cpp), the original C converted to C++ for the 2004 Visual C++ project; the conversion needed only a pointer cast and a few small cleanups. [`mactech/`](mactech/) has the submitted copy in C, the original challenge text, a scan of the magazine pages, and links to the published [challenge](http://preserve.mactech.com/articles/mactech/Vol.13/13.03/Mar97Challenge/index.html) (March 1997) and [results](http://preserve.mactech.com/articles/mactech/Vol.13/13.06/Jun97Challenge/index.html) (June 1997). Besides the source, the submission includes `Hexorcist.µ`, the Metrowerks CodeWarrior project, and `Hexorcist`, the test app it built: a classic Mac OS PowerPC executable that won't run on modern macOS. [`Console.cpp`](Console.cpp) is a small test driver, not part of the engine. It plays a 16×16 game, reads the opponent's moves from standard input and prints the board after each reply. Define `HEX_DEBUG` to also print the engine's internal boards (move bonuses, terrain heights, flooded hexes and connection groups) as it thinks; these debug printers come from the pre-submission working copy. [`mactech/submission/main.c`](mactech/submission/main.c) is the original Macintosh version of that driver, which used Toolbox `NewPtr` for storage.

This is historical source. The code dates from 1997 and was brought into a Visual C++ 6.0 AppWizard console project in 2004 (`Console.dsp`/`Console.dsw`, later converted to Visual Studio .NET as `Hexorcist.sln`/`Console.vcproj`; `StdAfx.h` maps the Mac `Boolean` type to `bool`). In 2016 it was compiled with GCC under Eclipse Neon CDT (`.project`/`.cproject`).
