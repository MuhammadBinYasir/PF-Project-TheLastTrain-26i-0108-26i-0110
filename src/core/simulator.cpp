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

const int BOX_WIDTH = 110;

void printBorder(const string &color = "\033[33m")
{
    cout << color
         << "+" << string(BOX_WIDTH, '-') << "+"
         << "\033[0m" << endl;
}

void printBoxLine(const string &text, const string &color = "\033[33m")
{
    cout << color
         << "| " << left << setw(BOX_WIDTH - 2) << text << " |"
         << "\033[0m" << endl;
}

void dispalyMenu()
{
    // cout << "\033[34m"
    //      << "==============================================================================================================\n"
    //      << "=============     = === =     ======= ======   ===    =     =======     =    ===   ==     = === ==============\n"
    //      << "=============== === === = =========== ===== === = ======= =========== === === = === === ===  == ==============\n"
    //      << "=============== === === = =========== ===== === = ======= =========== === === = === === === = = ==============\n"
    //      << "=============== ===     =    ======== =====     ==   ==== =========== ===    ==     === === = = ==============\n"
    //      << "=============== === === = =========== ===== === ===== === =========== === = === === === === ==  ==============\n"
    //      << "=============== === === = =========== ===== === ===== === =========== === == == === === === === ==============\n"
    //      << "=============== === === =     =======     = === =    ==== =========== === === = === =     = === ==============\n"
    //      << "==============================================================================================================\n"
    //      << "\033[0m";

    printBorder();

    cout << "\033[30;43m" << setw(BOX_WIDTH) << ' ' << "\033[0m" << endl;

    cout << "\033[30;43m" << left << setw(BOX_WIDTH)
         << " ##### #   # #####     #       ###    ####  #####     ##### ####    ###   ##### #   # "
         << "\033[0m" << endl;

    cout << "\033[30;43m" << left << setw(BOX_WIDTH)
         << "   #   #   # #         #      #   #  #        #         #   #   #  #   #    #   ##  # "
         << "\033[0m" << endl;

    cout << "\033[30;43m" << left << setw(BOX_WIDTH)
         << "   #   ##### ####      #      #####   ###     #         #   ####   #####    #   # # # "
         << "\033[0m" << endl;

    cout << "\033[30;43m" << left << setw(BOX_WIDTH)
         << "   #   #   # #         #      #   #      #    #         #   #  #   #   #    #   #  ## "
         << "\033[0m" << endl;

    cout << "\033[30;43m" << left << setw(BOX_WIDTH)
         << "   #   #   # #####     #####  #   #  ####     #         #   #   #  #   #  ##### #   # "
         << "\033[0m" << endl;

    cout << "\033[30;43m" << setw(BOX_WIDTH) << ' ' << "\033[0m" << endl;

    printBorder();

    printBoxLine("1. Start New Game                      2. Load Saved Game");
    cout << endl;
    printBoxLine("3. Game Instructions");
    cout << endl;
    printBoxLine("4. Exit From Game");
    printBorder();
}

