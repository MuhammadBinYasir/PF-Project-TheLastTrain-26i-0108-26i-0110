#include <iostream>
#include "simulator.h"
#include "physics.h"
#include "switches.h"
#include "logger.h"
#include "viewer_iface.h"
#include "util.h"
#include <string>
#include <iomanip>
#include "level.h"
using namespace std;

// Modular simulation loop using primitive variables

void dispalyMenu()
{
    cout << "\033[34m"
         << "==============================================================================================================\n"
         << "=============     = === =     ======= ======   ===    =     =======     =    ===   ==     = === ==============\n"
         << "=============== === === = =========== ===== === = ======= =========== === === = === === ===  == ==============\n"
         << "=============== === === = =========== ===== === = ======= =========== === === = === === === = = ==============\n"
         << "=============== ===     =    ======== =====     ==   ==== =========== ===    ==     === === = = ==============\n"
         << "=============== === === = =========== ===== === ===== === =========== === = === === === === ==  ==============\n"
         << "=============== === === = =========== ===== === ===== === =========== === == == === === === === ==============\n"
         << "=============== === === =     =======     = === =    ==== =========== === === = === =     = === ==============\n"
         << "==============================================================================================================\n"
         << "\033[0m";

    cout << endl
         << "1. Start New Game" << endl;
    cout << "2. Load Saved Game" << endl;
    cout << "3. Instructions Game" << endl;
    cout << "4. Exit Game" << endl
         << endl;

    cout << "\033[34m"
         << endl
         << string(100, '=') << "\033[0m" << endl
         << endl;
}

int usersMenu()
{
    int userChoice;
    dispalyMenu();
    while (true)
    {
        cout << "\033[35mEnter Your Choice > \033[0m";

        cin >> userChoice;
        if (cin.fail() || userChoice < 1 || userChoice > 4)
        {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "\033[31mInvalid Input. Please enter correct value.\033[0m" << endl
                 << endl;
            continue;
        }

        return userChoice;
    }
}

bool initializeNewGame(
    int &playerFloor, int &playerRoom,
    int &energy, int &time, int &score,
    char inventory[MAX_INVENTORY_CAPACITY], int &inventoryCount,
    bool objectives[NUM_OBJECTIVES], int &emergencyTripsLeft,
    char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
    char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
    int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    if (!loadBuildingMap(
            "data/building.txt",
            horizontalConn, roomEntities, entityCounts))
    {
        return false;
    }

    if (!loadInitialState(
            "data/initial-state.txt",
            playerFloor, playerRoom,
            energy, time, score,
            inventory, inventoryCount,
            objectives, emergencyTripsLeft))
    {
        return false;
    }

    return true;
}

bool returnMenuFunction()
{
    int in;
    while (true)
    {
        cout << "\033[35mEnter -1 to Move Back to Main Menu > \033[0m";
        cin >> in;
        if (in != -1 || cin.fail())
        {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "\033[31mInvalid Input. Please Enter Valid Answer.\033[0m\n";
            continue;
        }
        break;
    }
    return true;
}

bool handleMoveCommand(int targetFloor, int targetRoom,
                       int &currentFloor, int &currentRoom,
                       int &energy, int &time,
                       const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                       const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                       int &emergencyTripsLeft, bool lift2Operational)
{

    char outConnType;
    bool isDirectlyConnect = isDirectlyConnected(currentFloor, currentRoom, targetFloor, targetRoom, horizontalConn, outConnType);
    if (isDirectlyConnect)
    {
        bool Traverse = canTraverse(targetFloor, targetRoom, outConnType, inventory, inventoryCount, emergencyTripsLeft, lift2Operational);
        if (Traverse)
        {
            if (outConnType == CONN_DOOR || outConnType == CONN_LOCKED_DOOR || outConnType == CONN_STAIRS)
            {
                applyResourceDelta(energy, time, COST_MOVE_ORDINARY_ENERGY, COST_MOVE_ORDINARY_TIME);
            }
            else if (outConnType == CONN_WINDOW)
            {
                applyResourceDelta(energy, time, COST_MOVE_WINDOW_ENERGY, COST_MOVE_WINDOW_TIME);
            }
            else if (outConnType == CONN_LIFT1 || outConnType == CONN_LIFT2)
            {
                applyResourceDelta(energy, time, COST_MOVE_LIFT_ENERGY, COST_MOVE_LIFT_TIME);
            }

            if (outConnType == CONN_LIFT2 && !lift2Operational)
                emergencyTripsLeft--;

            currentFloor = targetFloor;
            currentRoom = targetRoom;
            return true;
        }
    }
    return false;
}

