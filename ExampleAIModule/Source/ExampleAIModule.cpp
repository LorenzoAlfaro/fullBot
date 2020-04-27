#include "ExampleAIModule.h"

#include "UnitFun.h"
#include "BuildManager.h"
#include "CommMngr.h"
#include "auxFun.h"
#include "TaskFun.h"

#include <iostream>
#include <fstream>
#include <array>
#include <windows.h> // simulate mouse clicks

using namespace BWAPI;
using namespace Filter;
using namespace std;

//using namespace WinUser;
//TODO: fix bug, TASK ID is not being represented corectly
//ADD building map file, where to build my buildings location
//Fix bug, when using scan it crashes! recognize the spell unit
//Implement error handling
//Add logic to build turrets
//Able to create control groups with more than 12 units

//Add one button commands: DONE
//Useful functions
//Broodwar->self()->allUnitCount();
//Broodwar->self()->completedUnitCount();
//Broodwar->self()->supplyTotal();
//Broodwar->self()->supplyUsed();

#pragma region GameState
int UnitCount[2] = { 0,0 }; //SCV,Marines, Medics, etc
int maxUnit[2] = { 50,150 }; //SCV,Marines, Medics, etc
int BuildingCount[3] = { 0,0,0 }; //CC, supplydepots, barracks
int maxBuilding[3] = { 3,20,4 }; //CC, supplydepots, barracks
int deadSCVs = 0;
int TaskCount = 0; //unique Task ID
list<array<int, 7>> taskQueue; // 0=timeStamp,1=callbacktime,2=action,3=SCVID or build, 4=taskOwner,5=status, 6=uniqueID
#pragma endregion

#pragma region UnitLists
list<Unit> commandCenters;
list<Unit> supplyDepots;

list<int> Miners; //could be replace byd an array
list<int> Builders; //could be replaced by an array
list<int> deadUnits;
#pragma endregion

#pragma region uniqueUnits
UnitType supplyProviderType = UnitTypes::Terran_Supply_Depot;
#pragma endregion

#pragma region MaxCount
int supplyLimit = 0; // max supply 200
int supplyLeft = 0; //how many units can I support yet
int roomForProduction = 0; //how much before I need to build supply depots
int virtualBudget = 0;
//add an array parallel to the unit array that should be a regulator array, the target array
#pragma endregion

#pragma region Boolean Variables
bool almostSupplyBlocked = false; //when true AI needs to build supplydepots pre emptively
bool displayStats = false;
#pragma endregion

//methods
#pragma region ExtraFunctions
int readSettingsFile()
{
    ofstream myfile;
    myfile.open("Pikachu.txt");
    myfile << "Writing this to a file.\n";
    myfile.close();
    return 0;
}

int readSettings()
{
    string line;
    ifstream myfile("Pikachu.txt");
    if (myfile.is_open())
    {
        while (getline(myfile, line))
        {
            //cout << line << '\n';
            Broodwar->sendText(line.c_str());
        }
        myfile.close();
    }

    else Broodwar->sendText("Unable to open file");//cout << "Unable to open file";
    return 0;
}

void MouseMove(int x, int y)
{
    double fScreenWidth = GetSystemMetrics(SM_CXSCREEN) - 1;
    double fScreenHeight = GetSystemMetrics(SM_CYSCREEN) - 1;
    double fx = x * (65535.0f / fScreenWidth);
    double fy = y * (65535.0f / fScreenHeight);
    INPUT Input = { 0 };
    Input.type = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_MOVE;
    //Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    Input.mi.dx = fx;
    Input.mi.dy = fy;
    ::SendInput(1, &Input, sizeof(INPUT));
}

void LeftClick()
{
    INPUT    Input = { 0 };
    // left down 
    Input.type = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    ::SendInput(1, &Input, sizeof(INPUT));

    // left up
    ::ZeroMemory(&Input, sizeof(INPUT));
    Input.type = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    ::SendInput(1, &Input, sizeof(INPUT));
}

void RightClick()
{
    INPUT    Input = { 0 };
    // left down 
    Input.type = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    ::SendInput(1, &Input, sizeof(INPUT));

    // left up
    ::ZeroMemory(&Input, sizeof(INPUT));
    Input.type = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    ::SendInput(1, &Input, sizeof(INPUT));
}

#pragma endregion

#pragma region SupplyInfoMethods

