#include "game.h"
#include "surface.h"
#include "template.h"


namespace Tmpl8  //Everything else does this, I copy it.
{
    class Player
    {
    public: // Public I guess, maybe private is better idk?
        Player()
        {
            x = ScreenWidth / 2;    // Centers my character's starting point to the center of the horizontal plane (X)
            y = ScreenHeight / 2;   // Centers my character's starting point to the center of the vertical plane (Y)
            maxhealth = 100;
            currenthealth = maxhealth;
            damage = 10;
            healthpercentage = (currenthealth / maxhealth);
            healthcolor = 0;
            speed = 2;
        }
        void Refresh();
        bool isDead();
        int x; int y; float maxhealth; float currenthealth; int damage; float healthpercentage; int healthcolor; int speed; // so my variables know what they are
    };
}