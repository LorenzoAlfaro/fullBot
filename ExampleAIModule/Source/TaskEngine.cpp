#include "TaskEngine.h"

void assessTask(array<int, 12>& myTask, int frameCount, int minerals, int gas, Unit CommandCenter, list<int>& Miners, list<int>& Builders)
{
    int taskStatus = myTask[(int)tsk::Status];
    int callBackTime = myTask[(int)tsk::TimeStamp] + myTask[(int)tsk::Delay];

    if (taskStatus == (int)taskStatus::Created || taskStatus == (int)taskStatus::waitingMin)
    {
        if (frameCount > callBackTime)
        {
            if (TaskFun::mineralsAvailable(myTask, minerals) && TaskFun::gasAvailable(myTask, gas)) //and location available
            {
                //assign SCV, instead of SCV just get the ID directly, remove BWAPI 
                Unit SCV = UnitFun::getWorker(CommandCenter, Miners, Builders);
                //pass the worker id to the task
                myTask[(int)tsk::UID] = SCV->getID();

                if (myTask[(int)tsk::X] == 0 && myTask[(int)tsk::Y] == 0)
                {
                    const TilePosition targetBuildLocation = BuildManager::returnBuildPosition(myTask[(int)tsk::Action], SCV, 20);
                    myTask[(int)tsk::X] = targetBuildLocation.x;
                    myTask[(int)tsk::Y] = targetBuildLocation.y;
                }
                TaskFun::startTask(myTask, SCV);//set to PendingStart, onUnitCreate set to Start, onUnitComplete set to Complete
            }
            else
            {
                //do we have resources? assign, no? set callback time
                TaskFun::callBack(myTask, 200, (int)taskStatus::waitingMin);
            }
        }
    }
    else if (taskStatus == (int)taskStatus::PendingStart)
    {
        if (frameCount > callBackTime)
        {
            myTask[(int)tsk::Status] = (int)taskStatus::Cancelled;
            Broodwar->sendText("TaskMngr: Failed to start task id: %d : ", myTask[(int)tsk::ID]); //remove the log to broodwar, send a error code
        }
    }
    else
    {

    }
}


void TaskEngine::taskManager(list<array<int, 12>>& myTaskQueue, int frameCount, int minerals, int gas, Unit CommandCenter, list<int>& Miners, list<int>& Builders)
{
    //TODO: eliminate the need to pass a commandcenter unit
    //if my task status is 0, not started, check timestamp
    //0 created, 1 reviewed but no resources to assign, 2 assigned with resources, 3 started, 4 completed, 5 cancel
    //production manager checks if there are tasks waiting for minerals before pumping units
    //assessTask(task) will determine wich task has high priority, for now if resources available just start them
    for (auto& task : myTaskQueue)
    {
        //assessTask(task,frameCount,minerals,gas,CommandCenter,Miners,Builders);
        int taskStatus = task[(int)tsk::Status];
        int callBackTime = task[(int)tsk::TimeStamp] + task[(int)tsk::Delay];

        if (taskStatus == (int)taskStatus::Created || taskStatus == (int)taskStatus::waitingMin)
        {
            if (frameCount > callBackTime)
            {
                if (TaskFun::mineralsAvailable(task, minerals) && TaskFun::gasAvailable(task, gas)) //and location available
                {
                    //assign SCV, instead of SCV just get the ID directly, remove BWAPI 
                    Unit SCV = UnitFun::getWorker(CommandCenter, Miners, Builders);
                    //pass the worker id to the task
                    task[(int)tsk::UID] = SCV->getID();

                    if (task[(int)tsk::X] == 0 && task[(int)tsk::Y] == 0)
                    {
                        const TilePosition targetBuildLocation = BuildManager::returnBuildPosition(task[(int)tsk::Action], SCV, 20);
                        task[(int)tsk::X] = targetBuildLocation.x;
                        task[(int)tsk::Y] = targetBuildLocation.y;
                    }
                    TaskFun::startTask(task, SCV);//set to PendingStart, onUnitCreate set to Start, onUnitComplete set to Complete
                }
                else
                {
                    //do we have resources? assign, no? set callback time
                    TaskFun::callBack(task, 200, (int)taskStatus::waitingMin);
                }
            }
        }
        else if (taskStatus == (int)taskStatus::PendingStart)
        {
            if (frameCount > callBackTime)
            {
                task[(int)tsk::Status] = (int)taskStatus::Cancelled;
                Broodwar->sendText("TaskMngr: Failed to start task id: %d : ", task[(int)tsk::ID]); //for some error 
            }
        }
        else
        {
                
        }        
    }   
}



