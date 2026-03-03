#include <Arduino.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lvgl_v8_port.h"
#include <driver/twai.h>
#include "CAN_Bus/VehicleData.h"
#include "CAN_Bus/MegasquirtDecoder.h"
#include "CAN_Bus/CanDataField.h"
#include "CAN_Bus/CANBus.h"
#include "DashboardUI.h"

using namespace esp_panel::board;

Board *board = nullptr;
CANBus canBus;
VehicleData vehicleSignals;
VehicleData vehicleSignalsReadBuffer;
MegasquirtDecoder msDecoder(vehicleSignals);
DashboardUI dashboardUI;

lv_obj_t *rpmLabel;
lv_obj_t *mapLabel;
lv_obj_t *fuelLabel;

// ----------------- CAN Task -----------------
void canRecieveTask(void *param)
{
  while (true)
  {
    twai_message_t msg;
    if (canBus.receiveMessage(msg))
    {
      // Route to correct decoder
      if (msg.identifier >= 0x5E8 && msg.identifier <= 0x5EA)
        msDecoder.processFrame(msg);

      // Atomic update for UI
      vehicleSignalsReadBuffer = vehicleSignals;
    }

    vTaskDelay(pdMS_TO_TICKS(50)); // 50 ms delay
  }
}

void setup()
{
  // Disable bluetooth and Wifi
  esp_wifi_disconnect();
  esp_wifi_stop();
  esp_wifi_deinit();
  btStop();

  Serial.begin(115200);

  // Initialize the Board (LCD + IO Expander)
  board = new Board();
  board->init();

  // 2. CONFIGURE BEFORE INIT
  auto lcd = board->getLCD();
#if LVGL_PORT_AVOID_TEARING_MODE
  lcd->configFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);
#endif

#if ESP_PANEL_DRIVERS_BUS_ENABLE_RGB && CONFIG_IDF_TARGET_ESP32S3
    auto lcd_bus = lcd->getBus();
    /**
     * As the anti-tearing feature typically consumes more PSRAM bandwidth, for the ESP32-S3, we need to utilize the
     * "bounce buffer" functionality to enhance the RGB data bandwidth.
     * This feature will consume `bounce_buffer_size * bytes_per_pixel * 2` of SRAM memory.
     */
    if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB) {
        static_cast<BusRGB *>(lcd_bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 10);
    }
#endif

  // Start the hardware
  assert(board->begin());

  // Render the UI
  dashboardUI.begin(board, &vehicleSignalsReadBuffer);

  // Start CanBus
  canBus.begin();

  Serial.println("UI Rendered Successfully");

  // Create CAN task pinned to core 0
  xTaskCreatePinnedToCore(
      canRecieveTask, // Task function
      "CAN Task",     // Name
      4096,           // Stack size (bytes)
      NULL,           // Parameter
      1,              // Priority
      NULL,           // Task handle
      0               // Core ID
  );
}

void loop()
{
  dashboardUI.render();
  // Empty loop, all work is done in the CAN task
  vTaskDelay(pdMS_TO_TICKS(20));
}
