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
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "ifaddrs.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

static const char *TAG = "main";

void updateParam(int id, const std::string &path, float value) {
  std::cout << path << " : " << value << '\n';
}

float buffer = 0;
float output = 0;

void sigLoopInHandler(mpr_sig sig, mpr_sig_evt evt, mpr_id inst, int length,
                      mpr_type type, const void *value, mpr_time time) {
  buffer = *((float *)value);
}

void sigHandler(mpr_sig sig, mpr_sig_evt evt, mpr_id inst, int length,
                mpr_type type, const void *value, mpr_time time) {
  output = *((float *)value);
}

void printHandler(mpr_sig sig, mpr_sig_evt evt, mpr_id inst, int length,
                  mpr_type type, const void *value, mpr_time time) {
  printf("print: %f\n", *((float *)value));
}

void automap(mpr_graph graph) {
  mpr_graph_subscribe(graph, 0, MPR_SIG, -1);
  mpr_graph_add_cb(
      graph,
      [](mpr_graph g, mpr_obj obj, const mpr_graph_evt evt, const void *data) {
        const char *srcName = mpr_obj_get_prop_as_str(obj, MPR_PROP_NAME, 0);

        if (strcmp(srcName, "button1") == 0) {
          mpr_list sigs = mpr_graph_get_objs(g, MPR_SIG);
          // sigs = mpr_list_filter(sigs, MPR_PROP_NAME, 0, 1, MPR_STR, "mix",
          //                        MPR_OP_EQ);
          while (sigs) {
            if (strcmp(mpr_obj_get_prop_as_str(*sigs, MPR_PROP_NAME, 0),
                       "mix") == 0) {
              mpr_obj_push(mpr_map_new(1, &obj, 1, (mpr_sig *)sigs));
              break;
            }
            sigs = mpr_list_get_next(sigs);
          }
        }
      },
      MPR_SIG, 0);
}

void maptest() {
  mpr_dev dev = mpr_dev_new("feedback-test", 0);

  mpr_sig sigTest =
      mpr_sig_new(dev, MPR_DIR_OUT, "sigTest", 1, MPR_FLT, 0, 0, 0, 0, 0, 0);

  mpr_sig sigLoopIn = mpr_sig_new(dev, MPR_DIR_IN, "loopIn", 1, MPR_FLT, 0, 0,
                                  0, 0, sigLoopInHandler, MPR_SIG_UPDATE);

  mpr_sig sigLoopOut =
      mpr_sig_new(dev, MPR_DIR_OUT, "loopOut", 1, MPR_FLT, 0, 0, 0, 0, 0, 0);

  mpr_sig sigMix = mpr_sig_new(dev, MPR_DIR_IN, "mix", 1, MPR_FLT, 0, 0, 0, 0,
                               printHandler, MPR_SIG_UPDATE);

  mpr_sig sigLocalOut =
      mpr_sig_new(dev, MPR_DIR_OUT, "localOut", 1, MPR_FLT, 0, 0, 0, 0, 0, 0);

  mpr_sig sigLocalIn = mpr_sig_new(dev, MPR_DIR_IN, "localIn", 1, MPR_FLT, 0, 0,
                                   0, 0, sigHandler, MPR_SIG_UPDATE);

  while (!mpr_dev_get_is_ready(dev)) {
    mpr_dev_poll(dev, 100);
  }

  mpr_sig sigs[] = {sigLocalOut, sigMix};
  mpr_map map = mpr_map_new(2, sigs, 1, &sigLocalIn);
  const char *expr = "y=(1-x1)*x0+x1*y{-1000}";
  mpr_obj_set_prop(map, MPR_PROP_EXPR, 0, 1, MPR_STR, expr, 1);
  mpr_obj_push(map);

  mpr_obj_push(mpr_map_new(1, &sigTest, 1, &sigLoopIn));

  // automap(mpr_obj_get_graph(dev));

  float mix = 0.0;
  mpr_sig_set_value(sigMix, 0, 1, MPR_FLT, &mix);

  float value = 0.0;
  for (;;) {
    mpr_sig_set_value(sigLocalOut, 0, 1, MPR_FLT, &buffer);
    mpr_sig_set_value(sigTest, 0, 1, MPR_FLT, &value);
    mpr_sig_set_value(sigLoopOut, 0, 1, MPR_FLT, &output);

    value += 0.01;
    if (value > 20) {
      value = 0;
    }

    if (value > 1 && value < 8) {
      mix = 1.0;
    } else {
      mix = 0;
    }
    mpr_sig_set_value(sigMix, 0, 1, MPR_FLT, &mix);

    printf("out: %f\n", output);

    mpr_dev_poll(dev, 0);
    vTaskDelay(10);
  }
}

extern "C" void app_main() {
  // Connect to WiFi
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_netif_create_default_wifi_ap();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  wifi_config_t wifi_config = {
      .ap = {"no-internet", "yoloyolo", .authmode = WIFI_AUTH_WPA2_PSK,
             .max_connection = 4},
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  // MapLooper::MapLooper* mapLooper = new MapLooper::MapLooper();

  // mapLooper->addSignal("testSignal1", 0.0f, 1.0f, updateParam);
  // mapLooper->addSignal("testSignal2", 0.0f, 1.0f, updateParam);

  // mapLooper->autoMap();

  xTaskCreate([](void *) { maptest(); }, "maptest", 16384, 0, 10, 0);

  vTaskDelay(portMAX_DELAY);
}
