#include "template.h"

namespace Tmpl8  //Everything else does this, I copy it.
{
    class Enemy
    {
    public: // Public I guess, maybe private is better idk?
        Enemy()
        {
            x = 0;
            y = 0;
            health = 100.0f;
            damage = 1.0f;
            screenpos = IRand(4); //Random value so I know where to put the enemy.
            alive = true;
            dist = 0.0f;
        }
        void Reposition();
        void Refresh();
        void Collision();
        float GetDistance();
        void Move(int player_drawx, int player_drawy);
        bool isAlive();
        float x; float y; float health; float damage; int screenpos; bool alive; float dist;// so my variables know what they are
    };
}