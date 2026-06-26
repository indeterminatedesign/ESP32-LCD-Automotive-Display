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

        }
        break;

        // Turn signals + lights
        case 0x601:
        {
            _vehicleData.leftTurn.value  = msg.data[0];
            _vehicleData.rightTurn.value = msg.data[1];
            _vehicleData.highBeam.value  = msg.data[2];

        }
        break;

        // Vehicle speed
        case 0x602:
        {
            uint16_t raw = (msg.data[0] << 8) | msg.data[1];

            _vehicleData.speed.value = raw * 0.01f;
        }
        break;
        case 0x603:
        {
            _vehicleData.button0.value = msg.data[0];
            _vehicleData.button1.value = msg.data[1];
        }
        break;
    }
}