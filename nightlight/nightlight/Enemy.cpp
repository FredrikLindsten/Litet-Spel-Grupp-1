#include "Enemy.h"
#include "Collision.h"
#include <math.h>

Enemy::Enemy(int id, XMFLOAT3 position, float rotation, RenderObject* renderObject, int coordX, int coordY, std::string enemyType)
	: GameObject(id, position, rotation, renderObject, coordX, coordY)
{
	if (enemyType == "small")
		this->enemyType = EnemyType::SMALL;
	else if (enemyType == "average")
		this->enemyType = EnemyType::MEDIUM;
	else if (enemyType == "large")
		this->enemyType = EnemyType::LARGE;

	followingPlayer = false;

	SetUpAnimation(renderObject, 3);
	SetPrimaryAnimation(0);
	PlayAnimation(0);
}

Enemy::~Enemy()
{
	path.clear();
}

bool Enemy::Update(Level* level, LightObject* spotlight, bool inLight)
{
	if (path.size() > 0)
	{
		XMFLOAT3 nextPos = position;
		if (!inLight)
		{
			nextPos.x += direction.x / SPEED;
			nextPos.z += direction.z / SPEED;
		}
		else
		{
			float pushSpeed = SPEED / 2;
			nextPos.x += spotlight->getDirection().x / pushSpeed;
			nextPos.z += spotlight->getDirection().z / pushSpeed;
		}

		float radius = 0.5f;
		bool result = false;

		Tile* currentTile = level->getTile(tileCoord);
		if (currentTile != nullptr)
		{
			Coord nextTileCoord = Coord((int)(abs(nextPos.x)), (int)(abs(nextPos.z)));
			for (int x = nextTileCoord.x - 1; x <= nextTileCoord.x + 1; x++)
			{
				for (int y = nextTileCoord.y - 1; y <= nextTileCoord.y + 1; y++)
				{
					Coord iteratorTileCoord = Coord(x, y);
					Tile* iteratorTile = level->getTile(iteratorTileCoord);
					if (!iteratorTile->IsWalkableAI())
					{
						nextPos = NextPositionFromCollision(result, nextPos, radius, iteratorTileCoord);

						if (result && iteratorTile) 
						{
							Container* shadowContainer = iteratorTile->getShadowContainer();
							if (shadowContainer && !shadowContainer->GetIsActivated()) 
							{
								shadowContainer->ActivateContainer();
								return false;
							}
						}
					}
				}
			}
		}

		float xRem = -(nextPos.x - (int)nextPos.x);
		float zRem = -(nextPos.z - (int)nextPos.z);


		if (tileCoord == next && xRem >= 0.45f && zRem >= 0.45f)
		{
			XMINT2 p = path.at(path.size() - 1);
			this->path.pop_back();

			next = Coord(p.x, p.y);
			direction = XMFLOAT3((-position.x - (next.x + (TILE_SIZE / 2))), 0.0f, -position.z - (next.y + (TILE_SIZE / 2)));
			//desiredRotation = -atan2(direction.z, direction.x) * 180 / XM_PI;
			rotation.y = -atan2(direction.z, direction.x) * 180 / XM_PI;
			SetRotationDeg(rotation);
		}

		//float difference = desiredRotation - rotation.y;
		//cout << difference << endl;

		//if (difference < 180)
		//	rotation.y += difference / 10;
		//else
		//	rotation.y -= difference / 10;


		//SetRotationDeg(rotation);

		SetPosition(nextPos);
		moved = 1;
	}
	else
	{
		moved = 0;
	}

	return true;
}

bool Enemy::IsFollowingPlayer()
{
	return followingPlayer;
}

void Enemy::SetFollowingPlayer(bool val)
{
	followingPlayer = val;
}

bool Enemy::HasValidPath(Level* level)
{
	return CheckPathValidity(level);
}

