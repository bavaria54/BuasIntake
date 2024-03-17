#include "template.h"
#include "enemy.h"

namespace Tmpl8  //Everything else does this, I copy it.
{
    void Enemy::Reposition() //Repositions enemies just outside screen boundary
    {
		screenpos = IRand(4); //New screenpos to spawn from
		switch (screenpos) {
		case 0: //Leftside screen spawn
			x = -32 - IRand(50);
			y = IRand(ScreenHeight);
			break;

		case 1: //Rightside screen spawn
			x = -50 + ScreenWidth + IRand(50);
			y = IRand(ScreenHeight);
			break;

		case 2: //Topside screen spawn
			x = IRand(ScreenWidth);
			y = -32 - IRand(50);
			break;

		case 3: //Bottomside screen spawn
			x = IRand(ScreenWidth);
			y = -50 + ScreenHeight + IRand(50);
			break;
		}
    }

	void Enemy::Refresh()
	{
		health = 100;
		alive = true;
		Reposition();
	}

	void Enemy::Collision()
	{
		// WIP I tried to get the code that makes my enemies overlap less in here, but I couldn't get it to work.
	}

	void Enemy::Move(int player_drawx, int player_drawy)
	{
		float dx = x - player_drawx, dy = y - player_drawy;
		dist = sqrtf(dx * dx + dy * dy);
		x -= dx / dist, y -= dy / dist;
	}

	float Enemy::GetDistance()
	{
		return dist;
	}

	bool Enemy::isAlive()
	{
		return alive;
	}
}