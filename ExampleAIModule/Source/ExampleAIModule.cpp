#include "ExampleAIModule.h"
#include "UnitFun.h"
#include "BuildManager.h"
#include "CommMngr.h"
#include "auxFun.h"
#include <iostream>
#include <array>
#include "TaskFun.h"
using namespace BWAPI;
using namespace Filter;

int UnitCount[2] = {0,0}; //SCV,Marines, Medics, etc
int maxUnit[2] = {50,150}; //SCV,Marines, Medics, etc
int BuildingCount[3]={0,0,0}; //CC, supplydepots, barracks
int maxBuilding[3] = { 3,20,4 }; //CC, supplydepots, barracks
int deadSCVs = 0;
std::list<std::array<int,6>> taskQueue; //0=timeStamp,1=callbacktime,2=action,3=SCVID or build, 4=taskOwner,5=status


#pragma region UnitLists
std::list<Unit> commandCenters;
//std::list<Unit> workers; //dont try to maitain this
//std::list<Unit> barracks;
//std::list<Unit> marines;
std::list<Unit> supplyDepots;

std::list<int> Miners; //could be replace byd an array
std::list<int> Builders; //could be replaced by an array
std::list<int> deadUnits;
#pragma endregion

#pragma region uniqueUnits
UnitType supplyProviderType = BWAPI::UnitTypes::Terran_Supply_Depot;
#pragma endregion

#pragma region MaxCount
int supplyLimit = 0; // max supply 200
int supplyLeft = 0; //how many units can I support yet
int roomForProduction = 0; //how much before I need to build supply depots
int virtualBudget = 0;
//add an array parallel to the unit array that should be a regulator array, the target array
#pragma endregion

#pragma region BoolVariable
bool almostSupplyBlocked = false; //when true AI needs to build supplydepots pre emptively
bool displayStats = false;
#pragma endregion

//methods
#pragma region SupplyInfoMethods

