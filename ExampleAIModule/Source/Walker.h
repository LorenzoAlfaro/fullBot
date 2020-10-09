#pragma once
#include <BWAPI.h>
#include "UnitFun.h"
using namespace Filter;
using namespace std;
using namespace BWAPI;
class Walker
{
public:

	static void Walk(Unit myUnit, bool Attack,int frameCount, int& callBack);
	static void moveRight(Unit myUnit, bool Attack);
	static void moveLeft(Unit myUnit, bool Attack);
	static void moveUp(Unit myUnit, bool Attack);
	static void moveDown(Unit myUnit, bool Attack);

};

