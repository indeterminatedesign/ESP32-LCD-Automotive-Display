#pragma once
#include <Arduino.h>

enum class CANDataType
{
    U16,
    S16,
    Float
};

class CANDataField
{
public:
    CANDataType type;
    float value = 0.0f;
    uint32_t lastUpdate = 0;

    // CAN mapping
    uint32_t canId;
    uint8_t offset;
    float scale;
    float uiUpdateTolerance; // Amout the value needs to change before a change is made to the UI

    CANDataField(uint32_t id = 0, uint8_t off = 0, float scl = 1.0f, float tolerance = 1.0f, CANDataType t = CANDataType::U16)
        : canId(id),
          offset(off),
          scale(scl),
          uiUpdateTolerance(tolerance), // Add this!
          type(t)
    {
    }
    void decode(const uint8_t *data)
    {
        uint16_t raw = (data[offset] << 8) | data[offset + 1];
        switch (type)
        {
        case CANDataType::S16:
            value = ((int16_t)raw) * scale;
            break;
        case CANDataType::U16:
            value = raw * scale;
            break;
        case CANDataType::Float:
            value = *((float *)&raw) * scale;
            break;
        }
        lastUpdate = millis();
    }
};
