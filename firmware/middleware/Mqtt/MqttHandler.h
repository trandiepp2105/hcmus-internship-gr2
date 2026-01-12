#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include "MqttDriver.h"
#include "../../lib/Storage/Storage.h"
#include "../../lib/TelemetryData/TelemetryRecord.h"
#include <ArduinoJson.h>

// ======== MQTT Configuration ========
#define MQTT_RECONNECT_INTERVAL_MS 5000

// ======== ThingsBoard MQTT Topics ========
// Provisioning
#define TB_TOPIC_PROVISION_REQUEST   "/provision/request"
#define TB_TOPIC_PROVISION_RESPONSE  "/provision/response"

// Telemetry (Device -> Server)
#define TB_TOPIC_TELEMETRY           "v1/devices/me/telemetry"

// Attributes (Shared/Client)
#define TB_TOPIC_ATTRIBUTES          "v1/devices/me/attributes"
#define TB_TOPIC_ATTRIBUTES_REQUEST  "v1/devices/me/attributes/request/"
#define TB_TOPIC_ATTRIBUTES_RESPONSE "v1/devices/me/attributes/response/"

// RPC (Remote Procedure Calls)
#define TB_TOPIC_RPC_REQUEST         "v1/devices/me/rpc/request/+"
#define TB_TOPIC_RPC_RESPONSE        "v1/devices/me/rpc/response/"

/**
 * @class MqttHandler
 * @brief BSP/Middleware for MQTT and ThingsBoard communication
 */
class MqttHandler {
public:
    MqttHandler(Storage* storage);
    
    /**
     * @brief Initialize MQTT
     * @param server MQTT broker address
     * @param port MQTT port
     */
    void begin(const char* server, uint16_t port);
    
    /**
     * @brief Check if device needs provisioning, run if needed
     * @param deviceName Device name for registration
     * @param provisionKey ThingsBoard provision key
     * @param provisionSecret ThingsBoard provision secret
     */
    void checkAndProvision(const char* deviceName, const char* provisionKey, const char* provisionSecret);
    
    /**
     * @brief Keep MQTT alive, auto-reconnect
     * @param deviceName Client ID for connection
     */
    void update(const char* deviceName);
    
    /**
     * @brief Send telemetry data to ThingsBoard
     */
    bool pushTelemetry(const TelemetryRecord& record);
    
    /**
     * @brief Quick telemetry push with individual params
     */
    bool pushTelemetry(float ph, float temp, uint8_t outputs, uint8_t mode, uint8_t errorCode);
    
    bool isConnected();

private:
    MqttDriver _driver;
    Storage* _storage;
    
    String _token;
    const char* _server;
    uint16_t _port;
    unsigned long _lastReconnectAttempt = 0;
    
    bool runProvisioning(const char* deviceName, const char* key, const char* secret);
    void loadToken();
    void saveToken(const String& token);
};

#endif
