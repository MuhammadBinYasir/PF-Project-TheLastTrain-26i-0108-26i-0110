#include "switches.h"
#include "util.h"
#include "level.h"
#include "physics.h"

// Door lock mechanics, lifts, and cabinet verification without structures

bool canUseLift1(int fromFloor, int toFloor)
{
    if ((fromFloor == 1 && toFloor == 2) || (fromFloor == 2 && toFloor == 1))
        return true;
    return false;
}
bool canUseLift2(int fromFloor, int toFloor, bool lift2Operational, int emergencyTripsLeft)
{
    bool fromLift2Floor = (fromFloor == 2 || fromFloor == 4 || fromFloor == 5);
    bool toLift2Floor = (toFloor == 2 || toFloor == 4 || toFloor == 5);

    if (!fromLift2Floor || !toLift2Floor || (fromFloor == toFloor))
        return false;

    if (lift2Operational)
        return true;

    if (emergencyTripsLeft > 0)
        return true;

    return false;
}
bool unlockFloor3Door(const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount)
{
    if (inventoryCount < 0 || inventoryCount > MAX_INVENTORY_CAPACITY)
        return false;
    return hasItem(inventory, inventoryCount, ENT_MASTER_KEY);
}

bool installFuseControlPanel(char inventory[MAX_INVENTORY_CAPACITY], int &inventoryCount, bool objectives[NUM_OBJECTIVES], int &score)
{
    if (inventoryCount < 0 || inventoryCount > MAX_INVENTORY_CAPACITY)
        return false;

    if (objectives[OBJ_FUSE_INSTALLED])
        return false;

    if (!removeItem(inventory, inventoryCount, ENT_FUSE))
    {
        return false;
    }

    objectives[OBJ_FUSE_INSTALLED] = true;
    objectives[OBJ_LIFT2_ACTIVE] = true;
    applyScoreDelta(score, SCORE_INSTALL_FUSE);
    return true;
}

bool verifyCabinetCode(const char* enteredCode, const bool objectives[NUM_OBJECTIVES]){
    if (enteredCode == nullptr) return false;

    if(!objectives[OBJ_CLUE_4] || !objectives[OBJ_CLUE_2] || !objectives[7]) return false;

    if(stringEquals(enteredCode, "427")) return true;

    return false;
}