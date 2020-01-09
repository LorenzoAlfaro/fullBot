#pragma once
#include <BWAPI.h>
using namespace BWAPI;


class CommMngr
{
	public:
		static void attackUnits(std::list<Unit> units, Position myPos);
		static void moveUnits(std::list<Unit> units, Position myPos);
		static void setRallyPoint(std::list<Unit> buildings, Position myPos);

};