void updateUnitCount(bool created, BWAPI::Unit unit)
{   
    UnitType myType = unit->getType();
    char result[100];
    strcpy(result, "new: ");
    strcat(result, myType.c_str());
    if (created)
    {
        if (displayStats)
        {
            //Broodwar->sendText(result); //strcat crashing broodwar
            result[0] = '\0'; //clear
        }
        if (myType == BWAPI::UnitTypes::Terran_Barracks)
        {
            BuildingCount[2] = BuildingCount[2] + 1;
            //barracks.push_back(unit);
        }
        if (myType == BWAPI::UnitTypes::Terran_Marine)
        {
            UnitCount[1] = UnitCount[1] + 1;

            //marines.push_back(unit); //how to remove the dead ones?
        }
        if (myType == BWAPI::UnitTypes::Terran_SCV)
        {
            UnitCount[0] = UnitCount[0] + 1;
            //workers.push_back(unit);
            Miners.push_back(unit->getID());//assign to mine
        }
        if (myType == BWAPI::UnitTypes::Terran_Supply_Depot)
        {
            BuildingCount[1] = BuildingCount[1] + 1;
            supplyDepots.push_back(unit);
        }
        if (myType == BWAPI::UnitTypes::Terran_Command_Center)
        {
            BuildingCount[0] = BuildingCount[0] + 1;
        }

        supplyLimit = auxFun::SupplyTotal(BuildingCount[0], BuildingCount[1]);

        supplyLeft = supplyLimit - auxFun::usedSupplyTotal(UnitCount[1], UnitCount[0]);

    }
    else
    {
        char result2[100];
        strcpy(result2, "dead: ");
        strcat(result2, myType.c_str());
        if (displayStats)
        {
            //Broodwar->sendText(result2);
        }

        if (myType == BWAPI::UnitTypes::Terran_Barracks)
        {
            BuildingCount[2] = BuildingCount[2] - 1;
        }
        if (myType == BWAPI::UnitTypes::Terran_Marine)
        {
            UnitCount[1] = UnitCount[1] - 1;
        }
        if (myType == BWAPI::UnitTypes::Terran_SCV)
        {
            UnitCount[0] = UnitCount[0] - 1;
        }
        if (myType == BWAPI::UnitTypes::Terran_Supply_Depot)
        {
            BuildingCount[1] = BuildingCount[1] - 1;
        }
        if (myType == BWAPI::UnitTypes::Terran_Command_Center)
        {
            BuildingCount[0] = BuildingCount[0] - 1;
        }
        supplyLimit = auxFun::SupplyTotal(BuildingCount[0], BuildingCount[1]);

        supplyLeft = supplyLimit - auxFun::usedSupplyTotal(UnitCount[1], UnitCount[0]);
    }
    
}
void displayInsights()
{
    int supplyLeft2 = Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed();
    Broodwar->drawTextScreen(100, 0, "FPS: %d", Broodwar->getFPS());
    //Broodwar->drawTextScreen(200, 20, "Average FPS: %f", Broodwar->getAverageFPS());
    Broodwar->drawTextScreen(100, 40, "Barrack Count: %d", BuildingCount[2]);
    Broodwar->drawTextScreen(100, 60, "Marine Count: %d", UnitCount[1]);
    Broodwar->drawTextScreen(100, 80, "SCV Count: %d", UnitCount[0]);
    Broodwar->drawTextScreen(100, 100, "Builders: %d", Builders.size());
    //Broodwar->drawTextScreen(200, 100, "Mouse Cursor: %d  %d", Broodwar->getMousePosition().x, Broodwar->getMousePosition().y);
    //Broodwar->drawTextScreen(200, 120, "Screen: %d  %d", Broodwar->getScreenPosition().x, Broodwar->getScreenPosition().y);
    Broodwar->drawTextScreen(100, 140, "Tasks: %d ", taskQueue.size());
    Broodwar->drawTextScreen(100, 110, "supply limit: %d ", supplyLeft);
    Broodwar->drawTextScreen(100, 120, "supply limit2: %d ", supplyLeft2);
    Broodwar->drawTextScreen(100, 130, "room for next round: %d ", auxFun::roomNeeded(BuildingCount[0], BuildingCount[2]));    
    Broodwar->drawTextScreen(100, 160, "Dead: %d ", deadUnits.size());
}


#pragma endregion

#pragma region UnitHandler

void supplyBlock(Unit CommandCenter)
{
    Error lastErr = Broodwar->getLastError();   
    BuildManager::createEventTag(CommandCenter, lastErr);            
    // If we are supply blocked and haven't tried constructing more recently
    if (lastErr == Errors::Insufficient_Supply )
    {  
        BuildManager::antiSpammingDepots(CommandCenter, Colors::Blue, 400, 2, Miners, Builders, supplyProviderType, supplyDepots);        
        
    } // closure: insufficient supply
}

void unitHandler(Unitset units)
{
    for (auto& u : units)
    {
        // Finally make the unit do some stuff!
        if (auxFun::validUnit(u, deadUnits))
        {
            // If the unit is a worker unit
            if (u->getType().isWorker())
            {               
                // if our worker is idle
                if (u->isIdle())
                {
                    if (Broodwar->self()->minerals() >= UnitTypes::Terran_Barracks.mineralPrice() && BuildingCount[2] < 4)
                    {
                        Unit supplyBuilder = u;

                        TilePosition buildPosition = Broodwar->getBuildLocation(BWAPI::UnitTypes::Terran_Barracks, u->getTilePosition());
                        u->build(UnitTypes::Terran_Barracks, buildPosition);                    
                        continue; //skip this unit
                    }
                    // Order workers carrying a resource to return them to the center,
                    // otherwise find a mineral patch to harvest.
                    if (u->isCarryingGas() || u->isCarryingMinerals())
                    {
                        u->returnCargo();
                    }
                    else if (!u->getPowerUp())  // The worker cannot harvest anything if it
                    {                             // is carrying a powerup such as a flag
                      // Harvest from the nearest mineral patch or gas refinery
                        if (!u->gather(u->getClosestUnit(IsMineralField || IsRefinery)))
                        {
                            // If the call fails, then print the last error message
                            Broodwar << Broodwar->getLastError() << std::endl;
                        }

                    } // closure: has no powerup
                } // closure: if idle

            }
            else if (u->getType().isResourceDepot()) // A resource depot is a Command Center, Nexus, or Hatchery
            {                
                almostSupplyBlocked = false;

                if (supplyLeft < 4) {

                    almostSupplyBlocked = true;

                    supplyBlock(u);

                }
                // Order the depot to construct more workers! But only when it is idle.

                if (UnitCount[0] < maxUnit[0])
                {
                    if (u->isIdle())
                    {
                        if (!almostSupplyBlocked)
                        {
                            bool failedTrainingSCV = u->train(u->getType().getRace().getWorker());

                            if (!failedTrainingSCV)
                            {
                                supplyBlock(u);
                            }
                        }
                    } // closure: failed to train idle unit
                }
            }
        }
    } // closure: unit iteratore
}
#pragma endregion