void updateUnitCount(bool created, Unit unit)
{   
    UnitType myType = unit->getType();
    char result[100];
    strcpy(result, "new: ");
    strcat(result, myType.c_str());
    if (created)
    {
        if (displayStats)
        {
            //Broodwar->sendText(result); //strcat crashing broodwar
            result[0] = '\0'; //clear
        }
        if (myType == UnitTypes::Terran_Barracks)
        {
            BuildingCount[2] = BuildingCount[2] + 1;
            //barracks.push_back(unit);
        }
        if (myType == UnitTypes::Terran_Marine)
        {
            UnitCount[1] = UnitCount[1] + 1;

            //marines.push_back(unit); //how to remove the dead ones?
        }
        if (myType == UnitTypes::Terran_SCV)
        {
            UnitCount[0] = UnitCount[0] + 1;
            //workers.push_back(unit);
            Miners.push_back(unit->getID());//assign to mine
        }
        if (myType == UnitTypes::Terran_Supply_Depot)
        {
            BuildingCount[1] = BuildingCount[1] + 1;
            supplyDepots.push_back(unit);
        }
        if (myType == UnitTypes::Terran_Command_Center)
        {
            BuildingCount[0] = BuildingCount[0] + 1;
        }
        
    }
    else
    {
        char result2[100];
        strcpy(result2, "dead: ");
        strcat(result2, myType.c_str());
        if (displayStats)
        {
            //Broodwar->sendText(result2);
        }

        if (myType == UnitTypes::Terran_Barracks)
        {
            BuildingCount[2] = BuildingCount[2] - 1;
        }
        if (myType == UnitTypes::Terran_Marine)
        {
            UnitCount[1] = UnitCount[1] - 1;
        }
        if (myType == UnitTypes::Terran_SCV)
        {
            UnitCount[0] = UnitCount[0] - 1;
        }
        if (myType == UnitTypes::Terran_Supply_Depot)
        {
            BuildingCount[1] = BuildingCount[1] - 1;
        }
        if (myType == UnitTypes::Terran_Command_Center)
        {
            BuildingCount[0] = BuildingCount[0] - 1;
        }
        
    }

    supplyLeft = Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed(); //x2
    
}
void displayInsights()
{
    int supplyLeft2 = Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed();
    Broodwar->drawTextScreen(0, 0, "FPS!: %d", Broodwar->getFPS());
    //Broodwar->drawTextScreen(200, 20, "Average FPS: %f", Broodwar->getAverageFPS());
    Broodwar->drawTextScreen(45, 0, "Brks: %d", BuildingCount[2]);
    Broodwar->drawTextScreen(90, 0, "Mrn: %d", UnitCount[1]);
    Broodwar->drawTextScreen(135, 0, "SCV: %d", UnitCount[0]);
    Broodwar->drawTextScreen(180, 0, "Bldrs: %d", Builders.size());
    Broodwar->drawTextScreen(225, 0, "Dead: %d ", deadUnits.size());
    //Broodwar->drawTextScreen(200, 100, "Mouse Cursor: %d  %d", Broodwar->getMousePosition().x, Broodwar->getMousePosition().y);
    //Broodwar->drawTextScreen(200, 120, "Screen: %d  %d", Broodwar->getScreenPosition().x, Broodwar->getScreenPosition().y);
    
    Broodwar->drawTextScreen(0, 10, "supply limit: %d ", supplyLeft);
    Broodwar->drawTextScreen(0, 20, "supply limit2: %d ", supplyLeft2);
    Broodwar->drawTextScreen(0, 30, "room for next round: %d ", auxFun::roomNeeded(BuildingCount[0], BuildingCount[2]));    
    Broodwar->drawTextScreen(0, 40, "Tasks: %d ", taskQueue.size());
    
}

#pragma endregion

#pragma region TasksFunctions
void assessTask(array<int, 7>& newTask)
{
    //ok new task, what do you want?
    //do we have resources to complete your task?
    // what priority should I give you?
}

void CreateTask(list<array<int, 7>> &myTaskQueue, int timeStamp, int taskOwner, int action)
{       
    array<int, 7> newArray{ timeStamp,0,action,0,taskOwner,(int)taskStatus::Created, TaskCount };

    myTaskQueue.push_back(newArray);

    TaskCount = TaskCount + 1;
}

void callBack(array<int, 7>& Task, int When, int Why)
{
    //do we have resources? assign, no? set callback time
    Task[0] = Broodwar->getFrameCount(); //reset timer
    Task[1] = When; //frames, should be determined by the income rate, (miners working)
    Task[5] = Why;
}

