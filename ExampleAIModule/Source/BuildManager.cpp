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
    if (targetBuildLocation)
    {
        BuildManager::createSupplyBox(targetBuildLocation, Building, color); //draw where to build
        // Order the builder to construct the supply structure
        if (!supplyBuilder->build(Building, targetBuildLocation))
        {
            Broodwar->sendText("Failed building %s :", Building.c_str());
            Broodwar->sendText(Broodwar->getLastError().c_str());
            //Broodwar << Broodwar->getLastError() << std::endl;;
        }
    }
}