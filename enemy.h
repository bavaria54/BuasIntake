#include "template.h"

namespace Tmpl8  //Everything else does this, I copy it.
{
    class Enemy
    {
    public:
        Enemy()
        {
            x = 0;
            y = 0;
            maxhealth = 100.0f;
            health = maxhealth;
            damage = 1.0f;
            screenpos = IRand(4); //Random value so I know where to put the enemy.
            alive = false;
            dist = 0.0f;
            recentlyDamaged = 0;
            recentDamage = 0;
        }
        void Reposition();
        void Refresh();
        void Collision(Enemy &enemy);
        float GetDistance(int targetX, int targetY);
        void isKilled();
        void Move(int player_drawx, int player_drawy, float deltaTime);
        bool isAlive();
        float x; float y; float health; float damage; int screenpos; bool alive; float dist; float maxhealth; int recentlyDamaged; int recentDamage;// so my variables know what they are
    };
}