bool handleInspectCommand(int currentFloor, int currentRoom,
                          int &energy, int &time,
                          const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                          int entityCount,
                          bool objectives[NUM_OBJECTIVES], int &score)
{
    if (currentFloor < 1 || currentFloor > NUM_FLOORS || currentRoom < 1 || currentRoom > ROOMS_PER_FLOOR || entityCount < 0 || entityCount > MAX_ENTITIES_PER_ROOM)
        return false;

    applyResourceDelta(energy, time, COST_INSPECT_ENERGY, COST_INSPECT_TIME);

    cout << endl;

    if (energy <= 0 || time <= 0)
    {
        return true;
    }

    renderRoomDetails(currentFloor, currentRoom, roomEntities, entityCount);

    for (int i = 0; i < entityCount; i++)
    {
        char symbol = roomEntities[currentFloor - 1][currentRoom - 1][i];
        int c_Index = -1;

        if (symbol == ENT_CLUE_4)
            c_Index = OBJ_CLUE_4;
        else if (symbol == ENT_CLUE_2)
            c_Index = OBJ_CLUE_2;
        else if (symbol == ENT_CLUE_7)
            c_Index = OBJ_CLUE_7;

        if (c_Index != -1 && !objectives[c_Index])
        {
            objectives[c_Index] = true;
            applyScoreDelta(score, SCORE_CLUE_FOUND);

            cout << "\033[32mClue " << symbol << " discovered! +"
                 << SCORE_CLUE_FOUND << " points.\033[0m" << endl;
        }
    }

    return true;
}

