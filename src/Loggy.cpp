#include "Loggy.h"

// check this:
// https://grafana.com/blog/2021/03/23/how-i-fell-in-love-with-logs-thanks-to-grafana-loki

bool Loggy::_connected = false;

static bool registerWifiEvents = true;

Loggy::Loggy(const char *ctx) {
  if (registerWifiEvents) {
    registerWifiEvents = false;

    WiFi.onEvent(Loggy::_wifiEvents);
  }

  this->context = ctx;
}

Loggy::~Loggy() { this->_loggers.clear(); }

void Loggy::_wifiEvents(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.println("loggy: WiFi connected");
      Loggy::_connected = true;
      break;

    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
      Serial.println("loggy: WiFi lost IP?");
      /* fall through */
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("loggy: WiFi lost connection");
      Loggy::_connected = false;
      break;

    default:
      Serial.printf("loggy: WiFi evt: %d\n", (int)event);
      break;
  }
}

void Loggy::addLogger(Print *logger, bool reliesOnWiFi) {
  // check if there is already a loggable using this logger!
  for (const auto ref_l : this->_loggers) {
    const auto loggable = ref_l;
    if ((loggable.logger) == logger) return;
  }

  Loggable loggable(logger, reliesOnWiFi);

  this->_loggers.push_back(loggable);
}

#define LOG_USER (1 << 3) /* random user-level messages */

size_t Loggy::write(uint8_t v) {
  for (const auto ref_l : this->_loggers) {
    const auto loggable = ref_l;
    if (loggable.reliesOnWiFi && !this->_connected) continue;

    loggable.logger->write(v);
  }

  if (!this->_connected) {
    this->_bIdx = 0;
    return 0;
  }

  // just an empty line '\n'? skip it..!
  if (this->_bIdx == 0 && v == '\n') {
    return 1;
  }

  // just an empty line '\r'? skip it..!
  if (this->_bIdx == 0 && v == '\r') {
    return 1;
  }

  // just an empty line '\r\n'? skip it..!
  if (this->_bIdx == 1 && v == '\n' && this->_buffer[0] == '\r') {
    return 1;
  }

  this->_buffer[this->_bIdx++] = v;

  // do we need to "flush"?
  bool isNewLine = v == '\r' || v == '\n';
  if (isNewLine || this->_bIdx >= LOGGY_BUFFER_SIZE - 1) {
    uint16_t terminationIdxOffset = 0;
    if (isNewLine) {
      terminationIdxOffset = 1;
      if (this->_buffer[this->_bIdx - 2] == '\r') {
        terminationIdxOffset = 2;
      }
    }

    // terminate buffer
    this->_buffer[this->_bIdx - terminationIdxOffset] = 0;

    // send out
    // 3 => seems to be "err" ?
    // 4 => warning
    // 5 => notice
    // 6 => info
    // this->_syslog->log(6, this->_buffer);

    extern bool sendSyslog(WiFiUDP & client, uint16_t pri, const char *procId,
                           const char *message);
    sendSyslog(this->_udpClient, LOG_USER + 5, this->context, this->_buffer);

    // reset buffer!
    this->_bIdx = 0;
  }

  return 1;
}
