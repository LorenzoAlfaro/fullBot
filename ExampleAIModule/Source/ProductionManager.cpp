#include "ProductionManager.h"
#include "TaskFun.h"
#include "UnitFun.h"
#include "Buildmanager.h"
#include "auxFun.h"
#include "CommMngr.h"

void ProductionManager::productionManager(int minerals, int gas, int frameCount, list<array<int, 12>>& myTaskQueue, int myTaskCount, list<int> myDeadUnits, int myBuildingCount[], int myUnitCount[], int mySupplyLeft, int myMaxBuilding[],int myMaxUnit[])
{
    bool almostSupplyBlocked = false;
    int roomNeeded = auxFun::roomNeeded(myBuildingCount[0], myBuildingCount[2]);

    if (mySupplyLeft < roomNeeded) { //instead of 4,should be the max output of production at a given time

        almostSupplyBlocked = true;
    }

    if (!almostSupplyBlocked && !TaskFun::tasksWaitingResources(myTaskQueue))
    {
        if (myUnitCount[0] < myMaxUnit[0])
        {
            CommMngr::buildSCVs(UnitFun::getUnitList(UnitTypes::Terran_Command_Center, Broodwar->self()->getUnits(), myDeadUnits));//train SCVs until 50
        }

        CommMngr::trainMarines(UnitFun::getUnitList(UnitTypes::Terran_Barracks, Broodwar->self()->getUnits(), myDeadUnits));//pump marines

        if (minerals >= UnitTypes::Terran_Barracks.mineralPrice() && (myBuildingCount[2] < myMaxBuilding[2]) && (myUnitCount[0] > 10))
        {
            //Test isMyTaskInQueue
            if (!TaskFun::TaskQueued(myTaskQueue, (int)taskOwner::ProductionManager, (int)action::BuildBarrack))
            {
                TaskFun::CreateTask2(myTaskQueue, frameCount,0, (int)taskOwner::ProductionManager, (int)action::BuildBarrack, myTaskCount);
            }
        }
    }
    else
    {
        if (!TaskFun::TaskQueued(myTaskQueue, (int)taskOwner::ProductionManager, (int)action::BuildSupplyDepot) && Broodwar->self()->supplyTotal() != 400)
        {
            for (int i = 0; i < roomNeeded; i += 7)
            {
                TaskFun::CreateTask2(myTaskQueue, frameCount,0, (int)taskOwner::ProductionManager, (int)action::BuildSupplyDepot, myTaskCount);
            }
        }
    }
}