void startTask(array<int, 7> &Task, Unit builder, TilePosition targetBuildLocation)
{
    //Unit builder = UnitFun::returnUnitByID(Broodwar->self()->getUnits(), Task[3]); //the task has an available worker assigned
    //int builderID = builder->getID();
    //TilePosition targetBuildLocation; //TODO: improve space allocation
    //targetBuildLocation = Broodwar->getBuildLocation(UnitTypes::Terran_Supply_Depot, builder->getTilePosition());
    //TODO: better target location method needed
    
    switch (Task[2])
    {

        case (int)action::BuildSupplyDepot:

            BuildManager::buildBuilding(builder, UnitTypes::Terran_Supply_Depot, Colors::Blue, targetBuildLocation);
        
            break; //optional
        case (int)action::BuildBarrack:

            BuildManager::buildBuilding(builder, UnitTypes::Terran_Barracks, Colors::Green, targetBuildLocation);
        
            break; //optional

         // you can have any number of case statements.
        default: //Optional
            //statement(s);
            break;
    }

    callBack(Task, 200, (int)taskStatus::PendingStart); //things can happen during travel time
}

TilePosition returnBuildPosition(int action, Unit SCV)
{
    TilePosition myBuildingLocation;
    switch (action)
    {
        case (int)action::BuildSupplyDepot:

            myBuildingLocation = Broodwar->getBuildLocation(UnitTypes::Terran_Supply_Depot, SCV->getTilePosition());
            break; //optional
        case (int)action::BuildBarrack:

            myBuildingLocation = Broodwar->getBuildLocation(UnitTypes::Terran_Barracks, SCV->getTilePosition());
            break; //optional

         // you can have any number of case statements.
        default: //Optional
            myBuildingLocation = Broodwar->getBuildLocation(UnitTypes::Terran_Barracks, SCV->getTilePosition());
            //statement(s);
            break;
    }
    return myBuildingLocation;
}

#pragma endregion

#pragma region MainMethods

void initialAssigment(Unitset units)
{
    for (auto& u : units)
    {
        if (auxFun::validUnit(u, deadUnits))
        {
            if (u->getType().isWorker())
            {
                //workers.push_back(u);
                //Miners.push_back(u->getID());
            }
            if (u->getType().isResourceDepot())
            {
                commandCenters.push_back(u);
            }
        }
    }
}

void productionManager()
{    
    almostSupplyBlocked = false;
    int roomNeeded = auxFun::roomNeeded(BuildingCount[0], BuildingCount[2]);
    
    if (supplyLeft < roomNeeded) { //instead of 4,should be the max output of production at a given time

        almostSupplyBlocked = true;
    }

    CommMngr::scvManager(Miners);//go mine for me minions!
    if (!almostSupplyBlocked && !TaskFun::tasksWaitingResources(taskQueue))
    {
        if (UnitCount[0] < maxUnit[0])
        {
            CommMngr::buildSCVs(commandCenters);//train SCVs until 50
        }
        
        CommMngr::trainMarines(UnitFun::getUnitList(UnitTypes::Terran_Barracks, Broodwar->self()->getUnits(),deadUnits));//pump marines

        if (Broodwar->self()->minerals() >= UnitTypes::Terran_Barracks.mineralPrice() && 
            (BuildingCount[2] < maxBuilding[2]) 
            && (UnitCount[0] > 10))
        {            
            //Test isMyTaskInQueue
            if (!TaskFun::TaskQueued(taskQueue,(int)taskOwner::ProductionManager, (int)action::BuildBarrack))
            {
                CreateTask(taskQueue, Broodwar->getFrameCount(), (int)taskOwner::ProductionManager,(int)action::BuildBarrack);
            }            
        }
    }
    else
    {
        if (!TaskFun::TaskQueued(taskQueue, (int)taskOwner::ProductionManager, (int)action::BuildSupplyDepot) && Broodwar->self()->supplyTotal() != 400)
        {
            for (int i = 0; i < roomNeeded; i+=7)
            {
                CreateTask(taskQueue, Broodwar->getFrameCount(), (int)taskOwner::ProductionManager, (int)action::BuildSupplyDepot);
            }            
        }                
    }
}

