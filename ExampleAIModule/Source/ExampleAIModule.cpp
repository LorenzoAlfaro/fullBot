#include "ExampleAIModule.h"
#include "UnitFun.h"
#include "BuildManager.h"
#include "CommMngr.h"
#include "auxFun.h"
#include <iostream>
#include <array>

using namespace BWAPI;
using namespace Filter;

int UnitCount[2] = {0,0}; //SCV,Marines, Medics, etc
int maxUnit[2] = {50,150}; //SCV,Marines, Medics, etc
int BuildingCount[3]={0,0,0}; //CC, supplydepots, barracks
int maxBuilding[3] = { 3,20,4 }; //CC, supplydepots, barracks

#pragma region UnitLists
std::list<Unit> commandCenters;
std::list<Unit> workers;
std::list<Unit> barracks;
std::list<Unit> marines;
std::list<Unit> supplyDepots;

std::list<int> Miners; //could be replace byd an array
std::list<int> Builders; //could be replaced by an array
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
    if (IsOwned(unit))
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
                barracks.push_back(unit);
            }
            if (myType == BWAPI::UnitTypes::Terran_Marine)
            {
                UnitCount[1] = UnitCount[1] + 1;

                marines.push_back(unit); //how to remove the dead ones?
            }
            if (myType == BWAPI::UnitTypes::Terran_SCV)
            {
                UnitCount[0] = UnitCount[0] + 1;
                workers.push_back(unit);
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
}
void displayInsights()
{
    Broodwar->drawTextScreen(200, 0, "FPS: %d", Broodwar->getFPS());
    //Broodwar->drawTextScreen(200, 20, "Average FPS: %f", Broodwar->getAverageFPS());
    Broodwar->drawTextScreen(200, 40, "Barrack Count: %d", BuildingCount[2]);
    Broodwar->drawTextScreen(200, 60, "Marine Count: %d", UnitCount[1]);
    Broodwar->drawTextScreen(200, 80, "SCV Count: %d", UnitCount[0]);
    Broodwar->drawTextScreen(200, 100, "Builders: %d", Builders.size());
    //Broodwar->drawTextScreen(200, 100, "Mouse Cursor: %d  %d", Broodwar->getMousePosition().x, Broodwar->getMousePosition().y);
    //Broodwar->drawTextScreen(200, 120, "Screen: %d  %d", Broodwar->getScreenPosition().x, Broodwar->getScreenPosition().y);
    Broodwar->drawTextScreen(200, 140, "supply limit: %d ", supplyLeft);
    Broodwar->drawTextScreen(200, 120, "room for next round: %d ", auxFun::roomNeeded(BuildingCount[0], BuildingCount[2]));
}

#pragma endregion

std::list<std::array<int,6>> taskQueue;

#pragma region UnitHandler

void antiSpammingBuilding(Unit commandCenter, UnitType Building, Color color, int offSet)
{  
    static int lastChecked = 0;

    if (lastChecked + offSet < Broodwar->getFrameCount() &&
        Broodwar->self()->incompleteUnitCount(Building) == 0)
    {
        lastChecked = Broodwar->getFrameCount();    //is int big enough to hold this?                  
        BuildManager::buildBuilding(UnitFun::getWorker(commandCenter, Miners, Builders, supplyProviderType,workers), Building, color, commandCenter->getTilePosition());
    } 
}

void antiSpammingDepots(Unit commandCenter, Color color, int offSet, int numberNeeded)
{
    static int lastChecked = 0;

    if (lastChecked + offSet < Broodwar->getFrameCount() &&
        Broodwar->self()->incompleteUnitCount(BWAPI::UnitTypes::Terran_Supply_Depot) != numberNeeded)
    {
        lastChecked = Broodwar->getFrameCount();

        for (int i =0;i < numberNeeded; i++)
        {
            TilePosition targetBuildLocation;
            if (supplyDepots.size() != 0)
            {
               targetBuildLocation = Broodwar->getBuildLocation(BWAPI::UnitTypes::Terran_Supply_Depot, supplyDepots.front()->getTilePosition());
            }
            else
            {
               targetBuildLocation = Broodwar->getBuildLocation(BWAPI::UnitTypes::Terran_Supply_Depot, commandCenter->getTilePosition());
            }

            //buildBuilding(getWorker(commandCenter), BWAPI::UnitTypes::Terran_Supply_Depot, color);
            BuildManager::buildBuilding(UnitFun::getWorker(commandCenter, Miners, Builders, supplyProviderType, workers), BWAPI::UnitTypes::Terran_Supply_Depot, color, targetBuildLocation);
        }

        
    }
}

void antiSpammingBarracks(Unit commandCenter, Color color, int offSet, int numberNeeded)
{
    static int lastChecked = 0;

    if (lastChecked + offSet < Broodwar->getFrameCount() &&
        Broodwar->self()->incompleteUnitCount(BWAPI::UnitTypes::Terran_Barracks) != numberNeeded)
    {
        lastChecked = Broodwar->getFrameCount();
        for (int i = 0; i < numberNeeded; i++)
        {
            TilePosition targetBuildLocation;
            if (barracks.size() != 0)
            {
                targetBuildLocation = Broodwar->getBuildLocation(BWAPI::UnitTypes::Terran_Supply_Depot, barracks.front()->getTilePosition());
            }
            else
            {
                targetBuildLocation = Broodwar->getBuildLocation(BWAPI::UnitTypes::Terran_Supply_Depot, workers.front()->getTilePosition());
            }
            BuildManager::buildBuilding(UnitFun::getWorker(commandCenter, Miners, Builders, supplyProviderType, workers), BWAPI::UnitTypes::Terran_Barracks, color, targetBuildLocation);
        }
        
    }
}

