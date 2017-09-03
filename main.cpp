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
};

struct Table
{
	unsigned w;
	unsigned h;
	
	Cell cells[10][10];
	list<Island *> islands;
};

struct Unsolvable {};

bool blackenCell(Cell *cell)
{
	cout << "blackening " << cell->x << " " << cell->y << endl;
	if (cell->state == Cell::S_WHITE)
		throw Unsolvable();
	if (cell->state == Cell::S_BLACK)
		return false;
	
	cell->possibleOwners.clear();
	
	cell->state = Cell::S_BLACK;
	return true;
}

bool whitenCell(Cell *cell)
{
	if (cell->state == Cell::S_BLACK)
		throw Unsolvable();
	if (cell->state == Cell::S_WHITE)
		return false;
	
	if (cell->possibleOwners.empty())
		throw Unsolvable();
	
	cell->state = Cell::S_BLACK;
	return true;
}

bool declareUnreachable(Cell *cell, Island *island)
{
	bool found = cell->possibleOwners.erase(island) > 0;
	if (cell->possibleOwners.empty())
	{
		cout << "no owners" << endl;
		blackenCell(cell);
	}
	return found;
}

Table readTable(string filename)
{
	ifstream infile(filename.c_str());
	string line;
	Table table;
	unsigned lines = 0;
	
	while (getline(infile, line))
	{
		cout << "read " << line << endl;
		if (lines == 0)
			table.w = line.size();
		else if (line.size() != table.w)
			throw exception();
		for (unsigned i = 0; i < table.w; i++)
		{
			Cell *cell = &table.cells[i][lines];
			
			cell->x = i;
			cell->y = lines;
			
			if (line[i] > '0'  && line[i] <= '9')
			{
				Island *island = new Island(i, lines, line[i] - '0');
				
				table.islands.push_back(island);
				
				cell->state = Cell::S_WHITE;
				cell->possibleOwners.insert(island);
			}
			else if (line [i] == '.')
			{
				cell->state = Cell::S_GREY;
			}
			else
			{
				throw exception();
			}
		}
		lines++;
	}
	table.h = lines;
	
	for (unsigned i = 0; i < table.w; i++)
	{
		for (unsigned j = 0; j < table.h; j++)
		{
			if (table.cells[i][j].possibleOwners.empty())
				table.cells[i][j].possibleOwners.insert(table.islands.begin(), table.islands.end());
		}
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
				cout << "*";
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
	set<pair<int, int> > blacks;
	
	for (unsigned i = 0; i < table->w; i++)
	{
		for (unsigned j = 0; j < table->h; j++)
		{
			if (table->cells[i][j].state == Cell::S_BLACK)
				blacks.insert(pair<int, int>(i, j));
		}
	}
	if (blacks.size() <= 1)
		return;
	pair<int, int> start = *blacks.begin();
	set<pair<int, int> > visited;
	floodBlackReachability(table, &blacks, &visited, start);
	
	if (blacks.size() > 0)
		throw Unsolvable();
}

bool drawBorders(Table *table)
{
	bool change = false;
	
	for (unsigned i = 0; i < table->w; i++)
	{
		for (unsigned j = 0; j < table->h; j++)
		{
			Cell *cell = &table->cells[i][j];
			
			if (cell->state == Cell::S_BLACK)
				continue;
			
			bool done = false;
			
			Cell *neighbours[4] = { NULL };
			int neighCount = 0;
			
			if ((int)i - 1 >= 0)
				neighbours[neighCount++] = &table->cells[i-1][j];
			if (i + 1 < table->w)
				neighbours[neighCount++] = &table->cells[i+1][j];
			if ((int)j - 1 >= 0)
				neighbours[neighCount++] = &table->cells[i][j-1];
			if (j + 1 < table->h)
				neighbours[neighCount++] = &table->cells[i][j+1];
			
			for (int k = 0; k < neighCount - 1; k++)
			{
				if (done)
					break;
				
				if (neighbours[k]->state == Cell::S_BLACK)
					continue;
				
				for (int l = 1; l < neighCount; l++)
				{
					if (neighbours[l]->state == Cell::S_BLACK)
						continue;
					
					bool intersect = false;
					
					BOOST_FOREACH(Island *island, neighbours[k]->possibleOwners)
					{
						if (neighbours[l]->possibleOwners.find(island) != neighbours[l]->possibleOwners.end())
						{
							intersect = true;
							break;
						}
					}
					
					if (!intersect)
					{
						change = true;
						cout << "border cell " <<
							"(" << neighbours[k]->x << "," << neighbours[k]->y << ") "<<
							"(" << neighbours[l]->x << "," << neighbours[l]->y << ") "<<
							endl;
						blackenCell(cell);
						done = true;
						break;
					}
				}
			}
		}
	}
	
	return change;
}

void floodConquer(Table *table, Island *island, int x, int y, int distance, set<pair<int, int> > *reachable)
{
	if (distance == 0)
		return;
	
	if (x < 0 || x >= (int)table->w || y < 0 || y >= (int)table->h)
		return;
	
	if (table->cells[x][y].possibleOwners.find(island) == table->cells[x][y].possibleOwners.end())
		return;
	
	if (reachable->find(pair<int, int>(x, y)) != reachable->end())
		return;
	
	reachable->insert(pair<int, int>(x, y));
	
	floodConquer(table, island, x + 1, y    , distance - 1, reachable);
	floodConquer(table, island, x - 1, y    , distance - 1, reachable);
	floodConquer(table, island, x    , y + 1, distance - 1, reachable);
	floodConquer(table, island, x    , y - 1, distance - 1, reachable);
}

bool checkReachability(Table *table)
{
	bool change = false;
	
	BOOST_FOREACH(Island *island, table->islands)
	{
		set<pair<int, int> > reachable;
		set<pair<int, int> > unreachable;
		for (unsigned i = 0; i < table->w; i++)
		{
			for(unsigned j = 0; j < table->h; j++)
			{
				if (table->cells[i][j].state == Cell::S_BLACK)
					continue;
				unreachable.insert(pair<int, int>(i, j));
			}
		}
		
		floodConquer(table, island, island->x, island->y, island->size, &reachable);
		if (reachable.size() < island->size)
		{
			cout << "island doesn't fit" << endl;
			dumpTable(*table);
			throw Unsolvable();
		}
		
		pair<int, int> coords;
		BOOST_FOREACH(coords, reachable)
		{
			unreachable.erase(coords);
		}
		BOOST_FOREACH(coords, unreachable)
		{
			change |= declareUnreachable(&table->cells[coords.first][coords.second], island);
		}
	}
	
	return change;
}

void check(Table *table)
{
	map<Island *, set<Cell *> > reachableCells;
	map<Island *, set<Cell *> > ownedCells;
	BOOST_FOREACH(Island *island, table->islands)
	{
		reachableCells[island] = set<Cell *>();
		ownedCells[island] = set<Cell *>();
	}
	
	bool again;
	do
	{
		again = false;
		blackReachability(table);
		cout << "drawing borders" << endl;
		again |= drawBorders(table);
		cout << "checking reachability" << endl;
		again |= checkReachability(table);
		dumpTable(*table);
	} while (again);	
}

int main(int argc, char *argv[])
{
	Table table = readTable("/home/vlad/nuri.txt");
	cout << "table is " << table.w << " x " << table.h << endl;
	check(&table);
	dumpTable(table);
	
	return 0;
}
