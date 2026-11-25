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
    return true;
}