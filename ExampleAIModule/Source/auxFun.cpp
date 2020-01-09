#include "auxFun.h"

int auxFun::SupplyTotal(int commandCenterCount, int supplyDepotCount)
{
    int total = (commandCenterCount * 10) + (supplyDepotCount * 8);
    return total;
}


int auxFun::usedSupplyTotal(int marineCount, int scvCount)
{
    int total = (marineCount * 1) + (scvCount * 1);
    return total;
}

int auxFun::roomNeeded(int commandCenterCount, int barrackCount)
{
    int offset = 1;
    int supply = offset + commandCenterCount * 1 + barrackCount * 1; //will need finer adjuments

    return supply;
}