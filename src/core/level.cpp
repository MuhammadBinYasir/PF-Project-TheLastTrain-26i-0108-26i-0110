#include <iostream>
#include "level.h"
#include "util.h"
#include <fstream>
#include <string>
using namespace std;

bool loadBuildingMap(const char *filename,
                     char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                     char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                     int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    ifstream file(filename);
    if (!file.is_open())
    {
        return false;
    }
    string line;

    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR - 1; r++)
        {
            horizontalConn[f][r] = CONN_NONE;
        }

        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            entityCounts[f][r] = 0;

            for (int k = 0; k < MAX_ENTITIES_PER_ROOM; k++)
            {
                roomEntities[f][r][k] = ENT_NONE;
            }
        }
    }

    bool floorFound[NUM_FLOORS] = {};
    int floorsLoaded = 0;

    while (floorsLoaded < NUM_FLOORS && getline(file, line))
    {
        // Horizontal Connections
        for (int i = 1; i <= NUM_FLOORS; i++)
        {
            string floorLabel = "F" + to_string(i) + "-R1";

            if (line.find(floorLabel) == string::npos)
            {
                continue;
            }
            if (floorFound[i - 1])
            {
                return false;
            }
            for (int j = 1; j < ROOMS_PER_FLOOR; j++)
            {
                string s1 = "F" + to_string(i) + "-R" + to_string(j);
                string s2 = "F" + to_string(i) + "-R" + to_string(j + 1);

                int plc1 = static_cast<int>(line.find(s1));
                int plc2 = static_cast<int>(line.find(s2));

                if (plc1 == -1 ||
                    plc2 == -1 ||
                    plc2 < plc1 + 5)
                {
                    return false;
                }

                int start = plc1 + 5;
                int length = plc2 - start;

                string connText = line.substr(start, length);
                int first = static_cast<int>(connText.find_first_not_of(" \t"));
                int last = static_cast<int>(connText.find_last_not_of(" \t"));

                if (first == -1)
                {
                    return false;
                }

                connText = connText.substr(first, last - first + 1);

                if (connText == "L1")
                {
                    horizontalConn[i - 1][j - 1] = CONN_NONE;
                }
                else if (connText == "D" ||
                         connText == "W" ||
                         connText == "L" ||
                         connText == "#")
                {
                    horizontalConn[i - 1][j - 1] = connText[0];
                }
                else
                    return false;
            }

            // Room Entities
            string entityLine;

            if (!getline(file, entityLine))
                return false;

            const string validEntities = "<)\\4*~;(!%$27^@&";
            for (int j = 1; j <= ROOMS_PER_FLOOR; j++)
            {
                string label = "F" + to_string(i) + "-R" + to_string(j);
                int roomStart = static_cast<int>(line.find(label));

                if (roomStart == -1 ||
                    roomStart >= static_cast<int>(entityLine.length()))
                {
                    return false;
                }

                int roomEnd;

                if (j < ROOMS_PER_FLOOR)
                {
                    string nextLabel = "F" + to_string(i) + "-R" + to_string(j + 1);
                    roomEnd = static_cast<int>(line.find(nextLabel));
                }
                else
                {
                    roomEnd = static_cast<int>(entityLine.find('|', roomStart));
                }

                if (roomEnd == -1 ||
                    roomEnd <= roomStart ||
                    roomEnd > static_cast<int>(entityLine.length()))
                {
                    return false;
                }

                string roomText = entityLine.substr(roomStart, roomEnd - roomStart);

                for (char symbol : roomText)
                {
                    if (symbol == ' ' || symbol == '\t' || symbol == '|')
                        continue;
                    if (validEntities.find(symbol) == string::npos)
                        return false;
                    int count = entityCounts[i - 1][j - 1];

                    if (count >= MAX_ENTITIES_PER_ROOM)
                    {
                        return false;
                    }

                    roomEntities[i - 1][j - 1][count] = symbol;
                    entityCounts[i - 1][j - 1]++;
                }
            }

            floorFound[i - 1] = true;
            floorsLoaded++;
            break;
        }
    }
    file.close();
    return floorsLoaded == NUM_FLOORS;
}

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

bool validateStartingRoom(int startFloor, int startRoom,
                          const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1])
{
    if (startFloor < 1 || startFloor > NUM_FLOORS || startRoom < 1 || startRoom > ROOMS_PER_FLOOR)
    {
        return false;
    }

    int f = startFloor - 1;
    int r = startRoom - 1;

    if (r > 0)
    {
        char connection = horizontalConn[f][r - 1];

        if (connection == CONN_DOOR ||
            connection == CONN_WINDOW)
        {
            return true;
        }
    }

    if (r < ROOMS_PER_FLOOR - 1)
    {
        char connection = horizontalConn[f][r];

        if (connection == CONN_DOOR ||
            connection == CONN_WINDOW)
        {
            return true;
        }
    }

    if (startRoom == 1)
    {
        return true;
    }

    if ((startFloor == 1 || startFloor == 2) &&
        (startRoom == 2 || startRoom == 3))
    {
        return true;
    }

    return false;
}

const char* getEntityName(char entitySymbol)
{
    switch (entitySymbol) {
        case '<':  return "Master Key";
        case ')':  return "Energy Pack";
        case '\\': return "Emergency Toolkit";
        case '4':  return "Clue Digit 4";
        case '*':  return "Fuse";
        case '~':  return "Cat";
        case ';':  return "Control Panel";
        case '(':  return "Lift Pass";
        case '!':  return "Passenger";
        case '%':  return "Medical Kit";
        case '$':  return "Emergency Access Card";
        case '2':  return "Clue Digit 2";
        case '7':  return "Clue Digit 7";
        case '^':  return "Cabinet / Train Key";
        case '@':  return "Cat Carrier";
        case '&':  return "Observation Token";
        default:   return "Unknown Entity";
    }
}