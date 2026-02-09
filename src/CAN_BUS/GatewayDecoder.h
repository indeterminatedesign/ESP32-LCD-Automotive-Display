#pragma once
#include <Arduino.h>
#include "VehicleData.h"
#include "driver/twai.h"

class GatewayDecoder
{
public:
    GatewayDecoder(VehicleData& _signalsRef);

    void processFrame(const twai_message_t& msg);

private:
    VehicleData& _vehicleSignalData;
};
