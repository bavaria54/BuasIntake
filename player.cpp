#include "player.h"


namespace Tmpl8
{
	void Player::Refresh()
	{
		maxhealth = 100;
		currenthealth = maxhealth;
		healthpercentage = (currenthealth / maxhealth);
		healthcolor = 0;
		speed = 1.3;
		level = 1;
		experience = 0;
		attackSpeed = 1.0f;
		attackFrame = 0;
		attackInterval = 165 * 2;
		damageMultiplier = 1.0f;
	}

	void Player::LevelUp(int levelupID) // 1= Attackspeed, 2 = Damage, 3 = Health & Speed
	{
		switch (levelupID) 
		{
		case 1: //Levelup Attackspeed
			attackSpeed += 0.5f;
			experience -= level * 100;
			level += 1;
			break;
		case 2: //Levelup Damage
			damageMultiplier += 0.5f;
			experience -= level * 100;
			level += 1;
			break;
		case 3: //Levelup Health & Speed
			maxhealth += 50;
			currenthealth += 50;
			speed += 0.4;
			experience -= level * 100;
			level += 1;
			break;
		}
	}

	bool Player::isDead()
	{
		return currenthealth < 0.1;
	}
}