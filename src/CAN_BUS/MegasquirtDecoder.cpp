#include "MegasquirtDecoder.h"

MegasquirtDecoder::MegasquirtDecoder(VehicleSignals& _signalsRef)
    : _signals(_signalsRef)
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
    _signals.map.value = readS16(d,0) / 10.0f;
    _signals.rpm.value = readU16(d,2);
    _signals.coolant.value = readS16(d,4) / 10.0f;
    _signals.tps.value = readS16(d,6) / 10.0f;
    _signals.map.lastUpdate = millis();
    _signals.rpm.lastUpdate = millis();
    _signals.coolant.lastUpdate = millis();
    _signals.tps.lastUpdate = millis();
}

void MegasquirtDecoder::decodeGroup1(const uint8_t* d)
{
    _signals.pw1.value = readU16(d,0) / 1000.0f;
    _signals.afr1.value = readU16(d,4) / 10.0f;
    _signals.pw1.lastUpdate = millis();
    _signals.afr1.lastUpdate = millis();
}

void MegasquirtDecoder::decodeGroup2(const uint8_t* d)
{
    _signals.battery.value = readU16(d,2) / 10.0f;
    _signals.battery.lastUpdate = millis();
}
