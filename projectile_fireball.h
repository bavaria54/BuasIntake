#include "template.h"

namespace Tmpl8  //Everything else does this, I copy it.
{
    class Projectile_Fireball
    {
    public: //Public I guess, maybe private is better idk?
        Projectile_Fireball()
        {
            x = 0;
            y = 0;
            alive = false;
            speed = 3;
            damage = 35;
            dx = 1.0f;
            dy = 2.0f;
            dist = 3.0f;
        }
        float dx; float dy; float dist; int speed; float x; float y; int damage; bool alive;
        void Move(float deltaTime);
        bool Collision(int targetX, int targetY);
        void SetDirection(float nearestEnemyX, float nearestEnemyY);
    };
}