#include <iostream>
#include "simulator.h"
#include "physics.h"
#include "switches.h"
#include "logger.h"
#include "viewer_iface.h"
#include "util.h"
#include <string>
#include <iomanip>
using namespace std;

// Modular simulation loop using primitive variables

void dispalyMenu()
{
    cout << endl
         << string(30, '=') << endl;
    cout << setw(22) << "The Last Train" << endl;
    cout << string(30, '=') << endl
         << endl;

    cout << "1. Start New Game" << endl;
    cout << "2. Load Saved Game" << endl;
    cout << "3. Instructions Game" << endl;
    cout << "4. Display Map" << endl;
    cout << "5. Exit Game" << endl
         << endl;

    cout << string(30, '=') << endl
         << endl;
}

int usersMenu()
{
    int userChoice;
    while (true)
    {
        dispalyMenu();

        cout << "Enter Your Choice > ";
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
            cout << "Starting New Game..... " << endl;
            break;
        case 2:
            cout << "Loading Saved Game...." << endl;
            break;
        case 3:
        {
            cout << "\nMISSION OBJECTIVES\n";
            cout << "Rescue the passenger and cat.\n";
            cout << "Find clues and unlock the cabinet.\n";
            cout << "Restore the lift and board the train.\n";

            int in;
            while (true)
            {
                cout << "Enter -1 to Move Back to Main Menu > ";
                cin >> in;
                if (in != -1 || cin.fail())
                {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cout << "Invalid Input. Please Enter Valid Answer.";
                    continue;
                }
                break;
            }

            break;
        }
        case 4:
            cout << "Displaying COmplete Map of Game: " << endl;
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