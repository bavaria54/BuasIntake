#include "pickup_experience.h"

namespace Tmpl8  //Everything else does this, I copy it.
{
	void Pickup_Exp::isPickedUp()
	{
		alive = 0;
	}
	void Pickup_Exp::isSpawned(float target_x, float target_y)
	{
		x = target_x;
		y = target_y;
	}
	void Pickup_Exp::isMagneted()
	{
		float dx = x - ScreenWidth/2, dy = y - ScreenHeight/2;
		dist = sqrtf(dx * dx + dy * dy);
		x -= (dx / dist)*4, y -= (dy / dist)*4;
	}
	float Pickup_Exp::GetDistance()
	{
		float dx = x - ScreenWidth / 2, dy = y - ScreenHeight / 2;
		return sqrtf(dx * dx + dy * dy);
	}
}