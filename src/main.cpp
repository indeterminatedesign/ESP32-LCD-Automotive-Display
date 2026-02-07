#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lvgl_v8_port.h" // Ensure this file is in your src folder

using namespace esp_panel::board;

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

  // 4. Turn on Backlight (Brightness 0-100)
  board->getBacklight()->setBrightness(100);

  // 5. Initialize LVGL Porting Layer
  // This sets up the internal timers so you don't have to manually call lv_timer_handler
  lvgl_port_init(board->getLCD(), board->getTouch());

  // 6. Create the UI (Thread-safe)
  lvgl_port_lock(-1);

  // Create a dark background style
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_make(20, 20, 20), 0);

  // Create "Hello World" Label
  lv_obj_t *label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, "Hello Gemini!");

  // Position and Font
  lv_obj_set_style_text_font(label, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(label, lv_palette_main(LV_PALETTE_CYAN), 0);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, -30);

  // Sub-text for Hardware Confirmation
  lv_obj_t *sub_label = lv_label_create(lv_scr_act());
  lv_label_set_text(sub_label, "Waveshare 7\" S3 - LVGL 8.4");
  lv_obj_set_style_text_color(sub_label, lv_palette_main(LV_PALETTE_GREY), 0);
  lv_obj_align(sub_label, LV_ALIGN_CENTER, 0, 40);

  lvgl_port_unlock();

  Serial.println("UI Rendered Successfully");
}

void loop()
{
  // With lvgl_port_init, the refresh is handled in a separate task.
  // Your loop is now free for your own logic!
  static uint32_t counter = 0;
  Serial.printf("System Uptime: %d seconds\n", counter++);
  delay(1000);
}