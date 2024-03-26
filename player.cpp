#include "player.h"


namespace Tmpl8  //Everything else does this, I copy it.
{
	void Player::Refresh()
	{
		maxhealth = 100;
		currenthealth = maxhealth;
		healthpercentage = (currenthealth / maxhealth);
		healthcolor = 0;
		speed = 1.8;
		level = 1;
		experience = 0;
		attackSpeed = 1.0f;
		attackFrame = 0;
		attackInterval = 165 * 2;
		damageMultiplier = 1.0f;
	}

	void Player::LevelUp() //Increase some stats for now, later skills
	{
		maxhealth += 10;
		currenthealth += 10;
		speed += 0.1;
		attackSpeed += 0.15;
		experience -= level*100;
		level++;
		attackFrame = 0;
		damageMultiplier += 0.05f;
	}

	bool Player::isDead()
	{
		return currenthealth < 0.1;
	}
}