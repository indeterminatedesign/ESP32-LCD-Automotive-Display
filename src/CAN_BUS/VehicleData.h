#pragma once
#include <Arduino.h>
#include "CANDataField.h"

class VehicleData
{
public:
    // Example _signals
    CANDataField rpm;
    CANDataField map;
    CANDataField tps;
    CANDataField coolant;
    CANDataField afr1;
    CANDataField pw1;
    CANDataField battery;
    CANDataField fuelLevel;
    CANDataField leftTurn;
    CANDataField rightTurn;
    CANDataField speed;

    VehicleData();

    void swapBuffers(); // optional if you implement double-buffering
};
