#include "ExampleAIModule.h"

using namespace BWAPI;
using namespace Filter;
using namespace std;
using namespace UnitTypes;
//TODO: instead of defining an amount of marines, define the porcentage from the total food supply quota i.e 50% marines = 100 marines or 50 vultures
//TODO: fix bug, TASK ID is not being represented corectly
//Change Task Array to use Task Enum instead of indexes
//ADD building map file, where to build my buildings location
//Fix bug, when using scan it crashes! recognize the spell unit
//Implement error handling
//Add logic to build turrets
//Able to create control groups with more than 12 units 

#pragma region MainEvents
void ExampleAIModule::onFrame()
{          
    Player BW = Broodwar->self();
    const int frameCount = Broodwar->getFrameCount();
    const int minerals =                    BW->minerals();
    const int gas =                         BW->gas();
    const Unitset myUnits =                 BW->getUnits();
    const int emptySupply =                 BW->supplyTotal() - BW->supplyUsed();
    const int barracksCount =               UnitFun::getUnitCount(myUnits,Terran_Barracks);
    const int CommandCenterCount =          UnitFun::getUnitCount(myUnits,Terran_Command_Center);
    const int SCVcount =                    UnitFun::getUnitCount(myUnits,Terran_SCV);
    const int marineCount =                 UnitFun::getUnitCount(myUnits,Terran_Marine);
    const int roomNeeded =                  auxFun::roomNeeded(CommandCenterCount, barracksCount);
    if (displayStats)
    {
        auxFun::displayInsights2(roomNeeded,emptySupply,SCVcount,barracksCount,marineCount,StatsCoordinates,Builders.size(),deadUnits.size(),taskQueue.size());
    }

    if (auxFun::validFrame())
    {           
        TaskEngine::taskManager(taskQueue,frameCount,minerals,gas,UnitFun::getUnitList(Terran_Command_Center,myUnits,deadUnits).front(),Miners,Builders);
       
        ProductionManager::Manage(minerals,gas,frameCount,taskQueue,TaskCount,deadUnits,barracksCount,SCVcount,maxBuilding,maxUnit,roomNeeded>emptySupply,roomNeeded);
        //eventually productionManager will be another task run by taskManager            
        CommMngr::scvManager(Miners);//go mine for me minions!
        
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
                TaskFun::CreateTask(taskQueue, Broodwar->getFrameCount(), 0, (int)taskOwner::Commander, (int)action::BuildEngineeringBay, TaskCount);
                callBack = frameCount + 60;
            }                        
        }
        if (GetKeyState('Y') & 0x8000/*Check if high-order bit is set (1 << 15)*/)
        {    
            if (frameCount >= callBack)
            {
                TaskFun::CreateTask(taskQueue, Broodwar->getFrameCount(), 0, (int)taskOwner::Commander, UnitTypes::Terran_Command_Center, TaskCount);
                callBack = frameCount + 60;
            }
            
        }
        if (GetKeyState('T') & 0x8000/*Check if high-order bit is set (1 << 15)*/)
        {
            if (frameCount >= callBack)
            {
                TaskFun::CreateTask(taskQueue, Broodwar->getFrameCount(), 0, (int)taskOwner::Commander, UnitTypes::Terran_Missile_Turret, TaskCount);
                callBack = frameCount + 60;
            }
            //The code here executes ONCE at the moment the other key was released            
            
        }
        if (GetKeyState('R') & 0x8000/*Check if high-order bit is set (1 << 15)*/)
        {
            if (frameCount >= callBack)
            {
                TaskFun::CreateTask(taskQueue, Broodwar->getFrameCount(), 0, (int)taskOwner::Commander, UnitTypes::Terran_Refinery, TaskCount);
                callBack = frameCount + 60;
            }
            
        }
        if (GetKeyState('E') & 0x8000/*Check if high-order bit is set (1 << 15)*/)
        {
            if (frameCount >= callBack)
            {
                TaskFun::CreateTask(taskQueue, Broodwar->getFrameCount(), 0, (int)taskOwner::Commander, UnitTypes::Terran_Barracks, TaskCount);
                callBack = frameCount + 60;
            }
            
        }
    }    
}

