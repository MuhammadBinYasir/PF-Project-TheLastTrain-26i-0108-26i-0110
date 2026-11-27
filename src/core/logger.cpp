#include <iostream>
#include "logger.h"
#include "util.h"
#include <fstream>
#include <string>
using namespace std;

// File persistence for savegame.txt and game_summary.txt using primitive variables

string OBJ_STATUS(bool completed)
{
    if (completed)
        return "YES";

    return "NO";
}

bool saveGameStateToFile(const char *filename,
                         int playerFloor, int playerRoom,
                         int energy, int time, int score,
                         const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                         const bool objectives[NUM_OBJECTIVES], int emergencyTripsLeft,
                         const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                         const int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{

    if (filename == nullptr || inventoryCount < 0 || inventoryCount > MAX_INVENTORY_CAPACITY)
        return false;

    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            if (entityCounts[f][r] < 0 || entityCounts[f][r] > MAX_ENTITIES_PER_ROOM)
                return false;
        }
    }

    ofstream file(filename);

    if (!file)
        return false;

    file << "GAME_STATUS: IN_PROGRESS" << endl;
    file << "PLAYER_FLOOR: " << playerFloor << endl;
    file << "PLAYER_ROOM: " << playerRoom << endl;
    file << "ENERGY: " << energy << endl;
    file << "TIME: " << time << endl;
    file << "SCORE: " << score << endl;
    file << "EMERGENCY_TRIPS_LEFT: " << emergencyTripsLeft << endl;

    file << "INVENTORY_COUNT: " << inventoryCount << endl;
    file << "INVENTORY:";

    if (inventoryCount == 0)
        file << " NONE";

    for (int i = 0; i < inventoryCount; i++)
        file << ' ' << inventory[i];

    file << endl;

    file << "MASTER_KEY_COLLECTED: " << OBJ_STATUS(objectives[OBJ_MASTER_KEY]) << endl;
    file << "CLUE_4: " << OBJ_STATUS(objectives[OBJ_CLUE_4]) << endl;
    file << "CLUE_2: " << OBJ_STATUS(objectives[OBJ_CLUE_2]) << endl;
    file << "CLUE_7: " << OBJ_STATUS(objectives[OBJ_CLUE_7]) << endl;
    file << "CABINET_UNLOCKED: " << OBJ_STATUS(objectives[OBJ_CABINET_UNLOCKED]) << endl;
    file << "TRAIN_KEY_COLLECTED: " << OBJ_STATUS(objectives[OBJ_TRAIN_KEY]) << endl;
    file << "FUSE_COLLECTED: " << OBJ_STATUS(objectives[OBJ_FUSE_COLLECTED]) << endl;
    file << "FUSE_INSTALLED: " << OBJ_STATUS(objectives[OBJ_FUSE_INSTALLED]) << endl;
    file << "LIFT_PASS_COLLECTED: " << OBJ_STATUS(objectives[OBJ_LIFT_PASS]) << endl;
    file << "LIFT2_ACTIVE: " << OBJ_STATUS(objectives[OBJ_LIFT2_ACTIVE]) << endl;
    file << "PASSENGER_RESCUED: " << OBJ_STATUS(objectives[OBJ_PASSENGER_RESCUED]) << endl;
    file << "CAT_CARRIER_COLLECTED: " << OBJ_STATUS(objectives[OBJ_CAT_CARRIER]) << endl;
    file << "CAT_RESCUED: " << OBJ_STATUS(objectives[OBJ_CAT_SECURED]) << endl;
    file << "TRAIN_PREPARED: " << OBJ_STATUS(objectives[OBJ_TRAIN_PREPARED]) << endl;
    file << "MISSION_WON: " << OBJ_STATUS(objectives[OBJ_MISSION_WON]) << endl;
    file << "MISSION_FAILED: " << OBJ_STATUS(objectives[OBJ_MISSION_FAILED]) << endl;

    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            file << "ROOM_F" << f + 1 << "_R" << r + 1 << ": " << entityCounts[f][r];

            for (int i = 0; i < entityCounts[f][r]; i++)
                file << ' ' << roomEntities[f][r][i];

            file << endl;
        }
    }

    file.close();
    return !file.fail();
}

