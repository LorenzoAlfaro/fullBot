#pragma once
#include <array>
#include <BWAPI.h>
using namespace std;
using namespace BWAPI;

class TaskFun
{
public:
	static array<int, 12>* findTaskAssignedToUnit(int ID, list<array<int, 12>>& Tasks);
    static array<int, 12>* findTaskAssignedToID(int TaskID, list<array<int, 12>>& Tasks);
	static bool TaskQueued(list<array<int, 12>>& myTaskQueue, int taskOwner, int action);
    static array<int, 3> TaskFun::resourceCost(const int action);
    static bool mineralsAvailable(array<int, 12> task, int CurrentMinerals);
    static bool gasAvailable(array<int, 12> task, int CurrentMinerals);
    static bool tasksWaitingResources(list<array<int, 12>>& myTaskQueue);
    static void taskStartedUpdate(list<array<int, 12>>& myTaskQueue, Unit Building);
    static void taskCompleted(list<array<int, 12>>& myTaskQueue, Unit Building);
    static bool taskStatusUpdate(int ID, list<array<int, 12>>& Tasks, int newID, int newStatus);  
    static void assessTask(array<int, 12>& newTask);    
    static void CreateTask(list<array<int, 12>>& myTaskQueue,int timeStamp,int delay,int taskOwner,int action, int& TaskCount);
    static void callBack(array<int, 12>& Task, int When, int Why);
    static void startTask(array<int, 12>& Task, Unit builder);
    static void logTaskUpdate(array<int, 12>& task);
};

enum class taskStatus {
    Created,
    Reviewed,
    PendingStart,  // has an SCV ID assign to build, or the building ID to research upgrade 
    waitingMin,  //the task is waiting for minerals
    waitingGas, //the task is waiting for gas
    waitingClear, // the task is waiting for the building place to clear, or the building to stop to add-on
    Started,
    Completed,
    Paused,
    Cancelled
};

enum class action {
    BuildSupplyDepot,
    BuildBarrack,
    BuildExtractor,
    BuildEngineeringBay,
    researchStim
};

enum class taskOwner {
    ProductionManager,
    StratManager,
    Commander
};

enum class tsk
{
    TimeStamp,
    Delay,
    Action,
    UID,
    Owner,
    Status,
    ID,
    X,
    Y,
    Minerals,
    Gas,
    Duration
};


