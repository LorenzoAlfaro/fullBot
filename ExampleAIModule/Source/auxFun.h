#pragma once
#include <BWAPI.h>
using namespace std;
class auxFun
{
public:
	static int SupplyTotal(int commandCenterCount, int supplyDepotCount);
	static int usedSupplyTotal(int marineCount, int scvCount);
	static int roomNeeded(int commandCenterCount, int barrackCount);
	static bool validUnit(BWAPI::Unit u, std::list<int> deadUnits);
	static bool validFrame();
	static bool isUnitDead(std::list<int>& deadUnits, int id);
	static BWAPI::Position getMousePosition();
	static void displayInsights2(int roomNeeded, int supplyLeft2, int SCVcount, int barracksCount, int marineCount, int StatsCoordinates[][2], int Builders, int deadUnits, int taskQueueCount);
};

