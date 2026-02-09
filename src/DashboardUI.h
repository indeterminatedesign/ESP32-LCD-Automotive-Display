#pragma once
#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lvgl_v8_port.h"
#include "CAN_Bus/VehicleData.h"

class DashboardUI
{
public:
    void begin(esp_panel::board::Board *board, VehicleData *vehicleSignals);
    void render();
private:
    esp_panel::board::Board *_board;
    lv_obj_t *_rpmLabel;
    lv_obj_t *_mapLabel;
    lv_obj_t *_fuelLabel;
    lv_meter_indicator_t *indic_needle;
    lv_obj_t *meter;
    VehicleData *_currentSignals = nullptr;
    VehicleData previousVehicleData;
    static void scale_label_format_cb(lv_event_t *e);
    static void ui_timer_cb(lv_timer_t *timer);

};