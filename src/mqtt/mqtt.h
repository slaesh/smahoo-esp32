#ifndef __MQTT_H__
#define __MQTT_H__

#include "freertos/task.h"

#ifdef __MQTT_C__
TaskHandle_t mqttTaskHandle;
#else
extern TaskHandle_t mqttTaskHandle;
#endif

#define MQTT_NOTIFY_IO_CHANGES() xTaskNotifyGiveIndexed(mqttTaskHandle, 0)

void mqtt_setup();
void mqtt_disconnect();

#endif  // __MQTT_H__
