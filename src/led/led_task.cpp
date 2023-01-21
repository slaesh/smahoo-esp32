#include <Arduino.h>

#include "../Loggy.h"
#include "../app_params/app_params.h"
#include "is_timer_expired.h"

Loggy ledLoggy("led");

#include <NeoPixelBus.h>

NeoPixelBus<NeoGrbFeature, NeoEsp32I2s1Ws2812xMethod> ws2812(1 /* led cnt */,
                                                             21 /* IO21 */);

#define colorSaturation 32

RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor orange(colorSaturation, 20 /* 65% of 32 */, 0);
RgbColor black(0);

RgbColor blinkyColor = orange;

void led_wiFiEvents(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      blinkyColor = green;
      return;

    case SYSTEM_EVENT_STA_DISCONNECTED:
      blinkyColor = orange;
      return;
  }
}

static void led_setup() {
  ws2812.Begin();
  ws2812.Show();

  vTaskDelay(10 / portTICK_PERIOD_MS);

  ws2812.SetPixelColor(0, red);
  ws2812.Show();

  WiFi.onEvent(led_wiFiEvents);
}

static void led_task(void *_) {
  ledLoggy.printf("led_task(): started @ core %d\n", xPortGetCoreID());

  led_setup();

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  for (;;) {
    ws2812.SetPixelColor(0, black);
    ws2812.Show();

    vTaskDelay(50 / portTICK_PERIOD_MS);

    static uint32_t lastPrinted = millis();
    if (is_timer_expired(millis(), lastPrinted, 10 * 60 * 1000)) {
      ledLoggy.printf("stack-watermark(%d): %lu bytes\n", xPortGetCoreID(),
                      uxTaskGetStackHighWaterMark(NULL));

      lastPrinted = millis();
    }

    const String useLed = iizi_get_parameter_value(IIZI_PARAM_USE_LED_KEY);
    if (useLed != "yes") {
      continue;
    }

    ws2812.SetPixelColor(0, blinkyColor);
    ws2812.Show();

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }

  // should not happen.. ?!
  vTaskDelete(NULL);
}

static void led_createTask() {
  ledLoggy.addLogger(&Serial);
  ledLoggy.println("io_createTask");

  xTaskCreate(led_task, "led", 1024 * 2, NULL, 1, NULL);
}

// PUBLIC METHODS

void led_task_setup() { led_createTask(); }
