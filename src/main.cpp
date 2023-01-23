
#define _MAIN_C_

#include <Arduino.h>
#include <esp_task_wdt.h>
#include <rom/rtc.h>

#include "Loggy.h"
#include "is_timer_expired.h"
#include "mqtt/mqtt.h"
#include "version.h"
#include "wifi/wifi_task.h"

Loggy mainLoggy("main");

String print_reset_reason(RESET_REASON reason) {
  switch (reason) {
    case 1:
      return String("POWERON_RESET");
      break; /**<1, Vbat power on reset*/
    case 3:
      return String("SW_RESET");
      break; /**<3, Software reset digital core*/
    case 4:
      return String("OWDT_RESET");
      break; /**<4, Legacy watch dog reset digital core*/
    case 5:
      return String("DEEPSLEEP_RESET");
      break; /**<5, Deep Sleep reset digital core*/
    case 6:
      return String("SDIO_RESET");
      break; /**<6, Reset by SLC module, reset digital core*/
    case 7:
      return String("TG0WDT_SYS_RESET");
      break; /**<7, Timer Group0 Watch dog reset digital core*/
    case 8:
      return String("TG1WDT_SYS_RESET");
      break; /**<8, Timer Group1 Watch dog reset digital core*/
    case 9:
      return String("RTCWDT_SYS_RESET");
      break; /**<9, RTC Watch dog Reset digital core*/
    case 10:
      return String("INTRUSION_RESET");
      break; /**<10, Instrusion tested to reset CPU*/
    case 11:
      return String("TGWDT_CPU_RESET");
      break; /**<11, Time Group reset CPU*/
    case 12:
      return String("SW_CPU_RESET");
      break; /**<12, Software reset CPU*/
    case 13:
      return String("RTCWDT_CPU_RESET");
      break; /**<13, RTC Watch dog Reset CPU*/
    case 14:
      return String("EXT_CPU_RESET");
      break; /**<14, for APP CPU, reseted by PRO CPU*/
    case 15:
      return String("RTCWDT_BROWN_OUT_RESET");
      break; /**<15, Reset when the vdd voltage is not stable*/
    case 16:
      return String("RTCWDT_RTC_RESET");
      break; /**<16, RTC Watch dog reset digital core and rtc module*/
    default:
      return String("NO_MEAN");
  }
}

String print_meaningful_reset_reason(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_UNKNOWN:
      return String("ESP_RST_UNKNOWN");

    case ESP_RST_POWERON:
      return String("ESP_RST_POWERON");

    case ESP_RST_EXT:
      return String("ESP_RST_EXT");

    case ESP_RST_SW:
      return String("ESP_RST_SW");

    case ESP_RST_PANIC:
      return String("ESP_RST_PANIC");

    case ESP_RST_INT_WDT:
      return String("ESP_RST_INT_WDT");

    case ESP_RST_TASK_WDT:
      return String("ESP_RST_TASK_WDT");

    case ESP_RST_WDT:
      return String("ESP_RST_WDT");

    case ESP_RST_DEEPSLEEP:
      return String("ESP_RST_DEEPSLEEP");

    case ESP_RST_BROWNOUT:
      return String("ESP_RST_BROWNOUT");

    case ESP_RST_SDIO:
      return String("ESP_RST_SDIO");
  }

  return String("LOL!");
}

String reset_reason1;
String reset_reason2;
String meaningful_reason;

void setup() {
  Serial.begin(115200);

  mainLoggy.addLogger(&Serial);

  // https://github.com/espressif/arduino-esp32/issues/595
  // make sure we don't get killed for our long running tasks
  esp_task_wdt_init(20, true);

  mainLoggy.println("\nsetup");
  mainLoggy.println(version);

  reset_reason1     = print_reset_reason(rtc_get_reset_reason(0));
  reset_reason2     = print_reset_reason(rtc_get_reset_reason(1));
  meaningful_reason = print_meaningful_reset_reason(esp_reset_reason());

  mainLoggy.println(reset_reason1);
  mainLoggy.println(reset_reason2);
  mainLoggy.println(meaningful_reason);

  extern void led_task_setup();
  led_task_setup();

  extern void io_task_setup();
  io_task_setup();

  wifi_createTask();

  mqtt_setup();
}

// default is 8k..
SET_LOOP_TASK_STACK_SIZE(4 * 1024)

void loop() {
  static TickTimer watermark_timer = TickTimer(10 * 60 * 1000);

  if (watermark_timer.isExpired(millis())) {
    mainLoggy.printf("stack-watermark(%d): %lu bytes\n", xPortGetCoreID(),
                     uxTaskGetStackHighWaterMark(NULL));

    watermark_timer.reset(millis());
  }
}
