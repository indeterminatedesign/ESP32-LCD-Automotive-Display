#pragma once
#include <Arduino.h>

enum class CANSignalDataType
{
    U16,
    S16,
    Float
};

class CANSignal
{
public:
    CANSignalDataType type;
    float value = 0.0f;
    uint32_t lastUpdate = 0;

    // CAN mapping
    uint32_t canId;
    uint8_t offset;
    float scale;

    CANSignal(uint32_t id = 0, uint8_t off = 0, float scl = 1.0f, CANSignalDataType t = CANSignalDataType::U16)
        : canId(id), offset(off), scale(scl), type(t) {}

    void decode(const uint8_t *data)
    {
        uint16_t raw = (data[offset] << 8) | data[offset + 1];
        switch (type)
        {
        case CANSignalDataType::S16:
            value = ((int16_t)raw) * scale;
            break;
        case CANSignalDataType::U16:
            value = raw * scale;
            break;
        case CANSignalDataType::Float:
            value = *((float *)&raw) * scale;
            break;
        }
        lastUpdate = millis();
    }
};

