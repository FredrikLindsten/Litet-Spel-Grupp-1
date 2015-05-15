#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

#include <vector>
#include <deque>
#include <list>

#include "Level.h"
#include "GameObject.h"
#include "Tile.h"

using namespace std;
using DirectX::XMFLOAT2;
using DirectX::XMINT2;

static bool AiUtil_ShowDebugPath = false;

static int ManhattanDistance(Tile* n1, Tile* n2) //Cheap, less accurate
{
	XMINT2 n1c = n1->GetTileCoord();
	XMINT2 n2c = n2->GetTileCoord();

	return (abs(n1c.x - n2c.x) + abs(n1c.y - n2c.y));
}

static int EuclideanDistance(Tile* n1, Tile* n2) //Expensive, more accurate
{
	XMINT2 n1c = n1->GetTileCoord();
	XMINT2 n2c = n2->GetTileCoord();

	return (int)sqrt((pow((n1c.x - n2c.x), 2) + pow((n1c.y - n2c.y), 2)));
}

static int ChebyshevDistance(Tile* n1, Tile* n2)
{
	XMINT2 n1c = n1->GetTileCoord();
	XMINT2 n2c = n2->GetTileCoord();

	return max(abs(n1c.x - n2c.x), abs(n1c.y - n2c.y));
}

static bool Equals(XMINT2 f1, XMINT2 f2)
{
	if (f1.x == f2.x && f1.y == f2.y)
		return true;
	else
		return false;
}

static int GenerateF(Tile* child, Tile* end)
{
	return child->GetG() + ManhattanDistance(child, end);
}

static vector<XMINT2> aStar(Level* level, XMINT2 startPosXZ, XMINT2 endPosXZ)
{
	vector<XMINT2> path;

	Tile* start = level->getTile(startPosXZ.x, startPosXZ.y);
	Tile* end = level->getTile(endPosXZ.x, endPosXZ.y);
	Tile* current = nullptr;
	Tile* child = nullptr;

	list<Tile*> openList;
	list<Tile*> closedList;

	list<Tile*>::iterator i;

	if (start && end)
	{
		//Counter to limit path length
		int n = 0;
		int limit = 100;

		openList.push_back(start);
		start->SetInOpen(true);
		start->SetParent(nullptr);

		while (n == 0 || (current != end && n < limit))
		{
			//Find the smallest F value in openList and make it the current tile
			for (i = openList.begin(); i != openList.end(); ++i)
				if (i == openList.begin() || (*i)->GetF() <= current->GetF())
					current = (*i);

			//Stop if it reached the end or the current tile holds a closed door
			Door* door = current->getDoor();
			if (current == end || (door && !door->getIsOpen()))
				break;

			openList.remove(current);
			current->SetInOpen(false);

			closedList.push_back(current);
			current->SetInClosed(true);

			//Get all adjacent tiles
			for (int x = -1; x <= 1; x++)
			{
				for (int y = -1; y <= 1; y++)
				{
					XMINT2 currentTileCoord = current->GetTileCoord();
					if (!(x == 0 && y == 0))
					{
						int xc = currentTileCoord.x + x;
						int yc = currentTileCoord.y + y;
						child = level->getTile(xc, yc);

						if (child && child != current)
						{
							XMINT2 childTileCoord = child->GetTileCoord();

							bool inClosed = child->InClosed();
							bool inOpen = child->InOpen();

							//Do not use when paths are generated each frame!
							//cout << "Node: " << childTileCoord.x << " " << childTileCoord.y << " inOpen: " << boolalpha << inOpen << " inClosed: " << boolalpha << inClosed << endl;

							if (!inClosed && child->IsWalkableAI())
							{
								int tentativeG = current->GetG() + (int)TILE_SIZE;

								//Already in open but a better solution found, update it
								if (inOpen)
								{
									if (child->GetG() > tentativeG)
									{
										child->SetParent(current);
										child->SetG(tentativeG);
										child->SetF(GenerateF(child, end));
									}
								}
								else if (current->GetParent() != child)
								{
									openList.push_back(child);
									child->SetInOpen(true);

									child->SetParent(current);
									child->SetG(tentativeG);
									child->SetF(GenerateF(child, end));
								}
							}
						}
					}
					else
					{
						//Utanf�r eller current sj�lv?
						continue;
					}
				}
			}

			//Add to the counter
			n++;
		}

		//Reset open/closed in tiles
		for (i = openList.begin(); i != openList.end(); ++i)
			(*i)->SetInOpen(false);
		for (i = closedList.begin(); i != closedList.end(); ++i)
			(*i)->SetInClosed(false);


		if (AiUtil_ShowDebugPath)
			start->getFloorTile()->SetSelected(true);

		//Retrace the path from the end to start
		while (current->GetParent() && current != start)
		{
			XMINT2 currentCoord = current->GetTileCoord();
			path.push_back(XMINT2(currentCoord.x,currentCoord.y));

			if (AiUtil_ShowDebugPath)
			{
				if (current->getFloorTile())
					current->getFloorTile()->SetSelected(true);
				if (current->getPressurePlate())
					current->getPressurePlate()->SetSelected(true);
			}

			current = current->GetParent();
			n++;
		}
	}

	return path;
}


//Check for corners 
//ALMOST WORKS, BUT IT KILLS THE LOOP SOMETIMES

//XMINT2 currentCoord = current->GetTileCoord();

//Tile* nextY = level->getTile(currentCoord.x, currentCoord.y + y);
//Tile* nextX = level->getTile(currentCoord.x + x, currentCoord.y);

////Tile* nextY = nullptr;
////Tile* nextX = nullptr;
////if (level->withinBounds(currentCoord.x, currentCoord.y + y))
////	nextY = level->getTile(currentCoord.x, currentCoord.y + y);
////if (level->withinBounds(currentCoord.x + x, currentCoord.y))
////	nextX = level->getTile(currentCoord.x + x, currentCoord.y);

//if (nextY != nullptr)
//	if (!nextY->IsWalkable() || nextY->InClosed())
//		continue;

//if (nextX != nullptr)
//	if (!nextX->IsWalkable() || nextX->InClosed())
//		continue;