#ifndef __APP_PARAMS_DEFAULTS_H__
#define __APP_PARAMS_DEFAULTS_H__

// default values can be overwritten via an '.env' file.
// see patch_env_file_definitions.py

// mqtt

#ifndef MQTT_DEFAULT_HOST
#define MQTT_DEFAULT_HOST ""
#endif

#ifndef MQTT_DEFAULT_PORT
#define MQTT_DEFAULT_PORT ""
#endif

#ifndef MQTT_DEFAULT_USER
#define MQTT_DEFAULT_USER ""
#endif

#ifndef MQTT_DEFAULT_PASS
#define MQTT_DEFAULT_PASS ""
#endif

// ota

// ota host url without protocol, HTTP is used!
#ifndef OTA_DEFAULT_HOST
#define OTA_DEFAULT_HOST ""
#endif

#ifndef OTA_DEFAULT_PORT
#define OTA_DEFAULT_PORT ""
#endif

#ifndef OTA_DEFAULT_URL_PATH
#define OTA_DEFAULT_URL_PATH "/fw/update"
#endif

#ifndef OTA_DEFAULT_CHANNEL
#define OTA_DEFAULT_CHANNEL "stable"
#endif

// syslog

#ifndef SYSLOG_DEFAULT_HOST
#define SYSLOG_DEFAULT_HOST ""
#endif

#ifndef SYSLOG_DEFAULT_PORT
#define SYSLOG_DEFAULT_PORT ""
#endif

#endif
