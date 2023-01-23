#define __MQTT_C__

#include <ArduinoJson.h>
#include <AsyncMqttClient.h>
#include <NeoPixelBus.h>
#include <WiFi.h>
#include <stdlib.h>

#include "Loggy.h"
#include "board.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "version.h"
Loggy mqttLoggy("mqtt");

#include "../app_params/app_params.h"
#include "is_timer_expired.h"
#include "mqtt.h"
#include "mqtt_callbacks.h"

static char payload[256];
uint32_t loopCounter                  = 0;
static uint32_t mqttDisconnectedSince = 0;

static String mqttServer = "";
static uint16_t mqttPort = 1883;
static String mqttUser   = "";
static String mqttPass   = "";
static AsyncMqttClient mqttClient;

extern const char *getOurHostname();

static void parseMqttParameters() {
  mqttServer = iizi_get_parameter_value(IIZI_PARAM_MQTT_HOST_KEY);
  mqttServer.trim();

  String mqttPort_s = iizi_get_parameter_value(IIZI_PARAM_MQTT_PORT_KEY);
  mqttPort_s.trim();
  mqttPort = mqttPort_s.toInt();

  mqttUser = iizi_get_parameter_value(IIZI_PARAM_MQTT_USER_KEY);
  mqttUser.trim();

  mqttPass = iizi_get_parameter_value(IIZI_PARAM_MQTT_PASS_KEY);
  mqttPass.trim();
}

static void connectToMqtt() {
  static uint32_t disconnected_while_wifi_is_okay = 0;

  if (mqttClient.connected()) {
    disconnected_while_wifi_is_okay = 0;
    mqttLoggy.println("already connected...");
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    disconnected_while_wifi_is_okay = 0;
    // mqttLoggy.println("WiFi NOT connected...");
    return;
  }

  parseMqttParameters();

  if (mqttServer.isEmpty()) {
    return;
  }

  if (mqttPort == 0) {
    mqttPort = 1883;
  }

  String brokerUri = mqttUser;
  if (!brokerUri.isEmpty()) {
    brokerUri += ":";
    brokerUri += mqttPass;
    brokerUri += "@";
  }
  brokerUri += mqttServer + ":" + mqttPort;

  const auto mqttClientId = getOurHostname();
  mqttLoggy.printf("connecting to '%s' using clientId '%s'.. %d\n",
                   brokerUri.c_str(), mqttClientId,
                   disconnected_while_wifi_is_okay);

  // init client

  mqttClient.setServer(mqttServer.c_str(), mqttPort);
  mqttClient.setCredentials(mqttUser.c_str(), mqttPass.c_str());
  mqttClient.setCleanSession(true);
  mqttClient.setClientId(mqttClientId);
  mqttClient.setKeepAlive(20);

  mqttClient.disconnect(true);
  mqttClient.connect();

  ++disconnected_while_wifi_is_okay;

  if (disconnected_while_wifi_is_okay >= 15) {
    // disconnected for a minute? but wifi seems fine? Oo

    mqttLoggy.println("something seems odd?! we will reset now!!");

    vTaskDelay(2000 / portTICK_PERIOD_MS);

    ESP.restart();
  }
}

static String baseTopic = getOurHostname();

static void publishHomeAssistantAutoDiscovery() {
  /*
  https://www.home-assistant.io/docs/mqtt/discovery/#another-example-using-abbreviations-topic-name-and-base-topic
  {
    "~": "<base-topic>",
    "name": "<device-id OR user-entered-string-via-config-portal>",
    "uniq_id": "<device-id>", // uniq_id short for unique_id
    "obj_id":"<device-id>", // object_id short for obj_id
    "cmd_t": "~/set",
    "stat_t": "~/state",
    "schema": "json",
    "val_tpl": '{{ value_json.o }}' // state_value_template
    "stat_on": 1,
    "stat_off": 0,
    "pl_on": "ON" // payload_on
    "pl_off": "OFF" // payload_off
  }
  */

  // allocate the memory for the document
  const size_t CAPACITY = JSON_OBJECT_SIZE(12 + 1 /* number of keys */);
  StaticJsonDocument<CAPACITY> doc;
  JsonObject object = doc.to<JsonObject>();

  const auto deviceId = getOurHostname();
  String friendlyName = iizi_get_parameter_value(IIZI_PARAM_FRIENDLY_NAME_KEY);
  // no friendly name given (yet)?
  if (friendlyName == "") {
    // .. fallback to our device-id
    friendlyName = deviceId;
  }

  object["~"]        = baseTopic.c_str();
  object["name"]     = friendlyName.c_str();
  object["uniq_id"]  = deviceId;
  object["obj_id"]   = deviceId;
  object["cmd_t"]    = "~/set";
  object["stat_t"]   = "~/state";
  object["schema"]   = "json";
  object["val_tpl"]  = "{{ value_json.o }}";  // do NOT use stat_val_tpl ?!
  object["stat_on"]  = 1;
  object["stat_off"] = 0;
  object["pl_on"]    = "on";
  object["pl_off"]   = "off";

  // serialize the object and send the result to Serial
  char buf[512] = {0};
  const auto n  = serializeJson(doc, buf, sizeof(buf));

  // sending to "homeassistant/switch/__id__/config"
  String haConfigTopic("homeassistant/switch/");
  haConfigTopic += deviceId;
  haConfigTopic += "/config";

  mqttClient.publish(haConfigTopic.c_str(), 1, true, buf, n);
}