#pragma region TasksFunctions
void assessTask(std::array<int, 6>& newTask)
{
    //ok new task, what do you want?
    //do we have resources to complete your task?
    // what priority should I give you?
}

void CreateTask(std::list<std::array<int, 6>> &myTaskQueue, int timeStamp, int taskOwner, int action)
{
    //add the logic for adding a task to the queueu
    //task = timestamp / action / assign worker id / taskOwner / status /    
    std::array<int, 6> newArray{ timeStamp,0,action,0,taskOwner,(int)taskStatus::Created };
    myTaskQueue.push_back(newArray);
}

void startTask(std::array<int, 6> &Task)
{
    Unit builder = UnitFun::returnUnitByID(Broodwar->self()->getUnits(), Task[3]); //the task has an available worker assigned
    int builderID = builder->getID();
    TilePosition targetBuildLocation; //TODO: improve space allocation
    targetBuildLocation = Broodwar->getBuildLocation(BWAPI::UnitTypes::Terran_Supply_Depot, builder->getTilePosition());
    //TODO: better target location method needed
    
    switch (Task[2])
    {

    case (int)action::BuildSupplyDepot:

        BuildManager::buildBuilding(builder, BWAPI::UnitTypes::Terran_Supply_Depot, Colors::Blue, targetBuildLocation);
        Task[5] = (int)taskStatus::Assigned; //make sure nothings steals the minerals until it starts
        Task[0] = Broodwar->getFrameCount(); //reset timer
        Task[1] = 200; //check if an error happend
        break; //optional
    case (int)action::BuildBarrack:

        BuildManager::buildBuilding(builder, BWAPI::UnitTypes::Terran_Barracks, Colors::Green, targetBuildLocation);
        Task[5] = (int)taskStatus::Assigned;
        Task[0] = Broodwar->getFrameCount(); //reset timer
        Task[1] = 200; //check if an error happend)
        break; //optional

     // you can have any number of case statements.
    default: //Optional
        //statement(s);
        break;
    }
}

#pragma endregion

#pragma region MainMethods

void initialAssigment(Unitset units)
{
    for (auto& u : units)
    {
        if (auxFun::validUnit(u, deadUnits))
        {
            if (u->getType().isWorker())
            {
                //workers.push_back(u);
                //Miners.push_back(u->getID());
            }
            if (u->getType().isResourceDepot())
            {
                commandCenters.push_back(u);
            }
        }
    }
}