void runSimulation(int &playerFloor, int &playerRoom,
                   int &energy, int &time, int &score,
                   char inventory[MAX_INVENTORY_CAPACITY], int &inventoryCount,
                   bool objectives[NUM_OBJECTIVES], int &emergencyTripsLeft,
                   char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                   char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                   int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{

    while (true)
    {
        int userChoice = usersMenu();

        switch (userChoice)
        {
        case 1:
        {
            int floor, room;
            bool initialization = initializeNewGame(
                playerFloor, playerRoom,
                energy, time, score,
                inventory, inventoryCount,
                objectives, emergencyTripsLeft,
                horizontalConn, roomEntities, entityCounts);

            if (!initialization)
            {
                cout << "\033[31mCould not initialize the new game.\033[0m" << endl;
                break;
            }
            else
            {
                cout << endl
                     << "\033[32m-> Game initialization Successfully!\033[0m" << endl
                     << endl;
            }
            while (true)
            {

                cout << "\033[35mEnter Floor (1-5) > \033[0m";
                cin >> floor;
                if (floor < 1 || floor > 5 || cin.fail())
                {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cout << "\033[31mInvalid Input. Please Enter Valid Answer.\033[0m" << endl;
                    continue;
                }
                cout << "\033[35mEnter Starting Room (1-4) > \033[0m";
                cin >> room;

                if (room < 1 || room > 4 || cin.fail())
                {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cout << "\033[31mInvalid Input. Please Enter Valid Answer.\033[0m" << endl;
                    continue;
                }

                if (validateStartingRoom(floor, room, horizontalConn))
                {
                    playerFloor = floor;
                    playerRoom = room;

                    cout << endl
                         << "\033[32m-> We are spawning you to " << "F" << floor << "-R" << room << "!\033[0m" << endl
                         << endl;
                    break;
                }
                else
                {
                    cout << "\033[31m->Invalid Room Number. Enterance Not Allowed.\033[0m\n";
                    continue;
                }
            }

            bool playing = true;
            while (playing)
            {
                clearScreen();

                cout << "What You can Do? " << endl;
                cout << "1. Move" << endl
                     << "2. Inspect room" << endl
                     << "3. Collect item" << endl
                     << "4. Use item / interact" << endl
                     << "5. Show current state" << endl
                     << "6. Show map" << endl
                     << "7. Show objectives" << endl
                     << "8. Save game" << endl
                     << "0. Return to main menu" << endl;

                int in;
                while (true)
                {
                    cout << "\033[35mEnter What you want to do > \033[0m";
                    cin >> in;
                    if (in < 0 || in > 8 || cin.fail())
                    {
                        cin.clear();
                        cin.ignore(10000, '\n');
                        cout << "\033[31mInvalid Input. Please Enter Valid Answer.\033[0m" << endl;
                        continue;
                    }
                    break;
                }

                switch (in)
                {
                case 0:
                    playing = false;
                    clearScreen();
                    break;
                case 1:
                {
                    int floor, room;
                    cout << "\033[35mEnter Destination Floor (1-5) > \033[0m";
                    cin >> floor;
                    if (cin.fail())
                    {
                        cin.clear();
                        cin.ignore(10000, '\n');
                        cout << "\033[31mInvalid Input. Please Enter Valid Answer.\033[0m" << endl;
                        continue;
                    }
                    cout << "\033[35mEnter Destination Room (1-4) > \033[0m";
                    cin >> room;

                    if (cin.fail())
                    {
                        cin.clear();
                        cin.ignore(10000, '\n');
                        cout << "\033[31mInvalid Input. Please Enter Valid Answer.\033[0m" << endl;
                        continue;
                    }

                    bool movement = handleMoveCommand(floor, room, playerFloor, playerRoom, energy, time, horizontalConn, inventory, inventoryCount, emergencyTripsLeft, objectives[OBJ_FUSE_INSTALLED]);
                    if (movement)
                    {
                        cout << endl
                             << "\033[32m-> MOVED! You are Now at F" << playerFloor << "-R" << playerRoom << ". \033[0m" << endl
                             << endl;
                    }
                    else
                    {
                        cout << "\033[31mInvalid Input. You cannot Move to that room!.\033[0m" << endl;
                    }

                    if (energy <= 0 || time <= 0)
                    {
                        objectives[OBJ_MISSION_FAILED] = true;
                        playing = false;

                        cout << endl
                             << "\033[31m";
                        cout << "MISSION FAILED!" << endl;

                        if (energy <= 0)
                        {
                            cout << "Your Energy has run out." << endl;
                        }

                        if (time <= 0)
                        {
                            cout << "Your Time has run out." << endl;
                        }

                        cout << "\033[0m";
                    }

                    returnMenuFunction();
                    break;
                }
                case 2:
                { // renderRoomDetails(playerFloor, playerRoom, roomEntities, entityCounts[playerFloor - 1][playerRoom - 1]);
                    bool res = handleInspectCommand(playerFloor, playerRoom, energy, time, roomEntities, entityCounts[playerFloor - 1][playerRoom - 1], objectives, score);

                    if (!res)
                    {
                        cout << "\033[31mCould not inspect this room.\033[0m"
                             << endl;
                    }
                    else if (res && (energy <= 0 || time <= 0))
                    {
                        objectives[OBJ_MISSION_FAILED] = true;
                        playing = false;

                        cout << endl
                             << "\033[31m";
                        cout << "MISSION FAILED!" << endl;

                        if (energy <= 0)
                        {
                            cout << "Your Energy has run out." << endl;
                        }

                        if (time <= 0)
                        {
                            cout << "Your Time has run out." << endl;
                        }
                    }

                    returnMenuFunction();
                    break;
                }
                case 5:
                    renderDashboard(playerFloor, playerRoom, energy, time, score, inventory, inventoryCount, emergencyTripsLeft);
                    returnMenuFunction();
                    break;
                case 6:
                    renderAsciiBuilding(playerFloor, playerRoom);
                    returnMenuFunction();
                    break;
                case 7:
                    renderObjectivesChecklist(objectives);
                    returnMenuFunction();
                    break;
                }
            }

            // renderDashboard(playerFloor, playerRoom, energy, time, score, inventory, inventoryCount, emergencyTripsLeft);
            // renderRoomDetails(playerFloor, playerRoom, roomEntities, entityCounts[playerFloor - 1][playerRoom - 1]);
            // renderAsciiBuilding(playerFloor, playerRoom);
            // renderObjectivesChecklist(objectives);
            break;
        }
        case 2:
            cout << "Loading Saved Game...." << endl;
            break;
        case 3:
        {
            cout << endl
                 << "MISSION OBJECTIVES" << endl;
            cout << endl
                 << "1. Rescue the passenger and cat." << endl;
            cout << "2. Find clues and unlock the cabinet." << endl;
            cout << "3. Restore the lift and board the train." << endl;

            clearScreen();
            returnMenuFunction();

            break;
        }

        case 4:
            cout << "Thank you for playing The Last Train!" << endl;
            return;

        default:
            cout << "Invalid Choice. Please Enter Valid Answer" << endl;
            dispalyMenu();
            break;
        }
    }
}