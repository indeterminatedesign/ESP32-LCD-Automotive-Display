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

    // Create the RPM Bar
    barRPM = lv_bar_create(lv_scr_act());
    static lv_style_t style_bg_RPM;
    static lv_style_t style_indic_RPM;
    lv_obj_set_size(barRPM, 800, 50);
      lv_obj_align(barRPM, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_style_radius(barRPM, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(barRPM, 0, LV_PART_INDICATOR);
    lv_bar_set_range(barRPM, 0, 80);
    lv_bar_set_value(barRPM, 0, LV_ANIM_OFF);
    lv_style_init(&style_bg_RPM);
    lv_style_set_border_color(&style_bg_RPM, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_border_width(&style_bg_RPM, 2);
    lv_style_set_pad_all(&style_bg_RPM, 4); /*To make the indicator smaller*/
    lv_style_init(&style_indic_RPM);
    lv_style_set_bg_opa(&style_indic_RPM, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indic_RPM, lv_palette_main(LV_PALETTE_GREY));
    lv_obj_add_style(barRPM, &style_bg_RPM, LV_PART_MAIN);
    lv_obj_add_style(barRPM, &style_indic_RPM, LV_PART_INDICATOR);

    // Style used for all labels
    static lv_style_t style_Labels;
    lv_style_init(&style_Labels);
    lv_style_set_text_font(&style_Labels, &lv_font_montserrat_16);
    lv_style_set_text_color(&style_Labels, lv_palette_main(LV_PALETTE_GREY));

    // Style used for all labels values
    static lv_style_t style_ValueLabels;
    lv_style_init(&style_ValueLabels);
    lv_style_set_text_font(&style_ValueLabels, &lv_font_montserrat_30);
    lv_style_set_text_color(&style_ValueLabels, lv_palette_main(LV_PALETTE_GREY));

    // RPM
    _rpmLabel = lv_label_create(lv_scr_act());
    lv_obj_add_style(_rpmLabel, &style_Labels, 0);
    lv_label_set_text(_rpmLabel, "RPM:");
    lv_obj_align(_rpmLabel, LV_ALIGN_TOP_LEFT, 20, 160);
    _rpmValueLabel = lv_label_create(lv_scr_act());
    lv_obj_add_style(_rpmValueLabel, &style_ValueLabels, 0);
    lv_obj_align(_rpmValueLabel, LV_ALIGN_TOP_LEFT, 80, -160);

    // MAP
    _mapLabel = lv_label_create(lv_scr_act());
    lv_obj_add_style(_mapLabel, &style_Labels, 0);
    lv_label_set_text(_mapLabel, "MAP (kPa):");
    lv_obj_align(_mapLabel, LV_ALIGN_TOP_LEFT, 20, -80);
    _mapValueLabel = lv_label_create(lv_scr_act());
    lv_obj_add_style(_mapValueLabel, &style_ValueLabels, 0);
    lv_obj_align(_mapValueLabel, LV_ALIGN_TOP_LEFT, 20, -120);

    // Fuel Bar
    static lv_style_t style_bg;
    static lv_style_t style_indic;
    fuelBar = lv_bar_create(lv_scr_act());
    lv_obj_set_size(fuelBar, 120, 20);
    lv_obj_align(fuelBar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_radius(fuelBar, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(fuelBar, 0, LV_PART_INDICATOR);
    lv_bar_set_range(fuelBar, 0, 8);
    lv_bar_set_value(fuelBar, 4, LV_ANIM_OFF); // HARDCODED FOR TESTING
    lv_style_init(&style_bg);
    lv_style_set_border_color(&style_bg, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_border_width(&style_bg, 2);
    lv_style_set_pad_all(&style_bg, 4); /*To make the indicator smaller*/
    lv_style_init(&style_indic);
    lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_BLUE));
    lv_obj_add_style(fuelBar, &style_bg, LV_PART_MAIN);
    lv_obj_add_style(fuelBar, &style_indic, LV_PART_INDICATOR);

    // Fuel Level Label
    _fuelLabel = lv_label_create(lv_scr_act());
    lv_obj_add_style(_fuelLabel, &style_Labels, 0);
    lv_label_set_text(_fuelLabel, "Fuel Level:");
    lv_obj_align(_fuelLabel, LV_ALIGN_TOP_LEFT, 0, -40);

    // Coolant Temp
    _coolantLabel = lv_label_create(lv_scr_act());
    lv_obj_add_style(_coolantLabel, &style_Labels, 0);
    lv_obj_align(_coolantLabel, LV_ALIGN_LEFT_MID, 20, 0);
    lv_label_set_text(_coolantLabel, "Coolant(C):");
    _coolantValueLabel = lv_label_create(lv_scr_act());
    lv_obj_add_style(_coolantValueLabel, &style_ValueLabels, 0);
    lv_obj_align(_coolantValueLabel, LV_ALIGN_TOP_LEFT, 160, 0);

    // Battery Voltage
    _batteryLabel = lv_label_create(lv_scr_act());
    lv_obj_add_style(_batteryLabel, &style_Labels, 0);
    lv_obj_align(_batteryLabel, LV_ALIGN_TOP_LEFT, 20, 80);
    lv_label_set_text(_batteryLabel, "Battery(V):");
    _batteryValueLabel = lv_label_create(lv_scr_act());
    lv_obj_add_style(_batteryValueLabel, &style_ValueLabels, 0);
    lv_obj_align(_batteryValueLabel, LV_ALIGN_TOP_LEFT, 160, 80);

    lvgl_port_unlock();
}

void DashboardUI::render()
{
    VehicleData current = *_currentSignals;
    char buf[32];

    lvgl_port_lock(-1);

    if (abs(current.rpm.value - previousVehicleData.rpm.value) > current.rpm.uiUpdateTolerance)
    {

        lv_label_set_text_fmt(_rpmValueLabel, "%d", (int)current.rpm.value);
        lv_bar_set_value(barRPM, current.rpm.value / 100, LV_ANIM_OFF);
        previousVehicleData.rpm.lastUpdate = current.rpm.lastUpdate;
        previousVehicleData.rpm.value = current.rpm.value;
    }

    if (abs(current.map.value - previousVehicleData.map.value) > current.map.uiUpdateTolerance)
    {
        dtostrf(current.map.value, 0, 1, buf);
        lv_label_set_text_fmt(_mapValueLabel, "%s", buf);
        previousVehicleData.map.value = current.map.value;
        previousVehicleData.map.lastUpdate = current.map.lastUpdate;
    }

    if (abs(current.fuelLevel.value - previousVehicleData.fuelLevel.value) > current.fuelLevel.uiUpdateTolerance)
    {
        lv_bar_set_value(fuelBar, 4, LV_ANIM_OFF); // HARDCODED FOR TESTING
        previousVehicleData.fuelLevel.value = current.fuelLevel.value;
    }

    if (abs(current.coolant.value - previousVehicleData.coolant.value) > current.coolant.uiUpdateTolerance)
    {
        dtostrf((int)current.coolant.value, 0, 1, buf);
        lv_label_set_text_fmt(_coolantValueLabel, "%s", buf);
        previousVehicleData.coolant.value = current.coolant.value;
    }

    if (abs(current.battery.value - previousVehicleData.battery.value) > current.battery.uiUpdateTolerance)
    {
        dtostrf(current.battery.value, 0, 1, buf);
        lv_label_set_text_fmt(_batteryValueLabel, "%s", buf);
        previousVehicleData.battery.value = current.battery.value;
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