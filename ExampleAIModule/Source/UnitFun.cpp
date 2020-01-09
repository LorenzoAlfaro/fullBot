#include "UnitFun.h"
#include <iostream>

using namespace BWAPI;
using namespace Filter;

Unit UnitFun::getUnitByID(std::list<Unit> Units, int ID)
{    
    Unit myUnit;

    for (auto& u : Units)
    {
        if (u->getID() == ID)
        {
            myUnit = u;
        }
    }

    return myUnit;
}

Unit UnitFun::getSCVfromCC(Unit CommandCenter, std::list<int> Miners, std::list<int> Builders, UnitType supplyProviderType)
{
    Unit worker = CommandCenter->getClosestUnit(GetType == supplyProviderType.whatBuilds().first &&
        (IsIdle || IsGatheringMinerals) && IsOwned);

    Miners.remove(worker->getID());//change duty to builder
    Builders.push_back(worker->getID());

    return worker;

}

Unit UnitFun::returnFirstAvaibleBuilder(std::list<int> Builders, std::list<Unit> workers)
{
    Unit builder = NULL;
    for (auto& u : Builders)
    {
        Unit myBuilder = UnitFun::getUnitByID(workers, u);
        if (!myBuilder->isConstructing())
        {
            builder = myBuilder;
        }
    }
    return builder;
}

Unit UnitFun::getWorker(Unit CommandCenter, std::list<int> Miners, std::list<int> Builders, UnitType supplyProviderType, std::list<Unit> workers)
{
    Unit worker;

    if (Builders.size() != 0)
    {
        worker = UnitFun::returnFirstAvaibleBuilder(Builders, workers);
        //worker = getUnitByID(workers,Builders.front()); //needs improvement
        if (worker == NULL)
        {
            worker = UnitFun::getSCVfromCC(CommandCenter, Miners, Builders, supplyProviderType);
        }
    }
    else
    {
        worker = UnitFun::getSCVfromCC(CommandCenter, Miners, Builders, supplyProviderType);
    }
    return worker;
}