// Helper function to convert "YES" or "NO" back to boolean
bool parseYesNo(const string &val, bool &outFlag)
{
    if (val == "YES")
    {
        outFlag = true;
        return true;
    }
    if (val == "NO")
    {
        outFlag = false;
        return true;
    }
    return false;
}

bool loadGameStateFromFile(const char *filename,
                           int &playerFloor, int &playerRoom,
                           int &energy, int &time, int &score,
                           char inventory[MAX_INVENTORY_CAPACITY], int &inventoryCount,
                           bool objectives[NUM_OBJECTIVES], int &emergencyTripsLeft,
                           char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                           int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    if (filename == nullptr)
        return false;

    ifstream file(filename);
    if (!file.is_open())
        return false;

    inventoryCount = 0;
    for (int i = 0; i < MAX_INVENTORY_CAPACITY; i++)
        inventory[i] = ENT_NONE;

    for (int i = 0; i < NUM_OBJECTIVES; i++)
        objectives[i] = false;

    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            entityCounts[f][r] = 0;
            for (int k = 0; k < MAX_ENTITIES_PER_ROOM; k++)
                roomEntities[f][r][k] = ENT_NONE;
        }
    }

    int loadedInventoryCount = -1;
    bool hasFloor = false, hasRoom = false, hasEnergy = false, hasTime = false, hasScore = false;
    int roomsLoaded = 0;
    bool roomSeen[NUM_FLOORS][ROOMS_PER_FLOOR] = {};

    string line;
    try
    {
        while (getline(file, line))
        {
            size_t pos = line.find(':');
            if (pos == string::npos)
                continue;

            string key = line.substr(0, pos);
            string value = (pos + 2 <= line.length()) ? line.substr(pos + 2) : "";

            if (value.empty())
                return false;

            if (key == "PLAYER_FLOOR" || key == "PLAYER_ROOM" ||
                key == "ENERGY" || key == "TIME" || key == "SCORE" ||
                key == "EMERGENCY_TRIPS_LEFT" || key == "INVENTORY_COUNT")
            {
                if (value.empty())
                    return false;

                for (char ch : value)
                {
                    if (ch < '0' || ch > '9')
                        return false;
                }

                int number = stoi(value);

                if (key == "PLAYER_FLOOR" && (number < 1 || number > NUM_FLOORS))
                    return false;

                if (key == "PLAYER_ROOM" && (number < 1 || number > ROOMS_PER_FLOOR))
                    return false;

                if (key == "ENERGY" && (number < 1 || number > MAX_ENERGY))
                    return false;

                if (key == "TIME" && (number < 1 || number > MAX_TIME))
                    return false;

                if (key == "EMERGENCY_TRIPS_LEFT" && number > 3)
                    return false;

                if (key == "INVENTORY_COUNT" && number > MAX_INVENTORY_CAPACITY)
                    return false;
            }

            if (key == "GAME_STATUS")
            {
                if (value != "IN_PROGRESS")
                    return false; // Cannot resume a finished/invalid save
            }
            else if (key == "PLAYER_FLOOR")
            {
                playerFloor = stoi(value);
                hasFloor = true;
            }
            else if (key == "PLAYER_ROOM")
            {
                playerRoom = stoi(value);
                hasRoom = true;
            }
            else if (key == "ENERGY")
            {
                energy = stoi(value);
                hasEnergy = true;
            }
            else if (key == "TIME")
            {
                time = stoi(value);
                hasTime = true;
            }
            else if (key == "SCORE")
            {
                score = stoi(value);
                hasScore = true;
            }
            else if (key == "EMERGENCY_TRIPS_LEFT")
            {
                emergencyTripsLeft = stoi(value);
            }
            else if (key == "INVENTORY_COUNT")
            {
                loadedInventoryCount = stoi(value);
                if (loadedInventoryCount < 0 || loadedInventoryCount > MAX_INVENTORY_CAPACITY)
                    return false;
            }
            else if (key == "INVENTORY")
            {
                inventoryCount = 0;
                if (value != "NONE")
                {
                    for (size_t i = 0; i < value.length(); i++)
                    {
                        if (value[i] == ' ')
                            continue;
                        if (inventoryCount >= MAX_INVENTORY_CAPACITY)
                            return false;
                        inventory[inventoryCount++] = value[i];
                    }
                }
            }
            else if (key == "MASTER_KEY_COLLECTED")
            {
                if (!parseYesNo(value, objectives[OBJ_MASTER_KEY]))
                    return false;
            }
            else if (key == "CLUE_4")
            {
                if (!parseYesNo(value, objectives[OBJ_CLUE_4]))
                    return false;
            }
            else if (key == "CLUE_2")
            {
                if (!parseYesNo(value, objectives[OBJ_CLUE_2]))
                    return false;
            }
            else if (key == "CLUE_7")
            {
                if (!parseYesNo(value, objectives[OBJ_CLUE_7]))
                    return false;
            }
            else if (key == "CABINET_UNLOCKED")
            {
                if (!parseYesNo(value, objectives[OBJ_CABINET_UNLOCKED]))
                    return false;
            }
            else if (key == "TRAIN_KEY_COLLECTED")
            {
                if (!parseYesNo(value, objectives[OBJ_TRAIN_KEY]))
                    return false;
            }
            else if (key == "FUSE_COLLECTED")
            {
                if (!parseYesNo(value, objectives[OBJ_FUSE_COLLECTED]))
                    return false;
            }
            else if (key == "FUSE_INSTALLED")
            {
                if (!parseYesNo(value, objectives[OBJ_FUSE_INSTALLED]))
                    return false;
            }
            else if (key == "LIFT_PASS_COLLECTED")
            {
                if (!parseYesNo(value, objectives[OBJ_LIFT_PASS]))
                    return false;
            }
            else if (key == "LIFT2_ACTIVE")
            {
                if (!parseYesNo(value, objectives[OBJ_LIFT2_ACTIVE]))
                    return false;
            }
            else if (key == "PASSENGER_RESCUED")
            {
                if (!parseYesNo(value, objectives[OBJ_PASSENGER_RESCUED]))
                    return false;
            }
            else if (key == "CAT_CARRIER_COLLECTED")
            {
                if (!parseYesNo(value, objectives[OBJ_CAT_CARRIER]))
                    return false;
            }
            else if (key == "CAT_RESCUED")
            {
                if (!parseYesNo(value, objectives[OBJ_CAT_SECURED]))
                    return false;
            }
            else if (key == "TRAIN_PREPARED")
            {
                if (!parseYesNo(value, objectives[OBJ_TRAIN_PREPARED]))
                    return false;
            }
            else if (key == "MISSION_WON")
            {
                if (!parseYesNo(value, objectives[OBJ_MISSION_WON]))
                    return false;
            }
            else if (key == "MISSION_FAILED")
            {
                if (!parseYesNo(value, objectives[OBJ_MISSION_FAILED]))
                    return false;
            }
            else if (key.rfind("ROOM_F", 0) == 0 && key.length() == 10 && key[7] == '_' && key[8] == 'R')
            {
                // Format: ROOM_F<f>_R<r>: <count> <sym1> <sym2>...
                int f = (key[6] - '0') - 1;
                int r = (key[9] - '0') - 1;
                if (f < 0 || f >= NUM_FLOORS || r < 0 || r >= ROOMS_PER_FLOOR)
                    return false;

                if (roomSeen[f][r])
                    return false;
                roomSeen[f][r] = true;

                int count = value[0] - '0';
                if (count < 0 || count > MAX_ENTITIES_PER_ROOM)
                    return false;

                entityCounts[f][r] = count;
                int parsedItems = 0;
                for (size_t i = 1; i < value.length() && parsedItems < count; i++)
                {
                    if (value[i] == ' ')
                        continue;
                    roomEntities[f][r][parsedItems++] = value[i];
                }
                if (parsedItems != count)
                    return false;

                roomsLoaded++;
            }
        }
    }
    catch (const exception &)
    {
        return false; // Handles corrupt numbers in stoi() without crashing
    }

    file.close();

    // Final validation of loaded bounds and completeness
    if (!hasFloor || !hasRoom || !hasEnergy || !hasTime || !hasScore)
        return false;
    if (playerFloor < 1 || playerFloor > NUM_FLOORS || playerRoom < 1 || playerRoom > ROOMS_PER_FLOOR)
        return false;
    if (energy <= 0 || energy > MAX_ENERGY || time <= 0 || time > MAX_TIME || score < 0)
        return false;
    if (loadedInventoryCount != -1 && loadedInventoryCount != inventoryCount)
        return false;
    if (roomsLoaded != TOTAL_ROOMS)
        return false;

    return true;
}

