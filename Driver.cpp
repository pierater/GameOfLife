// ***********************************************************************
// A basic example of Conway's Game of Life.
// Author: Martin Almaraz
// ***********************************************************************


#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <vector>

// LIFE
void prepareTerminal();
void teardownTerminal();
void initializeLife();
void advanceGeneration();
void setCellInGrid(int, int, char);
char readCellFromGrid(int, int);
char readCellFromTerminal(int, int);
void printGridToTerminal();
int getNumNeighborsFromTerminal(int, int, bool);
void printGrid();
bool isAlive(int, int);
bool isAllDead();
bool isValidCoor(int, int);
bool isBorn(int);
bool isSurvive(int);
void debug(std::string);
void debug(char);
void debug(int);


// LIFE VARIABLES
// The probability of any cell to be alive on initialization
#define SPAWN_PERC 35

// Enable for debug logs
#define DEBUG false

// Symbol for "alive" cells
#define ALIVE_SYM 'x'

// Symbol for "dead" cells
#define DEAD_SYM ' '

// Number of generations, infinite if < 0
#define ITERATIONS -1

// Nanos to sleep between generations
#define ONE_SEC 1000000
#define SLEEP_NS ONE_SEC / 20

// A cell will survive if it has N alive neighbors that are present in this set
const std::vector<int> SURVIVE {2, 3};

// A cell will be born if it has N alive neighbors that are present in this set
const std::vector<int> BORN {3, 6};

// X cols length, if unspecified will inherits from term
int MAX_X;
// Y rows length, if unspecified will inherits from term
int MAX_Y;

// Current representation of the screen. ie. What's on the terminal
std::vector<std::vector<char>> CUR_GRID;
// Next representation of the screen. ie What will be printed to the terminal next
std::vector<std::vector<char>> NEXT_GRID;

int main()
{
    int iteration = 0;
    prepareTerminal();
    initializeLife();
    while (iteration != ITERATIONS)
    {
        iteration++;
        debug("1");
        clear();
        debug("2");
        printGridToTerminal();
        debug("3");
        move(0, 0);
        printw("Gen=%d", iteration);
        refresh();
        debug("4");
        advanceGeneration();
        debug("5");
        usleep(SLEEP_NS);
        debug("6");
        if (isAllDead())
        {
            std::cout << "ALL DEAD" << std::endl << "LEVEL=" << (ITERATIONS - iteration) << std::endl;
            printGrid();
            break;
        }
        debug("7");

        char input = getch();
        if (input == ' ') // Space
        {
            initializeLife();
            iteration = 0;
        }
    }
}

void interruptHandler(int signum)
{
    teardownTerminal();
    exit(signum);
}

void prepareTerminal()
{
    // Prepare ncurses
    initscr();
    // Hide cursor
    curs_set(0);
    // Set no timeout for input
    timeout(1);
    // Prepare random number generator
    srand(time(NULL));
    // Handle ctrl + c
    signal(SIGINT, interruptHandler);

    // Get screen size if not provided
    if (MAX_X == 0 || MAX_Y == 0)
    {
        int x, y;
        getmaxyx(stdscr, y, x);
        // ncurses reports it backwards
        MAX_X = y;
        MAX_Y = x;
    }
    CUR_GRID = std::vector<std::vector<char>>(MAX_Y);
    NEXT_GRID = std::vector<std::vector<char>>(MAX_Y);
    for (int y = 0; y < MAX_Y; y++)
    {
        CUR_GRID[y] = std::vector<char>(MAX_X);
        NEXT_GRID[y] = std::vector<char>(MAX_X);
    }
    debug("X=");
    debug(MAX_X);
    debug("Y=");
    debug(MAX_Y);
}

void teardownTerminal()
{
    // Disable ncurses
    curs_set(1);
    getch();
    endwin();
}

void initializeLife()
{
    for (int x = 0; x < MAX_X; x++)
    {
        for (int y = 0; y < MAX_Y; y++)
        {
            bool isFilled = (rand() % 100 + 1) <= SPAWN_PERC;
            setCellInGrid(x, y, isFilled ? ALIVE_SYM : DEAD_SYM);
        }
    }
}

void setCellInGrid(int x, int y, char out)
{
    NEXT_GRID[y][x] = out;
}

char readCellFromGrid(int x, int y)
{
    return NEXT_GRID[y][x];
}

void printGridToTerminal()
{
    for (int x = 0; x < MAX_X; x++)
    {
        for (int y = 0; y < MAX_Y; y++)
        {
//            debug(readCellFromGrid(x, y));
            move(x, y);
            if (readCellFromGrid(x, y) == ALIVE_SYM)
            {
                printw("%c", ALIVE_SYM);
                CUR_GRID[y][x] = ALIVE_SYM;
            }
            else
            {
                printw("%c", DEAD_SYM);
                CUR_GRID[y][x] = DEAD_SYM;
            }
        }
    }
}

