#include "DashboardUI.h"

void DashboardUI::begin(esp_panel::board::Board *board, VehicleSignals *vehicleSignals)
{
    // Store local reference to vehicle signals
    _currentSignals = vehicleSignals;

    // Initialize LVGL Porting Layer
    // This sets up the internal timers so you don't have to manually call lv_timer_handler
    lvgl_port_init(board->getLCD(), board->getTouch());

    board->getBacklight()->setBrightness(100);
    // Configure RGB bounce buffer for smoother updates
    auto lcd_bus = board->getLCD()->getBus();
    if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB)
    {
        auto rgb_bus = static_cast<esp_panel::drivers::BusRGB *>(lcd_bus);
        rgb_bus->configRGB_BounceBufferSize(800 * 20); // 10 lines = ~16 KB
    }

    // Allocate LVGL draw buffers in PSRAM
    static lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(800 * 20 * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    static lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(800 * 20 * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    lv_disp_draw_buf_t draw_buf;
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, 800 * 20);

    lv_timer_create(ui_timer_cb, 33, this); // ~30 FPS

    // Create the UI (Thread-safe)
    lvgl_port_lock(-1);

    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_make(0, 0, 0), 0);

    // All tachometer creation code moved here
    meter = lv_meter_create(lv_scr_act());
    lv_obj_set_size(meter, 440, 440);
    lv_obj_align(meter, LV_ALIGN_RIGHT_MID, -15, 20);
    lv_obj_set_style_border_width(meter, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(meter, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_color(meter, lv_color_white(), LV_PART_TICKS);
    lv_obj_set_style_text_font(meter, &lv_font_montserrat_30, 0);

    lv_meter_scale_t *scale1 = lv_meter_add_scale(meter);
    lv_meter_set_scale_range(meter, scale1, 0, 8000, 270, 135);

    lv_meter_indicator_t *indic_redline = lv_meter_add_arc(
        meter, scale1, 15, lv_palette_main(LV_PALETTE_RED), 0);
    lv_meter_set_indicator_start_value(meter, indic_redline, 7200);
    lv_meter_set_indicator_end_value(meter, indic_redline, 8000);

    lv_obj_add_event_cb(meter, scale_label_format_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);

    lv_obj_t *unit_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(unit_label, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(unit_label, "1/min x 1000");
    lv_obj_align_to(unit_label, meter, LV_ALIGN_OUT_BOTTOM_MID, 0, -320);

    lv_meter_set_scale_ticks(meter, scale1, 41, 2, 12, lv_color_white());
    lv_meter_set_scale_major_ticks(meter, scale1, 5, 6, 18, lv_color_white(), 25);

    indic_needle = lv_meter_add_needle_line(meter, scale1, 8, lv_color_hex(0xFF2226), -20);

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
    VehicleSignals current = *_currentSignals;

    char buf[32];

    if (current.rpm.value != previousVehicleSignals.rpm.value)
    {
        dtostrf(current.rpm.value, 0, 0, buf);
        lv_label_set_text_fmt(_rpmLabel, "RPM: %s", buf);
        /*
        if (meter && indic_needle)
        {
            lv_meter_set_indicator_value(meter, indic_needle, current.rpm.value);
        }
            */
    }

    if (current.map.value != previousVehicleSignals.map.value)
    {
        dtostrf(current.map.value, 0, 2, buf);
        lv_label_set_text_fmt(_mapLabel, "MAP: %s kPa", buf);
    }

    if (current.fuelLevel.value != previousVehicleSignals.fuelLevel.value)
    {
        dtostrf(current.fuelLevel.value, 0, 1, buf);
        lv_label_set_text_fmt(_fuelLabel, "Fuel: %s %%", buf);
    }

    previousVehicleSignals = current;
}

void DashboardUI::scale_label_format_cb(lv_event_t *e)
{
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);

    if (dsc->part == LV_PART_TICKS && dsc->text != NULL)
    {
        int display_value = dsc->value / 1000;
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