void almostSupplyBlock(Unit CommandCenter)
{                     
    antiSpammingDepots(CommandCenter, Colors::Blue, 400,2);    
}

void supplyBlock(Unit CommandCenter)
{
    Error lastErr = Broodwar->getLastError();
    

    BuildManager::createEventTag(CommandCenter, lastErr);
            
    // If we are supply blocked and haven't tried constructing more recently
    if (lastErr == Errors::Insufficient_Supply )
    {        
        almostSupplyBlock(CommandCenter);
        
    } // closure: insufficient supply
}

void unitHandler(Unitset units)
{
    for (auto& u : units)
    {
        // Finally make the unit do some stuff!
        if (auxFun::validUnit(u))
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

                    almostSupplyBlock(u);

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

void CreateTask(std::list<std::array<int, 6>> &myTaskQueue,int timeStamp,int callbackTime, int action, int workerId, int squadronID, int status)
{
    //add the logic for adding a task to the queueu
    //task = timestamp / action / assign worker id / assign squadron / status /
    
    std::array<int, 6> newArray{timeStamp,callbackTime,action,workerId,squadronID,status};   
    myTaskQueue.push_back(newArray);
    
}

void taskManager(std::list<std::array<int, 6>>& myTaskQueue)
{
    for (auto& task : myTaskQueue)
    {      
        if (task[4]==0)
        {
            //if my task status is 0, not started, check timestamp
            //0 created, 1 reviewed but no resources to assign, 2 assigned with resources, 3 started, 4 completed, 5 cancel
            assessTask(task);
        }        
        else if (Broodwar->getFrameCount() > task[0] + task[1])//timeStamp + callbackTime
        {
            //dont bother me until 5mins (offset have passed)
            //ok what do you want now?


        }
    }
}

void assessTask(std::array<int, 6> newTask)
{
    //ok new task, what do you want?
    //do we have resources to complete your task?
    // what priority should I give you?
}

#pragma endregion

#pragma region MainMethods

void initialAssigment(Unitset units)
{
    for (auto& u : units)
    {
        if (auxFun::validUnit(u))
        {
            if (u->getType().isWorker())
            {
                workers.push_back(u);
                Miners.push_back(u->getID());
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

    if (supplyLeft < auxFun::roomNeeded(BuildingCount[0], BuildingCount[2])) { //instead of 4,should be the max output of production at a given time

        almostSupplyBlocked = true;
    }
    CommMngr::scvManager(Miners,workers);
    if (!almostSupplyBlocked)
    {
        if (UnitCount[0] < maxUnit[0])
        {
            CommMngr::buildSCVs(commandCenters);
        }
        
        CommMngr::trainMarines(barracks);

        if (Broodwar->self()->minerals() >= UnitTypes::Terran_Barracks.mineralPrice() && BuildingCount[2] < maxBuilding[2] & UnitCount[0] > 10)
        {
            antiSpammingBarracks(commandCenters.front(), Colors::Green, 200,1);
        }
    }
    else
    {
        antiSpammingDepots(commandCenters.front(), Colors::Blue, 250, 1);
        //almostSupplyBlock();
        //Broodwar->sendText("Supply Blocked");
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
        productionManager();        
    }
}

void ExampleAIModule::onUnitCreate(BWAPI::Unit unit)
{
    updateUnitCount(true, unit);
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


    //updateUnitCount(true, unit);


}

void ExampleAIModule::onUnitDestroy(BWAPI::Unit unit)
{
    updateUnitCount(false, unit);

}

void ExampleAIModule::onSendText(std::string text)
{ 
    Position myPos(Broodwar->getScreenPosition().x + Broodwar->getMousePosition().x, Broodwar->getScreenPosition().y + Broodwar->getMousePosition().y);
    if (text == "b")
    {
        //build supply        
    }
    if (text == "stats")
    {
        displayStats = !displayStats;
    }
    if (text == "'")
    {
        CommMngr::attackUnits(marines, myPos);
    }
    if (text == ";")
    {
        CommMngr::moveUnits(marines, myPos);
    }
    if (text == "p")
    {       
        CommMngr::setRallyPoint(barracks, myPos);
    }
    if (text == "q")
    {
        //Position myPos(Broodwar->getScreenPosition().x + Broodwar->getMousePosition().x, Broodwar->getScreenPosition().y + Broodwar->getMousePosition().y);
        for (auto& u : Miners)
        {            
            Unit miner = UnitFun::getUnitByID(workers, u);            
            miner->move(myPos);
            miner->stop(true);
            miner->gather(miner->getClosestUnit(IsMineralField),true); //the everyframe logic messes up this
        }
    }
    if (text == "w")
    {
        Unit builder = UnitFun::returnFirstAvaibleBuilder(Builders, workers);
        builder->move(myPos);
        builder->build(BWAPI::UnitTypes::Terran_Command_Center, builder->getTilePosition());
    }
    if (text == "e")
    {
        Unit builder = UnitFun::returnFirstAvaibleBuilder(Builders, workers);
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

