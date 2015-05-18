#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "Level.h"
#include "GameObject.h"
#include "GameLogic.h"
#include "Lever.h"

using namespace DirectX;

enum CollisionTypes { CHARACTER, ENEMY, MOVABLEOBJECT };

static float Clamp(float x, float a, float b) { return x < a ? a : (x > b ? b : x); };

static XMFLOAT3 NextPositionFromCollision(bool& result, XMFLOAT3 nextPos, float radius, Coord tileCoord)
{
	//Calculate the tiles edge coordinates.
	float xMin = -(tileCoord.x + TILE_SIZE);
	float xMax = (float)-tileCoord.x;
	float yMin = -(tileCoord.y + TILE_SIZE);
	float yMax = (float)-tileCoord.y;

	//Find the closest point to the character on the perimiter of the tile.
	float closestX = Clamp(nextPos.x, xMin, xMax);
	float closestY = Clamp(nextPos.z, yMin, yMax);

	float distanceX = nextPos.x - closestX;
	float distanceY = nextPos.z - closestY;
	float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);

	if (distanceSquared < (radius * radius))
	{
		float length = sqrt(distanceSquared);

		nextPos.x = (distanceX / length) * radius + closestX;
		nextPos.z = (distanceY / length) * radius + closestY;

		result = true;
	}
	else
	{
		result = false;
	}
	return nextPos;
}

static XMFLOAT3 NextPositionFromDoorCollision(bool& result, XMFLOAT3 nextPos, float radius, Coord iteratorTileCoord, Coord nextTileCoord, Door* door)
{
	if (door != nullptr && !door->getIsOpen()) {
		XMFLOAT3 doorRot = door->GetRotationRad();
		int doorRotX = (int)round(cos(doorRot.y));
		int doorRotY = (int)round(sin(doorRot.y));

		if (iteratorTileCoord.y == nextTileCoord.y){
			if (doorRotY < 0){
				nextPos = NextPositionFromCollision(result, nextPos, radius, Coord(iteratorTileCoord.x, iteratorTileCoord.y - 1));
			}
			else if (doorRotY > 0){
				nextPos = NextPositionFromCollision(result, nextPos, radius, Coord(iteratorTileCoord.x, iteratorTileCoord.y + 1));
			}
		}
		else if (iteratorTileCoord.y < nextTileCoord.y){
			if (doorRotY > 0){
				nextPos = NextPositionFromCollision(result, nextPos, radius, Coord(iteratorTileCoord.x, iteratorTileCoord.y));
			}
		}
		else if (iteratorTileCoord.y > nextTileCoord.y){
			if (doorRotY < 0){
				nextPos = NextPositionFromCollision(result, nextPos, radius, Coord(iteratorTileCoord.x, iteratorTileCoord.y));
			}
		}


		if (iteratorTileCoord.x == nextTileCoord.x){
			if (doorRotX > 0){
				nextPos = NextPositionFromCollision(result, nextPos, radius, Coord(iteratorTileCoord.x - 1, iteratorTileCoord.y));
			}
			else if (doorRotX < 0){
				nextPos = NextPositionFromCollision(result, nextPos, radius, Coord(iteratorTileCoord.x + 1, iteratorTileCoord.y));
			}
		}
		else if (iteratorTileCoord.x < nextTileCoord.x){
			if (doorRotX < 0){
				nextPos = NextPositionFromCollision(result, nextPos, radius, Coord(iteratorTileCoord.x, iteratorTileCoord.y));
			}
		}
		else if (iteratorTileCoord.x > nextTileCoord.x){
			if (doorRotX > 0){
				nextPos = NextPositionFromCollision(result, nextPos, radius, Coord(iteratorTileCoord.x, iteratorTileCoord.y));
			}
		}
	}

	return nextPos;
}

static XMFLOAT3 ManagePlayerCollision(GameLogic* gl, Tile* iteratorTile, Coord iteratorTileCoord, Coord nextTileCoord, Coord currentTileCoord, float characterRadius, XMFLOAT3 nextPos)
{
	bool result = false;

	if (!iteratorTile->IsWalkable(gl->GetMoveObjectMode(), gl->GetSelectedObject()))
	{
		nextPos = NextPositionFromCollision(result, nextPos, characterRadius, iteratorTileCoord);
		if (iteratorTile != nullptr)
		{
			MovableObject* movableObject = iteratorTile->getMovableObject();
			if (movableObject != nullptr)
			{
				if (gl->GetMoveObjectMode())
					gl->SelectObject(movableObject);
				else
				{
					if (result && (currentTileCoord.x == iteratorTileCoord.x || currentTileCoord.y == iteratorTileCoord.y))
						gl->SelectObject(movableObject);
					else
						gl->SelectObject(nullptr);
				}
			}
		}
	}
	else
	{
		Door* door = iteratorTile->getDoor();
		nextPos = NextPositionFromDoorCollision(result, nextPos, characterRadius, iteratorTileCoord, nextTileCoord, door);

		Lever* lever = iteratorTile->getLever();
		if (lever != nullptr)
		{
			nextPos = NextPositionFromCollision(result, nextPos, 0.15f, iteratorTileCoord);

			if (result)
				gl->SelectObject(lever);
			else
				gl->SelectObject(nullptr);
		}

		PressurePlate* pressurePlate = iteratorTile->getPressurePlate();
		if (pressurePlate != nullptr)
		{
			if (iteratorTile->getMovableObject() != nullptr || nextTileCoord == iteratorTileCoord)
			{
				if (!pressurePlate->getIsActivated())
					pressurePlate->ActivatePressurePlate();
			}
			else
			{
				if (pressurePlate->getIsActivated())
					pressurePlate->DeactivatePressurePlate();
			}
		}
	}
	return nextPos;
}

static XMFLOAT3 ManageCollisions(GameLogic* gl, Level* currentLevel, GameObject* gameObject, XMFLOAT3 nextPos, CollisionTypes type)
{
	XMFLOAT3 currentPos = gameObject->GetPosition();
	float characterRadius = 1.0f;

	if (type == CollisionTypes::CHARACTER)
		characterRadius = ((Character*)gameObject)->getRadius();

	Tile* currentTile = currentLevel->getTile((int)(abs(currentPos.x)), (int)(abs(currentPos.z)));
	if (currentTile != nullptr)
	{
		Coord nextTileCoord = Coord((int)(abs(nextPos.x)), (int)(abs(nextPos.z)));
		Tile* nextTile = currentLevel->getTile(nextTileCoord.x, nextTileCoord.y);
		if (nextTile != nullptr)
		{
			for (int x = nextTileCoord.x - 1; x <= nextTileCoord.x + 1; x++)
			{
				for (int y = nextTileCoord.y - 1; y <= nextTileCoord.y + 1; y++)
				{
					Tile* iteratorTile = currentLevel->getTile(x, y);
					Coord iteratorTileCoord = Coord(x, y);
					nextPos = ManagePlayerCollision(gl, iteratorTile, iteratorTileCoord, nextTileCoord, gameObject->GetTileCoord(), characterRadius, nextPos);
				}
			}
		}
	}
	return nextPos;
}