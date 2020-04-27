#include "TaskFun.h"
#include "BuildManager.h"
#include <BWAPI.h>
using namespace BWAPI;
using namespace std;

std::array<int, 7>* TaskFun::findTaskAssignedToUnit(int UnitID, std::list<std::array<int, 7>> &Tasks)
{
    bool found = false;
    std::array<int, 7>* mytask; //pass the address of the array//initialize first to something

    for (auto& task : Tasks)
    {
        if (task[3] == UnitID) //
        {
            mytask = &task;
            found = true;
        }
    }

    if (found)
    {
        return mytask;
    }
    else
    {
        return nullptr;
    }
    
}

std::array<int, 7>* TaskFun::findTaskAssignedToID(int TaskID, std::list<std::array<int, 7>>& Tasks)
{
    bool found = false;
    std::array<int, 7>* mytask; //pass the address of the array//initialize first to something

    for (auto& task : Tasks)
    {
        if (task[6] == TaskID) //
        {
            mytask = &task;
        }
    }

    if (found)
    {
        return mytask;
    }
    else
    {
        return nullptr;
    }
}

bool TaskFun::TaskQueued(std::list<std::array<int, 7>> &myTaskQueue, int taskOwner, int action)
{
    bool taskInQueue = false;
    for (auto& task : myTaskQueue)
    {
        if (task[4] == taskOwner)
        {
            if (task[2] == action)//task action, build supplydepot, build barrack, etc
            {
                int status = task[5];
                //found a task that belongs to me
                if (status != (int)taskStatus::Completed &&
                    status != (int)taskStatus::Cancelled)
                {
                    taskInQueue = true;
                }
            }
        }
    }

    return taskInQueue;
}

std::array<int, 2> TaskFun::resourceCost(std::array<int, 7> Task)
{
    std::array<int, 2> price;
    switch (Task[2])
    {
    case (int)action::BuildBarrack:

        price[0] = 150;
        price[1] = 0;

        break;
    case (int)action::BuildSupplyDepot:

        price[0] = 100;
        price[1] = 0;

        break;

    default:
        price[0] = 0;
        price[1] = 0;
        break;
    }
    return price;
}

bool TaskFun::mineralsAvailable(std::array<int, 7> task, int CurrentMinerals)
{    
    std::array<int, 2> price = resourceCost(task);
    if (CurrentMinerals >= price[0])
    {        
        return true;
    }
    else
    {
        return false;
    }    
}

bool TaskFun::gasAvailable(std::array<int, 7> task, int CurrentGas)
{    
    std::array<int, 2> price = TaskFun::resourceCost(task);
    if (CurrentGas >= price[1])
    {
        return true;
    }
    else
    {
        return false;
    }    
}

bool TaskFun::tasksWaitingResources(std::list<std::array<int, 7>> &myTaskQueue)
{
    //dont keep pumping units until we can start the building of depots or etc...
    bool waiting = false;
    for (auto& task : myTaskQueue)
    {
        int status = task[5];
        if (status == (int)taskStatus::waitingGas ||
            status == (int)taskStatus::PendingStart ||  //make sure production doesn't steal minerals while scv travelling
            status == (int)taskStatus::waitingMin)
        {
            waiting = true;
        }
    }
    return waiting;
}

void TaskFun::taskStartedUpdate(std::list<std::array<int, 7>> &myTaskQueue, Unit Building)
{
    Unit builder = Building->getBuildUnit();    
    for (auto& task : myTaskQueue)
    {
        if (&task == TaskFun::findTaskAssignedToID(builder->getID(), myTaskQueue))
        {
            task[5] = (int)taskStatus::Started;
            task[3] = Building->getID(); //updated the task scvID to the building currently in progress
        }
    }    
}

void TaskFun::taskCompleted(std::list<std::array<int, 7>> &myTaskQueue, Unit Building)
{    
    for (auto& task : myTaskQueue)
    {
        if (&task == TaskFun::findTaskAssignedToID(Building->getID(), myTaskQueue))
        {
            task[5] = (int)taskStatus::Completed;
            task[3] = Building->getID(); //updated the task scvID to the building currently in progress
        }
    }    
}

bool TaskFun::taskStatusUpdate(int ID, std::list<std::array<int, 7>> &Tasks, int newID, int newStatus)
{
    bool updatedTask = false;
    for (auto& task : Tasks)
    {
        if (task[3] == ID)
        {            
            task[5] = newStatus;            
            task[3] = newID;
            updatedTask = true;
        }
    }
    return updatedTask;
}

void TaskFun::assessTask(array<int, 7>& newTask)
{
    //ok new task, what do you want?
    //do we have resources to complete your task?
    // what priority should I give you?
}

void TaskFun::CreateTask(list<array<int, 7>>& myTaskQueue, int timeStamp, int taskOwner, int action, int TaskCount)
{
    array<int, 7> newArray{ timeStamp,0,action,0,taskOwner,(int)taskStatus::Created, TaskCount };

    myTaskQueue.push_back(newArray);

    TaskCount = TaskCount + 1;
}

void TaskFun::callBack(array<int, 7>& Task, int When, int Why)
{
    //do we have resources? assign, no? set callback time
    Task[0] = Broodwar->getFrameCount(); //reset timer
    Task[1] = When; //frames, should be determined by the income rate, (miners working)
    Task[5] = Why;
}

void TaskFun::startTask(array<int, 7>& Task, Unit builder, TilePosition targetBuildLocation)
{
    //Unit builder = UnitFun::returnUnitByID(Broodwar->self()->getUnits(), Task[3]); //the task has an available worker assigned
    //int builderID = builder->getID();
    //TilePosition targetBuildLocation; //TODO: improve space allocation
    //targetBuildLocation = Broodwar->getBuildLocation(UnitTypes::Terran_Supply_Depot, builder->getTilePosition());
    //TODO: better target location method needed

    switch (Task[2])
    {

    case (int)action::BuildSupplyDepot:

        BuildManager::buildBuilding(builder, UnitTypes::Terran_Supply_Depot, Colors::Blue, targetBuildLocation);

        break; //optional
    case (int)action::BuildBarrack:

        BuildManager::buildBuilding(builder, UnitTypes::Terran_Barracks, Colors::Green, targetBuildLocation);

        break; //optional

     // you can have any number of case statements.
    default: //Optional
        //statement(s);
        break;
    }

    callBack(Task, 200, (int)taskStatus::PendingStart); //things can happen during travel time
}