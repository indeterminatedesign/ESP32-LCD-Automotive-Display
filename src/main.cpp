#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lvgl_v8_port.h"
#include <driver/twai.h>
#include "CAN_Bus/VehicleSignals.h"
#include "CAN_Bus/MegasquirtDecoder.h"
#include "CAN_Bus/CanSignal.h"
#include "CAN_Bus/CANBus.h"
#include "DashboardUI.h"

using namespace esp_panel::board;
Board *board = nullptr;
CANBus canBus;
VehicleSignals vehicleSignals;
VehicleSignals vehicleSignalsReadBuffer;
MegasquirtDecoder msDecoder(vehicleSignals);
DashboardUI dashboardUI;

lv_obj_t *rpmLabel;
lv_obj_t *mapLabel;
lv_obj_t *fuelLabel;

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting Waveshare 7-inch Hello World...");

  // 1. Initialize the Board (LCD + IO Expander)
  Board *board = new Board();
  board->init();


  // 3. Start the hardware
  assert(board->begin());

  // Start CanBus
  canBus.begin();

  // Render the UI
  dashboardUI.begin(board, &vehicleSignalsReadBuffer);

  Serial.println("UI Rendered Successfully");
}

void loop()
{
  twai_message_t msg;
  if (canBus.receiveMessage(msg))
  {
    // Route to correct decoder
    if (msg.identifier >= 0x5E8 && msg.identifier <= 0x5EA)
      msDecoder.processFrame(msg);
    vehicleSignalsReadBuffer = vehicleSignals; // atomic update for UI
  }
  delay(5);
}