void taskManager(list<array<int, 7>> &myTaskQueue)
{
    //if my task status is 0, not started, check timestamp
    //0 created, 1 reviewed but no resources to assign, 2 assigned with resources, 3 started, 4 completed, 5 cancel
    //production manager checks if there are tasks waiting for minerals before pumping units
    //assessTask(task) will determine wich task has high priority, for now if resources available just start them
    for (auto& task : myTaskQueue)
    {
        int taskStatus = task[5];
        if (Broodwar->getFrameCount() > (task[0] + task[1])
            && taskStatus != (int)taskStatus::Started
            && taskStatus != (int)taskStatus::Completed
            && taskStatus != (int)taskStatus::Cancelled
            && taskStatus != (int)taskStatus::PendingStart)
        {
            if (TaskFun::mineralsAvailable(task, Broodwar->self()->minerals()) && 
                TaskFun::gasAvailable(task, Broodwar->self()->gas())) //and location available
            {
                //assign SCV
                Unit SCV = UnitFun::getWorker(commandCenters.front(), Miners, Builders, supplyProviderType);
                int id = SCV->getID();

                TilePosition targetBuildLocation = returnBuildPosition(task[2],SCV);
                //pass the worker id to the task
                task[3] = id; 

                startTask(task, SCV, targetBuildLocation);
            }
            else
            {
                //do we have resources? assign, no? set callback time
                callBack(task, 200, (int)taskStatus::waitingMin);                
            }
        }
        else if (Broodwar->getFrameCount() > (task[0] + task[1]) && taskStatus == (int)taskStatus::PendingStart)
        {
            task[5] = (int)taskStatus::Cancelled;
            Broodwar->sendText("TaskMngr: Failed to start task id: %d : ", task[6]); //for some error            
        }
    }
}

#pragma endregion

#pragma region MainEvents

void ExampleAIModule::onFrame()
{
    // Called once every game frame
    // Display the game frame rate as text in the upper left area of the screen
    if (displayStats)
    {
        displayInsights();
    }
    if (auxFun::validFrame())
    {
        taskManager(taskQueue);
        productionManager(); //eventually productionManager will be another task run by taskManager    
    }

    if (GetKeyState('A') & 0x8000/*Check if high-order bit is set (1 << 15)*/)
    {
        Position myPos = auxFun::getMousePosition();
        CommMngr::attackUnits(UnitFun::getUnitList(UnitTypes::Terran_Marine, Broodwar->self()->getUnits(), deadUnits), myPos);
        // Do stuff
    }
    if (GetKeyState('Q') & 0x8000/*Check if high-order bit is set (1 << 15)*/)
    {
        Position myPos = auxFun::getMousePosition();
        //CommMngr::attackUnits(UnitFun::getListofUnitType(UnitTypes::Terran_Marine, Broodwar->self()->getUnits(), deadUnits), myPos);

        Unitset myUnits = Broodwar->getSelectedUnits();
        for (auto& u : myUnits)
        {
            //int x = u->getTilePosition().x;
            //int y = u->getTilePosition().y;
            //Broodwar->printf("x: %d y: %d", x, y);

            LeftClick();// I cant believe this works!
            //u->rightClick(auxFun::getMousePosition(), false);

        }
        // Do stuff
    }

    if (GetKeyState('W') & 0x8000/*Check if high-order bit is set (1 << 15)*/)
    {
        Position myPos = auxFun::getMousePosition();
        //CommMngr::attackUnits(UnitFun::getListofUnitType(UnitTypes::Terran_Marine, Broodwar->self()->getUnits(), deadUnits), myPos);

        Unitset myUnits = Broodwar->getSelectedUnits();

        RightClick();// I cant believe this works!
        
        for (auto& u : myUnits)
        {
            //int x = u->getTilePosition().x;
            //int y = u->getTilePosition().y;
            //Broodwar->printf("x: %d y: %d", x, y);

            
            //u->rightClick(auxFun::getMousePosition(), false);

        }
        // Do stuff
    }
}

