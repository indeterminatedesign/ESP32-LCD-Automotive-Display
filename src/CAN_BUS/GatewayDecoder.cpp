#include "GatewayDecoder.h"

GatewayDecoder::GatewayDecoder(VehicleData& _signalsRef)
    : _vehicleSignalData(_signalsRef)
{}

void GatewayDecoder::processFrame(const twai_message_t& msg)
{
    switch(msg.identifier)
    {
        case 0x600:
            _vehicleSignalData.fuelLevel.value = (msg.data[0] << 8 | msg.data[1]) * 0.1f;
            _vehicleSignalData.fuelLevel.lastUpdate = millis();
            break;
        case 0x601:
            _vehicleSignalData.leftTurn.value  = msg.data[0];
            _vehicleSignalData.rightTurn.value = msg.data[1];
            _vehicleSignalData.leftTurn.lastUpdate  = millis();
            _vehicleSignalData.rightTurn.lastUpdate = millis();
            break;
    }
}
