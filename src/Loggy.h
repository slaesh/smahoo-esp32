
#ifndef __LOGGY_H__
#define __LOGGY_H__

#include <WiFi.h>
#include <WiFiUdp.h>
#include <vector>

#define LOGGY_BUFFER_SIZE (512)

class Loggable
{
public:
    Print *logger;
    bool reliesOnWiFi;

    Loggable(Print *_logger, bool reliesOnWiFi) : logger(_logger)
    {
        this->reliesOnWiFi = reliesOnWiFi;
    }
};

class Loggy : public Print
{
private:
    WiFiUDP _udpClient;

    static void _wifiEvents(WiFiEvent_t event);
    static bool _connected;

    std::vector<Loggable> _loggers;

    const char *context;

    char _buffer[LOGGY_BUFFER_SIZE];
    uint16_t _bIdx = 0;

public:
    Loggy(const char *ctx);
    ~Loggy();

    size_t write(uint8_t v) override;

    void addLogger(Print *logger, bool reliesOnWiFi = false);
};

#endif // __LOGGY_H__