void writeGameSummaryReport(const char *filename,
                            int score, int remainingEnergy, int remainingTime,
                            const bool objectives[NUM_OBJECTIVES], bool victory)
{
    if (filename == nullptr)
        return;

    ofstream file(filename);
    if (!file.is_open())
        return;

    file << "===========================================" << endl;
    file << "      THE LAST TRAIN - SESSION SUMMARY     " << endl;
    file << "===========================================" << endl;
    file << "FINAL_OUTCOME: " << (victory ? "VICTORY" : "DEFEAT") << endl;
    file << "FINAL_SCORE: " << score << " / 520" << endl;
    file << endl;

    file << "--- REMAINING RESOURCES ---" << endl;
    file << "REMAINING_ENERGY: " << remainingEnergy << " / " << MAX_ENERGY << endl;
    file << "REMAINING_TIME: " << remainingTime << " / " << MAX_TIME << endl;
    file << endl;

    const char *objectiveNames[NUM_OBJECTIVES] = {
        "Master Key Collected",
        "Clue Digit 4 Discovered",
        "Clue Digit 2 Discovered",
        "Clue Digit 7 Discovered",
        "Security Cabinet Unlocked",
        "Train Key Acquired",
        "Fuse Collected",
        "Fuse Installed at Control Panel",
        "Lift Pass Collected",
        "Lift 2 Activated",
        "Injured Passenger Rescued",
        "Cat Carrier Collected",
        "Stranded Cat Secured",
        "Train Prepared & Boarded",
        "Mission Won",
        "Mission Failed"};

    int completedCount = 0;
    file << "--- OBJECTIVES CHECKLIST ---" << endl;
    for (int i = 0; i < NUM_OBJECTIVES; i++)
    {
        file << "[" << (objectives[i] ? "COMPLETED" : "INCOMPLETE") << "] "
             << objectiveNames[i] << endl;

        if (i <= OBJ_TRAIN_PREPARED && objectives[i])
        {
            completedCount++;
        }
    }
    file << endl;

    file << "--- SESSION AUDIT ---" << endl;
    file << "MILESTONES_COMPLETED: " << completedCount << " / 14" << endl;
    file << "NET_ENERGY_CHANGE: " << remainingEnergy - INITIAL_ENERGY << endl;
    file << "NET_TIME_CHANGE: " << remainingTime - INITIAL_TIME << endl;

    if (victory)
    {
        file << "TERMINATION_REASON: All rescue and train boarding objectives achieved." << endl;
    }
    else if (remainingEnergy <= 0)
    {
        file << "TERMINATION_REASON: Player Energy depleted (0)." << endl;
    }
    else if (remainingTime <= 0)
    {
        file << "TERMINATION_REASON: Mission Time expired (0)." << endl;
    }
    else
    {
        file << "TERMINATION_REASON: Session terminated prematurely." << endl;
    }

    file << "===========================================" << endl;
    file.close();
}