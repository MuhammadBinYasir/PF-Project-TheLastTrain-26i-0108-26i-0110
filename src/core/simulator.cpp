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
    cout << "4. Display Map" << endl;
    cout << "5. Exit Game" << endl
         << endl;

    cout << "\033[34m"
         << endl
         << string(100, '=') << "\033[0m" << endl
         << endl;
}

int usersMenu()
{
    int userChoice;
    while (true)
    {
        dispalyMenu();

        cout << "\033[35mEnter Your Choice > \033[0m";

        cin >> userChoice;
        if (cin.fail() || userChoice < 1 || userChoice > 5)
        {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid Input. Please Enter Correct Input" << endl;
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

            cout << "\033[35mEnter Floor (1-5) > \033[0m";
            cin >> floor;
            cout << "\033[35mEnter Starting Room (1-4) > \033[0m";
            cin >> room;

            if (validateStartingRoom(floor, room, horizontalConn))
            {
                playerFloor = floor;
                playerRoom = room;

                cout << endl
                     << "\033[32m-> We are spawning you to " << "F" << floor << "-R" << room << "!\033[0m" << endl
                     << endl;
            }
            else
            {
                cout << "\033[31m->Invalid Room Number. Enterance Not Allowed.\033[0m\n";
                continue;
            }

            // renderDashboard(playerFloor, playerRoom, energy, time, score, inventory, inventoryCount, emergencyTripsLeft);
            // renderRoomDetails(playerFloor, playerRoom, roomEntities, entityCounts[playerFloor - 1][playerRoom - 1]);
            // renderAsciiBuilding(playerFloor, playerRoom);
            renderObjectivesChecklist(objectives);
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

            break;
        }
        case 4:
            cout << "Displaying Complete Map of Game: " << endl;
            break;

        case 5:
            cout << "Thank you for playing The Last Train!" << endl;
            return;

        default:
            cout << "Invalid Choice. Please Enter Valid Answer" << endl;
            dispalyMenu();
            break;
        }
    }
}