void ExampleAIModule::onUnitCreate(Unit unit)
{

    if (IsOwned(unit))
    {
        updateUnitCount(true, unit);

        if (!taskQueue.empty() && !unit->canAttack()
            && unit->getType() != UnitTypes::Terran_SCV
            && unit->getType() != UnitTypes::Terran_Marine)
        {
            Unit builder = unit->getBuildUnit();
            int builderID = builder->getID();
            int BuildingID = unit->getID();


            array<int, 7> mytask = {0,0,0,0,0,0,0};
            /*array<int, 7>* pointer = TaskFun::findTaskAssignedToUnit(builderID, taskQueue);
            if (pointer != nullptr)
            {
                mytask = *pointer;
            }*/

            
            //only for buildings
            if (TaskFun::taskStatusUpdate(builderID, taskQueue, BuildingID, (int)taskStatus::Started))
            {
                Broodwar->sendText("Started task id: %d : %s", mytask[6], unit->getType().c_str());
            }
            else
            {
                Broodwar->sendText("Failed update to started task id: %d : %s", unit->getID(), unit->getType().c_str());
            }
        }
    }


        
    if (Broodwar->isReplay())
    {
        // if we are in a replay, then we will print out the build order of the structures
        if (unit->getType().isBuilding() && !unit->getPlayer()->isNeutral())
        {
            int seconds = Broodwar->getFrameCount() / 24;
            int minutes = seconds / 60;
            seconds %= 60;
           // Broodwar->sendText("%.2d:%.2d: %s creates a %s", minutes, seconds, unit->getPlayer()->getName().c_str(), unit->getType().c_str());
        }
    }
}

void ExampleAIModule::onUnitComplete(Unit unit)
{
    if (IsOwned(unit))
    {
        if (!taskQueue.empty() && !unit->canAttack()) //avoid running in buildings
        {
            array<int, 7> mytask = { 0,0,0,0,0,0,0 };
            /*array<int, 7>* pointer = TaskFun::findTaskAssignedToUnit(unit->getID(), taskQueue);
            if (pointer != nullptr)
            {
                mytask = *pointer;
            }*/

            if (TaskFun::taskStatusUpdate(unit->getID(), taskQueue, unit->getID(), (int)taskStatus::Completed))
            {
                Broodwar->sendText("Completed task id: %d : %s", mytask[6], unit->getType().c_str());
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
        updateUnitCount(false, unit);
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
        //Position myPos(Broodwar->getScreenPosition().x + Broodwar->getMousePosition().x, Broodwar->getScreenPosition().y + Broodwar->getMousePosition().y);
        for (auto& u : Miners)
        {            
            //Unit miner = UnitFun::getUnitByID(workers, u);
            Unit miner = Broodwar->getUnit( u);
            miner->move(myPos);
            miner->stop(true);
            miner->gather(miner->getClosestUnit(IsMineralField),true); //the everyframe logic messes up this
        }
    }
    if (text == "w")
    {
        Unit builder = UnitFun::getBuilder(Builders);
        builder->move(myPos);
        builder->build(UnitTypes::Terran_Command_Center, builder->getTilePosition());
    }
    if (text == "e")
    {
        Unit builder = UnitFun::getBuilder(Builders);
        builder->move(myPos);
        builder->build(UnitTypes::Terran_Engineering_Bay, builder->getTilePosition());
    }
    if (text == "read")
    {
        readSettings();
    }
    if (text == "u")
    {
        //Broodwar->canBuildHere
        //Broodwar->elapsedTime
        //Broodwar->getAPM
        //Broodwar->getSelectedUnits
        //Broodwar->getUnit // need to implemet
        //Broodwar->hasPath
        //Broodwar->isExplored
        //Broodwar->issueCommand
        //Broodwar->mapWidth
        //Broodwar->pingMinimap
        //Broodwar->printf        

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

            LeftClick();// I cant believe this works!
            //u->rightClick(auxFun::getMousePosition(), false);
            
        }

        
    }
    // Send the text to the game if it is not being processed.
    Broodwar->sendText("%s", text.c_str());

    


    // Make sure to use %s and pass the text as a parameter,
    // otherwise you may run into problems when you use the %(percent) character!

}

void ExampleAIModule::onStart()
{
    readSettingsFile();
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
    if (Broodwar->isReplay())
    {

        // Announce the players in the replay
        Broodwar << "The following players are in this replay:" << endl;

        // Iterate all the players in the game using a  iterator
        Playerset players = Broodwar->getPlayers();
        for (auto p : players)
        {
            // Only print the player if they are not an observer
            if (!p->isObserver())
                Broodwar << p->getName() << ", playing as " << p->getRace() << endl;
        }

    }
    else // if this is not a replay
    {
        initialAssigment(Broodwar->self()->getUnits()); //assign my firt units, 4 workers, CC
        // Retrieve you and your enemy's races. enemy() will just return the first enemy.
        // If you wish to deal with multiple enemies then you must use enemies().
        if (Broodwar->enemy()) // First make sure there is an enemy
            Broodwar << "The matchup is " << Broodwar->self()->getRace() << " vs " << Broodwar->enemy()->getRace() << endl;
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

//not useful ... yet
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


