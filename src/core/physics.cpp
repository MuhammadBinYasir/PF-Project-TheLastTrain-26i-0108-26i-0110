#include "physics.h"
#include "util.h"

// Movement validation and resource adjustments without structures

bool hasItem(const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount, char itemSymbol)
{
    for (int i = 0; i < inventoryCount; i++)
    {
        if (inventory[i] == itemSymbol)
            return true;
    }
    return false;
}

bool addItem(char inventory[MAX_INVENTORY_CAPACITY], int &inventoryCount, char itemSymbol)
{
    if (inventoryCount < 0 || inventoryCount >= MAX_INVENTORY_CAPACITY)
        return false;
    if (hasItem(inventory, inventoryCount, itemSymbol))
        return false;
    inventory[inventoryCount] = itemSymbol;
    inventoryCount++;
    return true;
}

bool removeItem(char inventory[MAX_INVENTORY_CAPACITY], int &inventoryCount, char itemSymbol)
{
    if (inventoryCount < 0 || inventoryCount > MAX_INVENTORY_CAPACITY)
        return false;
    if (!hasItem(inventory, inventoryCount, itemSymbol))
        return false;
    for (int i = 0; i < inventoryCount; i++)
    {
        if (inventory[i] == itemSymbol)
        {
            for (int j = i; j < inventoryCount - 1; j++)
            {
                inventory[j] = inventory[j + 1];
            }
            inventoryCount--;
            inventory[inventoryCount] = ENT_NONE;

            return true;
        }
    }
    return false;
}

void applyResourceDelta(int &energy, int &time, int deltaEnergy, int deltaTime)
{
    energy += deltaEnergy;
    time += deltaTime;
    if (energy < 0)
        energy = 0;
    else if (energy > MAX_ENERGY)
        energy = MAX_ENERGY;

    if (time < 0)
        time = 0;
    else if (time > MAX_TIME)
        time = MAX_TIME;
}

void applyScoreDelta(int &score, int deltaScore)
{
    score += deltaScore;
}

bool isDirectlyConnected(int fromF, int fromR, int toF, int toR,
                         const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                         char &outConnType)
{
    outConnType = CONN_NONE;
    if (fromF < 1 || fromF > NUM_FLOORS || toF < 1 || toF > NUM_FLOORS ||
        fromR < 1 || fromR > ROOMS_PER_FLOOR || toR < 1 || toR > ROOMS_PER_FLOOR)
    {
        return false;
    }
    if (fromF == toF)
    {
        if (fromR == toR)
            return false;
        int boundary;
        if (fromR < toR)
            boundary = fromR - 1;
        else
            boundary = toR - 1;

        char connection = horizontalConn[fromF - 1][boundary];
        if (connection == CONN_DOOR || connection == CONN_WINDOW || connection == CONN_LOCKED_DOOR)
        {
            outConnType = connection;
            return true;
        }

        return false;
    }

    if (fromR == 1 && toR == 1)
    {
        bool staircase1 = (fromF == 1 && toF == 2) || (fromF == 2 && toF == 1);
        bool staircase2 = (fromF == 2 && toF == 3) || (fromF == 3 && toF == 2);
        bool staircase3 = (fromF == 4 && toF == 5) || (fromF == 5 && toF == 4);

        if (staircase1 || staircase2 || staircase3)
        {
            outConnType = CONN_STAIRS;
            return true;
        }
    }

    bool lift1Floors = (fromF == 1 && toF == 2) || (fromF == 2 && toF == 1);

    bool fromLift1Room = (fromR == 2 || fromR == 3);
    bool toLift1Room = (toR == 2 || toR == 3);

    if (lift1Floors && fromLift1Room && toLift1Room)
    {
        outConnType = CONN_LIFT1;
        return true;
    }

    bool fromLift2Floor = (fromF == 2 || fromF == 4 || fromF == 5);

    bool toLift2Floor = (toF == 2 || toF == 4 || toF == 5);

    if (fromR == 4 && toR == 4 &&
        fromLift2Floor && toLift2Floor)
    {
        outConnType = CONN_LIFT2;
        return true;
    }

    return false;
}

bool canTraverse(int targetFloor, int targetRoom, char connType,
                 const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                 int emergencyTripsLeft, bool lift2Operational)
{

    if (targetFloor < 1 || targetFloor > NUM_FLOORS || targetRoom < 1 || targetRoom > ROOMS_PER_FLOOR) return false;

    if (connType == CONN_DOOR || connType == CONN_LIFT1 || connType == CONN_WINDOW || connType == CONN_STAIRS)
        return true;
        
    if (connType == CONN_SOLID_WALL)
        return false;

    if (connType == CONN_LOCKED_DOOR)
    {
        if (!hasItem(inventory, inventoryCount, '<'))
            return false;
        return true;
    }

    return false;
}