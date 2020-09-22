/*
 MapLooper - Embedded Live-Looping Tools for Digital Musical Instruments
 Copyright (C) 2020 Mathias Bredholt

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include "MapLooper/MapLooper.hpp"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

static const char* TAG = "main";

extern "C" void app_main() {
  // Connect to WiFi
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_netif_create_default_wifi_ap();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  wifi_config_t wifi_config = {
      .ap = {"MapLooper", "mappings", .authmode = WIFI_AUTH_WPA2_PSK,
             .max_connection = 4},
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  MapLooper::MapLooper* mapLooper = new MapLooper::MapLooper();

  MapLooper::Loop* loop =
      mapLooper->createLoop("test", MPR_FLT, 1, "slider1", "in/slider2");
  loop->mapMixTo("button1");

  xTaskCreate(
      [](void* userParam) {
        MapLooper::MapLooper* m =
            reinterpret_cast<MapLooper::MapLooper*>(userParam);
        for (;;) {
          m->update();
          vTaskDelay(1);
        }
      },
      "MapLooper", 16384, mapLooper, 3, nullptr);

  vTaskDelay(portMAX_DELAY);
}
