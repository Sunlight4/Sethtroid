/*
 * Tile.h
 *
 *  Created on: Jun 1, 2019
 *      Author: triforce
 */

#ifndef TILE_H_
#define TILE_H_
#include "Physics.h"
class Tile {
public:
	Tile(int, int, Shape*, int flag, CollisionInfo* info, int cflag = 0);
	virtual ~Tile();
	int originX, originY;
	Shape* shape;
	CollisionInfo* info;
	int flag;
	int cflag;
};

#endif /* TILE_H_ */
