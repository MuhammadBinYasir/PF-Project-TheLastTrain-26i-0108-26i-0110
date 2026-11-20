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
    if (inventoryCount < 0 || inventoryCount > MAX_INVENTORY_CAPACITY) return false;
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

void applyResourceDelta(int& energy, int& time, int deltaEnergy, int deltaTime){
    energy += deltaEnergy;
    time += deltaTime;
    if (energy < 0) energy = 0;
    else if (energy > MAX_ENERGY) energy = MAX_ENERGY;

    if (time < 0) time = 0;
    else if (time > MAX_TIME) time = MAX_TIME;
}
void applyScoreDelta(int& score, int deltaScore){
    score += deltaScore;
}