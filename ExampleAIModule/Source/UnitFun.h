#pragma once
#include <BWAPI.h>

using namespace BWAPI;
class UnitFun
{
public:
	
	static Unit getUnitByID(std::list<Unit> Units, int ID);

	static Unit getSCVfromCC(Unit CommandCenter, std::list<int> &Miners, std::list<int> &Builders, UnitType supplyProviderType);

	static Unit returnFirstAvaibleBuilder(std::list<int> Builders, std::list<Unit> workers);

	static Unit getWorker(Unit CommandCenter, std::list<int> &Miners, std::list<int> &Builders, UnitType supplyProviderType, std::list<Unit> workers);
};

