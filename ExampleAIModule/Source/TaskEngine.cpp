#include "TaskEngine.h"
#include "TaskFun.h"
#include "UnitFun.h"
#include "Buildmanager.h"


void TaskEngine::taskManager(list<array<int, 12>>& myTaskQueue, int frameCount, int minerals, int gas, Unit CommandCenter, list<int>& Miners, list<int>& Builders)
{
    //if my task status is 0, not started, check timestamp
    //0 created, 1 reviewed but no resources to assign, 2 assigned with resources, 3 started, 4 completed, 5 cancel
    //production manager checks if there are tasks waiting for minerals before pumping units
    //assessTask(task) will determine wich task has high priority, for now if resources available just start them
    for (auto& task : myTaskQueue)
    {
        int taskStatus = task[(int)tsk::Status];
        if (frameCount > (task[(int)tsk::TimeStamp] + task[(int)tsk::Delay])
            && taskStatus != (int)taskStatus::Started
            && taskStatus != (int)taskStatus::Completed
            && taskStatus != (int)taskStatus::Cancelled
            && taskStatus != (int)taskStatus::PendingStart)
        {
            if (TaskFun::mineralsAvailable(task, minerals) &&
                TaskFun::gasAvailable(task, gas)) //and location available
            {
                //assign SCV
                Unit SCV = UnitFun::getWorker(CommandCenter, Miners, Builders);
                int id = SCV->getID();

                
                //pass the worker id to the task
                task[(int)tsk::UID] = id;

                if (task[(int)tsk::X] == 0 && task[(int)tsk::Y] == 0)
                {
                    const TilePosition targetBuildLocation = BuildManager::returnBuildPosition(task[(int)tsk::Action], SCV, 20);

                    task[(int)tsk::X] = targetBuildLocation.x;
                    task[(int)tsk::Y] = targetBuildLocation.y;

                }

                TaskFun::startTask(task, SCV);
            }
            else
            {
                //do we have resources? assign, no? set callback time
                TaskFun::callBack(task, 200, (int)taskStatus::waitingMin);
            }
        }
        else if (frameCount > (task[(int)tsk::TimeStamp] + task[(int)tsk::Delay]) && taskStatus == (int)taskStatus::PendingStart)
        {
            task[(int)tsk::Status] = (int)taskStatus::Cancelled;
            Broodwar->sendText("TaskMngr: Failed to start task id: %d : ", task[(int)tsk::ID]); //for some error            
        }
    }
    
}


