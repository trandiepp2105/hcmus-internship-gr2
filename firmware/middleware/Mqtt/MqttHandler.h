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

// Callback type for control mode changes
typedef void (*ControlModeCallback)(bool isAuto);

// Callback type for RPC commands
// Parameters: method, relay number, state, requestId for response
typedef bool (*RpcCallback)(const char* method, int relay, bool state);

// Callback type for MQTT connect event
typedef void (*OnConnectCallback)();

// Callback type for threshold changes (min_threshold, max_threshold)
typedef void (*ThresholdCallback)(float minThreshold, float maxThreshold);

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
    
    /**
     * @brief Set callback for control mode changes
     */
    void setControlModeCallback(ControlModeCallback callback);
    
    /**
     * @brief Set callback for RPC commands
     */
    void setRpcCallback(RpcCallback callback);
    
    /**
     * @brief Send RPC response back to ThingsBoard
     */
    bool sendRpcResponse(int requestId, bool success, const char* message);
    
    /**
     * @brief Push client attribute to ThingsBoard (bool)
     */
    bool pushAttribute(const char* key, bool value);
    
    /**
     * @brief Push client attribute to ThingsBoard (float)
     */
    bool pushAttribute(const char* key, float value);
    
    /**
     * @brief Set callback for threshold changes from ThingsBoard
     */
    void setThresholdCallback(ThresholdCallback callback);
    
    /**
     * @brief Set callback for when MQTT connects
     */
    void setOnConnectCallback(OnConnectCallback callback);

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

// Global callback pointers (for static callback function)
extern ControlModeCallback g_controlModeCallback;
extern RpcCallback g_rpcCallback;
extern OnConnectCallback g_onConnectCallback;
extern ThresholdCallback g_thresholdCallback;
extern MqttHandler* g_mqttHandler;

#endif
