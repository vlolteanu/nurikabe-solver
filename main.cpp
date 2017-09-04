#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <boost/foreach.hpp>
#include <sstream>
#include <string>
#include <fstream>
#include <exception>
#include <iostream>
#include <map>
#include <algorithm>
#include <limits.h>

using namespace std;
using namespace boost;

struct Cell;

struct Island
{
	unsigned x;
	unsigned y;
	unsigned size;
	
	Island(unsigned x, unsigned y, unsigned size)
		: x(x), y(y), size(size) {}
};

struct Cell
{
	enum State
	{
		S_WHITE,
		S_GREY,
		S_BLACK,
	};
	
	State state;
	unsigned x;
	unsigned y;
	set<Island *> possibleOwners;
	bool claimed;
};

struct Table
{
	unsigned w;
	unsigned h;
	
	Cell cells[10][10];
	list<Island *> islands;
	
	set<pair<int, int> > greyCells;
	set<pair<int, int> > whiteCells;
	set<pair<int, int> > blackCells;
	
	set<Island *> unsolvedIslands;
};

struct Unsolvable {};

bool declareUnreachable(Table *table, Cell *cell, Island *island);

vector<Cell *> getNeighbours(Table *table, Cell *cell)
{
	vector<Cell *> ret;
	int x = cell->x;
	int y = cell->y;
	
	if (x - 1 >= 0)
		ret.push_back(&table->cells[x-1][y]);
	if (x + 1 < (int)table->w)
		ret.push_back(&table->cells[x+1][y]);
	if (y - 1 >= 0)
		ret.push_back(&table->cells[x][y-1]);
	if (y + 1 < (int)table->h)
		ret.push_back(&table->cells[x][y+1]);
	
	return ret;
}

bool blackenCell(Table *table, Cell *cell)
{
	//cout << "blackening " << cell->x << " " << cell->y << endl;
	if (cell->state == Cell::S_WHITE)
	{
		//cout << "busted" << endl;
		throw Unsolvable();
	}
	if (cell->state == Cell::S_BLACK)
		return false;
	
	cell->possibleOwners.clear();
	cell->state = Cell::S_BLACK;
	
	table->greyCells.erase(pair<int, int>(cell->x, cell->y));
	table->blackCells.insert(pair<int, int>(cell->x, cell->y));
	
	return true;
}

bool whitenCell(Table *table, Cell *cell)
{
	//cout << "whitening " << cell->x << " " << cell->y << endl;
	if (cell->state == Cell::S_BLACK)
		throw Unsolvable();
	if (cell->state == Cell::S_WHITE)
		return false;
	
	if (cell->possibleOwners.empty())
		throw Unsolvable();
	
	cell->state = Cell::S_WHITE;
	
	table->greyCells.erase(pair<int, int>(cell->x, cell->y));
	table->whiteCells.insert(pair<int, int>(cell->x, cell->y));
	
	BOOST_FOREACH(Cell *neighbour, getNeighbours(table, cell))
	{
		set<Island *> neighbourOwners = neighbour->possibleOwners;
		
		BOOST_FOREACH(Island *island, neighbourOwners)
		{
			if (cell->possibleOwners.find(island) == cell->possibleOwners.end())
				declareUnreachable(table, neighbour, island);
		}
	}
	
	return true;
}

bool declareOwner(Table *table, Cell *cell, Island *owner)
{
	bool ret = false;
	
	//cout << "claiming " << cell->x << " " << cell->y << endl;
	
	set<Island *> possibleOwners = cell->possibleOwners;
	BOOST_FOREACH(Island *island, possibleOwners)
	{
		if (island != owner)
			ret |= declareUnreachable(table, cell, island);
	}
	
	return ret;
}

