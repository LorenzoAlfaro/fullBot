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

	static TilePosition returnBuildPosition(int action, Unit SCV);
		
};

