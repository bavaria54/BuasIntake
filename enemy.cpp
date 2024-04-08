#include "enemy.h"

namespace Tmpl8  //Everything else does this, I copy it.
{
    void Enemy::Reposition() //Repositions enemies just outside screen boundary
    {
		screenpos = IRand(4); //New screenpos to spawn from
		switch (screenpos) {
		case 0: //Leftside screen spawn
			x = -64;
			y = IRand(ScreenHeight);
			break;

		case 1: //Rightside screen spawn
			x = ScreenWidth;
			y = IRand(ScreenHeight);
			break;

		case 2: //Topside screen spawn
			x = IRand(ScreenWidth);
			y = -64;
			break;

		case 3: //Bottomside screen spawn
			x = IRand(ScreenWidth);
			y = ScreenHeight;
			break;
		}
    }

	void Enemy::Refresh() // Refresh makes the stats back to their natural values
	{
		health = 100;
		alive = true;
		recentlyDamaged = 0;
		Reposition();
	}

	void Enemy::Collision(Enemy &enemy) // If i'm too close to another Enemy then we push eachother away from another
	{
		float jdx = x - enemy.x, jdy = y - enemy.y;
		float jdist = sqrtf(jdx * jdx + jdy * jdy);

		if (jdist < 48 && enemy.alive == true)
		{
			if (enemy.x > enemy.x){
				x -= 1; enemy.x += 1;}
			if (enemy.x < x){
				x += 1; enemy.x -= 1;}
			if (enemy.y > y){
				y -= 1; enemy.y += 1;}
			if (enemy.y < y){
				y += 1; enemy.y -= 1;}
		}
	}

	void Enemy::Move(int player_drawx, int player_drawy, float deltaTime) // Move towards player, scales with deltaTime so FPS doesnt give slowdowns/speedups
	{
		float dx = x - player_drawx, dy = y - player_drawy;
		dist = sqrtf(dx * dx + dy * dy);
		x -= (dx / dist) * deltaTime / 5, y -= (dy / dist) * deltaTime / 5;
	}

	float Enemy::GetDistance(int targetX, int targetY) // Calculates distance using pythagoras theorem
	{
		float dx = x - targetX, dy = y - targetY;
		float dist = sqrtf(dx * dx + dy * dy);
		return dist;
	}

	bool Enemy::isAlive()
	{
		return alive;
	}

	void Enemy::isKilled()
	{
		alive = false;
		health = maxhealth;
	}
}