bool declareUnreachable(Table *table, Cell *cell, Island *island)
{
	bool ret = cell->possibleOwners.erase(island) > 0;
	if (cell->possibleOwners.empty())
	{
		//cout << "no owners" << endl;
		blackenCell(table, cell);
	}
	if (cell->state == Cell::S_WHITE && ret)
	{
		BOOST_FOREACH(Cell *neighbour, getNeighbours(table, cell))
		{
			ret |= declareUnreachable(table, neighbour, island);
		}
	}
	return ret;
}

Table readTable(string filename)
{
	ifstream infile(filename.c_str());
	string line;
	Table table;
	unsigned lines = 0;
	
	while (getline(infile, line))
	{
		//cout << "read " << line << endl;
		if (lines == 0)
			table.w = line.size();
		else if (line.size() != table.w)
			throw exception();
		for (unsigned i = 0; i < table.w; i++)
		{
			Cell *cell = &table.cells[i][lines];
			
			cell->x = i;
			cell->y = lines;
			cell->state = Cell::S_GREY;
			cell->claimed = false;
			table.greyCells.insert(pair<int, int>(i, lines));
			
			if (line[i] > '0'  && line[i] <= '9')
			{
				Island *island = new Island(i, lines, line[i] - '0');
				
				table.islands.push_back(island);
				table.unsolvedIslands.insert(island);
			}
			else if (line [i] == '.')
			{
				//Nothing
			}
			else
			{
				throw exception();
			}
		}
		lines++;
	}
	table.h = lines;
	
	pair<int, int> coords;
	BOOST_FOREACH(coords, table.greyCells)
	{
		table.cells[coords.first][coords.second].possibleOwners.insert(table.islands.begin(), table.islands.end());
	}
	
	BOOST_FOREACH(Island *island, table.islands)
	{
		whitenCell(&table, &table.cells[island->x][island->y]);
		declareOwner(&table, &table.cells[island->x][island->y], island);
	}
	
	return table;
}

void dumpTable(const Table &table)
{
	for (unsigned j = 0; j < table.h; j++)
	{
		for (unsigned i = 0; i < table.w; i++)
		{
			Cell::State state = table.cells[i][j].state;
			if (state == Cell::S_BLACK)
			{
				cout << "#";
			}
			else if (state == Cell::S_GREY)
			{
				cout << ".";
			}
			else
			{
				bool found = false;
				BOOST_FOREACH (const Island *island, table.islands)
				{
					if (island->x == i && island->y == j)
					{
						cout << island->size;
						found = true;
						break;
					}
				}
				if (!found)
					cout << " ";
			}
		}
		cout << endl;
	}
}

void floodBlackReachability(const Table *table, set<pair<int, int> > *blacks, set<pair<int, int> > *visited, const pair<int, int> &start)
{
	int x = start.first;
	int y = start.second;
	
	if (x < 0 || x >= (int)table->w || y < 0 || y >= (int)table->h)
		return;
	
	if (visited->find(start) != visited->end())
		return;
	
	visited->insert(start);
	
	if (table->cells[x][y].state == Cell::S_WHITE)
		return;
	
	set<pair<int, int> >::iterator it = blacks->find(start);
	if (it != blacks->end())
		blacks->erase(it);
	
	floodBlackReachability(table, blacks, visited, pair<int, int>(x + 1, y));
	floodBlackReachability(table, blacks, visited, pair<int, int>(x - 1, y));
	floodBlackReachability(table, blacks, visited, pair<int, int>(x,     y + 1));
	floodBlackReachability(table, blacks, visited, pair<int, int>(x,     y - 1));
}

void blackReachability(const Table *table)
{
	set<pair<int, int> > blacks = table->blackCells;
	
	if (blacks.size() <= 1)
		return;
	pair<int, int> start = *blacks.begin();
	set<pair<int, int> > visited;
	floodBlackReachability(table, &blacks, &visited, start);
	
	if (blacks.size() > 0)
		throw Unsolvable();
}

