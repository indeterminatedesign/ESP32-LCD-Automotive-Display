#include "VehicleSignals.h"

VehicleSignals::VehicleSignals()
    : rpm(0x5E8,2,1.0f,CANSignalDataType::U16),
      map(0x5E8,0,0.1f,CANSignalDataType::S16),
      tps(0x5E8,6,0.1f,CANSignalDataType::S16),
      coolant(0x5E8,4,0.1f,CANSignalDataType::S16),
      pw1(0x5E9,0,0.001f,CANSignalDataType::U16),
      afr1(0x5E9,4,0.1f,CANSignalDataType::U16),
      battery(0x5EA,2,0.1f,CANSignalDataType::U16),
      fuelLevel(0x600,0,0.1f,CANSignalDataType::U16),
      leftTurn(0x601,0,1.0f,CANSignalDataType::U16),
      rightTurn(0x601,1,1.0f,CANSignalDataType::U16)
{}
