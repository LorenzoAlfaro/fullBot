#include "CommMngr.h"
#include "UnitFun.h"
using namespace Filter;


void CommMngr::attackUnits(std::list<Unit> units, Position myPos)
{
    for (auto& u : units)
    {        
        u->attack(myPos, false);
    }
}
void CommMngr::moveUnits(std::list<Unit> units, Position myPos)
{
    for (auto& u : units)
    {
        u->move(myPos, false);
    }
}
void CommMngr::setRallyPoint(std::list<Unit> buildings, Position myPos)
{
    for (auto& u : buildings)
    {
        u->setRallyPoint(myPos);
    }
}

void CommMngr::scvManager(std::list<int> Miners, std::list<Unit> workers)
{
    for (auto& ID : Miners)
    {
        Unit u = UnitFun::getUnitByID(workers, ID);
        if (u->isIdle())
        {
            u->gather(u->getClosestUnit(IsMineralField || IsRefinery));
        }
    }
}

void CommMngr::buildSCVs(std::list<Unit> commandCenters)
{
    for (auto& u : commandCenters)
    {
        if (u->isIdle())
        {
            u->train(BWAPI::UnitTypes::Terran_SCV);
        }
    }
}

void CommMngr::trainMarines(std::list<Unit> barracks)
{
    for (auto& u : barracks)
    {
        if (u->isIdle() && !u->train(BWAPI::UnitTypes::Terran_Marine))
        {
            //supplyBlock(u);

        } // closure: failed to train idle unit
    }
}