char readCellFromTerminal(int x, int y) {
    if (!isValidCoor(x, y))
    {
        // Invalid coordinate, assume empty
        debug("INVALID COOR");
        debug(x);
        debug(y);
        return DEAD_SYM;
    }
    return CUR_GRID[y][x];
}

int getNumNeighborsFromTerminal(int x, int y, bool alive)
{
    int ret = 0;
    char target = alive ? ALIVE_SYM : DEAD_SYM;
    if (isValidCoor(x, y - 1))
    {
        char above = readCellFromTerminal(x, y - 1) != ALIVE_SYM ? DEAD_SYM : ALIVE_SYM;
        above == target ? ret++ : ret;
    }

    if (isValidCoor(x, y + 1))
    {
        char below = readCellFromTerminal(x, y + 1) != ALIVE_SYM ? DEAD_SYM : ALIVE_SYM;
        below == target ? ret++ : ret;
    }

    if (isValidCoor(x + 1, y))
    {
        char right = readCellFromTerminal(x + 1, y) != ALIVE_SYM ? DEAD_SYM : ALIVE_SYM;
        right == target ? ret++ : ret;
    }

    if (isValidCoor(x - 1, y))
    {
        char left = readCellFromTerminal(x - 1, y) != ALIVE_SYM ? DEAD_SYM : ALIVE_SYM;
        left == target ? ret++ : ret;
    }

    if (isValidCoor(x + 1, y - 1))
    {
        char up_right = readCellFromTerminal(x + 1, y - 1) != ALIVE_SYM ? DEAD_SYM : ALIVE_SYM;
        up_right == target ? ret++ : ret;
    }

    if (isValidCoor(x - 1, y - 1))
    {
        char up_left = readCellFromTerminal(x - 1, y - 1) != ALIVE_SYM ? DEAD_SYM : ALIVE_SYM;
        up_left == target ? ret++ : ret;
    }

    if (isValidCoor(x - 1 ,y + 1))
    {
        char bottom_left = readCellFromTerminal(x - 1, y + 1) != ALIVE_SYM ? DEAD_SYM : ALIVE_SYM;
        bottom_left == target ? ret++ : ret;
    }

    if (isValidCoor(x + 1, y + 1))
    {
        char bottom_right = readCellFromTerminal(x + 1, y + 1) != ALIVE_SYM ? DEAD_SYM : ALIVE_SYM;
        bottom_right == target ? ret++ : ret;
    }

    return ret;
}

bool isAlive(int x, int y)
{
    // https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life
    bool isCurAlive = readCellFromTerminal(x, y) == ALIVE_SYM;
//    debug(isCurAlive ? "CUR_ALIVE" : "CUR_DEAD");
//    debug(readCellFromTerminal(x, y));
    int numAlive = getNumNeighborsFromTerminal(x, y, true);


    // Any live cell with two or three live neighbours survives.
    if (isCurAlive && isSurvive(numAlive))
    {
        return true;
    }

    // Any dead cell with three live neighbours becomes a live cell.
    if (!isCurAlive && isBorn(numAlive))
    {
        return true;
    }

    // All other live cells die in the next generation. Similarly, all other dead cells stay dead.
    return false;
}

void advanceGeneration()
{
    for (int x = 0; x < MAX_X; x++)
    {
        for (int y = 0; y < MAX_Y; y++)
        {
            char out;
            if (isAlive(x, y))
            {
//                debug("ALIVE");
                out = ALIVE_SYM;
            }
            else
            {
//                debug("DEAD");
                out = DEAD_SYM;
            }
//            debug("PLACING=");
//            debug(out);
            setCellInGrid(x, y, out);
        }
    }
}

void debug(char ch)
{
    debug(std::string(1, ch));
}

void debug(int in)
{
    if (DEBUG)
    {
        std::cout << in << std::endl;
    }
}

void debug(std::string msg)
{
    if (DEBUG)
    {
        std::cout << msg << std::endl;
    }
}

bool isAllDead()
{
    for (int x = 0; x < MAX_X; x++)
    {
        for (int y = 0; y < MAX_Y; y++)
        {
            if (CUR_GRID[y][x] == ALIVE_SYM)
            {
                return false;
            }
        }
    }
    return true;
}

void printGrid()
{
    std::cout << "CURRENT GRID" << std::endl;
    for (int x = 0; x < MAX_X; x++)
    {
        for (int y = 0; y < MAX_Y; y++)
        {
            std::cout << CUR_GRID[y][x];
        }
        std::cout << std::endl;
    }

    std::cout << "LAST GRID" << std::endl;
    for (int x = 0; x < MAX_X; x++)
    {
        for (int y = 0; y < MAX_Y; y++)
        {
            std::cout << NEXT_GRID[y][x];
        }
        std::cout << std::endl;
    }
}

bool isValidCoor(int x, int y)
{
    return x >= 0 && x < MAX_X && y >= 0 && y < MAX_Y;
}

bool isBorn(int target)
{
    return std::find(BORN.begin(), BORN.end(), target) != BORN.end();
}

bool isSurvive(int target)
{
    return std::find(SURVIVE.begin(), SURVIVE.end(), target) != SURVIVE.end();
}

