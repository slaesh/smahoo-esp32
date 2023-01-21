
#include "../app_params/app_params.h"
#include "../board.h"
#include "../version.h"
#include "is_timer_expired.h"
#include "wifi_loggy.h"

#define AUTO_WIFI_UPDATES (true)

#if AUTO_WIFI_UPDATES

#if BOARD == BOARD_IWR_OLD_WO_INPUT
#define OTA_UPDATE_HW_HANDLE F("inwall-relay-old")
#elif BOARD == BOARD_IWR
#define OTA_UPDATE_HW_HANDLE F("inwall-relay")
#else
#error "INVALID BOARD"
#endif

#include <HTTPClient.h>
#include <HTTPUpdate.h>

// https?
// =>
// https://github.com/espressif/arduino-esp32/blob/master/libraries/Update/examples/HTTPS_OTA_Update/HTTPS_OTA_Update.ino

WiFiClient wifiClient;

static String otaUpdate_uri  = "";
bool http_update_in_progress = false;

#define HTTPUPTDATE_INTERVAL (30 * (60 * 1000))  // every 30mins
uint32_t httpUpdate_lastCheck = -HTTPUPTDATE_INTERVAL + 10000;

void ota_update_init() {
  httpUpdate.onProgress([](int cur, int total) {
    static int last = 0;
    if (cur == 0 || cur == total || cur - last > 100000) {
      Serial.printf("%03d/%03d\n", cur, total);
      last = cur;
    }
  });

  httpUpdate.onStart([]() {
    wifiLoggy.println("start updating..");
    http_update_in_progress = true;
  });

  httpUpdate.onEnd([]() {
    Serial.println("update ends");
    http_update_in_progress = false;
  });
}

void ota_update_loop() {
  if (httpUpdate_lastCheck != 0 &&
      is_timer_ticking(millis(), httpUpdate_lastCheck, HTTPUPTDATE_INTERVAL)) {
    return;
  }

  String otaUpdateServer =
      iizi_get_parameter_value(IIZI_PARAM_OTA_UPDATE_HOST_KEY);
  if (otaUpdateServer == "") {
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    return;
  }

  String otaUpdatePort_s =
      iizi_get_parameter_value(IIZI_PARAM_OTA_UPDATE_PORT_KEY);
  uint16_t otaUpdatePort = otaUpdatePort_s.toInt();
  if (otaUpdatePort == 0) {
    otaUpdatePort = 80;
  }

  httpUpdate_lastCheck = millis();

  // TODO: make this configurable..
  httpUpdate.rebootOnUpdate(false);

  otaUpdate_uri = iizi_get_parameter_value(IIZI_PARAM_OTA_UPDATE_PATH_KEY);
  if (!otaUpdate_uri.endsWith("/")) {
    otaUpdate_uri += "/";
  }

  otaUpdate_uri += OTA_UPDATE_HW_HANDLE;
  otaUpdate_uri += "?rev=";
  otaUpdate_uri += 1;
  otaUpdate_uri += "&ch=";

  String otaUpdate_ch =
      iizi_get_parameter_value(IIZI_PARAM_OTA_UPDATE_CHANNEL_KEY);
  otaUpdate_uri += otaUpdate_ch;

  wifiLoggy.printf("checking updates... '%s:%d%s'\n", otaUpdateServer.c_str(),
                   otaUpdatePort, otaUpdate_uri.c_str());

  t_httpUpdate_return updateResult =
      httpUpdate.update(wifiClient, otaUpdateServer, otaUpdatePort,
                        otaUpdate_uri.c_str(), version);

  switch (updateResult) {
    case HTTP_UPDATE_FAILED:
      wifiLoggy.println("failed.");
      break;

    case HTTP_UPDATE_NO_UPDATES:
      wifiLoggy.println("no update found.");
      break;

    case HTTP_UPDATE_OK:
      wifiLoggy.println("DONE!!");
      wifiLoggy.printf("stack-watermark(after update): %lu bytes\n",
                       uxTaskGetStackHighWaterMark(NULL));

      vTaskDelay(
          1000 /
          portTICK_PERIOD_MS);  // give some time to print out the stuff... ;)

      ESP.restart();
      break;
  }
}

#endif
