/*
 * Player.h
 *
 *  Created on: Jun 5, 2019
 *      Author: triforce
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#include "PhysicsObject.h"
#include "SpritedObject.h"
#include "Animation.h"

enum State {
	STANDING, RUNNING, BRAKING, JUMPING, FALLING
};

class Player: public PhysicsObject, public SpritedObject {
public:
	Player(Game* game);
	virtual void GeneralUpdate();
	virtual ~Player();
	bool grounded=false;
	virtual Vector Gravity() = 0;
	virtual float Accel() = 0;
	virtual float Decel() = 0;
	virtual float TopSpeed() = 0;
	virtual float AirAccel() = 0;
	virtual float AirDecel() = 0;
	void ChangeState(State newState) {
		if (state==newState) return;
		Animation* anim;
		switch (state) {
			case STANDING:
				anim=standing;
				break;
			case RUNNING:
				anim=running;
				break;
			case BRAKING:
				anim=braking;
				break;
			case JUMPING:
				anim=jumping;
				break;
			case FALLING:
				anim=falling;
				break;
			default:
				anim=current;
				break;
		}
		anim->index=0;
		current=anim;
	}
	float gsp = 0;
protected:
	Animation* standing;
	Animation* running;
	Animation* braking;
	Animation* jumping;
	Animation* falling;
	Animation* current;
	State state = STANDING;
};

#endif /* PLAYER_H_ */