void productionManager()
{    
    almostSupplyBlocked = false;

    //Broodwar->self()->allUnitCount();
    //Broodwar->self()->completedUnitCount();
    //Broodwar->self()->supplyTotal();
    //Broodwar->self()->supplyUsed();

    if (supplyLeft < auxFun::roomNeeded(BuildingCount[0], BuildingCount[2])) { //instead of 4,should be the max output of production at a given time

        almostSupplyBlocked = true;
    }
    CommMngr::scvManager(Miners);//go mine for me minions!
    if (!almostSupplyBlocked && !TaskFun::tasksWaitingResources(taskQueue))
    {
        if (UnitCount[0] < maxUnit[0])
        {
            CommMngr::buildSCVs(commandCenters);//train SCVs until 50
        }
        
        CommMngr::trainMarines(UnitFun::getListofUnitType(BWAPI::UnitTypes::Terran_Barracks, Broodwar->self()->getUnits(),deadUnits));//pump marines

        if (Broodwar->self()->minerals() >= UnitTypes::Terran_Barracks.mineralPrice() && BuildingCount[2] < maxBuilding[2] && UnitCount[0] > 10)
        {            
            //Test isMyTaskInQueue
            if (!TaskFun::isMyTaskInQueue(taskQueue,(int)taskOwner::ProductionManager, (int)action::BuildBarrack))
            {
                CreateTask(taskQueue, Broodwar->getFrameCount(), (int)taskOwner::ProductionManager,(int)action::BuildBarrack);
            }            
        }
    }
    else
    {
        if (!TaskFun::isMyTaskInQueue(taskQueue, (int)taskOwner::ProductionManager, (int)action::BuildSupplyDepot))
        {
            CreateTask(taskQueue, Broodwar->getFrameCount(), (int)taskOwner::ProductionManager, (int)action::BuildSupplyDepot);
        }                
    }
}

void taskManager(std::list<std::array<int, 6>> &myTaskQueue)
{
    //if my task status is 0, not started, check timestamp
    //0 created, 1 reviewed but no resources to assign, 2 assigned with resources, 3 started, 4 completed, 5 cancel
    //production manager checks if there are tasks waiting for minerals before pumping units
    //assessTask(task) will determine wich task has high priority, for now if resources available just start them
    for (auto& task : myTaskQueue)
    {
        int taskStatus = task[5];
        if (Broodwar->getFrameCount() > (task[0] + task[1])
            && taskStatus != (int)taskStatus::Started
            && taskStatus != (int)taskStatus::Completed
            && taskStatus != (int)taskStatus::Cancelled
            && taskStatus != (int)taskStatus::Assigned)
        {
            if (TaskFun::mineralsAvailable(task, Broodwar->self()->minerals()) && TaskFun::gasAvailable(task, Broodwar->self()->gas())) //and location available
            {
                //assign SCV
                Unit SCV = UnitFun::getWorker(commandCenters.front(), Miners, Builders, supplyProviderType);
                int id = SCV->getID();
                task[3] = id; //assign the worker id to the task
                startTask(task);
            }
            else
            {
                //do we have resources? assign, no? set callback time
                task[0] = Broodwar->getFrameCount(); //reset timer
                task[1] = 200; //frames, should be determined by the income rate, (miners working)
                task[5] = (int)taskStatus::waitingMin;
            }
        }
        else if (Broodwar->getFrameCount() > (task[0] + task[1]) && taskStatus == (int)taskStatus::Assigned)
        {
            task[5] = (int)taskStatus::Cancelled;
            Broodwar->sendText("TaskMngr: Failed to start task id: %d : ", task[3]); //for some error
        }
    }
}

#pragma endregion

#pragma region MainEvents

void ExampleAIModule::onFrame()
{
    // Called once every game frame
    // Display the game frame rate as text in the upper left area of the screen
    if (displayStats)
    {
        displayInsights();
    }
    if (auxFun::validFrame())
    {
        taskManager(taskQueue);
        productionManager(); //eventually productionManager will be another task run by taskManager    
    }
}

