#define __PARAMETERS_C__

#include "app_params.h"

#include "app_params_defaults.h"

static char cps_friendly_name[IIZI_PARAM_FRIENDLY_NAME_LEN + 1] = "";
static char cps_use_led[IIZI_PARAM_USE_LED_LEN + 1]             = "yes";
static char cps_input_toggles_output[IIZI_PARAM_INPUT_TOGGLES_OUTPUT_LEN + 1] =
    "yes";

static char cps_mqtt_host[IIZI_PARAM_MQTT_HOST_LEN + 1] = MQTT_DEFAULT_HOST;
static char cps_mqtt_port[IIZI_PARAM_MQTT_PORT_LEN + 1] = MQTT_DEFAULT_PORT;
static char cps_mqtt_user[IIZI_PARAM_MQTT_USER_LEN + 1] = MQTT_DEFAULT_USER;
static char cps_mqtt_pass[IIZI_PARAM_MQTT_PASS_LEN + 1] = MQTT_DEFAULT_PASS;

static char cps_ota_host[IIZI_PARAM_OTA_UPDATE_HOST_LEN + 1] = OTA_DEFAULT_HOST;
static char cps_ota_port[IIZI_PARAM_OTA_UPDATE_PORT_LEN + 1] = OTA_DEFAULT_PORT;
static char cps_ota_path[IIZI_PARAM_OTA_UPDATE_PATH_LEN + 1] =
    OTA_DEFAULT_URL_PATH;
static char cps_ota_chan[IIZI_PARAM_OTA_UPDATE_CHANNEL_LEN + 1] =
    OTA_DEFAULT_CHANNEL;

static char cps_syslog_host[IIZI_PARAM_SYSLOG_HOST_LEN + 1] =
    SYSLOG_DEFAULT_HOST;
static char cps_syslog_port[IIZI_PARAM_SYSLOG_PORT_LEN + 1] =
    SYSLOG_DEFAULT_PORT;

t_iizi_app_parameter iizi_params[] = {

#if true  // DEVICE

    {
        .key        = IIZI_PARAM_FRIENDLY_NAME_KEY,
        .label      = "friendly name",
        .value      = cps_friendly_name,
        .type       = "str",
        .max_length = IIZI_PARAM_FRIENDLY_NAME_LEN,
    },

    {
        .key        = IIZI_PARAM_USE_LED_KEY,
        .label      = "blinky led",
        .value      = cps_use_led,
        .type       = "enum|yes|no",
        .max_length = IIZI_PARAM_USE_LED_LEN,
    },

    {
        .key        = IIZI_PARAM_INPUT_TOGGLES_OUTPUT_KEY,
        .label      = "input toggles output",
        .value      = cps_input_toggles_output,
        .type       = "enum|yes|no",
        .max_length = IIZI_PARAM_INPUT_TOGGLES_OUTPUT_LEN,
    },

#endif

#if true  // MQTT

    {
        .key        = IIZI_PARAM_MQTT_HOST_KEY,
        .label      = "mqtt server",
        .value      = cps_mqtt_host,
        .type       = "str",
        .max_length = IIZI_PARAM_MQTT_HOST_LEN,
    },

    {
        .key        = IIZI_PARAM_MQTT_PORT_KEY,
        .label      = "mqtt port",
        .value      = cps_mqtt_port,  // TODO: change default back to 1883
        .type       = "int",
        .max_length = IIZI_PARAM_MQTT_PORT_LEN,
    },

    {
        .key        = IIZI_PARAM_MQTT_USER_KEY,
        .label      = "mqtt user",
        .value      = cps_mqtt_user,
        .type       = "str",
        .max_length = IIZI_PARAM_MQTT_USER_LEN,
    },

    {
        .key        = IIZI_PARAM_MQTT_PASS_KEY,
        .label      = "mqtt pass",
        .value      = cps_mqtt_pass,
        .type       = "str",
        .max_length = IIZI_PARAM_MQTT_PASS_LEN,
    },

#endif

#if true  // OTA UPDATES

    {
        .key        = IIZI_PARAM_OTA_UPDATE_HOST_KEY,
        .label      = "ota update server",
        .value      = cps_ota_host,
        .type       = "str",
        .max_length = IIZI_PARAM_OTA_UPDATE_HOST_LEN,
    },

    {
        .key        = IIZI_PARAM_OTA_UPDATE_PORT_KEY,
        .label      = "ota update port",
        .value      = cps_ota_port,
        .type       = "int",
        .max_length = IIZI_PARAM_OTA_UPDATE_PORT_LEN,
    },

    {
        .key        = IIZI_PARAM_OTA_UPDATE_PATH_KEY,
        .label      = "ota update url",
        .value      = cps_ota_path,
        .type       = "str",
        .max_length = IIZI_PARAM_OTA_UPDATE_PATH_LEN,
    },

    {
        .key        = IIZI_PARAM_OTA_UPDATE_CHANNEL_KEY,
        .label      = "ota update channel",
        .value      = cps_ota_chan,
        .type       = "enum|dev|beta|stable",
        .max_length = IIZI_PARAM_OTA_UPDATE_CHANNEL_LEN,
    },

#endif

#if true  // SYSLOG

    {
        .key        = IIZI_PARAM_SYSLOG_HOST_KEY,
        .label      = "syslog server",
        .value      = cps_syslog_host,
        .type       = "str",
        .max_length = IIZI_PARAM_SYSLOG_HOST_LEN,
    },

    {
        .key        = IIZI_PARAM_SYSLOG_PORT_KEY,
        .label      = "syslog port",
        .value      = cps_syslog_port,
        .type       = "int",
        .max_length = IIZI_PARAM_SYSLOG_PORT_LEN,
    },

#endif

    // ..
};

const uint16_t n_iizi_params =
    sizeof(iizi_params) / sizeof(t_iizi_app_parameter);
