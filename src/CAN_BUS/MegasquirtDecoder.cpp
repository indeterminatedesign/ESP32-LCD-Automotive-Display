#include "MegasquirtDecoder.h"


MegasquirtDecoder::MegasquirtDecoder(VehicleData &_signalsRef)
    : _vehicleSignalData(_signalsRef)
{
}

uint16_t MegasquirtDecoder::readU16(const uint8_t *d, uint8_t offset)
{
    return (d[offset] << 8) | d[offset + 1];
}

int16_t MegasquirtDecoder::readS16(const uint8_t *d, uint8_t offset)
{
    return (int16_t)((d[offset] << 8) | d[offset + 1]);
}

uint8_t MegasquirtDecoder::readU8(const uint8_t *d, uint8_t offset)
{
    return d[offset];
}

void MegasquirtDecoder::processFrame(const twai_message_t &msg)
{
    if (msg.extd)
        return;
    if (msg.data_length_code != 8)
        return;

    switch (msg.identifier)
    {
    case 0x5E8:
        decodeGroup0(msg.data);
        break;
    case 0x5E9:
        decodeGroup1(msg.data);
        break;
    case 0x5EA:
        decodeGroup2(msg.data);
        break;
    case 0x5EB:
        decodeGroup3(msg.data);
        break;
    }
}

// 0x5E8 - Group 0
void MegasquirtDecoder::decodeGroup0(const uint8_t *d)
{
    _vehicleSignalData.map.value = readS16(d, 0) / 10.0f;
    _vehicleSignalData.rpm.value = readU16(d, 2);
    _vehicleSignalData.coolant.value = readS16(d, 4) / 10.0f;
    _vehicleSignalData.tps.value = readS16(d, 6) / 10.0f;
}

// 0x5E9 - Group 1
void MegasquirtDecoder::decodeGroup1(const uint8_t *d)
{
        _vehicleSignalData.pw1.value = readU16(d, 0) / 1000.0f; // PW1 is at bytes 0-1
    _vehicleSignalData.iat.value = readS16(d, 4) / 10.0f;  // MAT is at bytes 4-5
  // _vehicleSignalData.iat.decode(d);
    
}

// 0x5EA - Group 2
void MegasquirtDecoder::decodeGroup2(const uint8_t *d)
{
    // Target AFR is at byte 0, AFR 1 is a single byte at offset 1
    _vehicleSignalData.afr1.value = readU8(d, 1) / 10.0f;
    Serial.print("AFR");
    Serial.println(_vehicleSignalData.afr1.value);
}

// 0x5EB - Group 3
void MegasquirtDecoder::decodeGroup3(const uint8_t *d)
{
    // Battery Voltage is at Offset 0 (Bytes 0 and 1)
    _vehicleSignalData.battery.value = readU16(d, 0) / 10.0f;
}