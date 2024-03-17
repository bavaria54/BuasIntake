#include "player.h"


namespace Tmpl8  //Everything else does this, I copy it.
{
	void Player::Refresh()
	{
		currenthealth = maxhealth;
	}

	bool Player::isDead()
	{
		return currenthealth < 0.1;
	}
}