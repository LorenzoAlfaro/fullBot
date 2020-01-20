#pragma once
#include <BWAPI.h>
#include "UnitFun.h"

using namespace BWAPI;
class BuildManager
{
public:
	static void createEventTag(Unit Building, Error lastErr);

	static void createSupplyBox(TilePosition targetBuildLocation, UnitType Building, Color color);

	static void buildBuilding(Unit supplyBuilder, UnitType Building, Color color, TilePosition targetBuildLocation);

	static void antiSpammingBuilding(Unit commandCenter, UnitType Building, Color color, int offSet, std::list<int> Miners, std::list<int> Builders, UnitType supplyProviderType, std::list<Unit> workers);

	static void antiSpammingDepots(Unit commandCenter, Color color, int offSet, int numberNeeded, std::list<int> Miners, std::list<int> Builders, UnitType supplyProviderType, std::list<Unit> workers, std::list<Unit> supplyDepots);

	static void antiSpammingBarracks(Unit commandCenter, Color color, int offSet, int numberNeeded, std::list<int> Miners, std::list<int> Builders, UnitType supplyProviderType, std::list<Unit> workers, std::list<Unit> barracks);

	static void almostSupplyBlock(Unit CommandCenter);
};

