#include "TaskFun.h"
using namespace BWAPI;


std::array<int, 6>* TaskFun::findTaskAssignedToID(int ID, std::list<std::array<int, 6>> &Tasks)
{

    std::array<int, 6>* mytask; //pass the address of the array//initialize first to something

    for (auto& task : Tasks)
    {
        if (task[3] == ID)
        {
            mytask = &task;
        }
    }
    return mytask;
}

bool TaskFun::isMyTaskInQueue(std::list<std::array<int, 6>> &myTaskQueue, int taskOwner, int action)
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
                if (status != (int)taskStatus::Completed ||
                    status != (int)taskStatus::Cancelled)
                {
                    taskInQueue = true;
                }
            }
        }
    }

    return taskInQueue;
}

std::array<int, 2> TaskFun::resourceCost(std::array<int, 6> Task)
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

        break;
    }
    return price;
}

bool TaskFun::mineralsAvailable(std::array<int, 6> task, int CurrentMinerals)
{
    bool resourcesAvailabilty = false;

    std::array<int, 2> price = resourceCost(task);

    if (CurrentMinerals >= price[0])
    {
        resourcesAvailabilty = true;
    }
    return resourcesAvailabilty;
}

bool TaskFun::gasAvailable(std::array<int, 6> task, int CurrentGas)
{
    bool resourcesAvailabilty = false;

    std::array<int, 2> price = TaskFun::resourceCost(task);

    if (CurrentGas >= price[1])
    {
        resourcesAvailabilty = true;
    }
    return resourcesAvailabilty;
}

bool TaskFun::tasksWaitingResources(std::list<std::array<int, 6>> &myTaskQueue)
{
    //dont keep pumping units until we can start the building of depots or etc...
    bool waiting = false;
    for (auto& task : myTaskQueue)
    {
        int status = task[5];
        if (status == (int)taskStatus::waitingGas ||
            status == (int)taskStatus::Assigned ||  //make sure production doesn't steal minerals while scv travelling
            status == (int)taskStatus::waitingMin)
        {
            waiting = true;
        }
    }
    return waiting;
}

void TaskFun::taskStartedUpdate(std::list<std::array<int, 6>> &myTaskQueue, Unit Building)
{
    Unit builder = Building->getBuildUnit();
    //std::array<int, 6> currentTask = *TaskFun::findTaskAssignedToID(builder->getID(), myTaskQueue);//seems the addres is setup here

    //int point = TaskFun::findTaskAssignedToID(builder->getID(), myTaskQueue);

    //currentTask = *TaskFun::findTaskAssignedToID(builder->getID(), myTaskQueue);

    for (auto& task : myTaskQueue)
    {
        if (&task == TaskFun::findTaskAssignedToID(builder->getID(), myTaskQueue))
        {
            task[5] = (int)taskStatus::Started;
            task[3] = Building->getID(); //updated the task scvID to the building currently in progress
        }
    }

    //in case there is no real task returned
    //if (currentTask[0] != 0)
    //{
    //    currentTask[5] = (int)taskStatus::Started;
    //    currentTask[3] = Building->getID(); //updated the task scvID to the building currently in progress
    //}
}

void TaskFun::taskCompleted(std::list<std::array<int, 6>> &myTaskQueue, Unit Building)
{
    //Unit builder = Building->getBuildUnit();
    

    for (auto& task : myTaskQueue)
    {
        if (&task == TaskFun::findTaskAssignedToID(Building->getID(), myTaskQueue))
        {
            task[5] = (int)taskStatus::Completed;
            task[3] = Building->getID(); //updated the task scvID to the building currently in progress
        }
    }
    
}