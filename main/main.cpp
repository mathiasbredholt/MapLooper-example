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

#include "MapLooper/MapLooper.hpp"
#include "esp_event.h"
#include "esp_ota_ops.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

static const char* TAG = "main";

extern "C" void app_main() {
  ESP_LOGI(TAG, "MapLooper v%s", esp_ota_get_app_description()->version);
  ESP_LOGI(TAG, "(c) Mathias Bredholt 2020");

  // Connect to WiFi
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(example_connect());

  MapLooper::MapLooper* mapLooper = new MapLooper::MapLooper();
  mapLooper->addSignal(
      "gate", 0, 1, [](int trackId, const std::string& path, float value) {});
  vTaskDelete(NULL);
}
