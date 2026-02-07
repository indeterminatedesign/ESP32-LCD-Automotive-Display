#include "GatewayDecoder.h"

GatewayDecoder::GatewayDecoder(VehicleSignals& _signalsRef)
    : _signals(_signalsRef)
{}

void GatewayDecoder::processFrame(const twai_message_t& msg)
{
    switch(msg.identifier)
    {
        case 0x600:
            _signals.fuelLevel.value = (msg.data[0] << 8 | msg.data[1]) * 0.1f;
            _signals.fuelLevel.lastUpdate = millis();
            break;
        case 0x601:
            _signals.leftTurn.value  = msg.data[0];
            _signals.rightTurn.value = msg.data[1];
            _signals.leftTurn.lastUpdate  = millis();
            _signals.rightTurn.lastUpdate = millis();
            break;
    }
}
