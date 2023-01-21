#include "mqtt_callbacks.h"

#include "Loggy.h"

extern Loggy mqttLoggy;

// TODO: use freertos task communication stuff here..
extern void io_setOutput(uint8_t _o, uint8_t _v);
extern void io_toggleOutput(uint8_t _o);

static void mqttSubCbOutputOn(String &topic, char *payload, int len) {
  mqttLoggy.println("output ON");

  io_setOutput(0, 1);
}

static void mqttSubCbOutputOff(String &topic, char *payload, int len) {
  mqttLoggy.println("output OFF");

  io_setOutput(0, 0);
}

static void mqttSubCbOutputToggle(String &topic, char *payload, int len) {
  mqttLoggy.println("toggle output");

  io_toggleOutput(0);
}

static void mqttSubCbOutputSet(String &topic, char *payload, int len) {
  static char p[60];  // min should be 52 ! "on": false + timestamp..
  if (len > sizeof(p)) return;

  // payload is NOT terminated!
  int pLen = min((int)sizeof(p) - 1, len);
  memcpy(p, payload, pLen);
  p[pLen] = 0;

  String msg = p;
  msg.trim();

  // just an "ON" or "OFF"?
  if (msg.length() <= 3) {
    msg.toLowerCase();

    mqttLoggy.printf("set output: '%s' => %d\n", msg.c_str(), msg == "on");

    io_setOutput(0, msg == "on");
    return;
  }

  // TODO: inspect JSON properly!
  const auto on = String(p).indexOf("true") > 0;
  mqttLoggy.printf("set output: %s => %d\n", p, on);

  io_setOutput(0, on);
}

static void mqttSubCbReset(String &topic, char *payload, int len) {
  static char p[50];
  if (len > sizeof(p)) return;

  // payload is NOT terminated!
  int pLen = min((int)sizeof(p) - 1, len);
  memcpy(p, payload, pLen);
  p[pLen] = 0;

  mqttLoggy.printf("TRY to reset.. '%s'\n", p);

  if (String(p) == "YES") {
    mqttLoggy.println("resetting..");
    vTaskDelay(100);

    ESP.restart();
  }
}

std::vector<t_s_mqtt_subscription> mqttSubscriptions = {
    {"/on", 1, mqttSubCbOutputOn},         {"/off", 1, mqttSubCbOutputOff},
    {"/toggle", 1, mqttSubCbOutputToggle}, {"/set", 1, mqttSubCbOutputSet},
    {"/reset", 1, mqttSubCbReset},
};
