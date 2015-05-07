#pragma once
#include <vector>
#include "Tile.h"
#include "AiUtil.h"

class Level 
{
private:
	std::vector<std::vector<Tile*>> tileGrid;
	std::vector<GameObject*> gameObjects;
public:
	Level ( );
	~Level ( );
	std::vector<GameObject*> GetGameObjects(){ return gameObjects; };
	void updateGameObjets();
	Tile* getTile (int x, int y);
	void setTile(Tile* tile, int x, int y);
};

