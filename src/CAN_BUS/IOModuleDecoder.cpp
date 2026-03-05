#include "IOModuleDecoder.h"

IOModuleDecoder::IOModuleDecoder(VehicleData& signalsRef)
    : _vehicleData(signalsRef)
{}

void IOModuleDecoder::processFrame(const twai_message_t& msg)
{
    switch(msg.identifier)
    {

        // Fuel level
        case 0x600:
        {
            uint16_t raw = (msg.data[0] << 8) | msg.data[1];

            _vehicleData.fuelLevel.value = raw * 0.1f;
            _vehicleData.fuelLevel.lastUpdate = millis();
        }
        break;

        // Turn signals + lights
        case 0x601:
        {
            _vehicleData.leftTurn.value  = msg.data[0];
            _vehicleData.rightTurn.value = msg.data[1];
            _vehicleData.highBeam.value  = msg.data[2];

            uint32_t t = millis();

            _vehicleData.leftTurn.lastUpdate  = t;
            _vehicleData.rightTurn.lastUpdate = t;
            _vehicleData.highBeam.lastUpdate  = t;
        }
        break;

        // Vehicle speed
        case 0x602:
        {
            uint16_t raw = (msg.data[0] << 8) | msg.data[1];

            _vehicleData.speed.value = raw * 0.01f;
            _vehicleData.speed.lastUpdate = millis();
        }
        break;
    }
}