bool Enemy::CheckPathValidity(Level* level)
{
	//Traverse the path and check all tiles for validity
	for (int i = path.size() - 1; i >= 0; i--)
	{
		XMINT2 p = path.at(i);

		Tile* tile = level->getTile(p.x, p.y);
		if (!(tile && tile->IsWalkableAI()))
		{
			return false;
		}
	}

	if (GetTileCoord() == end)
		return false;

	if (path.size() <= 0)
		return false;

	return true;
}

void Enemy::SetPath(vector<XMINT2> path)
{
	this->path = path;

	if (path.size() > 0)
	{
		end = Coord(path.at(0).x, path.at(0).y);
		next = Coord(path.at(path.size() - 1).x, path.at(path.size() - 1).y);
		direction = XMFLOAT3((-position.x - (next.x + (TILE_SIZE / 2))), 0.0f, -position.z - (next.y + (TILE_SIZE / 2)));
		//desiredRotation = -atan2(direction.z, direction.x) * 180 / XM_PI;
		rotation.y = -atan2(direction.z, direction.x) * 180 / XM_PI;
		SetRotationDeg(rotation);
	}
}

vector<XMINT2> Enemy::GetPath()
{
	return path;
}

void Enemy::UpdateAnimation()
{
	if (currentAnim==primaryAnim)
	{
		UpdatePrimaryAnimation();
	}
	else
	{
		UpdateSecondaryAnimation();
	}
}

void Enemy::UpdatePrimaryAnimation()
{
	if (moved)
	{
		frame += 0.1f;
		if (frame > (framelength * 3))
			frame = 0;

		if (frame < (framelength * 3))
		{
			Weights.x = 0;
			Weights.y = (frame - (framelength * 2)) / framelength;
			Weights.z = 0;
			Weights.w = (-frame + (framelength * 3)) / framelength;
		}
		if (frame < (framelength * 2))
		{
			Weights.x = 0;
			Weights.y = 0;
			Weights.z = (-frame + (framelength * 2)) / framelength;
			Weights.w = (frame - (framelength)) / framelength;
		}
		if (frame < (framelength))
		{
			Weights.x = 0;
			Weights.y = (-frame + (framelength)) / framelength;
			Weights.z = frame / framelength;
			Weights.w = 0;
		}
	}
	else{
		frame = 0;
		Weights = { 1, 0, 0, 0 };
	}
//	system("CLS");
// 	printf("%f\n%f\n%f\n%f", Weights.x, Weights.y, Weights.z, Weights.w);
}

void Enemy::UpdateSecondaryAnimation()
{
	frame += 0.1f;
	if (frame > (framelength * 4))
		PlayAnimation(primaryAnim);

	if (frame < (framelength * 4))
	{
		Weights.x = (frame - (framelength * 3)) / framelength;
		Weights.y = 0;
		Weights.z = 0;
		Weights.w = (-frame + (framelength * 4)) / framelength;
	}
	if (frame < (framelength * 3))
	{
		Weights.x = 0;
		Weights.y = 0;
		Weights.z = (-frame + (framelength * 3)) / framelength;
		Weights.w = (frame - (framelength * 2)) / framelength;
	}
	if (frame < (framelength * 2))
	{
		Weights.x = 0;
		Weights.y = (-frame + (framelength * 2)) / framelength;
		Weights.z = (frame - (framelength)) / framelength;
		Weights.w = 0;
	}
	if (frame < (framelength))
	{
		Weights.x = (-frame + (framelength)) / framelength;
		Weights.y = frame / framelength;
		Weights.z = 0;
		Weights.w = 0;
	}
}

void Enemy::PlayAnimation(int animID)
{
	frame = 0;
	currentAnim = animID;
	renderObject = animations[animID].animation;
	framelength = animations[animID].framelength;
}

void Enemy::SetUpAnimation(RenderObject* anim, float framelengthin)
{
	Animation temp;
	temp.animation = anim;
	temp.framelength = framelengthin;
	animations.push_back(temp);
}

void Enemy::SetPrimaryAnimation(int animID)
{
	frame = 0;
	primaryAnim = animID;
	currentAnim = animID;
	renderObject = animations[animID].animation;
	framelength = animations[animID].framelength;
}
