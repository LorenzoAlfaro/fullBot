#include "Walker.h"

int step = 24;

void Walker::moveLeft(Unit myUnit, bool Attack) {
    //if (GetKeyState('A') & 0x8000/*Check if high-order bit is set (1 << 15)*/)
    //{                    
        //Unit myMarine = UnitFun::getUnitList(Hero_Jim_Raynor_Marine, Broodwar->self()->getUnits(), deadUnits).front();
        
        //TilePosition myTile = myUnit->getTilePosition();
        Position movehere = myUnit->getPosition();
        movehere.x -= step;
        myUnit->move(movehere);
        Position myPos = auxFun::getUnitPosition(myUnit);
        Broodwar->setScreenPosition(myPos);

        //Position newPosition = (Position)myTile;
        //WalkPosition walkPos = (WalkPosition)movehere;
        //if (Broodwar->isWalkable(walkPos))
        //{
            
        //}                
        return;
    //}
}
void Walker::moveRight(Unit myUnit, bool Attack) {    
    Position movehere = myUnit->getPosition();
    movehere.x += step;
    myUnit->move(movehere);
    Position myPos = auxFun::getUnitPosition(myUnit);
    Broodwar->setScreenPosition(myPos);    
    return;    
}
void Walker::moveUp(Unit myUnit, bool Attack) {    
    Position movehere = myUnit->getPosition();
    movehere.y -= step;
    myUnit->move(movehere);
    Position myPos = auxFun::getUnitPosition(myUnit);
    Broodwar->setScreenPosition(myPos);    
    return;    
}
void Walker::moveDown(Unit myUnit, bool Attack) {    
    Position movehere = myUnit->getPosition();
    movehere.y += step;
    myUnit->move(movehere);
    Position myPos = auxFun::getUnitPosition(myUnit);
    Broodwar->setScreenPosition(myPos);    
    return;    
}

void Walker::Walk(Unit myUnit, bool Attack, int frameCount, int& callBack)
{
    //figure logic to keep moving
                           

    //TilePosition myTile = myUnit->getTilePosition();

    //Position newPosition = (Position)myTile;

    if (frameCount >= callBack)
    {
        Position nextMovement = myUnit->getPosition();


        int x = nextMovement.x;
        int y = nextMovement.y;

        int x0 = x;
        int y0 = y;

        nextMovement.x -= step;

        if (Broodwar->isWalkable((WalkPosition)nextMovement))
        {
            moveLeft(myUnit, Attack);
        }

        nextMovement.x = x;
        nextMovement.y = y;

        nextMovement.y += step;

        if (Broodwar->isWalkable((WalkPosition)nextMovement))
        {
            moveDown(myUnit, Attack);
        }

        nextMovement.x = x;
        nextMovement.y = y;

        nextMovement.y -= step;

        if (Broodwar->isWalkable((WalkPosition)nextMovement))
        {
            moveUp(myUnit, Attack);
        }

        nextMovement.x = x;
        nextMovement.y = y;

        nextMovement.x += step;

        callBack = frameCount + 20;

        return;
    }


      
}


