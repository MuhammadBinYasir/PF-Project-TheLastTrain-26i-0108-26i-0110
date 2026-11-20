#include <iostream>
#include "level.h"
#include "util.h"
#include <fstream>
#include <string>
using namespace std;


bool loadInitialState(const char *filename,
                      int &playerFloor, int &playerRoom,
                      int &energy, int &time, int &score,
                      char inventory[MAX_INVENTORY_CAPACITY], int &inventoryCount,
                      bool objectives[NUM_OBJECTIVES], int &emergencyTripsLeft)
{
    ifstream file(filename);
    if (!file)
        return false;
    string line;

    inventoryCount = 0;
    emergencyTripsLeft = 0;

    for (int i = 0; i < MAX_INVENTORY_CAPACITY; i++)
        inventory[i] = ENT_NONE;

    for (int i = 0; i < NUM_OBJECTIVES; i++)
        objectives[i] = false;

    playerFloor = 0;
    playerRoom = 0;

    bool energyStatus = false, timeStatus = false, scoreStatus = false, StartStatus = false;

    try
    {
        while (getline(file, line))
        {
            int pos = line.find(':');
            if (pos == -1)
                continue;

            string key = line.substr(0, pos);
            string value = line.substr(pos + 1);

            int NoOfDigit = 0;

            while (NoOfDigit < (int)value.length() && value[NoOfDigit] == ' ')
                NoOfDigit++;

            value = value.substr(NoOfDigit);

            if (key == "INITIAL_ENERGY")
            {
                energy = stoi(value);
                energyStatus = true;
            }
            else if (key == "INITIAL_TIME")
            {
                time = stoi(value);
                timeStatus = true;
            }
            else if (key == "INITIAL_SCORE")
            {
                score = stoi(value);
                scoreStatus = true;
            }
            else if (key == "START_ROOM")
            {
                if (value == "PLAYER_SELECTS")
                {
                    playerFloor = 0;
                    playerRoom = 0;
                }
                else
                {
                    if (value.length() == 5 && value[0] == 'F' && value[2] == '-' && value[3] == 'R')
                    {
                        playerFloor = value[1] - '0';
                        playerRoom = value[4] - '0';

                        if (playerFloor < 1 || playerFloor > NUM_FLOORS || playerRoom < 1 || playerRoom > ROOMS_PER_FLOOR)
                        {
                            return false;
                        }
                    }
                    else
                    {
                        return false;
                    }
                }
                StartStatus = true;
            }
            else if (key == "INITIAL_INVENTORY")
            {
                if (value == "NONE")
                {
                    inventoryCount = 0;
                }
                else
                {
                    for (int i = 0; i < (int)value.length() && inventoryCount < MAX_INVENTORY_CAPACITY; i++)
                    {
                        if (value[i] == ' ')
                            continue;
                        string allowed = "<*(%$@";
                        if (allowed.find(value[i]) == string::npos)
                            return false;
                        if (inventoryCount >= MAX_INVENTORY_CAPACITY)
                            return false;
                        inventory[inventoryCount] = value[i];
                        inventoryCount++;
                    }
                }
            }
            else if (key == "PASSENGER_RESCUED")
            {
                if (value != "YES" && value != "NO")
                    return false;
                objectives[OBJ_PASSENGER_RESCUED] = (value == "YES");
            }
            else if (key == "CAT_RESCUED")
            {
                if (value != "YES" && value != "NO")
                    return false;
                objectives[OBJ_CAT_SECURED] = (value == "YES");
            }
            else if (key == "CLUES_COLLECTED")
            {
                if (value == "NONE")
                {
                    objectives[OBJ_CLUE_4] = false;
                    objectives[OBJ_CLUE_2] = false;
                    objectives[OBJ_CLUE_7] = false;
                }
                else
                    return false;
            }
            else if (key == "CABINET_UNLOCKED")
            {
                if (value != "YES" && value != "NO")
                    return false;
                objectives[OBJ_CABINET_UNLOCKED] = (value == "YES");
            }
            else if (key == "TRAIN_KEY_COLLECTED")
            {
                if (value != "YES" && value != "NO")
                    return false;
                objectives[OBJ_TRAIN_KEY] = (value == "YES");
            }
            else if (key == "FUSE_INSTALLED")
            {
                if (value != "YES" && value != "NO")
                    return false;
                objectives[OBJ_FUSE_INSTALLED] = (value == "YES");
            }
            else if (key == "LIFT_PASS_COLLECTED")
            {
                if (value != "YES" && value != "NO")
                    return false;
                objectives[OBJ_LIFT_PASS] = (value == "YES");
            }
            else if (key == "TRAIN_PREPARED")
            {
                if (value != "YES" && value != "NO")
                    return false;
                objectives[OBJ_TRAIN_PREPARED] = (value == "YES");
            }
            else if (key == "PLAYER_AT_TRAIN")
            {
                if (value != "YES" && value != "NO")
                    return false;
            }
        }
    }
    catch (const exception &)
    {
        return false;
    }
    if (!energyStatus || !timeStatus || !scoreStatus || !StartStatus || energy < 0 || energy > MAX_ENERGY || time < 0 || time > MAX_TIME || score < 0)
    {
        return false;
    }

    file.close();
    return true;
}