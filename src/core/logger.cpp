#include <iostream>
#include "logger.h"
#include "util.h"
#include <fstream>
#include <climits>
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

    int savedFloor = 0, savedRoom = 0, savedEnergy = 0, savedTime = 0, savedScore = -1;
    int savedTrips = -1, savedItemCount = 0;
    char savedInventory[MAX_INVENTORY_CAPACITY];
    bool savedObjectives[NUM_OBJECTIVES] = {};
    char savedEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM];
    int savedCounts[NUM_FLOORS][ROOMS_PER_FLOOR] = {};
    bool hasStatus = false, hasInventory = false;
    bool objectiveSeen[NUM_OBJECTIVES] = {};
    const string objectiveNames[NUM_OBJECTIVES] = {
        "MASTER_KEY_COLLECTED", "CLUE_4", "CLUE_2", "CLUE_7", "CABINET_UNLOCKED",
        "TRAIN_KEY_COLLECTED", "FUSE_COLLECTED", "FUSE_INSTALLED", "LIFT_PASS_COLLECTED",
        "LIFT2_ACTIVE", "PASSENGER_RESCUED", "CAT_CARRIER_COLLECTED", "CAT_RESCUED",
        "TRAIN_PREPARED", "MISSION_WON", "MISSION_FAILED"};
    const string roomSymbols = "<)\\4*~;(!%$27^@&";
    const string carriedSymbols = "<*(%$@^";
    int loadedInventoryCount = -1;
    int roomsLoaded = 0;
    bool roomSeen[NUM_FLOORS][ROOMS_PER_FLOOR] = {};

    string line;
    try
    {
        while (getline(file, line))
        {
            if (line.empty())
                continue;
            if (line.length() > INT_MAX)
                return false;
            int length = static_cast<int>(line.length());
            int pos = 0;
            while (pos < length && line[pos] != ':')
                pos++;
            if (pos == length)
                return false;

            string key = line.substr(0, pos);
            int start = pos + 1;
            while (start < length && (line[start] == ' ' || line[start] == '\t'))
                start++;
            if (start == length)
                return false;
            string value = line.substr(start);

            if (key == "PLAYER_FLOOR" || key == "PLAYER_ROOM" ||
                key == "ENERGY" || key == "TIME" || key == "SCORE" ||
                key == "EMERGENCY_TRIPS_LEFT" || key == "INVENTORY_COUNT")
            {
                for (char ch : value)
                {
                    if (ch < '0' || ch > '9')
                        return false;
                }

                int number = stoi(value);

                if (key == "PLAYER_FLOOR")
                {
                    if (savedFloor != 0 || number < 1 || number > NUM_FLOORS)
                        return false;
                    savedFloor = number;
                }
                else if (key == "PLAYER_ROOM")
                {
                    if (savedRoom != 0 || number < 1 || number > ROOMS_PER_FLOOR)
                        return false;
                    savedRoom = number;
                }
                else if (key == "ENERGY")
                {
                    if (savedEnergy != 0 || number < 1 || number > MAX_ENERGY)
                        return false;
                    savedEnergy = number;
                }
                else if (key == "TIME")
                {
                    if (savedTime != 0 || number < 1 || number > MAX_TIME)
                        return false;
                    savedTime = number;
                }
                else if (key == "SCORE")
                {
                    if (savedScore != -1)
                        return false;
                    savedScore = number;
                }
                else if (key == "EMERGENCY_TRIPS_LEFT")
                {
                    if (savedTrips != -1 || number > 3)
                        return false;
                    savedTrips = number;
                }
                else if (key == "INVENTORY_COUNT")
                {
                    if (loadedInventoryCount != -1 || number > MAX_INVENTORY_CAPACITY)
                        return false;
                    loadedInventoryCount = number;
                }
            }
            else if (key == "GAME_STATUS")
            {
                if (hasStatus || value != "IN_PROGRESS")
                    return false;
                hasStatus = true;
            }
            else if (key == "INVENTORY")
            {
                if (hasInventory)
                    return false;
                hasInventory = true;
                if (value != "NONE")
                {
                    for (int i = 0; i < static_cast<int>(value.length()); i++)
                    {
                        if (value[i] == ' ')
                            continue;
                        if (savedItemCount >= MAX_INVENTORY_CAPACITY || carriedSymbols.find(value[i]) == string::npos)
                            return false;
                        savedInventory[savedItemCount++] = value[i];
                    }
                }
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

                savedCounts[f][r] = count;
                int parsedItems = 0;
                for (int i = 1; i < static_cast<int>(value.length()); i++)
                {
                    if (value[i] == ' ')
                        continue;
                    if (parsedItems >= count || roomSymbols.find(value[i]) == string::npos)
                        return false;
                    savedEntities[f][r][parsedItems++] = value[i];
                }
                if (parsedItems != count)
                    return false;

                roomsLoaded++;
            }
            else
            {
                int i = 0;
                while (i < NUM_OBJECTIVES && objectiveNames[i] != key)
                    i++;
                if (i == NUM_OBJECTIVES || objectiveSeen[i])
                    return false;
                if (!parseYesNo(value, savedObjectives[i]))
                    return false;
                objectiveSeen[i] = true;
            }
        }
    }
    catch (const exception &)
    {
        return false; // Handles corrupt numbers in stoi() without crashing
    }

    if (file.bad() || !hasStatus || !hasInventory || savedFloor == 0 || savedRoom == 0 ||
        savedEnergy == 0 || savedTime == 0 || savedScore < 0 || savedTrips < 0 ||
        loadedInventoryCount != savedItemCount || roomsLoaded != TOTAL_ROOMS)
        return false;

    for (int i = 0; i < NUM_OBJECTIVES; i++)
        if (!objectiveSeen[i])
            return false;

    // Update the game only after the entire save has passed validation.
    playerFloor = savedFloor;
    playerRoom = savedRoom;
    energy = savedEnergy;
    time = savedTime;
    score = savedScore;
    emergencyTripsLeft = savedTrips;
    inventoryCount = savedItemCount;
    for (int i = 0; i < MAX_INVENTORY_CAPACITY; i++)
        inventory[i] = i < inventoryCount ? savedInventory[i] : ENT_NONE;
    for (int i = 0; i < NUM_OBJECTIVES; i++)
        objectives[i] = savedObjectives[i];
    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            entityCounts[f][r] = savedCounts[f][r];
            for (int i = 0; i < MAX_ENTITIES_PER_ROOM; i++)
                roomEntities[f][r][i] = i < savedCounts[f][r] ? savedEntities[f][r][i] : ENT_NONE;
        }
    }
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