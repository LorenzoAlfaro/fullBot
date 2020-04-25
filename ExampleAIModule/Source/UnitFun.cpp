#include "UnitFun.h"
#include <iostream>
#include "auxFun.h"

using namespace BWAPI;
using namespace Filter;


Unit UnitFun::getSCV(Unit CommandCenter, std::list<int> &Miners, std::list<int> &Builders, UnitType supplyProviderType)
{
    Unit worker = CommandCenter->getClosestUnit(GetType == supplyProviderType.whatBuilds().first &&
        (IsIdle || IsGatheringMinerals) && IsOwned);
    Miners.remove(worker->getID());//change duty to builder
    Builders.push_back(worker->getID());
    return worker;
}

Unit UnitFun::getBuilder(std::list<int> Builders)
{
    Unit builder = NULL;
    for (auto& u : Builders)
    {
        Unit myBuilder = Broodwar->getUnit( u);        
        if (myBuilder != NULL)
        {
            if (!myBuilder->isConstructing() && myBuilder->getHitPoints() != 0)
            {
                return myBuilder;                
            }
        }        
    }
    return builder;
}

Unit UnitFun::getWorker(Unit CommandCenter, std::list<int> &Miners, std::list<int> &Builders, UnitType supplyProviderType)
{
    Unit worker;

    if (!Builders.empty())
    {
        worker = UnitFun::getBuilder(Builders);        
        if (worker == NULL)
        {
            worker = UnitFun::getSCV(CommandCenter, Miners, Builders, supplyProviderType);
        }
    }
    else
    {
        worker = UnitFun::getSCV(CommandCenter, Miners, Builders, supplyProviderType);
    }
    return worker;
}

std::list<Unit> UnitFun::getUnitList(UnitType Type, Unitset allUnits, std::list<int>& deadUnits)
{
    //TODO: TaskManager is not assigning the SCV to build, error
    std::list<Unit> myUnits;
    for (auto& u : allUnits)
    {
        int type = u->getType();
        if (u->getType() == Type && auxFun::validUnit(u, deadUnits))
        {
            myUnits.push_back(u);
        }
    }
    return myUnits;
}
