#include "webserver_routes.h"

#include "wifi_loggy.h"

const char HTML_CONTENT_TYPE_TEXTPLAIN[] = "text/plain";

void wifi_webserver_register_routes(AsyncWebServer *webserver) {
  // toggle the output
  webserver->on("/output/toggle", HTTP_POST,
                [](AsyncWebServerRequest *request) {
                  wifiLoggy.println("/output/toggle");

                  // TODO: use freertos task communication stuff here..
                  extern void io_toggleOutput(uint8_t _o);
                  io_toggleOutput(0);

                  request->send(200, HTML_CONTENT_TYPE_TEXTPLAIN, "ok");
                });

  // turn the output ON
  webserver->on("/output/on", HTTP_POST, [](AsyncWebServerRequest *request) {
    wifiLoggy.println("/output/on");

    // TODO: use freertos task communication stuff here..
    extern void io_setOutput(uint8_t _o, uint8_t _v);
    io_setOutput(0, 1);

    request->send(200, HTML_CONTENT_TYPE_TEXTPLAIN, "ok");
  });

  // turn the output OFF
  webserver->on("/output/off", HTTP_POST, [](AsyncWebServerRequest *request) {
    wifiLoggy.println("/output/off");

    // TODO: use freertos task communication stuff here..
    extern void io_setOutput(uint8_t _o, uint8_t _v);
    io_setOutput(0, 0);

    request->send(200, HTML_CONTENT_TYPE_TEXTPLAIN, "ok");
  });
}
