#pragma once
#include <BWAPI.h>
#include <iostream>
#include "auxFun.h"
using namespace BWAPI;
class UnitFun
{
public:
		
	static Unit getSCV(Unit CommandCenter, std::list<int> &Miners, std::list<int> &Builders);

	static Unit getBuilder(std::list<int> Builders);

	static Unit getWorker(Unit CommandCenter, std::list<int> &Miners, std::list<int> &Builders);

	static std::list<Unit> getUnitList(UnitType Type, Unitset allUnits, std::list<int>& deadUnits);

	static int getUnitCount(const Unitset& allUnits, UnitType myType);
};

