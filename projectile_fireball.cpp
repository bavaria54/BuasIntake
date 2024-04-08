#include "projectile_fireball.h"

namespace Tmpl8  //Everything else does this, I copy it.
{
	void Projectile_Fireball::SetDirection(float nearestEnemyX, float nearestEnemyY) 
	{
		dx = x - nearestEnemyX-32, dy = y - nearestEnemyY-32;
		dist = sqrtf(dx * dx + dy * dy);
	}

	void Projectile_Fireball::Move(float deltaTime)
	{
		x -= ((dx / dist) * speed) * deltaTime / 5, y -= ((dy / dist) * speed) * deltaTime / 5;
	}

	bool Projectile_Fireball::Collision(int targetX, int targetY)
	{
		if ((x > targetX && x < targetX + 64) && (y > targetY && y < targetY + 64))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}