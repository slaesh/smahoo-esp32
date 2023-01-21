
#include <WiFiUdp.h>

#include "../app_params/app_params.h"

// This device info
#define APP_NAME "SMAHOO"

bool sendSyslog(WiFiUDP &client, uint16_t pri, const char *procId,
                const char *message) {
  String syslogServer = iizi_get_parameter_value(IIZI_PARAM_SYSLOG_HOST_KEY);
  syslogServer.trim();

  String syslogPort_s = iizi_get_parameter_value(IIZI_PARAM_MQTT_PORT_KEY);
  syslogPort_s.trim();
  const uint16_t syslogPort = syslogPort_s.toInt();

  if (syslogServer.isEmpty() || !syslogPort) return false;

  const auto result = client.beginPacket(syslogServer.c_str(), syslogPort);
  if (result != 1) return false;

  // IETF Doc: https://tools.ietf.org/html/rfc5424
  // BSD Doc: https://tools.ietf.org/html/rfc3164
  client.print('<');
  client.print(pri);

  client.print(F(">1 - "));

  extern const char *getOurHostname();
  client.print(getOurHostname());
  client.print(' ');
  client.print(APP_NAME);

  client.print(' ');
  client.print(procId);
  client.print(F(" - - \xEF\xBB\xBF"));

  client.print(message);
  client.endPacket();

  return true;
}
