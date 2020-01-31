#include "UnitFun.h"
#include <iostream>
#include "auxFun.h"

using namespace BWAPI;
using namespace Filter;

//Unit UnitFun::getUnitByID(std::list<Unit> Units, int ID)
//{   
//    //TODO: TaskManager is not assigning the SCV to build, error
//    Unit myUnit;
//
//    for (auto& u : Units)
//    {
//        if (u->getID() == ID)
//        {
//            myUnit = u;
//        }
//    }
//
//    return myUnit;
//}

Unit UnitFun::returnUnitByID( int ID)
{

    
    //TODO: TaskManager is not assigning the SCV to build, error
	
    Unit myUnit= Broodwar->getUnit(ID);

    /*for (auto& u : Units)
    {
        if (u->getID() == ID)
        {
            myUnit = u;
        }
    }*/

    return myUnit;
}

Unit UnitFun::getSCVfromCC(Unit CommandCenter, std::list<int> &Miners, std::list<int> &Builders, UnitType supplyProviderType)
{
    Unit worker = CommandCenter->getClosestUnit(GetType == supplyProviderType.whatBuilds().first &&
        (IsIdle || IsGatheringMinerals) && IsOwned);

    Miners.remove(worker->getID());//change duty to builder
    Builders.push_back(worker->getID());

    return worker;

}

Unit UnitFun::returnFirstAvaibleBuilder(std::list<int> Builders)
{
    Unit builder = NULL;
    for (auto& u : Builders)
    {
        Unit myBuilder = UnitFun::returnUnitByID( u);
        //Unit myBuilder = UnitFun::getUnitByID(workers, u);

        if (myBuilder != NULL)
        {
            if (!myBuilder->isConstructing() && myBuilder->getHitPoints() != 0)
            {
                builder = myBuilder;
                break;
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
        worker = UnitFun::returnFirstAvaibleBuilder(Builders);
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

std::list<Unit> UnitFun::getListofUnitType(UnitType myType, Unitset allUnits, std::list<int>& deadUnits)
{
    //TODO: TaskManager is not assigning the SCV to build, error
    std::list<Unit> myUnits;

    for (auto& u : allUnits)
    {
        int type = u->getType();
        if (u->getType() == myType && auxFun::validUnit(u, deadUnits))
        {
            myUnits.push_back(u);
        }
    }

    return myUnits;
}
