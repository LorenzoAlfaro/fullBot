#pragma once
#include <array>
#include <BWAPI.h>

using namespace std;
using namespace BWAPI;
class ProductionManager
{
public:
	static void productionManager(int minerals, int gas, int frameCount, list<array<int, 12>>& myTaskQueue, int& myTaskCount, list<int> myDeadUnits, int myBuildingCount[], int myUnitCount[], int mySupplyLeftint,int myMaxBuilding[], int myMaxUnit[]);
};

