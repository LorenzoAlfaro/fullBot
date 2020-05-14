#pragma once

//#include <iostream>
#include "auxFun.h"
using namespace BWAPI; //can't remove this without errors
using namespace std;
class UnitFun
{
public:
		
	static Unit getSCV(Unit CommandCenter, list<int> &Miners, list<int> &Builders);

	static Unit getBuilder(list<int> Builders);

	static Unit getWorker(Unit CommandCenter, list<int> &Miners, list<int> &Builders);

	static list<Unit> getUnitList(UnitType Type, Unitset allUnits, list<int>& deadUnits);

	static int getUnitCount(const Unitset& allUnits, UnitType myType);
};