void ExampleAIModule::onUnitCreate(BWAPI::Unit unit)
{

    if (IsOwned(unit))
    {
        updateUnitCount(true, unit);

        if (taskQueue.size() != 0 && !unit->canAttack()
            && unit->getType() != UnitTypes::Terran_SCV
            && unit->getType() != UnitTypes::Terran_Marine)
        {
            Unit builder = unit->getBuildUnit();
            int builderID = builder->getID();
            int BuildingID = unit->getID();
            //only for buildings
            if (TaskFun::taskStatusUpdate(builderID, taskQueue, BuildingID, (int)taskStatus::Started))
            {
                Broodwar->sendText("Started task id: %d : %s", unit->getID(), unit->getType().c_str());
            }
            else
            {
                Broodwar->sendText("Failed update to started task id: %d : %s", unit->getID(), unit->getType().c_str());
            }
        }
    }


        
    if (Broodwar->isReplay())
    {
        // if we are in a replay, then we will print out the build order of the structures
        if (unit->getType().isBuilding() && !unit->getPlayer()->isNeutral())
        {
            int seconds = Broodwar->getFrameCount() / 24;
            int minutes = seconds / 60;
            seconds %= 60;
           // Broodwar->sendText("%.2d:%.2d: %s creates a %s", minutes, seconds, unit->getPlayer()->getName().c_str(), unit->getType().c_str());
        }
    }
}

void ExampleAIModule::onUnitComplete(BWAPI::Unit unit)
{
    if (IsOwned(unit))
    {
        if (taskQueue.size() != 0 && !unit->canAttack()) //avoid running in buildings
        {
            if (TaskFun::taskStatusUpdate(unit->getID(), taskQueue, unit->getID(), (int)taskStatus::Completed))
            {
                Broodwar->sendText("Completed task id: %d : %s", unit->getID(), unit->getType().c_str());
            }
            else
            {
                Broodwar->sendText("failed update to completed task id: %d : %s", unit->getID(), unit->getType().c_str());
            }
        }
        //TODO: test remove  
      //taskQueue.remove(TaskFun::findTaskAssignedToID(unit->getID(),taskQueue));
      //updateUnitCount(true, unit);
    }
    
}

void ExampleAIModule::onUnitDestroy(BWAPI::Unit unit)
{
    updateUnitCount(false, unit);

    deadUnits.push_back(unit->getID());

}

void ExampleAIModule::onSendText(std::string text)
{ 
    Position myPos(Broodwar->getScreenPosition().x + Broodwar->getMousePosition().x, Broodwar->getScreenPosition().y + Broodwar->getMousePosition().y);
    if (text == "b")
    {
        Broodwar->sendText("power overwhelming");      
    }
    if (text == "stats")
    {
        displayStats = !displayStats;
    }
    if (text == "'")
    {
        CommMngr::attackUnits(UnitFun::getListofUnitType(BWAPI::UnitTypes::Terran_Marine, Broodwar->self()->getUnits(), deadUnits), myPos);
    }
    if (text == ";")
    {
        CommMngr::moveUnits(UnitFun::getListofUnitType(BWAPI::UnitTypes::Terran_Marine, Broodwar->self()->getUnits(), deadUnits), myPos);
    }
    if (text == "p")
    {       
        CommMngr::setRallyPoint(UnitFun::getListofUnitType(BWAPI::UnitTypes::Terran_Barracks, Broodwar->self()->getUnits(), deadUnits), myPos);
    }
    if (text == "q")
    {
        //Position myPos(Broodwar->getScreenPosition().x + Broodwar->getMousePosition().x, Broodwar->getScreenPosition().y + Broodwar->getMousePosition().y);
        for (auto& u : Miners)
        {            
            //Unit miner = UnitFun::getUnitByID(workers, u);
            Unit miner = UnitFun::returnUnitByID(Broodwar->self()->getUnits(), u);
            miner->move(myPos);
            miner->stop(true);
            miner->gather(miner->getClosestUnit(IsMineralField),true); //the everyframe logic messes up this
        }
    }
    if (text == "w")
    {
        Unit builder = UnitFun::returnFirstAvaibleBuilder(Builders);
        builder->move(myPos);
        builder->build(BWAPI::UnitTypes::Terran_Command_Center, builder->getTilePosition());
    }
    if (text == "e")
    {
        Unit builder = UnitFun::returnFirstAvaibleBuilder(Builders);
        builder->move(myPos);
        builder->build(BWAPI::UnitTypes::Terran_Engineering_Bay, builder->getTilePosition());
    }
    // Send the text to the game if it is not being processed.
    Broodwar->sendText("%s", text.c_str());

    


    // Make sure to use %s and pass the text as a parameter,
    // otherwise you may run into problems when you use the %(percent) character!

}

