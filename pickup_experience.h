#include "template.h"

namespace Tmpl8  //Everything else does this, I copy it.
{
    class Pickup_Exp
    {
    public: //Public I guess, maybe private is better idk?
        Pickup_Exp()
        {
            x = 0;
            y = 0;
            frame = 0;
            alive = false;
            magnet = false;
            dist = 0;
            xpValue = 25;
        }
        void isPickedUp();
        float GetDistance();
        void isSpawned(float target_x, float target_y);
        void isMagneted();
        int xpValue; int frame = 0; float x; float y; bool alive; bool magnet; float dist;// so my variables know what they are
    };
}