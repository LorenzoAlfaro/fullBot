#include "BuildManager.h"
#include <iostream>

using namespace BWAPI;
using namespace Filter;

void BuildManager::createEventTag(Unit Building, Error lastErr)
{
    // If building scv fails, draw the error at the location so that you can visibly see what went wrong!
    // However, drawing the error once will only appear for a single frame
    // so create an event that keeps it on the screen for some frames
    Position pos = Building->getPosition();
    //
    Broodwar->registerEvent([pos, lastErr](Game*) { Broodwar->drawTextMap(pos, "%c%s", Text::White, lastErr.c_str()); },   // action
        nullptr,    // condition
        Broodwar->getLatencyFrames());  // frames to run

}

void BuildManager::createSupplyBox(TilePosition targetBuildLocation, UnitType Building, Color color)
{
    // Register an event that draws the target build location
    Broodwar->registerEvent([targetBuildLocation, Building, color](Game*)
        {
            Broodwar->drawBoxMap(Position(targetBuildLocation),
                Position(targetBuildLocation + Building.tileSize()),
                color);
        },
        nullptr,  // condition
            Building.buildTime() + 100);  // frames to run

}

void BuildManager::buildBuilding(Unit supplyBuilder, UnitType Building, Color color, TilePosition targetBuildLocation)
{
    //UnitType  supplyProviderType = BWAPI::UnitTypes::Terran_Supply_Depot;

    if (targetBuildLocation)
    {
        BuildManager::createSupplyBox(targetBuildLocation, Building, color); //draw where to build

        // Order the builder to construct the supply structure
        if (!supplyBuilder->build(Building, targetBuildLocation))
        {
            Broodwar->sendText("Failed building %s :", Building.c_str()); //might cause lag

            Broodwar->sendText(Broodwar->getLastError().c_str());
            //Broodwar << Broodwar->getLastError() << std::endl;;
        }

    }
}

void BuildManager::antiSpammingBuilding(Unit commandCenter, UnitType Building, Color color, int offSet, std::list<int> Miners, std::list<int> Builders, UnitType supplyProviderType, std::list<Unit> workers)
{
    static int lastChecked = 0;

    if (Broodwar->getFrameCount() > lastChecked + offSet &&
        Broodwar->self()->incompleteUnitCount(Building) == 0)
    {
        lastChecked = Broodwar->getFrameCount();    //is int big enough to hold this?                  
        BuildManager::buildBuilding(UnitFun::getWorker(commandCenter, Miners, Builders, supplyProviderType), Building, color, commandCenter->getTilePosition());
    }
}

void BuildManager::antiSpammingDepots(Unit commandCenter, Color color, int offSet, int numberNeeded, std::list<int> Miners, std::list<int> Builders, UnitType supplyProviderType, std::list<Unit> supplyDepots)
{
    static int lastChecked = 0;

    if (Broodwar->getFrameCount() > lastChecked + offSet &&
        Broodwar->self()->incompleteUnitCount(BWAPI::UnitTypes::Terran_Supply_Depot) != numberNeeded)
    {
        lastChecked = Broodwar->getFrameCount();

        for (int i = 0; i < numberNeeded; i++)
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
            BuildManager::buildBuilding(UnitFun::getWorker(commandCenter, Miners, Builders, supplyProviderType), BWAPI::UnitTypes::Terran_Supply_Depot, color, targetBuildLocation);
        }


    }
}

void BuildManager::antiSpammingBarracks(Unit commandCenter, Color color, int offSet, int numberNeeded, std::list<int> Miners, std::list<int> Builders, UnitType supplyProviderType, std::list<Unit> workers, std::list<Unit> barracks)
{
    static int lastChecked = 0;

    if (Broodwar->getFrameCount() > lastChecked + offSet &&
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
            BuildManager::buildBuilding(UnitFun::getWorker(commandCenter, Miners, Builders, supplyProviderType), BWAPI::UnitTypes::Terran_Barracks, color, targetBuildLocation);
        }

    }
}


