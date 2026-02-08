#pragma once
#include <Arduino.h>
#include "VehicleSignals.h"
#include "driver/twai.h"

class GatewayDecoder
{
public:
    GatewayDecoder(VehicleSignals& _signalsRef);

    void processFrame(const twai_message_t& msg);

private:
    VehicleSignals& _signals;
};
