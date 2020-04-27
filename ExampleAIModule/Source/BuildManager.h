#pragma once
#include <BWAPI.h>
#include "UnitFun.h"

using namespace BWAPI;
class BuildManager
{
public:
	static void createEventTag(const Unit& Building, Error lastErr);

	static void createSupplyBox(const TilePosition targetBuildLocation, const UnitType Building, const  Color color);

	static void buildBuilding(const Unit& supplyBuilder, const UnitType Building, const Color color, const TilePosition targetBuildLocation);

	static TilePosition returnBuildPosition(int action, Unit SCV, int maxRange);
		
};