int usersMenu()
{
    int userChoice;
    dispalyMenu();
    while (true)
    {
        cout << "\033[35mEnter Your Choice > \033[0m";

        cin >> userChoice;
        if (cin.fail() && cin.eof())
            return 4;
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
        cout << "\033[35mEnter -1 to Continue > \033[0m";
        cin >> in;
        if (cin.fail() && cin.eof())
            return false;
        if (cin.fail() || in != -1)
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

bool handleCollectCommand(char itemSymbol, int currentFloor, int currentRoom,
                          int &energy, int &time, int &score,
                          char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                          int &entityCount,
                          char inventory[MAX_INVENTORY_CAPACITY], int &inventoryCount,
                          bool objectives[NUM_OBJECTIVES], int &emergencyTripsLeft)
{

    if (currentFloor < 1 || currentFloor > NUM_FLOORS || currentRoom < 1 || currentRoom > ROOMS_PER_FLOOR || entityCount < 0 || entityCount > MAX_ENTITIES_PER_ROOM || inventoryCount < 0 || inventoryCount > MAX_INVENTORY_CAPACITY)
        return false;

    int itemIndex = -1;
    for (int i = 0; i < entityCount; i++)
    {
        if (roomEntities[currentFloor - 1][currentRoom - 1][i] == itemSymbol)
        {
            itemIndex = i;
            break;
        }
    }

    if (itemIndex == -1)
        return false;

    if (itemSymbol == ENT_MASTER_KEY || itemSymbol == ENT_FUSE || itemSymbol == ENT_LIFT_PASS || itemSymbol == ENT_MEDICAL_KIT || itemSymbol == ENT_EMERGENCY_CARD || itemSymbol == ENT_CAT_CARRIER || itemSymbol == ENT_CABINET_TRAIN_KEY)
    {
        if (inventoryCount < 0 || inventoryCount >= MAX_INVENTORY_CAPACITY)
            return false;

        if (itemSymbol == ENT_CABINET_TRAIN_KEY && !objectives[OBJ_CABINET_UNLOCKED])
        {
            return false;
        }

        if (!addItem(inventory, inventoryCount, itemSymbol))
            return false;
        applyResourceDelta(energy, time, COST_ITEM_PICKUP_ENERGY, COST_ITEM_PICKUP_TIME);
    }
    else if (itemSymbol != ENT_ENERGY_PACK && itemSymbol != ENT_EMERGENCY_TOOLKIT && itemSymbol != ENT_OBSERVATION_TOKEN)
        return false;

    for (int i = itemIndex; i < entityCount - 1; i++)
    {
        roomEntities[currentFloor - 1][currentRoom - 1][i] = roomEntities[currentFloor - 1][currentRoom - 1][i + 1];
    }

    entityCount--;
    roomEntities[currentFloor - 1][currentRoom - 1][entityCount] = ENT_NONE;

    if (itemSymbol == ENT_MASTER_KEY)
    {
        objectives[OBJ_MASTER_KEY] = true;
        applyScoreDelta(score, SCORE_KEY_COLLECT);
    }
    else if (itemSymbol == ENT_FUSE)
    {
        objectives[OBJ_FUSE_COLLECTED] = true;
    }
    else if (itemSymbol == ENT_LIFT_PASS)
    {
        objectives[OBJ_LIFT_PASS] = true;
        applyScoreDelta(score, SCORE_LIFT_PASS);
    }
    else if (itemSymbol == ENT_CAT_CARRIER)
    {
        objectives[OBJ_CAT_CARRIER] = true;
    }
    else if (itemSymbol == ENT_CABINET_TRAIN_KEY)
    {
        objectives[OBJ_TRAIN_KEY] = true;
        applyScoreDelta(score, SCORE_TRAIN_KEY);
    }
    else if (itemSymbol == ENT_ENERGY_PACK)
    {
        applyResourceDelta(energy, time, BONUS_ENERGY_PACK, 0);
    }
    else if (itemSymbol == ENT_EMERGENCY_TOOLKIT)
    {
        applyResourceDelta(energy, time, BONUS_EMERGENCY_TOOLKIT, 0);
    }
    else if (itemSymbol == ENT_OBSERVATION_TOKEN)
    {
        applyResourceDelta(energy, time, 0, BONUS_OBSERVATION_TOKEN);
    }
    else if (itemSymbol == ENT_EMERGENCY_CARD)
    {
        emergencyTripsLeft = 3;
    }

    return true;
}

bool handleUseCommand(char itemSymbol, int currentFloor, int currentRoom,
                      int &energy, int &time, int &score,
                      char inventory[MAX_INVENTORY_CAPACITY], int &inventoryCount,
                      bool objectives[NUM_OBJECTIVES])
{

    if (currentFloor < 1 || currentFloor > NUM_FLOORS || currentRoom < 1 || currentRoom > ROOMS_PER_FLOOR || inventoryCount < 0 || inventoryCount > MAX_INVENTORY_CAPACITY)
        return false;

    if (!hasItem(inventory, inventoryCount, itemSymbol))
        return false;
    if (itemSymbol == ENT_FUSE && (currentFloor != 3 || currentRoom != 4 || objectives[OBJ_FUSE_INSTALLED]))
        return false;
    else if (itemSymbol == ENT_MEDICAL_KIT && (currentFloor != 4 || currentRoom != 1 || objectives[OBJ_PASSENGER_RESCUED]))
        return false;
    else if (itemSymbol == ENT_CAT_CARRIER && (currentFloor != 3 || currentRoom != 3 || objectives[OBJ_CAT_SECURED]))
        return false;
    else if (itemSymbol == ENT_CABINET_TRAIN_KEY)
    {
        if (currentFloor != 5 || currentRoom != 2 || objectives[OBJ_TRAIN_PREPARED])
            return false;

        for (int i = 0; i < OBJ_TRAIN_PREPARED; i++)
        {
            if (!objectives[i])
                return false;
        }
    }
    else if (itemSymbol != ENT_FUSE && itemSymbol != ENT_MEDICAL_KIT && itemSymbol != ENT_CAT_CARRIER && itemSymbol != ENT_CABINET_TRAIN_KEY)
        return false;

    applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);

    if (energy <= 0 || time <= 0)
        return true;

    if (itemSymbol == ENT_FUSE)
    {
        installFuseControlPanel(inventory, inventoryCount, objectives, score);
        cout << "\033[32mFuse installed! Lift 2 repaired." << "\033[0m" << endl;
    }
    else if (itemSymbol == ENT_MEDICAL_KIT)
    {
        objectives[OBJ_PASSENGER_RESCUED] = true;
        applyScoreDelta(score, SCORE_RESCUE_PASSENGER);
        cout << "\033[32mPassenger rescued! +" << SCORE_RESCUE_PASSENGER << " points.\033[0m" << endl;
    }
    else if (itemSymbol == ENT_CAT_CARRIER)
    {
        objectives[OBJ_CAT_SECURED] = true;
        applyScoreDelta(score, SCORE_SECURE_CAT);
        cout << "\033[32mCat secured! +" << SCORE_SECURE_CAT << " points.\033[0m" << endl;
    }
    else if (itemSymbol == ENT_CABINET_TRAIN_KEY)
    {
        objectives[OBJ_TRAIN_PREPARED] = true;
        applyScoreDelta(score, SCORE_PREPARE_TRAIN);
        cout << "\033[32mTrain prepared and boarded! +" << SCORE_PREPARE_TRAIN << " points.\033[0m" << endl;
    }

    return true;
}

bool handleUnlockCabinetCommand(const char *code, int currentFloor, int currentRoom,
                                bool objectives[NUM_OBJECTIVES], int &score,
                                char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                                int &entityCount, int &energy, int &time)
{
    if (currentRoom != 1 || currentFloor != 5)
    {
        cout << "\033[31mInvalid Room to unlock cabinet.\033[0m" << endl;
        return false;
    }
    if (entityCount < 0 || entityCount > MAX_ENTITIES_PER_ROOM)
    {
        cout << "\033[31mYour Inventory has undefined behavior.\033[0m" << endl;
        return false;
    }
    if (objectives[OBJ_CABINET_UNLOCKED])
    {
        cout << "\033[31mCabinet ALready Unlocked.\033[0m" << endl;
        return false;
    }

    bool cabinetFound = false;

    for (int i = 0; i < entityCount; i++)
    {
        if (roomEntities[currentFloor - 1][currentRoom - 1][i] == ENT_CABINET_TRAIN_KEY)
        {
            cabinetFound = true;
            break;
        }
    }

    if (!cabinetFound)
    {
        cout << "\033[31mNo cabinet found in this room.\033[0m" << endl;
        return false;
    }

    if (!verifyCabinetCode(code, objectives))
    {
        cout << "\033[31mIncorrect Unlock COde entered. PLease Enter a valid code..\033[0m" << endl;
        return false;
    }

    applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);
    if (energy <= 0 || time <= 0)
        return true;

    objectives[OBJ_CABINET_UNLOCKED] = true;
    applyScoreDelta(score, SCORE_CABINET_UNLOCK);

    cout << "\033[32mCabinet unlocked! Collect the Train Key."
         << "\033[0m" << endl;

    return true;
}

