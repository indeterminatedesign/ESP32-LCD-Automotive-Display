#include "DashboardUI.h"

void DashboardUI::begin(esp_panel::board::Board *board, VehicleData *vehicleSignals)
{
    // Store local reference to vehicle signals
    _currentSignals = vehicleSignals;

    // Initialize LVGL Porting Layer
    // This sets up the internal timers so you don't have to manually call lv_timer_handler
    lvgl_port_init(board->getLCD(), board->getTouch());

#if LVGL_PORT_AVOID_TEARING_MODE
    auto lcd = board->getLCD();
    // When avoid tearing function is enabled, the frame buffer number should be set in the board driver
    lcd->configFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);
#endif
    board->getBacklight()->setBrightness(100);

    auto lcd_bus = board->getLCD()->getBus();
    if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB)
    {
        auto rgb_bus = static_cast<esp_panel::drivers::BusRGB *>(lcd_bus);
        rgb_bus->configRGB_BounceBufferSize(800 * 40);
    }

    // Create the UI (Thread-safe)
    lvgl_port_lock(-1);

    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_make(0, 0, 0), 0);
    
        // All tachometer creation code moved here
        meter = lv_meter_create(lv_scr_act());
        lv_obj_set_size(meter, 400, 400);
        lv_obj_align(meter, LV_ALIGN_RIGHT_MID, -15, 20);
        lv_obj_set_style_border_width(meter, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_color(meter, lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_text_color(meter, lv_color_white(), LV_PART_TICKS);
        lv_obj_set_style_text_font(meter, &lv_font_montserrat_30, 0);

        //Redline Stuff
        lv_meter_scale_t *scale1 = lv_meter_add_scale(meter);
        lv_meter_set_scale_range(meter, scale1, 0, 80, 270, 135);

        lv_meter_indicator_t *indic_redline = lv_meter_add_arc(
            meter, scale1, 15, lv_palette_main(LV_PALETTE_RED), 0);
        lv_meter_set_indicator_start_value(meter, indic_redline, 72);
        lv_meter_set_indicator_end_value(meter, indic_redline, 80);

        lv_obj_add_event_cb(meter, scale_label_format_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);

        lv_obj_t *unit_label = lv_label_create(lv_scr_act());
        lv_obj_set_style_text_color(unit_label, lv_color_white(), LV_PART_MAIN);
        lv_label_set_text(unit_label, "1/min x 1000");
        lv_obj_align_to(unit_label, meter, LV_ALIGN_OUT_BOTTOM_MID, 0, -50);

        lv_meter_set_scale_ticks(meter, scale1, 41, 2, 12, lv_color_white());
        lv_meter_set_scale_major_ticks(meter, scale1, 5, 6, 18, lv_color_white(), 25);

        indic_needle = lv_meter_add_needle_line(meter, scale1, 4, lv_palette_main(LV_PALETTE_RED), -60);
    
    _rpmLabel = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(_rpmLabel, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_color(_rpmLabel, lv_palette_main(LV_PALETTE_CYAN), 0);
    lv_obj_align(_rpmLabel, LV_ALIGN_TOP_LEFT, 20, 20);

    _mapLabel = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(_mapLabel, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_color(_mapLabel, lv_palette_main(LV_PALETTE_CYAN), 0);
    lv_obj_align(_mapLabel, LV_ALIGN_TOP_LEFT, 20, 80);

    _fuelLabel = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(_fuelLabel, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_color(_fuelLabel, lv_palette_main(LV_PALETTE_CYAN), 0);
    lv_obj_align(_fuelLabel, LV_ALIGN_TOP_LEFT, 20, 140);

    lvgl_port_unlock();

}

void DashboardUI::render()
{
    VehicleData current = *_currentSignals;
    char buf[32];

    lvgl_port_lock(-1);

    if (abs(current.rpm.value - previousVehicleData.rpm.value) > current.rpm.uiUpdateTolerance)
    {

        lv_label_set_text_fmt(_rpmLabel, "RPM: %d", (int)current.rpm.value);
        
        if (meter && indic_needle)
        {
            lv_meter_set_indicator_value(meter, indic_needle, current.rpm.value / 100);
        }
            previousVehicleData.rpm.value - current.rpm.value;
    }

    if (abs(current.map.value - previousVehicleData.map.value) > current.map.uiUpdateTolerance)
    {
        dtostrf(current.map.value, 0, 2, buf);
        lv_label_set_text_fmt(_mapLabel, "MAP: %s kPa", buf);
        previousVehicleData.map.value = current.map.value;
    }

      if (abs(current.fuelLevel.value - previousVehicleData.fuelLevel.value) > current.fuelLevel.uiUpdateTolerance)
    {
        dtostrf(current.fuelLevel.value, 0, 1, buf);
        lv_label_set_text_fmt(_fuelLabel, "Fuel: %s %%", buf);
        previousVehicleData.fuelLevel.value = current.fuelLevel.value;
    }

    lvgl_port_unlock();
}

void DashboardUI::scale_label_format_cb(lv_event_t *e)
{
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);

    if (dsc->part == LV_PART_TICKS && dsc->text != NULL)
    {
        int display_value = dsc->value / 10;
        sprintf(dsc->text, "%d", display_value);
    }
}
    

void DashboardUI::ui_timer_cb(lv_timer_t *timer)
{
    DashboardUI *self =
        static_cast<DashboardUI *>(timer->user_data);

    if (self)
        self->render();
}