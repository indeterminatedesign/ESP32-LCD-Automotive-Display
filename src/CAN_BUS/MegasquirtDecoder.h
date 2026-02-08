#pragma once
#include "VehicleSignals.h"
#include "driver/twai.h"

class MegasquirtDecoder
{
public:
    MegasquirtDecoder(VehicleSignals& signalsRef);

    void processFrame(const twai_message_t& msg);

private:
    VehicleSignals& _signals;

    void decodeGroup0(const uint8_t* data);
    void decodeGroup1(const uint8_t* data);
    void decodeGroup2(const uint8_t* data);

    uint16_t readU16(const uint8_t* d, uint8_t offset);
    int16_t  readS16(const uint8_t* d, uint8_t offset);
};
