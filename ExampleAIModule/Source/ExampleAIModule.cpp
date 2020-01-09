#include "ExampleAIModule.h"
#include "UnitFun.h"
#include "BuildManager.h"
#include "CommMngr.h"
#include <iostream>


using namespace BWAPI;
using namespace Filter;

#pragma region globalVariables

bool displayStats = false;

#pragma endregion

#pragma region UnitLists

std::list<Unit> commandCenters;
std::list<Unit> workers;
std::list<Unit> barracks;
std::list<Unit> marines;
std::list<int> Miners;
std::list<int> Builders;
std::list<Unit> supplyDepots;
#pragma endregion

#pragma region uniqueUnits

Unit myBuilder;
UnitType supplyProviderType = BWAPI::UnitTypes::Terran_Supply_Depot;

#pragma endregion

#pragma region UnitCount

int scvCount = 0;
int marineCount = 0;
int maxBarracks = 4;
#pragma endregion

#pragma region BuildingCount

int commandCenterCount = 0;
int supplyDepotsCount = 0; //completed supplydepots
int barrackCount = 0;

#pragma endregion

#pragma region MaxCount

int maxScvCount = 40;

int supplyLimit = 0; // max supply 200
int supplyLeft = 0; //how many units can I support yet

int roomForProduction = 0; //how much before I need to build supply depots

#pragma endregion

#pragma region BoolVariable

bool almostSupplyBlocked = false; //when true AI needs to build supplydepots pre emptively


#pragma endregion

#pragma region MacroVariables

int virtualBudget = 0;

#pragma endregion

//methods

#pragma region VerificationMethods
bool validUnit(Unit u)
{
    bool valid = false;
    // Ignore the unit if it no longer exists
    // Make sure to include this block when handling any Unit pointer!
    if (u->exists())
    {
        if (!u->isLockedDown() && !u->isMaelstrommed() && !u->isStasised())
        {
            if (!u->isLoaded() && u->isPowered() && !u->isStuck())
            {
                if (u->isCompleted() && !u->isConstructing())
                {
                    valid = true;
                }
            }
        }
    }

    return valid;
}

bool validFrame()
{
    bool valid = false;
    // Return if the game is a replay or is paused
    if (!Broodwar->isReplay() && !Broodwar->isPaused() && Broodwar->self())
    {
        // Prevent spamming by only running our onFrame once every number of latency frames.
        // Latency frames are the number of frames before commands are processed.
        if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() == 0)
        {
            valid = true;
        }
    }

    return valid;
}
#pragma endregion

#pragma region SupplyInfoMethods
int SupplyTotal(int commandCenterCount, int supplyDepotCount)
{
    int total = (commandCenterCount * 10) + (supplyDepotCount * 8);
    return total;
}

int usedSupplyTotal(int marineCount, int scvCount)
{
    int total = (marineCount * 1) + (scvCount * 1);
    return total;
}

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
                barrackCount = barrackCount + 1;
                barracks.push_back(unit);
            }
            if (myType == BWAPI::UnitTypes::Terran_Marine)
            {
                marineCount = marineCount + 1;

                marines.push_back(unit); //how to remove the dead ones?
            }
            if (myType == BWAPI::UnitTypes::Terran_SCV)
            {
                scvCount = scvCount + 1;
                workers.push_back(unit);
                Miners.push_back(unit->getID());//assign to mine
            }
            if (myType == BWAPI::UnitTypes::Terran_Supply_Depot)
            {
                supplyDepotsCount = supplyDepotsCount + 1;
                supplyDepots.push_back(unit);
            }
            if (myType == BWAPI::UnitTypes::Terran_Command_Center)
            {
                commandCenterCount = commandCenterCount + 1;
            }

            supplyLimit = SupplyTotal(commandCenterCount, supplyDepotsCount);

            supplyLeft = supplyLimit - usedSupplyTotal(marineCount, scvCount);

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
                barrackCount = barrackCount - 1;
            }
            if (myType == BWAPI::UnitTypes::Terran_Marine)
            {
                marineCount = marineCount - 1;
            }
            if (myType == BWAPI::UnitTypes::Terran_SCV)
            {
                scvCount = scvCount - 1;
            }
            if (myType == BWAPI::UnitTypes::Terran_Supply_Depot)
            {
                supplyDepotsCount = supplyDepotsCount - 1;
            }
            if (myType == BWAPI::UnitTypes::Terran_Command_Center)
            {
                commandCenterCount = commandCenterCount - 1;
            }
            supplyLimit = SupplyTotal(commandCenterCount, supplyDepotsCount);

            supplyLeft = supplyLimit - usedSupplyTotal(marineCount, scvCount);
        }
    }
}

