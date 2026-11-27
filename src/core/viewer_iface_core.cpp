#include <iostream>
#include "viewer_iface.h"
#include "util.h"
#include "level.h"
#include "fstream"
using namespace std;

// Terminal UI and dashboard rendering using primitive parameters

void renderDashboard(int playerFloor, int playerRoom, int energy, int time, int score,
                     const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                     int emergencyTripsLeft)
{
    cout << endl
         << endl
         << "\033[1;33m";
    cout << "|     YOUR CURRENT STATE:" << endl;
    cout << "|" << endl;
    cout << "|     Location: F" << playerFloor << "-R" << playerRoom << endl;
    cout << "|     Energy: " << energy << endl;
    cout << "|     Time: " << time << endl;
    cout << "|     Score: " << score << endl;
    cout << "|     Emergency Trip Left: " << emergencyTripsLeft << endl;
    cout << "|" << endl;
    cout << "|     Inventory: ";

    if (inventoryCount == 0)
    {
        cout << "Empty" << endl;
    }
    else
    {
        for (int i = 0; i < inventoryCount; i++)
        {
            cout << getEntityName(inventory[i]) << " ( " << inventory[i] << " ) | ";
        }
    }
    cout << "\033[0m";
    cout << endl;
}

void renderRoomDetails(int floor, int room,
                       const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                       int entityCount)
{
    cout << endl
         << endl
         << "\033[1;33m";
    cout << "|     YOUR ROOM DETAILS:" << endl;
    cout << "|" << endl;
    cout << "|     Location: F" << floor << "-R" << room << endl;
    cout << "|     Room Entities:" << endl;
    if (entityCount == 0)
    {
        cout << "|           - None" << endl;
    }
    for (int i = 0; i < entityCount; i++)
    {
        cout << "|           -" << getEntityName(roomEntities[floor - 1][room - 1][i]) << endl;
    }
    cout << "\033[0m";
    cout << endl;
}

void renderAsciiBuilding(int playerFloor, int playerRoom, const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM], const int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    ifstream file("data/building.txt");

    if (!file.is_open())
    {
        cout << "\033[31mCould not open building map.\033[0m" << endl;
        return;
    }

    string playerLabel = "F" + to_string(playerFloor) + "-R" + to_string(playerRoom);

    string line;

    getline(file, line); // Skip the first line.

    cout << endl;

    while (getline(file, line))
    {
        if (line.find("D = Ordinary door") == 0)
            break;

        int position = static_cast<int>(line.find(playerLabel));

        if (position != -1)
        {
            cout << line.substr(0, position);

            cout << "\033[1;32m" << playerLabel << "\033[0m";

            cout << line.substr(position + playerLabel.length()) << endl;
        }
        else
        {
            cout << line << endl;
        }

        for (int f = 0; f < NUM_FLOORS; f++)
        {
            string label = "F" + to_string(f + 1) + "-R1";
            int col = static_cast<int>(line.find(label));
            if (col == -1)
                continue;

            string items;
            if (!getline(file, items))
                return;

            for (int r = 0; r < ROOMS_PER_FLOOR; r++)
            {
                int i = 0;
                while (col < static_cast<int>(items.length()) && items[col] != '|')
                {
                    if (i < entityCounts[f][r] && items[col] == roomEntities[f][r][i])
                        i++;
                    else
                        items[col] = ' ';
                    col++;
                }
                col++;
            }

            cout << items << endl;
            break;
        }
    }
}

void renderObjectivesChecklist(const bool objectives[NUM_OBJECTIVES])
{
    const char *names[NUM_OBJECTIVES] = {
        "Master Key collected", "Clue 4 discovered", "Clue 2 discovered", "Clue 7 discovered", "Cabinet unlocked",
        "Train Key collected", "Fuse collected", "Fuse installed", "Lift Pass collected", "Lift 2 active", "Passenger rescued",
        "Cat Carrier collected", "Cat secured", "Train prepared", "Mission won", "Mission failed"};

    cout << endl
         << "OBJECTIVES CHECKLIST" << endl
         << endl;

    for (int i = 0; i < NUM_OBJECTIVES; i++)
    {
        if (objectives[i])
            cout << "\033[32m[OK]\033[0m ";
        else
            cout << "\033[31m[X]\033[0m ";

        cout << names[i] << endl;
    }
}
