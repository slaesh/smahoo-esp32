

#include <Arduino.h>
#include <WiFi.h>

#include "../app_params/app_params.h"
#include "../version.h"
#include "iizi_wifi_mng.h"
#include "is_timer_expired.h"
#include "mdns.h"
#include "ota_update.h"
#include "webserver_routes.h"
#include "wifi_loggy.h"
#include "wifi_utils.h"

static bool print_ip = false;

static void wifi_event(arduino_event_t *event) {
  if (event == NULL) return;

  switch (event->event_id) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP: {
      print_ip = true;
      break;
    }
  }
}

void wifi_task(void *_) {
  wifiLoggy.println("wifi_task(): started");

  ota_update_init();

  iizi_wifi_mng_init(getOurHostname());
  iizi_app_parameters(iizi_params, n_iizi_params, true);

  WiFi.onEvent(wifi_event);

  const auto webserver = iizi_wifi_mng_webserver_instance();
  wifi_webserver_register_routes(webserver);

  start_mdns_service();

  for (;;) {
    vTaskDelay(100 / portTICK_PERIOD_MS);

    // >>>>>> TODO REMOVE ME
    static uint32_t lastPrinted = millis();
    if (is_timer_expired(millis(), lastPrinted, 10 * 60 * 1000)) {
      wifiLoggy.printf("stack-watermark(%d): %lu bytes\n", xPortGetCoreID(),
                       uxTaskGetStackHighWaterMark(NULL));

      lastPrinted = millis();
    }
    // <<<<<< TODO REMOVE ME

    // if we are NOT connected, just skip the rest..
    if (WiFi.status() != WL_CONNECTED) {
      continue;
    }

    if (print_ip && WiFi.status() == WL_CONNECTED) {
      print_ip = false;

      // wait a little longer.. to be sure we can "print" already! ;)
      vTaskDelay(2000 / portTICK_PERIOD_MS);

      wifiLoggy.printf("connected to %s\n", WiFi.SSID().c_str());
      wifiLoggy.printf("with IP: %s\n", WiFi.localIP().toString().c_str());
      wifiLoggy.printf("fw version: %s\n", version);
    }

    ota_update_loop();
  }

  // should not happen.. ?!
  vTaskDelete(NULL);
}

void wifi_createTask() {
  wifiLoggy.addLogger(&Serial);
  wifiLoggy.println("wifi_createTask");

  // TODO: check this out.. running task on specific cores!
  // https://randomnerdtutorials.com/esp32-dual-core-arduino-ide

  xTaskCreate(wifi_task, "wifi", 1024 * 4, NULL, 2, NULL);
}