int roomNeeded()
{
    int offset = 1;
    int supply = offset + commandCenterCount * 1 + barrackCount * 1; //will need finer adjuments

    return supply;
}

void displayInsights()
{
    Broodwar->drawTextScreen(200, 0, "FPS: %d", Broodwar->getFPS());
    //Broodwar->drawTextScreen(200, 20, "Average FPS: %f", Broodwar->getAverageFPS());
    Broodwar->drawTextScreen(200, 40, "Barrack Count: %d", barrackCount);
    Broodwar->drawTextScreen(200, 60, "Marine Count: %d", marineCount);
    Broodwar->drawTextScreen(200, 80, "SCV Count: %d", scvCount);
    Broodwar->drawTextScreen(200, 100, "Builders: %d", Builders.size());
    //Broodwar->drawTextScreen(200, 100, "Mouse Cursor: %d  %d", Broodwar->getMousePosition().x, Broodwar->getMousePosition().y);
    //Broodwar->drawTextScreen(200, 120, "Screen: %d  %d", Broodwar->getScreenPosition().x, Broodwar->getScreenPosition().y);
    Broodwar->drawTextScreen(200, 140, "supply limit: %d ", supplyLeft);
    Broodwar->drawTextScreen(200, 120, "room for next round: %d ", roomNeeded());
}

#pragma endregion

#pragma region UnitHandlers

void antiSpammingBuilding(Unit commandCenter, UnitType Building, Color color, int offSet)
{
    static int lastChecked = 0;

    if (lastChecked + offSet < Broodwar->getFrameCount() &&
        Broodwar->self()->incompleteUnitCount(Building) == 0)
    {
        lastChecked = Broodwar->getFrameCount();                      
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

void scvManager()
{
    for (auto& ID : Miners)
    {
        Unit u = UnitFun::getUnitByID(workers,ID);
        if (u->isIdle())
        {
            u->gather(u->getClosestUnit(IsMineralField || IsRefinery));
        }
    }
}

void buildSCVs()
{
    for (auto& u : commandCenters)
    {
        if (u->isIdle())
        {
            u->train(BWAPI::UnitTypes::Terran_SCV);
        }
    }
}

void trainMarines()
{
    for (auto& u : barracks)
    {
        if (u->isIdle() && !u->train(BWAPI::UnitTypes::Terran_Marine))
        {
            //supplyBlock(u);

        } // closure: failed to train idle unit
    }
}

void unitHandler(Unitset units)
{
    for (auto& u : units)
    {
        // Finally make the unit do some stuff!
        if (validUnit(u))
        {
            // If the unit is a worker unit
            if (u->getType().isWorker())
            {               
                // if our worker is idle
                if (u->isIdle())
                {
                    if (Broodwar->self()->minerals() >= UnitTypes::Terran_Barracks.mineralPrice() && barrackCount < 4)
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

                //int supplyLeft = SupplyTotal(commandCenterCount, supplyDepots) - usedSupplyTotal(marineCount, scvCount);
                almostSupplyBlocked = false;

                if (supplyLeft < 4) {

                    almostSupplyBlocked = true;

                    almostSupplyBlock(u);

                }
                // Order the depot to construct more workers! But only when it is idle.

                if (scvCount < 50)
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

#pragma region MainMethods

void initialAssigment(Unitset units)
{
    for (auto& u : units)
    {
        if (validUnit(u))
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
    //int supplyLeft = SupplyTotal(commandCenterCount, supplyDepots) - usedSupplyTotal(marineCount, scvCount);
    almostSupplyBlocked = false;

    if (supplyLeft < roomNeeded()) { //instead of 4,should be the max output of production at a given time

        almostSupplyBlocked = true;
    }
    scvManager();
    if (!almostSupplyBlocked)
    {
        if (scvCount < 50)
        {
            buildSCVs();
        }
        
        trainMarines();

        if (Broodwar->self()->minerals() >= UnitTypes::Terran_Barracks.mineralPrice() && barrackCount < maxBarracks & scvCount > 10)
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

    if (validFrame())
    {
        productionManager();
        // Iterate through all the units that we own
        //unitHandler(Broodwar->self()->getUnits());
        //trainMarines();
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


