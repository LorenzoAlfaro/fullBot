#include "TaskFun.h"
#include "BuildManager.h"
#include <BWAPI.h>
#include <fstream>
using namespace BWAPI;
using namespace std;

std::array<int, 12>* TaskFun::findTaskAssignedToUnit(int UnitID, std::list<std::array<int, 12>> &Tasks)
{
    bool found = false;
    std::array<int, 12>* mytask; //pass the address of the array//initialize first to something

    for (auto& task : Tasks)
    {
        if (task[(int)tsk::UID] == UnitID) //
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

std::array<int, 12>* TaskFun::findTaskAssignedToID(int TaskID, std::list<std::array<int, 12>>& Tasks)
{
    bool found = false;
    array<int, 12>* mytask; //pass the address of the array//initialize first to something

    for (auto& task : Tasks)
    {
        if (task[(int)tsk::ID] == TaskID) //
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

bool TaskFun::TaskQueued(list<array<int, 12>> &myTaskQueue, int taskOwner, int action)
{
    bool taskInQueue = false;
    for (auto& task : myTaskQueue)
    {
        if (task[(int)tsk::Owner] == taskOwner)
        {
            if (task[(int)tsk::Action] == action)//task action, build supplydepot, build barrack, etc
            {
                int status = task[(int)tsk::Status];
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

std::array<int, 3> TaskFun::resourceCost(const int action)
{
    array<int, 3> price = { 0,0 };
    switch (action)
    {
    case (int)action::BuildBarrack:

        price[0] = 150;
        price[1] = 0;
        price[2] = 50; //time needed in seconds

        break;
    case (int)action::BuildSupplyDepot:

        price[0] = 100;
        price[1] = 0;
        price[2] = 25;

        break;

    default:

        break;
    }
    return price;
}

bool TaskFun::mineralsAvailable(std::array<int, 12> task, int CurrentMinerals)
{    
    std::array<int, 3> price = resourceCost(task[(int)tsk::Action]);
    if (CurrentMinerals >= price[0])
    {        
        return true;
    }
    else
    {
        return false;
    }    
}

bool TaskFun::gasAvailable(std::array<int, 12> task, int CurrentGas)
{    
    std::array<int, 3> price = resourceCost(task[(int)tsk::Action]);
    if (CurrentGas >= price[1])
    {
        return true;
    }
    else
    {
        return false;
    }    
}

bool TaskFun::tasksWaitingResources(std::list<std::array<int, 12>> &myTaskQueue)
{
    //dont keep pumping units until we can start the building of depots or etc...
    bool waiting = false;
    for (auto& task : myTaskQueue)
    {
        int status = task[(int)tsk::Status];
        if (status == (int)taskStatus::waitingGas ||
            status == (int)taskStatus::PendingStart ||  //make sure production doesn't steal minerals while scv travelling
            status == (int)taskStatus::waitingMin)
        {
            waiting = true;
        }
    }
    return waiting;
}

void TaskFun::taskStartedUpdate(std::list<std::array<int, 12>> &myTaskQueue, Unit Building)
{
    Unit builder = Building->getBuildUnit();    
    for (auto& task : myTaskQueue)
    {
        if (&task == TaskFun::findTaskAssignedToID(builder->getID(), myTaskQueue))
        {
            task[(int)tsk::Status] = (int)taskStatus::Started;
            task[(int)tsk::UID] = Building->getID(); //updated the task scvID to the building currently in progress
        }
    }    
}

void TaskFun::taskCompleted(std::list<std::array<int, 12>> &myTaskQueue, Unit Building)
{    
    for (auto& task : myTaskQueue)
    {
        if (&task == TaskFun::findTaskAssignedToID(Building->getID(), myTaskQueue))
        {
            task[(int)tsk::Status] = (int)taskStatus::Completed;
            task[(int)tsk::UID] = Building->getID(); //updated the task scvID to the building currently in progress
        }
    }    
}

bool TaskFun::taskStatusUpdate(int ID, std::list<std::array<int, 12>> &Tasks, int newID, int newStatus)
{
    bool updatedTask = false;
    for (auto& task : Tasks)
    {
        if (task[(int)tsk::UID] == ID)
        {            
            task[(int)tsk::Status] = newStatus;
            task[(int)tsk::UID] = newID;
            updatedTask = true;
        }
    }
    return updatedTask;
}

void TaskFun::assessTask(array<int, 12>& newTask)
{
    //ok new task, what do you want?
    //do we have resources to complete your task?
    // what priority should I give you?
}

void TaskFun::CreateTask(list<array<int, 12>>& myTaskQueue, int timeStamp, int taskOwner, int action, int TaskCount)
{
    array<int, 12> newArray{ timeStamp,0,action,0,taskOwner,(int)taskStatus::Created, TaskCount };

    myTaskQueue.push_back(newArray);

    TaskCount = TaskCount + 1;
}

void TaskFun::CreateTask2(
    list<array<int, 12>>& myTaskQueue,
    int timeStamp,
    int delay,
    int taskOwner,
    int action,
    int& TaskCount)
{

    array<int, 3> price = TaskFun::resourceCost(action);
    array<int, 12> mytask;

    mytask[(int)tsk::TimeStamp] = timeStamp;
    mytask[(int)tsk::Delay] = delay;
    mytask[(int)tsk::Action] = action;
    mytask[(int)tsk::UID] = 0;
    mytask[(int)tsk::Owner] = taskOwner;
    mytask[(int)tsk::Status] = (int)taskStatus::Created;
    mytask[(int)tsk::ID] = TaskCount;
    mytask[(int)tsk::X] = 0;
    mytask[(int)tsk::Y] = 0;
    mytask[(int)tsk::Minerals] = price[0];
    mytask[(int)tsk::Gas] = price[1];
    mytask[(int)tsk::Duration] = price[2];


    //array<int, 12> newArray{
    //    timeStamp,
    //    delay,
    //    action,
    //    0,//not assigned to SCV yet, or building
    //    taskOwner,
    //    (int)taskStatus::Created,
    //    TaskCount,
    //    0,//X tile
    //    0,//y tile
    //    price[0],//mineral
    //    price[1],//gas
    //    price[2] };//time

    myTaskQueue.push_back(mytask);

    TaskCount += 1;

    TaskFun::logTaskUpdate(mytask);
}

void TaskFun::callBack(array<int, 12>& Task, int When, int Why)
{
    //do we have resources? assign, no? set callback time
    Task[(int)tsk::TimeStamp] = Broodwar->getFrameCount(); //reset timer
    Task[(int)tsk::Delay] = When; //frames, should be determined by the income rate, (miners working)
    Task[(int)tsk::Status] = Why;
}

void TaskFun::startTask(array<int, 12>& Task, Unit builder)
{
    //Unit builder = UnitFun::returnUnitByID(Broodwar->self()->getUnits(), Task[3]); //the task has an available worker assigned
    //int builderID = builder->getID();
    //TilePosition targetBuildLocation; //TODO: improve space allocation
    //targetBuildLocation = Broodwar->getBuildLocation(UnitTypes::Terran_Supply_Depot, builder->getTilePosition());
    //TODO: better target location method needed

    const TilePosition targetBuildLocation(Task[(int)tsk::X], Task[(int)tsk::Y]);

    switch (Task[(int)tsk::Action])
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

void TaskFun::logTaskUpdate(array<int, 12>& task)
{
    ofstream myfile;
    myfile.open("TaskRecord.txt", std::ofstream::app);
    myfile << task[(int)tsk::TimeStamp] << " "
        << task[(int)tsk::Delay] << " "
        << task[(int)tsk::Action] << " "
        << task[(int)tsk::UID] << " "
        << task[(int)tsk::Owner] << " "
        << task[(int)tsk::Status] << " "
        << task[(int)tsk::ID] << " "
        << task[(int)tsk::X] << " "
        << task[(int)tsk::Y] << " "
        << task[(int)tsk::Minerals] << " "
        << task[(int)tsk::Gas] << " "
        << task[(int)tsk::Duration] << " "
        << Broodwar->getFrameCount() << '\n';

    myfile.close();

}