bool checkEndConditions(int energy, int time, const bool objectives[NUM_OBJECTIVES])
{
    if (energy <= 0 || time <= 0)
        return true;

    for (int i = 0; i <= OBJ_TRAIN_PREPARED; i++)
    {
        if (!objectives[i])
        {
            return false;
        }
    }

    return true;
}

void showResultScreen(bool won, int score, int energy, int time)
{
    clearScreen();

    string title;
    string message;
    string color;
    string background = won ? "\033[30;42m" : "\033[30;41m";

    if (won)
    {
        title = "MISSION Completed!";
        message = "You rescued the passenger and the cat and boarded the last train!";
        color = "\033[32m";
    }
    else
    {
        title = "MISSION FAILED!";
        color = "\033[31m";

        if (energy <= 0 && time <= 0)
            message = "You ran out of energy and time.";
        else if (energy <= 0)
            message = "You ran out of energy.";
        else
            message = "You ran out of time.";
    }

    printBorder(color);

    cout << background << setw(BOX_WIDTH) << ' ' << " |\033[0m" << endl;
    cout << background
         << "| " << left << setw(BOX_WIDTH - 2)
         << title
         << " |\033[0m" << endl;
    cout << background << setw(BOX_WIDTH) << ' ' << " |\033[0m" << endl;
    printBorder(color);
    printBoxLine(message, color);
    printBorder(color);
    printBoxLine("Final score       " + to_string(score) + " / 520", color);
    printBoxLine("Energy remaining  " + to_string(energy), color);
    printBoxLine("Time remaining    " + to_string(time), color);
    printBorder(color);
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
                continue;
            }
            else
            {
                cout << endl
                     << "\033[32m-> Game initialization Successfully!\033[0m" << endl
                     << endl;
            }
            if (playerFloor != 0 && !validateStartingRoom(playerFloor, playerRoom, horizontalConn))
            {
                cout << "Invalid starting room in initial-state.txt." << endl;
                continue;
            }
            while (playerFloor == 0)
            {

                cout << "\033[35mEnter Floor (1-5) > \033[0m";
                cin >> floor;
                if (cin.fail() && cin.eof())
                    return;
                if (cin.fail() || floor < 1 || floor > 5)
                {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cout << "\033[31mInvalid Input. Please Enter Valid Answer.\033[0m" << endl;
                    continue;
                }
                cout << "\033[35mEnter Starting Room (1-4) > \033[0m";
                cin >> room;
                if (cin.fail() && cin.eof())
                    return;

                if (cin.fail() || room < 1 || room > 4)
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

            break;
        }
        case 2:
        {
            if (!loadGameStateFromFile("out/savegame.txt", playerFloor, playerRoom,
                                       energy, time, score, inventory, inventoryCount,
                                       objectives, emergencyTripsLeft, roomEntities, entityCounts))
            {
                cout << "\033[31mCould not load the saved game.\033[0m" << endl;
                if (!returnMenuFunction())
                    return;
                continue;
            }
            break;
        }
        case 3:
        {
            clearScreen();
            cout << endl
                 << "MISSION OBJECTIVES" << endl;
            cout << endl
                 << "1. Rescue the passenger and cat." << endl;
            cout << "2. Find clues and unlock the cabinet." << endl;
            cout << "3. Restore the lift and board the train." << endl;

            if (!returnMenuFunction())
                return;

            continue;
        }

        case 4:
            cout << "Thank you for playing The Last Train!" << endl;
            return;

        default:
            cout << "\033[31mInvalid Choice. Please Enter Valid Answer\033[0m" << endl;
            dispalyMenu();
            continue;
        }

        bool playing = true;
        while (playing)
        {
            clearScreen();

            printBorder();

            cout << "\033[30;43m" << setw(BOX_WIDTH) << ' ' << " |\033[0m" << endl;
            cout << "\033[30;43m"
                 << "| " << left << setw(BOX_WIDTH - 2)
                 << "What actions you can perform:"
                 << " |\033[0m" << endl;
            cout << "\033[30;43m" << setw(BOX_WIDTH) << ' ' << " |\033[0m" << endl;
            printBoxLine("");

            cout << endl;
            printBoxLine("1. Move                      5. Show current state");
            cout << endl;
            printBoxLine("2. Inspect room              6. Show map");
            cout << endl;
            printBoxLine("3. Collect item              7. Show objectives");
            cout << endl;
            printBoxLine("4. Use item / interact       8. Save game");
            cout << endl;
            printBoxLine("0. Return to main menu");

            printBorder();

            int in;
            while (true)
            {
                cout << "\033[35mEnter What you want to do > \033[0m";
                cin >> in;
                if (cin.fail() && cin.eof())
                    return;
                if (cin.fail() || in < 0 || in > 8)
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
                if (cin.fail() && cin.eof())
                    return;
                if (cin.fail())
                {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cout << "\033[31mInvalid Input. Please Enter Valid Answer.\033[0m" << endl;
                    continue;
                }
                cout << "\033[35mEnter Destination Room (1-4) > \033[0m";
                cin >> room;
                if (cin.fail() && cin.eof())
                    return;

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
                break;
            }

            case 3:
            {
                char itemSymbol;

                cout << "\033[35mEnter the symbol of the item to collect > \033[0m";
                cin >> itemSymbol;
                if (cin.fail() && cin.eof())
                    return;

                if (cin.fail())
                {
                    cin.clear();
                    cin.ignore(10000, '\n');

                    cout << "\033[31mInvalid input.\033[0m" << endl;
                    break;
                }

                bool collected = handleCollectCommand(
                    itemSymbol,
                    playerFloor, playerRoom,
                    energy, time, score,
                    roomEntities,
                    entityCounts[playerFloor - 1][playerRoom - 1],
                    inventory, inventoryCount,
                    objectives, emergencyTripsLeft);

                if (collected)
                {
                    cout << "\033[32mCollection performed.\033[0m" << endl;
                }
                else
                {
                    cout << "\033[31mCannot collect that item.\033[0m" << endl;
                }
                break;
            }

            case 4:
            {

                char itemSymbol;
                cout << "\033[35mEnter the item symbol to use > \033[0m";
                cin >> itemSymbol;
                if (cin.fail())
                {
                    if (cin.eof())
                        return;

                    cin.clear();
                    cin.ignore(10000, '\n');

                    cout << "\033[31mInvalid input.\033[0m" << endl;
                    break;
                }

                bool used;

                if (itemSymbol == ENT_CABINET_TRAIN_KEY && playerFloor == 5 && playerRoom == 1)
                {
                    string code;
                    cout << "\033[35mEnter the cabinet code > \033[0m";
                    if (!(cin >> code))
                        return;

                    used = handleUnlockCabinetCommand(
                        code.c_str(), playerFloor, playerRoom,
                        objectives, score, roomEntities,
                        entityCounts[playerFloor - 1][playerRoom - 1],
                        energy, time);
                }
                else
                {

                    used = handleUseCommand(itemSymbol, playerFloor, playerRoom, energy, time, score, inventory, inventoryCount, objectives);
                }
                if (!used)
                {
                    cout << "\033[31mCannot use that item here, "
                         << "or its requirements are not satisfied.\033[0m"
                         << endl;
                }

                break;
            }

            case 5:
                renderDashboard(playerFloor, playerRoom, energy, time, score, inventory, inventoryCount, emergencyTripsLeft);
                break;
            case 6:
                renderAsciiBuilding(playerFloor, playerRoom);
                break;
            case 7:
                renderObjectivesChecklist(objectives);
                break;

            case 8:
            {
                if (saveGameStateToFile("out/savegame.txt", playerFloor, playerRoom, energy, time, score, inventory, inventoryCount, objectives, emergencyTripsLeft, roomEntities, entityCounts))
                {
                    cout << "\033[32mGAME SAVED Successfuly!\033[0m" << endl;
                }
                else
                {
                    cout << "\033[31mError While Storing data!\033[0m" << endl;
                }
                break;
            }
            }

            if (!playing)
                break;

            if (checkEndConditions(energy, time, objectives))
            {
                playing = false;
                if (energy <= 0 || time <= 0)
                    objectives[OBJ_MISSION_FAILED] = true;
                else
                {
                    objectives[OBJ_MISSION_WON] = true;
                    applyScoreDelta(score, SCORE_MISSION_VICTORY);
                }
                showResultScreen(objectives[OBJ_MISSION_WON], score, energy, time);
                writeGameSummaryReport("out/game_summary.txt", score, energy, time, objectives, objectives[OBJ_MISSION_WON]);
            }
            if (!returnMenuFunction())
                return;
            if (!playing)
                clearScreen();
        }
    }
}