void floodExplore(Table *table, Island *island, int x, int y, int distance, set<pair<int, int> > *reachable, set<pair<int, int> > *satellites)
{
	//cout << "island (" << island->x << "x" << island->y << "x" << distance << ") explore " << x << "," << y << " ";
	if (distance == 0)
	{
		//cout << "too far" << endl;
		return;
	}
	
	if (x < 0 || x >= (int)table->w || y < 0 || y >= (int)table->h)
	{
		//cout << "oo bounds" << endl;
		return;
	}
	
	Cell *cell = &table->cells[x][y];
	
	if (cell->possibleOwners.find(island) == cell->possibleOwners.end())
	{
		//cout << "not ownable" << endl;
		return;
	}
	
	//cout << "conquered" << endl;
	
	reachable->insert(pair<int, int>(x, y));
	if (satellites && cell->state == Cell::S_WHITE && !cell->claimed && cell->possibleOwners.size() == 1)
		satellites->insert(pair<int, int>(x, y));
	
	floodExplore(table, island, x + 1, y    , distance - 1, reachable, satellites);
	floodExplore(table, island, x - 1, y    , distance - 1, reachable, satellites);
	floodExplore(table, island, x    , y + 1, distance - 1, reachable, satellites);
	floodExplore(table, island, x    , y - 1, distance - 1, reachable, satellites);
}

bool floodClaim(Table *table, Island *island, int x, int y, int distance, set<pair<int, int> > *claimed)
{
	bool ret = false;
	//cout << "island (" << island->x << "x" << island->y << "x" << distance << ") claim " << x << "," << y << " ";
	if (distance == 0)
	{
		//cout << "too far" << endl;
		return false;
	}
	
	if (x < 0 || x >= (int)table->w || y < 0 || y >= (int)table->h)
	{
		//cout << "oo bounds" << endl;
		return false;
	}
	
	if (table->cells[x][y].state != Cell::S_WHITE)
		return false;
	
	ret |= whitenCell(table, &table->cells[x][y]);
	ret |= declareOwner(table, &table->cells[x][y], island);
	
	claimed->insert(pair<int, int>(x, y));
	table->cells[x][y].claimed = true;
	
	ret |= floodClaim(table, island, x + 1, y    , distance - 1, claimed);
	ret |= floodClaim(table, island, x - 1, y    , distance - 1, claimed);
	ret |= floodClaim(table, island, x    , y + 1, distance - 1, claimed);
	ret |= floodClaim(table, island, x    , y - 1, distance - 1, claimed);
	
	return ret;
}

bool checkReachability(Table *table)
{
	bool change = false;
	set<Island *> unsolvedIslands = table->unsolvedIslands;
	
	BOOST_FOREACH(Island *island, unsolvedIslands)
	{
		set<pair<int, int> > reachable;
		set<pair<int, int> > unreachable;
		set<pair<int, int> > claimed;
		set<pair<int, int> > satellites;
		
		change |= floodClaim(table, island, island->x, island->y, island->size, &claimed);
		if (claimed.size() > island->size)
			throw Unsolvable();
		if (claimed.size() == island->size)
			table->unsolvedIslands.erase(island);
		
		
		for (unsigned i = 0; i < table->w; i++)
		{
			for(unsigned j = 0; j < table->h; j++)
			{
				if (table->cells[i][j].state == Cell::S_BLACK)
					continue;
				unreachable.insert(pair<int, int>(i, j));
			}
		}
		
		pair<int, int> coords;
		
		BOOST_FOREACH(coords, claimed)
		{
			floodExplore(table, island, coords.first, coords.second, island->size - claimed.size() + 1, &reachable, &satellites);
		}
		if (reachable.size() < island->size)
		{
			//cout << "island doesn't fit (" << island->x << "," << island->y << "," << island->size << ")" << endl;
			throw Unsolvable();
		}
		
		if (reachable.size() == island->size)
		{
			BOOST_FOREACH(coords, reachable)
			{
				Cell *cell = &table->cells[coords.first][coords.second];
				whitenCell(table, cell);
				declareOwner(table, cell, island);
			}
			table->unsolvedIslands.erase(island);
		}
			
		BOOST_FOREACH(coords, reachable)
		{
			unreachable.erase(coords);
		}
		BOOST_FOREACH(coords, unreachable)
		{
			change |= declareUnreachable(table, &table->cells[coords.first][coords.second], island);
		}
		
		BOOST_FOREACH(coords, satellites)
		{
			set<pair<int, int> > backwardsReachable;
			set<pair<int, int> > backwardsUnreachable = reachable;
			
			floodExplore(table, island, coords.first, coords.second, island->size, &backwardsReachable, NULL);
			
			BOOST_FOREACH(coords, backwardsReachable)
			{
				backwardsUnreachable.erase(coords);
			}
			BOOST_FOREACH(coords, backwardsUnreachable)
			{
				change |= declareUnreachable(table, &table->cells[coords.first][coords.second], island);
			}
		}
	}
	
	return change;
}

