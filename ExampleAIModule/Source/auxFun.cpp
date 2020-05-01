#include "auxFun.h"
using namespace BWAPI;
using namespace std;

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

void auxFun::displayInsights2(int roomNeeded, int supplyLeft2, int SCVcount, int barracksCount, int marineCount, int StatsCoordinates[][2], int Builders, int deadUnits,int taskQueueCount)
{
    //int supplyLeft2 = Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed();
    Broodwar->drawTextScreen(StatsCoordinates[0][0], StatsCoordinates[0][1], "FPS: %d", Broodwar->getFPS());
    //Broodwar->drawTextScreen(200, 20, "Average FPS: %f", Broodwar->getAverageFPS());
    Broodwar->drawTextScreen(StatsCoordinates[1][0], StatsCoordinates[1][1], "Brks: %d", barracksCount);
    Broodwar->drawTextScreen(StatsCoordinates[2][0], StatsCoordinates[2][1], "Mrn: %d", marineCount);
    Broodwar->drawTextScreen(StatsCoordinates[3][0], StatsCoordinates[3][1], "SCV: %d", SCVcount);
    Broodwar->drawTextScreen(StatsCoordinates[4][0], StatsCoordinates[4][1], "Bldrs: %d", Builders);
    Broodwar->drawTextScreen(StatsCoordinates[5][0], StatsCoordinates[5][1], "Dead: %d ", deadUnits);
    //Broodwar->drawTextScreen(StatsCoordinates[6][0], StatsCoordinates[6][1], "Mouse Cursor: %d  %d", Broodwar->getMousePosition().x, Broodwar->getMousePosition().y);
    //Broodwar->drawTextScreen(StatsCoordinates[7][0], StatsCoordinates[7][1], "Screen: %d  %d", Broodwar->getScreenPosition().x, Broodwar->getScreenPosition().y);

    //Broodwar->drawTextScreen(StatsCoordinates[8][0], StatsCoordinates[8][1], "supply limit: %d ", supplyLeft);
    Broodwar->drawTextScreen(StatsCoordinates[9][0], StatsCoordinates[9][1], "supply limit2: %d ", supplyLeft2 / 2);
    Broodwar->drawTextScreen(StatsCoordinates[10][0], StatsCoordinates[10][1], "room for next round: %d ", roomNeeded / 2);
    Broodwar->drawTextScreen(StatsCoordinates[11][0], StatsCoordinates[11][1], "Tasks: %d ", taskQueueCount);
    Broodwar->drawTextScreen(StatsCoordinates[12][0], StatsCoordinates[12][1], "APM: %d ", Broodwar->getAPM());
    Broodwar->drawTextScreen(StatsCoordinates[13][0], StatsCoordinates[13][1], "FrameCount: %d ", Broodwar->getFrameCount());

}

