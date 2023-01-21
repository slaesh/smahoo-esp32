#ifndef __MQTT_CALLBACKS_H__
#define __MQTT_CALLBACKS_H__

#include <Arduino.h>
#include <stdlib.h>
#include <vector>

typedef void (*mqtt_subscription_callback)(String &, char *, int);

typedef struct
{
    const char *topic;
    uint8_t qos;
    mqtt_subscription_callback cb;
} t_s_mqtt_subscription;

extern std::vector<t_s_mqtt_subscription> mqttSubscriptions;

#endif // __MQTT_CALLBACKS_H__