void checkBlackSquare(Table *table, unsigned x, unsigned y)
{
	for (unsigned i = 0; i < 2; i++)
	{
		for (unsigned j = 0; j < 2; j++)
		{
			if (table->cells[x + i][y + j].state != Cell::S_BLACK)
				return;
		}
	}
	//cout << "black square " << x << "," << y << endl;
	throw Unsolvable();
}

void checkBlackSquares(Table *table)
{
	for (unsigned i = 0; i < table->w - 1; i++)
	{
		for (unsigned j = 0; j < table->h - 1; j++)
			checkBlackSquare(table, i, j);
	}
}

void sanity(Table *table)
{
	bool again;
	do
	{
		again = false;
		//cout << "checking black reachability" << endl;
		blackReachability(table);
		//cout << "checking reachability" << endl;
		again |= checkReachability(table);
		//cout << "checking black squares" << endl;
		checkBlackSquares(table);
	} while (again);
}

bool solve(Table *table, int maxDepth)
{
	int depth;
beginning:
	
	sanity(table);
	
	if (table->greyCells.empty())
		return true;
	
	depth = 0;
deeper:
	depth++;
	
	if (maxDepth < depth)
		return false;
	
	pair<int, int> coords;
	BOOST_FOREACH(coords, table->greyCells)
	{
		int x = coords.first;
		int y = coords.second;
		
		try
		{
			Table alteration(*table);
			//cout << "assuming (" << i << "," << j << ") is white" << endl;
			whitenCell(&alteration, &alteration.cells[x][y]);
			if (solve(&alteration, depth - 1))
			{
				*table = alteration;
				return true;
			}
		}
		catch (Unsolvable)
		{
			//cout << "nope, blackening (" << i << "," << j << ")" << endl;
			blackenCell(table, &table->cells[x][y]);
			goto beginning;
		}
		
		//cout << "meh" << endl;
		
		try
		{
			Table alteration(*table);
			//cout << "assuming (" << i << "," << j << ") is black" << endl;
			blackenCell(&alteration, &alteration.cells[x][y]);
			if (solve(&alteration, depth - 1))
			{
				*table = alteration;
				return true;
			}
		}
		catch (Unsolvable)
		{
			//cout << "nope, whitening (" << i << "," << j << ")" << endl;
			whitenCell(table, &table->cells[x][y]);
			goto beginning;
		}
		
		//cout << "meh" << endl;
	}
	
	goto deeper;
}

static const int MAX_DEPTH = INT_MAX;

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << "usage: nurikabe <file>" << endl;
		return 1;
	}
	
	Table table = readTable(argv[1]);
	//cout << "table is " << table.w << " x " << table.h << endl;
	//dumpTable(table);
	if (!solve(&table, MAX_DEPTH))
		throw Unsolvable();
	//dumpTable(table);
	//cout << "DONE" << endl;
	dumpTable(table);
	
	//cerr << "Depth: " << (depth - 1) << endl;
	
	return 0;
}
