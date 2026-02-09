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
void canTask(void *param)
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
  btStop();
    //Disable bluetooth and Wifi
  esp_wifi_disconnect();
  esp_wifi_stop();
  esp_wifi_deinit();
  btStop();

  Serial.begin(115200);
  Serial.println("Starting Waveshare 7-inch Hello World...");

  // Initialize the Board (LCD + IO Expander)
  board = new Board();
  board->init();

  // Start the hardware
  assert(board->begin());

  // Start CanBus
  canBus.begin();

  // Render the UI
  dashboardUI.begin(board, &vehicleSignalsReadBuffer);

  Serial.println("UI Rendered Successfully");

  // Create CAN task pinned to core 0
  xTaskCreatePinnedToCore(
      canTask,          // Task function
      "CAN Task",       // Name
      4096,             // Stack size (bytes)
      NULL,             // Parameter
      1,                // Priority
      NULL,             // Task handle
      0                 // Core ID
  );
}

void loop()
{
  dashboardUI.render();
  // Empty loop, all work is done in the CAN task
  vTaskDelay(pdMS_TO_TICKS(50));
}
