#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

#include "iizi_app_parameters.h"

/*
xxx_KEY's need to be unique !
xxx_LEN is WITHOUT the null terminition
*/

#define IIZI_PARAM_FRIENDLY_NAME_KEY        ("frnd_n")
#define IIZI_PARAM_FRIENDLY_NAME_LEN        (40)
#define IIZI_PARAM_USE_LED_KEY              ("dev_led")
#define IIZI_PARAM_USE_LED_LEN              (3)  // yes / no
#define IIZI_PARAM_INPUT_TOGGLES_OUTPUT_KEY ("dev_ito")
#define IIZI_PARAM_INPUT_TOGGLES_OUTPUT_LEN (3)  // yes / no

#define IIZI_PARAM_MQTT_HOST_KEY            ("mqtt_s")
#define IIZI_PARAM_MQTT_HOST_LEN            (40)
#define IIZI_PARAM_MQTT_PORT_KEY            ("mqtt_po")
#define IIZI_PARAM_MQTT_PORT_LEN            (5)
#define IIZI_PARAM_MQTT_USER_KEY            ("mqtt_u")
#define IIZI_PARAM_MQTT_USER_LEN            (20)
#define IIZI_PARAM_MQTT_PASS_KEY            ("mqtt_pa")
#define IIZI_PARAM_MQTT_PASS_LEN            (20)

#define IIZI_PARAM_OTA_UPDATE_HOST_KEY      ("ota_s")
#define IIZI_PARAM_OTA_UPDATE_HOST_LEN      (40)
#define IIZI_PARAM_OTA_UPDATE_PORT_KEY      ("ota_p")
#define IIZI_PARAM_OTA_UPDATE_PORT_LEN      (5)
#define IIZI_PARAM_OTA_UPDATE_PATH_KEY      ("ota_u")
#define IIZI_PARAM_OTA_UPDATE_PATH_LEN      (40)
#define IIZI_PARAM_OTA_UPDATE_CHANNEL_KEY   ("ota_c")
#define IIZI_PARAM_OTA_UPDATE_CHANNEL_LEN   (6)

#define IIZI_PARAM_SYSLOG_HOST_KEY            ("syslog_s")
#define IIZI_PARAM_SYSLOG_HOST_LEN            (40)
#define IIZI_PARAM_SYSLOG_PORT_KEY            ("syslog_po")
#define IIZI_PARAM_SYSLOG_PORT_LEN            (5)

extern t_iizi_app_parameter iizi_params[];
extern const uint16_t n_iizi_params;

#endif  // __PARAMETERS_H__
