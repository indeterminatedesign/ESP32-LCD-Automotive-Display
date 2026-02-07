#pragma once
#include <Arduino.h>
#include "CanSignal.h"

class VehicleSignals
{
public:
    // Example _signals
    CanSignal rpm;
    CanSignal map;
    CanSignal tps;
    CanSignal coolant;
    CanSignal afr1;
    CanSignal pw1;
    CanSignal battery;
    CanSignal fuelLevel;
    CanSignal leftTurn;
    CanSignal rightTurn;

    VehicleSignals();

    void swapBuffers(); // optional if you implement double-buffering
};