void ExampleAIModule::onUnitCreate(Unit unit)
{

    if (IsOwned(unit))
    {        
        if (unit->getType() == Terran_SCV)
        {
            Miners.push_back(unit->getID());//assign to mine
        }
        
        if (!taskQueue.empty() && !unit->canAttack()
            && unit->getType() != Terran_SCV
            && unit->getType() != Terran_Marine)
        {
            Unit builder = unit->getBuildUnit();
            int builderID = builder->getID();
            int BuildingID = unit->getID();


            array<int, 12> mytask = {0,0,0,0,0,0,0};
            array<int, 12>* pointer = TaskFun::findTaskAssignedToUnit(builderID, taskQueue);
            if (pointer != nullptr)
            {
                mytask = *pointer;
            }            
            //only for buildings
            if (TaskFun::taskStatusUpdate(builderID, taskQueue, BuildingID, (int)taskStatus::Started))
            {
                Broodwar->sendText("Started task id: %d : %s", mytask[(int)tsk::ID], unit->getType().c_str());
            }
            else
            {
                Broodwar->sendText("Failed update to started task id: %d : %s", unit->getID(), unit->getType().c_str());
            }
        }
    }       
}
void ExampleAIModule::onUnitComplete(Unit unit)
{
    if (IsOwned(unit))
    {
        if (!taskQueue.empty() && !unit->canAttack()) //avoid running in buildings
        {
            array<int, 12> mytask = { 0,0,0,0,0,0,0 };
            array<int, 12>* pointer = TaskFun::findTaskAssignedToUnit(unit->getID(), taskQueue);
            if (pointer != nullptr)
            {
                mytask = *pointer;
            }

            if (TaskFun::taskStatusUpdate(unit->getID(), taskQueue, unit->getID(), (int)taskStatus::Completed))
            {
                Broodwar->sendText("Completed task id: %d : %s", mytask[(int)tsk::ID], unit->getType().c_str());
            }
            else
            {
                Broodwar->sendText("failed update to completed task id: %d : %s", unit->getID(), unit->getType().c_str());
            }
        }
        //TODO: test remove  
      //taskQueue.remove(TaskFun::findTaskAssignedToID(unit->getID(),taskQueue));
      //updateUnitCount(true, unit);
    }
    
}

void ExampleAIModule::onUnitDestroy(Unit unit)
{
   
    if (IsOwned(unit))
    {        
        deadUnits.push_back(unit->getID());
    }

}

void ExampleAIModule::onSendText(string text)
{ 
    Position myPos = auxFun::getMousePosition();
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
        CommMngr::attackUnits(UnitFun::getUnitList(UnitTypes::Terran_Marine, Broodwar->self()->getUnits(), deadUnits), myPos);
    }
    if (text == ";")
    {
        CommMngr::moveUnits(UnitFun::getUnitList(UnitTypes::Terran_Marine, Broodwar->self()->getUnits(), deadUnits), myPos);
    }
    if (text == "p")
    {       
        CommMngr::setRallyPoint(UnitFun::getUnitList(UnitTypes::Terran_Barracks, Broodwar->self()->getUnits(), deadUnits), myPos);
    }
    if (text == "q")
    {        
        for (auto& u : Miners)
        {                        
            Unit miner = Broodwar->getUnit(u);
            miner->move(myPos);
            miner->stop(true);
            miner->gather(miner->getClosestUnit(IsMineralField),true); //the everyframe logic messes up this
        }
    }
    if (text == "w")
    {
        Unit builder = UnitFun::getBuilder(Builders);
        builder->move(myPos);
        builder->build(Terran_Command_Center, builder->getTilePosition());
    }
    if (text == "e")
    {
        TaskFun::CreateTask(taskQueue, Broodwar->getFrameCount(), 0, (int)taskOwner::Commander, (int)action::BuildEngineeringBay, TaskCount);
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
            Broodwar->printf("x: %d y: %d",x,y);                                   
        }        
    }
    // Send the text to the game if it is not being processed.
    Broodwar->sendText("%s", text.c_str());    
    // Make sure to use %s and pass the text as a parameter,
    // otherwise you may run into problems when you use the %(percent) character!
}

