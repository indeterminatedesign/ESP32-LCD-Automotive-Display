#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lvgl_v8_port.h"
#include <driver/twai.h>
#include "VehicleSignals.h"
#include "MegasquirtDecoder.h"
#include "CanSignal.h"
#include "CANBus.h"

using namespace esp_panel::board;
CANBus canBus;
VehicleSignals vehicleSignals;
MegasquirtDecoder msDecoder(vehicleSignals);

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

  // 2. Bandwidth Optimization for 800x480
  // This uses a "Bounce Buffer" to prevent flickering on the RGB interface
  auto lcd = board->getLCD();
  auto lcd_bus = lcd->getBus();
  if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB)
  {
    auto rgb_bus = static_cast<esp_panel::drivers::BusRGB *>(lcd_bus);
    rgb_bus->configRGB_BounceBufferSize(800 * 10); // 10 lines of bounce buffer
  }

  // 3. Start the hardware
  assert(board->begin());

  // Start CanBus
  canBus.begin();

  // 4. Turn on Backlight (Brightness 0-100)
  board->getBacklight()->setBrightness(100);

  // 5. Initialize LVGL Porting Layer
  // This sets up the internal timers so you don't have to manually call lv_timer_handler
  lvgl_port_init(board->getLCD(), board->getTouch());

  // 6. Create the UI (Thread-safe)
  lvgl_port_lock(-1);

  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_make(0, 0, 0), 0);

  rpmLabel = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(rpmLabel, &lv_font_montserrat_30, 0);
  lv_obj_set_style_text_color(rpmLabel, lv_palette_main(LV_PALETTE_CYAN), 0);

  mapLabel = lv_label_create(lv_scr_act());
  lv_obj_align(mapLabel, LV_ALIGN_TOP_LEFT, 20, 80);
  lv_obj_set_style_text_font(mapLabel, &lv_font_montserrat_30, 0);
  lv_obj_set_style_text_color(mapLabel, lv_palette_main(LV_PALETTE_CYAN), 0);

  fuelLabel = lv_label_create(lv_scr_act());
  lv_obj_align(fuelLabel, LV_ALIGN_TOP_LEFT, 20, 140);
  lv_obj_set_style_text_font(fuelLabel, &lv_font_montserrat_30, 0);
  lv_obj_set_style_text_color(fuelLabel, lv_palette_main(LV_PALETTE_CYAN), 0);

  lvgl_port_unlock();

  Serial.println("UI Rendered Successfully");
}

void loop()
{
  // With lvgl_port_init, the refresh is handled in a separate task.
  // Your loop is now free for your own logic!
  static uint32_t counter = 0;
  Serial.printf("System Uptime: %d seconds\n", counter++);
  twai_message_t msg;
  if (twai_receive(&msg, pdMS_TO_TICKS(10)) == ESP_OK)
  {
    // Route to correct decoder
    if (msg.identifier >= 0x5E8 && msg.identifier <= 0x5EA)
      msDecoder.processFrame(msg);
  }
  // Example: read signals
  Serial.print("RPM: ");
  Serial.println(vehicleSignals.rpm.value);
  Serial.print("MAP: ");
  Serial.println(vehicleSignals.map.value);
  Serial.print("Fuel: ");
  Serial.println(vehicleSignals.fuelLevel.value);
  Serial.println("---");
  char buffer[64];

  lvgl_port_lock(-1);

  snprintf(buffer, sizeof(buffer), "RPM: %.0f", vehicleSignals.rpm.value);
  lv_label_set_text(rpmLabel, buffer);

  snprintf(buffer, sizeof(buffer), "MAP: %.1f", vehicleSignals.map.value);
  lv_label_set_text(mapLabel, buffer);

  snprintf(buffer, sizeof(buffer), "Fuel: %.1f", vehicleSignals.fuelLevel.value);
  lv_label_set_text(fuelLabel, buffer);

  lvgl_port_unlock();
  delay(100);
}