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
            healthpercentage = (currenthealth / maxhealth);
            healthcolor = 0;
            speed = 1.8;
            level = 1;
            experience = 0;
            attackSpeed = 1.0f;
            attackFrame = 0;
            attackInterval = 165*2;
            pickupRange = 150;
            damageMultiplier = 1.0f;
        }
        void Refresh();
        void LevelUp(int levelupID);
        bool isDead();
        int x; int y; float maxhealth; float currenthealth; float damageMultiplier; float healthpercentage; int healthcolor; float speed; int level; float experience; float attackSpeed; int attackFrame; int attackInterval; int pickupRange;// so my variables know what they are
    };
}