void ExampleAIModule::onStart()
{
    if (!FileIO::readStatsCoordinates(StatsCoordinates))
    {
        Broodwar->sendText("Unable to open file");//cout << "Unable to open file"
    }
    FileIO::WriteHeaders();//for task record      
    // Print the map name.
    // BWAPI returns string when retrieving a string, don't forget to add .c_str() when printing!
    //Broodwar << "The map is  " << Broodwar->mapName() << "!" << endl;

    // Enable the UserInput flag, which allows us to control the bot and type messages.
    Broodwar->enableFlag(Flag::UserInput);

    // Uncomment the following line and the bot will know about everything through the fog of war (cheat).
    //Broodwar->enableFlag(Flag::CompleteMapInformation);

    // Set the command optimization level so that common commands can be grouped
    // and reduce the bot's APM (Actions Per Minute).
    Broodwar->setCommandOptimizationLevel(2);

    // Check if this is a replay
    if (!Broodwar->isReplay())
    {
        // Retrieve you and your enemy's races. enemy() will just return the first enemy.
        // If you wish to deal with multiple enemies then you must use enemies().
        if (Broodwar->enemy()) // First make sure there is an enemy
            Broodwar << "Welcome to the Jungle v1.0 " << Broodwar->self()->getRace() << " vs " << Broodwar->enemy()->getRace() << endl;
        
    }   
}

#pragma endregion

#pragma region OtherEvents

void ExampleAIModule::onReceiveText(Player player, string text)
{
    // Parse the received text
    Broodwar << player->getName() << " said \"" << text << "\"" << endl;
}

void ExampleAIModule::onUnitDiscover(Unit unit)
{
}

void ExampleAIModule::onUnitEvade(Unit unit)
{
}

void ExampleAIModule::onUnitShow(Unit unit)
{
}

void ExampleAIModule::onUnitHide(Unit unit)
{
}

void ExampleAIModule::onEnd(bool isWinner)
{
    // Called when the game ends
    if (isWinner)
    {
        // Log your win here!
    }
}

#pragma endregion

#pragma region NotUsedEvents
void ExampleAIModule::onSaveGame(string gameName)
{
    Broodwar << "The game was saved to \"" << gameName << "\"" << endl;
}

void ExampleAIModule::onUnitRenegade(Unit unit)
{
}

void ExampleAIModule::onUnitMorph(Unit unit)
{
    if (Broodwar->isReplay())
    {
        // if we are in a replay, then we will print out the build order of the structures
        if (unit->getType().isBuilding() && !unit->getPlayer()->isNeutral())
        {
            int seconds = Broodwar->getFrameCount() / 24;
            int minutes = seconds / 60;
            seconds %= 60;
            Broodwar->sendText("%.2d:%.2d: %s morphs a %s", minutes, seconds, unit->getPlayer()->getName().c_str(), unit->getType().c_str());
        }
    }
}

void ExampleAIModule::onPlayerLeft(Player player)
{
    // Interact verbally with the other players in the game by
    // announcing that the other player has left.
    Broodwar->sendText("Goodbye %s!", player->getName().c_str());
}

void ExampleAIModule::onNukeDetect(Position target)
{
    // Check if the target is a valid position
    if (target)
    {
        // if so, print the location of the nuclear strike target
        Broodwar << "Nuclear Launch Detected at " << target << endl;
    }
    else
    {
        // Otherwise, ask other players where the nuke is!
        Broodwar->sendText("Where's the nuke?");
    }
    // You can also retrieve all the nuclear missile targets using Broodwar->getNukeDots()!
}

#pragma endregion
