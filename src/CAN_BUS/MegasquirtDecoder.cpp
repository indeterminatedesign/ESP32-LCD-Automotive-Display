#include "MegasquirtDecoder.h"

MegasquirtDecoder::MegasquirtDecoder(VehicleData& _signalsRef)
    : _vehicleSignalData(_signalsRef)
{}

uint16_t MegasquirtDecoder::readU16(const uint8_t* d, uint8_t offset)
{
    return (d[offset] << 8) | d[offset+1];
}

int16_t MegasquirtDecoder::readS16(const uint8_t* d, uint8_t offset)
{
    return (int16_t)((d[offset] << 8) | d[offset+1]);
}

void MegasquirtDecoder::processFrame(const twai_message_t& msg)
{
    if(msg.extd) return;
    if(msg.data_length_code != 8) return;

    switch(msg.identifier)
    {
        case 0x5E8: decodeGroup0(msg.data); break;
        case 0x5E9: decodeGroup1(msg.data); break;
        case 0x5EA: decodeGroup2(msg.data); break;
    }
}

void MegasquirtDecoder::decodeGroup0(const uint8_t* d)
{
    _vehicleSignalData.map.value = readS16(d,0) / 10.0f;
    _vehicleSignalData.rpm.value = readU16(d,2);
    _vehicleSignalData.coolant.value = readS16(d,4) / 10.0f;
    _vehicleSignalData.tps.value = readS16(d,6) / 10.0f;
    _vehicleSignalData.map.lastUpdate = millis();
    _vehicleSignalData.rpm.lastUpdate = millis();
    _vehicleSignalData.coolant.lastUpdate = millis();
    _vehicleSignalData.tps.lastUpdate = millis();
}

void MegasquirtDecoder::decodeGroup1(const uint8_t* d)
{
    _vehicleSignalData.pw1.value = readU16(d,0) / 1000.0f;
    _vehicleSignalData.afr1.value = readU16(d,4) / 10.0f;
    _vehicleSignalData.pw1.lastUpdate = millis();
    _vehicleSignalData.afr1.lastUpdate = millis();
}

void MegasquirtDecoder::decodeGroup2(const uint8_t* d)
{
    _vehicleSignalData.battery.value = readU16(d,2) / 10.0f;
    _vehicleSignalData.battery.lastUpdate = millis();
}
