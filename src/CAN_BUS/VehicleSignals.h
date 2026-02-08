#pragma once
#include <Arduino.h>
#include "CANSignal.h"

class VehicleSignals
{
public:
    // Example _signals
    CANSignal rpm;
    CANSignal map;
    CANSignal tps;
    CANSignal coolant;
    CANSignal afr1;
    CANSignal pw1;
    CANSignal battery;
    CANSignal fuelLevel;
    CANSignal leftTurn;
    CANSignal rightTurn;

    VehicleSignals();

    void swapBuffers(); // optional if you implement double-buffering
};
