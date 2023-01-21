
#include <Arduino.h>
#include <WiFi.h>

const char *getOurHostname() {
  static char hostname[253];
  static bool modifyHostnameOnlyOnce = true;
  static String _hostname = "slaesh-smahoo-" + WiFi.macAddress();

  if (modifyHostnameOnlyOnce) {
    _hostname.replace(":", "");
    _hostname.toLowerCase();
    modifyHostnameOnlyOnce = false;
    snprintf(hostname, sizeof(hostname), "%s", _hostname.c_str());
  }

  return hostname;
}
