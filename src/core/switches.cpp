#include "switches.h"
#include "util.h"

// Door lock mechanics, lifts, and cabinet verification without structures

bool canUseLift1(int fromFloor, int toFloor){
    if((fromFloor == 1 && toFloor == 2) || (fromFloor == 2 && toFloor == 1)) return true;
    return false;
}
bool canUseLift2(int fromFloor, int toFloor, bool lift2Operational, int emergencyTripsLeft){
    bool fromLift2Floor = (fromFloor == 2 || fromFloor == 4 || fromFloor == 5);
    bool toLift2Floor = (toFloor == 2 || toFloor == 4 || toFloor == 5);

    if(!fromLift2Floor || !toLift2Floor || (fromFloor == toFloor)) return false;

    if(lift2Operational) return true;

    if(emergencyTripsLeft > 0) return true;

    return false;
}
