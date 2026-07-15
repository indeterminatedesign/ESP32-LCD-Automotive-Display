#include <Arduino.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lvgl_v8_port.h"
#include <driver/twai.h>
#include "CAN_Bus/VehicleData.h"
#include "CAN_Bus/MegasquirtDecoder.h"
#include "CAN_Bus/IOModuleDecoder.h"
#include "CAN_Bus/CanDataField.h"
#include "CAN_Bus/CANBus.h"
#include "DashboardUI.h"

using namespace esp_panel::board;

Board *board = nullptr;
CANBus canBus;

// 1. Allocate three distinct buffers in memory
VehicleData vehicleDataCANBuffer;   // Buffer A: Modified exclusively by Core 0 decoders
VehicleData vehicleDataShadowBuffer;// Buffer B: The intermediate staging buffer
VehicleData vehicleDataUIBuffer;    // Buffer C: Read exclusively by Core 1 Dashboard

// Pointers to handle triple-buffering
VehicleData* uiBufferPtr = &vehicleDataUIBuffer;

// Decoders look directly at the dedicated CAN input buffer
MegasquirtDecoder msDecoder(vehicleDataCANBuffer);
IOModuleDecoder ioDecoder(vehicleDataCANBuffer);
DashboardUI dashboardUI;

// FreeRTOS Mutex is now ONLY used internally on Core 0 to protect the shadow copy
SemaphoreHandle_t shadowBufferMutex = NULL;

lv_obj_t *rpmLabel;
lv_obj_t *mapLabel;
lv_obj_t *fuelLabel;

// ----------------- CAN Task (PINNED TO CORE 0) -----------------
void canRecieveTask(void *param) {
    while (true) {
        twai_message_t msg;
        bool hasNewData = false;

        // Drain the hardware queue rapidly into vehicleDataCANBuffer (Buffer A)
        while (canBus.receiveMessage(msg)) {
            hasNewData = true;

            // Extended range to 0x5EE to include MAT, BAT V, and AFR frames (0x5E9, 0x5EA, 0x5EB, etc.)
            if (msg.identifier >= 0x5E8 && msg.identifier <= 0x5EE) {
                msDecoder.processFrame(msg);
            } 
            else if (msg.identifier >= 0x600 && msg.identifier <= 0x603) {
                ioDecoder.processFrame(msg);
            }
        }

        // Deep copy from CAN Buffer (A) into Shadow Buffer (B)
        if (hasNewData && shadowBufferMutex != NULL) {
            // Using a 0-tick timeout ensures Core 0 NEVER blocks waiting on Core 1's UI.
            // If the lock is held, we just skip this loop's copy and catch it 2ms later.
            if (xSemaphoreTake(shadowBufferMutex, 0) == pdTRUE) {
                vehicleDataShadowBuffer = vehicleDataCANBuffer; 
                xSemaphoreGive(shadowBufferMutex);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(2)); 
    }
}

void setup() {
    // Disable bluetooth and Wifi
    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_deinit();
    btStop();

    Serial.begin(512000);

    shadowBufferMutex = xSemaphoreCreateMutex();
    if (shadowBufferMutex == NULL) {
        while(1);
    }

    board = new Board();
    board->init();

    auto lcd = board->getLCD();
    #if LVGL_PORT_AVOID_TEARING_MODE
    lcd->configFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);
    #endif

    #if ESP_PANEL_DRIVERS_BUS_ENABLE_RGB && CONFIG_IDF_TARGET_ESP32S3
    auto lcd_bus = lcd->getBus();
    if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB) {
        static_cast<BusRGB *>(lcd_bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 20);
    }
    #endif

    assert(board->begin());

    // 2. Pass the UI Buffer pointer to your Dashboard
    dashboardUI.begin(board, uiBufferPtr);

    canBus.begin();
    Serial.println("System Initialized");

    xTaskCreatePinnedToCore(
        canRecieveTask, "CAN Task", 4096, NULL, 1, NULL, 0
    );
}

// ----------------- UI Task (RUNNING ON CORE 1) -----------------
void loop() {
    // 3. Atomically pull data out of the shadow buffer into the UI buffer.
    // Because we only copy if the mutex is immediately free, this takes less than 1 microsecond.
    if (shadowBufferMutex != NULL) {
        if (xSemaphoreTake(shadowBufferMutex, pdMS_TO_TICKS(0)) == pdTRUE) {
            // Memory block copy into the buffer your UI is holding a pointer to
            vehicleDataUIBuffer = vehicleDataShadowBuffer; 
            xSemaphoreGive(shadowBufferMutex);
        }
    }

    // 4. Render completely lock-free. 
    // Your UI looks at vehicleDataUIBuffer pointer safely, unhindered by CAN updates.
    dashboardUI.render(); 

    vTaskDelay(pdMS_TO_TICKS(10));
}