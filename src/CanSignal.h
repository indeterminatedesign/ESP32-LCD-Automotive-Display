#pragma once
#include <Arduino.h>

enum class CanSignalDataType
{
    U16,
    S16,
    Float
};

class CanSignal
{
public:
    CanSignalDataType type;
    float value = 0.0f;
    uint32_t lastUpdate = 0;

    // CAN mapping
    uint32_t canId;
    uint8_t offset;
    float scale;

    CanSignal(uint32_t id = 0, uint8_t off = 0, float scl = 1.0f, CanSignalDataType t = CanSignalDataType::U16)
        : canId(id), offset(off), scale(scl), type(t) {}

    void decode(const uint8_t *data)
    {
        uint16_t raw = (data[offset] << 8) | data[offset + 1];
        switch (type)
        {
        case CanSignalDataType::S16:
            value = ((int16_t)raw) * scale;
            break;
        case CanSignalDataType::U16:
            value = raw * scale;
            break;
        case CanSignalDataType::Float:
            value = *((float *)&raw) * scale;
            break;
        }
        lastUpdate = millis();
    }
};

