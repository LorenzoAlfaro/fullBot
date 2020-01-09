#include "CommMngr.h"

void CommMngr::attackUnits(std::list<Unit> units, Position myPos)
{
    for (auto& u : units)
    {        
        u->attack(myPos, false);
    }
}
void CommMngr::moveUnits(std::list<Unit> units, Position myPos)
{
    for (auto& u : units)
    {
        u->move(myPos, false);
    }
}
void CommMngr::setRallyPoint(std::list<Unit> buildings, Position myPos)
{
    for (auto& u : buildings)
    {
        u->setRallyPoint(myPos);
    }
}
