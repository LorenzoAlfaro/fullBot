#pragma once
#include <BWAPI.h>
class auxFun
{
public:
	static int SupplyTotal(int commandCenterCount, int supplyDepotCount);
	static int usedSupplyTotal(int marineCount, int scvCount);
	static int roomNeeded(int commandCenterCount, int barrackCount);
	static bool validUnit(BWAPI::Unit u, std::list<int> deadUnits);
	static bool validFrame();
	static bool isUnitDead(std::list<int>& deadUnits, int id);
};

