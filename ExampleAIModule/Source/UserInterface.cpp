#include "UserInterface.h"



void UserInterface::ReadCommand(int frameCount, int& callBack, list<int> deadUnits, list<array<int, 12>>& taskQueue)
{

    if (GetKeyState('I') & 0x8000/*Check if high-order bit is set (1 << 15)*/)
    {
        if (frameCount >= callBack)
        {
            Position myPos = auxFun::getMousePosition();
            CommMngr::attackUnits(UnitFun::getUnitList(UnitTypes::Terran_Marine, Broodwar->self()->getUnits(), deadUnits), myPos);
            callBack = frameCount + 60;
        }

    }
    if (GetKeyState('Q') & 0x8000/*Check if high-order bit is set (1 << 15) LEFT CLICK*/)
    {
        if (frameCount >= callBack)
        {
            auxFun::LeftClick();// I cant believe this works!   
            callBack = frameCount + 60;
        }
    }
    if (GetKeyState('W') & 0x8000/*Check if high-order bit is set (1 << 15)*/)
    {
        if (frameCount >= callBack)
        {
            auxFun::RightClick();
            callBack = frameCount + 60;
        }

    }
    if (GetKeyState('U') & 0x8000)
    {
        if (frameCount >= callBack)
        {
            TaskFun::CreateTask(taskQueue, Broodwar->getFrameCount(), 0, (int)taskOwner::Commander, Terran_Engineering_Bay);
            callBack = frameCount + 60;
        }
    }
    if (GetKeyState('Y') & 0x8000/*Check if high-order bit is set (1 << 15)*/)
    {
        if (frameCount >= callBack)
        {
            TaskFun::CreateTask(taskQueue, Broodwar->getFrameCount(), 0, (int)taskOwner::Commander, Terran_Command_Center);
            callBack = frameCount + 60;
        }

    }
    if (GetKeyState('T') & 0x8000/*Check if high-order bit is set (1 << 15)*/)
    {
        if (frameCount >= callBack)
        {
            TaskFun::CreateTask(taskQueue, Broodwar->getFrameCount(), 0, (int)taskOwner::Commander, Terran_Missile_Turret);
            callBack = frameCount + 60;
        }
        //The code here executes ONCE at the moment the other key was released            

    }
    if (GetKeyState('R') & 0x8000/*Check if high-order bit is set (1 << 15)*/)
    {
        if (frameCount >= callBack)
        {
            TaskFun::CreateTask(taskQueue, Broodwar->getFrameCount(), 0, (int)taskOwner::Commander, Terran_Refinery);
            callBack = frameCount + 60;
        }

    }
    if (GetKeyState('E') & 0x8000/*Check if high-order bit is set (1 << 15)*/)
    {
        if (frameCount >= callBack)
        {
            TaskFun::CreateTask(taskQueue, Broodwar->getFrameCount(), 0, (int)taskOwner::Commander, Terran_Barracks);
            callBack = frameCount + 60;
        }

    }
}

void UserInterface::ReadInput(string text, list<int> deadUnits, bool& displayStats, Position myPos)
{
    if (text == "b")
    {
        Broodwar->sendText("power overwhelming");
    }
    if (text == "stats")
    {
        displayStats = !displayStats;
    }
    if (text == "'")
    {
        CommMngr::attackUnits(UnitFun::getUnitList(Terran_Marine, Broodwar->self()->getUnits(), deadUnits), myPos);
    }
    if (text == ";")
    {
        CommMngr::moveUnits(UnitFun::getUnitList(Terran_Marine, Broodwar->self()->getUnits(), deadUnits), myPos);
    }
    if (text == "p")
    {
        CommMngr::setRallyPoint(UnitFun::getUnitList(Terran_Barracks, Broodwar->self()->getUnits(), deadUnits), myPos);
    }
    if (text == "q")
    {
        //for (auto& u : Miners)
        //{
           // Unit miner = Broodwar->getUnit(u);
            //miner->move(myPos);
           // miner->stop(true);
            //miner->gather(miner->getClosestUnit(IsMineralField), true); //the everyframe logic messes up this
       // }
    }
    if (text == "w")
    {
        //Unit builder = UnitFun::getBuilder(Builders);
        //builder->move(myPos);
        //builder->build(Terran_Command_Center, builder->getTilePosition());
    }
    if (text == "e")
    {
        //TaskFun::CreateTask(taskQueue, Broodwar->getFrameCount(), 0, (int)taskOwner::Commander, (int)action::BuildEngineeringBay);
        //Unit builder = UnitFun::getBuilder(Builders);
        //builder->move(myPos);
        //builder->build(Terran_Engineering_Bay, builder->getTilePosition());
    }

    if (text == "u")
    {
        static bool setGui = false;
        Broodwar->setGUI(setGui);
        setGui = !setGui;
        Broodwar->printf("Map x: %d y: %d", Broodwar->mapWidth(), Broodwar->mapHeight());
        Unitset myUnits = Broodwar->getSelectedUnits();
        for (auto& u : myUnits)
        {
            int x = u->getTilePosition().x;
            int y = u->getTilePosition().y;
            Broodwar->printf("x: %d y: %d", x, y);
        }
    }

}