void ExampleAIModule::onStart()
{
    // Print the map name.
    // BWAPI returns std::string when retrieving a string, don't forget to add .c_str() when printing!
    //Broodwar << "The map is  " << Broodwar->mapName() << "!" << std::endl;

    // Enable the UserInput flag, which allows us to control the bot and type messages.
    Broodwar->enableFlag(Flag::UserInput);

    // Uncomment the following line and the bot will know about everything through the fog of war (cheat).
    //Broodwar->enableFlag(Flag::CompleteMapInformation);

    // Set the command optimization level so that common commands can be grouped
    // and reduce the bot's APM (Actions Per Minute).
    Broodwar->setCommandOptimizationLevel(2);

    // Check if this is a replay
    if (Broodwar->isReplay())
    {

        // Announce the players in the replay
        Broodwar << "The following players are in this replay:" << std::endl;

        // Iterate all the players in the game using a std:: iterator
        Playerset players = Broodwar->getPlayers();
        for (auto p : players)
        {
            // Only print the player if they are not an observer
            if (!p->isObserver())
                Broodwar << p->getName() << ", playing as " << p->getRace() << std::endl;
        }

    }
    else // if this is not a replay
    {
        initialAssigment(Broodwar->self()->getUnits()); //assign my firt units, 4 workers, CC
        // Retrieve you and your enemy's races. enemy() will just return the first enemy.
        // If you wish to deal with multiple enemies then you must use enemies().
        if (Broodwar->enemy()) // First make sure there is an enemy
            Broodwar << "The matchup is " << Broodwar->self()->getRace() << " vs " << Broodwar->enemy()->getRace() << std::endl;
    }

}

#pragma endregion

#pragma region OtherEvents

void ExampleAIModule::onReceiveText(BWAPI::Player player, std::string text)
{
    // Parse the received text
    Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
}

void ExampleAIModule::onUnitDiscover(BWAPI::Unit unit)
{
}

void ExampleAIModule::onUnitEvade(BWAPI::Unit unit)
{
}

void ExampleAIModule::onUnitShow(BWAPI::Unit unit)
{
}

void ExampleAIModule::onUnitHide(BWAPI::Unit unit)
{
}

void ExampleAIModule::onEnd(bool isWinner)
{
    // Called when the game ends
    if (isWinner)
    {
        // Log your win here!
    }
}

#pragma endregion

#pragma region NotUsedEvents

//not useful ... yet
void ExampleAIModule::onSaveGame(std::string gameName)
{
    Broodwar << "The game was saved to \"" << gameName << "\"" << std::endl;
}

void ExampleAIModule::onUnitRenegade(BWAPI::Unit unit)
{
}

void ExampleAIModule::onUnitMorph(BWAPI::Unit unit)
{
    if (Broodwar->isReplay())
    {
        // if we are in a replay, then we will print out the build order of the structures
        if (unit->getType().isBuilding() && !unit->getPlayer()->isNeutral())
        {
            int seconds = Broodwar->getFrameCount() / 24;
            int minutes = seconds / 60;
            seconds %= 60;
            Broodwar->sendText("%.2d:%.2d: %s morphs a %s", minutes, seconds, unit->getPlayer()->getName().c_str(), unit->getType().c_str());
        }
    }
}

void ExampleAIModule::onPlayerLeft(BWAPI::Player player)
{
    // Interact verbally with the other players in the game by
    // announcing that the other player has left.
    Broodwar->sendText("Goodbye %s!", player->getName().c_str());
}

void ExampleAIModule::onNukeDetect(BWAPI::Position target)
{

    // Check if the target is a valid position
    if (target)
    {
        // if so, print the location of the nuclear strike target
        Broodwar << "Nuclear Launch Detected at " << target << std::endl;
    }
    else
    {
        // Otherwise, ask other players where the nuke is!
        Broodwar->sendText("Where's the nuke?");
    }

    // You can also retrieve all the nuclear missile targets using Broodwar->getNukeDots()!
}


#pragma endregion


