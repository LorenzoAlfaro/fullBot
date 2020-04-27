#pragma once
#include <array>
#include <BWAPI.h>

using namespace std;
using namespace BWAPI;

class TaskEngine
{
public:
	static void taskManager(list<array<int, 7>>& myTaskQueue, int frameCount, int minerals, int gas, Unit CommandCenter, list<int>Miners, list<int> Builders, UnitType supplyProviderType);
};

