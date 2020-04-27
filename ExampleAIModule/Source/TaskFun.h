#pragma once
#include <array>
#include <BWAPI.h>
using namespace std;
using namespace BWAPI;

class TaskFun
{
public:
	static std::array<int, 7>* findTaskAssignedToUnit(int ID, std::list<std::array<int, 7>>& Tasks);
    static std::array<int, 7>* findTaskAssignedToID(int TaskID, std::list<std::array<int, 7>>& Tasks);
	static bool TaskQueued(std::list<std::array<int, 7>>& myTaskQueue, int taskOwner, int action);
    static std::array<int, 2> resourceCost(std::array<int, 7> Task);
    static bool mineralsAvailable(std::array<int, 7> task, int CurrentMinerals);
    static bool gasAvailable(std::array<int, 7> task, int CurrentMinerals);
    static bool tasksWaitingResources(std::list<std::array<int, 7>>& myTaskQueue);
    static void taskStartedUpdate(std::list<std::array<int, 7>>& myTaskQueue, BWAPI::Unit Building);
    static void taskCompleted(std::list<std::array<int, 7>>& myTaskQueue, BWAPI::Unit Building);
    static bool taskStatusUpdate(int ID, std::list<std::array<int, 7>>& Tasks, int newID, int newStatus);  
    static void assessTask(array<int, 7>& newTask);
    static void CreateTask(list<array<int, 7>>& myTaskQueue, int timeStamp, int taskOwner, int action, int TaskCount);
    static void callBack(array<int, 7>& Task, int When, int Why);
    static void startTask(array<int, 7>& Task, Unit builder, TilePosition targetBuildLocation);
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


