#include "ProductionManager.h"
#include "TaskFun.h"
#include "UnitFun.h"
#include "Buildmanager.h"
#include "auxFun.h"
#include "CommMngr.h"
using namespace UnitTypes;
void ProductionManager::Manage(int minerals, int gas, int frameCount, list<array<int, 12>>& myTaskQueue, int& myTaskCount, list<int> myDeadUnits, int BarracksCount, int SCVCount, int myMaxBuilding[],int myMaxUnit[], bool almostSupplyBlocked, int roomNeeded)
{           
    if (!almostSupplyBlocked ) //build scvs, marines and barracs!
    {
        if (!TaskFun::tasksWaitingResources(myTaskQueue))
        {
            if (SCVCount < myMaxUnit[0])//train SCVs until 50
            {
                CommMngr::buildSCVs(UnitFun::getUnitList(Terran_Command_Center, Broodwar->self()->getUnits(), myDeadUnits));
            }

            CommMngr::trainMarines(UnitFun::getUnitList(Terran_Barracks, Broodwar->self()->getUnits(), myDeadUnits));//pump marines

            if (minerals >= Terran_Barracks.mineralPrice())//Have the money?
            {
                if (BarracksCount < myMaxBuilding[2])//Build until we have enough barracks
                {
                    if (SCVCount > 10) //wait until we have 10 SCVs
                    {
                        if (!TaskFun::TaskQueued(myTaskQueue, (int)taskOwner::ProductionManager, (int)action::BuildBarrack))//Don't create tasks if we already have a task in queue
                        {
                            TaskFun::CreateTask(myTaskQueue, frameCount, 0, (int)taskOwner::ProductionManager, (int)action::BuildBarrack, myTaskCount);
                        }
                    }
                }                                
            }
        }        
    }
    else //We need more Depots
    {
        if (!TaskFun::TaskQueued(myTaskQueue, (int)taskOwner::ProductionManager, (int)action::BuildSupplyDepot))
        {
            if (Broodwar->self()->supplyTotal() != 400)
            {
                for (int i = 0; i < roomNeeded; i += 7)//create more than one task as the production ramps up
                {
                    TaskFun::CreateTask(myTaskQueue, frameCount, 0, (int)taskOwner::ProductionManager, (int)action::BuildSupplyDepot, myTaskCount);
                }
            }            
        }
    }
}