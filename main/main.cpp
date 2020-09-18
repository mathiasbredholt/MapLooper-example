/*
           __
  /\/\    / /
 /    \  / /
/ /\/\ \/ /___
\/    \/\____/
MapLooper
(c) Mathias Bredholt 2020

*-0-*-0-*-0-*-0-*-0-*-0-*-0-*-0-*-0-*

main.cpp
main

*/

#include <iostream>

#include "MapLooper/MapLooper.hpp"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

static const char* TAG = "main";

void updateParam(int id, const std::string& path, float value) {
  std::cout << path << " : " << value << '\n';
}

extern "C" void app_main() {
  // Connect to WiFi
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(example_connect());

  MapLooper::MapLooper* mapLooper = new MapLooper::MapLooper();

  mapLooper->addSignal("testSignal1", 0.0f, 1.0f, updateParam);
  mapLooper->addSignal("testSignal2", 0.0f, 1.0f, updateParam);

  mapLooper->autoMap();

  for (;;) {
    vTaskDelay(portMAX_DELAY);
  }
}