static void onMqttConnect(bool sessionPresent) {
  mqttLoggy.println("connected!");

  for (const auto sub : mqttSubscriptions) {
    String subTopic  = baseTopic + sub.topic;
    const auto subId = mqttClient.subscribe(subTopic.c_str(), 2);

    mqttLoggy.printf("subscribed to '%s' .. %d\n", subTopic.c_str(), subId);
  }

  static uint32_t reconnectCnt = 0;
  snprintf(payload, 256, "{ \"cnt\": \"%d\" }", reconnectCnt);
  String connectedTopic = baseTopic + "/connected";
  mqttClient.publish(connectedTopic.c_str(), 2, true, payload);

  ++reconnectCnt;

  static bool doItOnce = true;
  if (doItOnce) {
    mqttLoggy.println("first connection!");

    extern String reset_reason1;
    extern String reset_reason2;
    extern String meaningful_reason;

    snprintf(payload, 256,
             "{ \"v\": \"%s\", \"r1\": \"%s\", \"r2\": \"%s\", \"mr\": \"%s\", "
             "\"hw\": \"%s\" }",
             version, reset_reason1.c_str(), reset_reason2.c_str(),
             meaningful_reason.c_str(),
#if BOARD == BOARD_IWR_OLD_WO_INPUT
             "old"
#elif BOARD == BOARD_IWR
             "rev1"
#else
#error "invalid config"
#endif
    );
    String startedTopic = baseTopic + "/started";
    mqttClient.publish(startedTopic.c_str(), 2, true, payload);

    doItOnce = false;
  }

  publishHomeAssistantAutoDiscovery();
}

static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  mqttLoggy.println("disconnected..");
  mqttClient.disconnect(true);
  mqttDisconnectedSince = millis();
}

static void onMqttSubscribe(uint16_t packetId, uint8_t qos) {}

static void onMqttUnsubscribe(uint16_t packetId) {}

static void onMqttMessage(char *topic, char *payload,
                          AsyncMqttClientMessageProperties properties,
                          size_t len, size_t index, size_t total) {
  mqttLoggy.printf("rx: %s .. payload-len: %d\n", topic, len);

  String t = topic;
  t.toLowerCase();

  for (const auto sub : mqttSubscriptions) {
    String subTopic = baseTopic + sub.topic;
    // Serial.printf("'%s' == '%s' => %d\n", subTopic.c_str(), t.c_str(),
    // subTopic == t); Serial.flush();

    if (subTopic == t) {
      sub.cb(t, payload, len);

      return;
    }
  }

  mqttLoggy.println(".. topic not processed ..");
}

static void onMqttPublish(uint16_t packetId) {}

static void publishState() {
  // allocate the memory for the document
  const size_t CAPACITY = JSON_OBJECT_SIZE(3 + 1 /* number of keys */);
  StaticJsonDocument<CAPACITY> doc;
  JsonObject object = doc.to<JsonObject>();

  extern uint8_t io_getInputState();
  auto io_inputState = io_getInputState();

  extern uint8_t io_getOutputState();
  auto io_outputState = io_getOutputState();

  object["c"] = loopCounter;
  object["o"] = io_outputState;
  object["i"] = io_inputState;

  // serialize the object and send the result to Serial
  char buf[64] = {0};
  const auto n = serializeJson(doc, buf, sizeof(buf));

  String stateTopic = baseTopic + "/state";
  mqttClient.publish(stateTopic.c_str(), 1, true, buf, n);
}

static void mqtt_task(void *_) {
  mqttLoggy.printf("mqtt_task(): started @ core %d\n", xPortGetCoreID());

  static TickTimer send_state_timer = TickTimer(5000);

  for (;;) {
    static const TickType_t taskDelayInTicks = 10 / portTICK_PERIOD_MS;
    auto ioChanges =
        ulTaskNotifyTakeIndexed(0 /* IO CHANGES */, pdTRUE, taskDelayInTicks);

    // >>>>>> TODO REMOVE ME
    static uint32_t lastPrinted = millis();
    if (is_timer_expired(millis(), lastPrinted, 10 * 60 * 1000)) {
      mqttLoggy.printf("stack-watermark(%d): %lu bytes\n", xPortGetCoreID(),
                       uxTaskGetStackHighWaterMark(NULL));

      lastPrinted = millis();
    }
    // <<<<<< TODO REMOVE ME

    if (mqttDisconnectedSince != 0 && mqttClient.connected()) {
      mqttLoggy.printf("mqtt was disconnected for %d millis..\n",
                       millis() - mqttDisconnectedSince);
      mqttDisconnectedSince = 0;
    }

    if (!mqttClient.connected()) {
      vTaskDelay(4000 / portTICK_PERIOD_MS);

      connectToMqtt();

      continue;
    }

    // publish a message roughly every 5 seconds.
    if (ioChanges || send_state_timer.isExpired(millis())) {
      send_state_timer.reset(millis());

      publishState();

      ++loopCounter;
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  // should not happen.. ?!
  vTaskDelete(NULL);
}

static void mqtt_createTask() {
  xTaskCreate(mqtt_task, "mqtt", 1024 * 3, NULL, 3, &mqttTaskHandle);
}

void mqtt_setup() {
  mqttLoggy.addLogger(&Serial);

  String mac = WiFi.macAddress();
  mac.replace(":", "");
  mac.toLowerCase();
  baseTopic = String("slaesh/smahoo/") + mac;

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);

  mqtt_createTask();
}

void mqtt_disconnect() { mqttClient.disconnect(true); }
