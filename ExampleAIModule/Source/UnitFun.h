#pragma once
#include <BWAPI.h>

using namespace BWAPI;
class UnitFun
{
public:
	
	//static Unit getUnitByID(std::list<Unit> Units, int ID);
	static Unit returnUnitByID(Unitset Units, int ID);//already discards destroyed units

	static Unit getSCVfromCC(Unit CommandCenter, std::list<int> &Miners, std::list<int> &Builders, UnitType supplyProviderType);

	static Unit returnFirstAvaibleBuilder(std::list<int> Builders);

	static Unit getWorker(Unit CommandCenter, std::list<int> &Miners, std::list<int> &Builders, UnitType supplyProviderType);

	static std::list<Unit> getListofUnitType(UnitType myType, Unitset allUnits, std::list<int>& deadUnits);

};

