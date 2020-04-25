#include "auxFun.h"


int auxFun::SupplyTotal(int commandCenterCount, int supplyDepotCount)
{
    return (commandCenterCount * 10) + (supplyDepotCount * 8);    
}

int auxFun::usedSupplyTotal(int marineCount, int scvCount)
{
    return (marineCount * 1) + (scvCount * 1);    
}

int auxFun::roomNeeded(int commandCenterCount, int barrackCount)
{
    int offset = 1;
    int supply = offset + commandCenterCount * 1 + barrackCount * 1; //will need finer adjuments per individual units

    return supply*2;
}

bool auxFun::validUnit(BWAPI::Unit u, std::list<int> deadUnits)
{
    bool valid = false;
    // Ignore the unit if it no longer exists
    // Make sure to include this block when handling any Unit pointer!
    if (!isUnitDead(deadUnits, u->getID()))
    {
        if (u->exists())
        {
            if (!u->isLockedDown() && !u->isMaelstrommed() && !u->isStasised())
            {
                if (!u->isLoaded() && u->isPowered() && !u->isStuck())
                {
                    if (u->isCompleted() && !u->isConstructing())
                    {
                        return true;
                    }
                }
            }
        }
    }    
    return valid;
}

bool auxFun::validFrame()
{
    bool valid = false;
    // Return if the game is a replay or is paused
    if (!BWAPI::Broodwar->isReplay() && !BWAPI::Broodwar->isPaused() && BWAPI::Broodwar->self())
    {
        // Prevent spamming by only running our onFrame once every number of latency frames.
        // Latency frames are the number of frames before commands are processed.
        if (BWAPI::Broodwar->getFrameCount() % BWAPI::Broodwar->getLatencyFrames() == 0)
        {
            valid = true;
        }
    }
    return valid;
}

bool auxFun::isUnitDead(std::list<int> &deadUnits, int id)
{
    bool isDead = false;
    for (auto& unit : deadUnits)
    {
        if (unit == id) 
        {
            return true;
        }
    }
    return isDead;
}

BWAPI::Position auxFun::getMousePosition()
{
    BWAPI::Position myPos(BWAPI::Broodwar->getScreenPosition().x + BWAPI::Broodwar->getMousePosition().x,
                            BWAPI::Broodwar->getScreenPosition().y + BWAPI::Broodwar->getMousePosition().y);
    return myPos;
}
