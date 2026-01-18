#include "MqttHandler.h"

// Global for callback (PubSubClient limitation)
static String _receivedToken = "";
static bool _provisionDone = false;

void provisionCallback(char* topic, uint8_t* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.print("[MQTT] Provision response: ");
    Serial.println(message);
    
    JsonDocument doc;
    if (deserializeJson(doc, message) == DeserializationError::Ok) {
        if (doc["credentialsValue"].is<const char*>()) {
            _receivedToken = doc["credentialsValue"].as<String>();
            _provisionDone = true;
        }
    }
}

MqttHandler::MqttHandler(Storage* storage) : _storage(storage) {
    _token = "";
    _server = NULL;
    _port = 1883;
}

void MqttHandler::begin(const char* server, uint16_t port) {
    _server = server;
    _port = port;
    _driver.init(server, port);
    loadToken();
}

void MqttHandler::loadToken() {
    // Load token from persistent storage (getString returns String)
    String tokenStr = _storage->getString("mqtt_token", "");
    if (tokenStr.length() > 0) {
        _token = tokenStr;
        Serial.println("[MQTT] Token loaded from storage.");
    } else {
        _token = "";
        Serial.println("[MQTT] No token found in storage.");
    }
}

void MqttHandler::saveToken(const String& token) {
    _storage->putString("mqtt_token", token.c_str());
    Serial.println("[MQTT] Token saved to storage.");
}

bool MqttHandler::runProvisioning(const char* deviceName, const char* key, const char* secret) {
    Serial.println("[MQTT] Running device provisioning...");
    
    // Connect with provision username
    if (!_driver.connect(deviceName, "provision", NULL)) {
        Serial.println("[MQTT] Provision connect failed");
        return false;
    }
    
    _driver.setCallback(provisionCallback);
    _driver.subscribe(TB_TOPIC_PROVISION_RESPONSE);
    
    // Build provision request
    JsonDocument doc;
    doc["deviceName"] = deviceName;
    doc["provisionDeviceKey"] = key;
    doc["provisionDeviceSecret"] = secret;
    
    char buffer[256];
    serializeJson(doc, buffer);
    _driver.publish(TB_TOPIC_PROVISION_REQUEST, buffer);
    
    // Wait for response
    _provisionDone = false;
    _receivedToken = "";
    unsigned long start = millis();
    
    while (!_provisionDone && millis() - start < 10000) {
        _driver.loop();
        delay(10);
    }
    
    _driver.disconnect();
    
    if (_provisionDone && _receivedToken.length() > 0) {
        _token = _receivedToken;
        saveToken(_token);
        return true;
    }
    
    return false;
}

void MqttHandler::checkAndProvision(const char* deviceName, const char* provisionKey, const char* provisionSecret) {
    if (_token.length() < 5) {
        Serial.println("[MQTT] Token missing, starting provisioning...");
        if (runProvisioning(deviceName, provisionKey, provisionSecret)) {
            Serial.println("[MQTT] Provisioning successful!");
        } else {
            Serial.println("[MQTT] Provisioning failed!, rc= " + String(_driver.getState()));
            
        }
    } else {
        Serial.println("[MQTT] Token exists, skipping provisioning.");
    }
}

// Global callback pointers
ControlModeCallback g_controlModeCallback = nullptr;
RpcCallback g_rpcCallback = nullptr;
OnConnectCallback g_onConnectCallback = nullptr;
ThresholdCallback g_thresholdCallback = nullptr;
MqttHandler* g_mqttHandler = nullptr;

void MqttHandler::setControlModeCallback(ControlModeCallback callback) {
    g_controlModeCallback = callback;
}

void MqttHandler::setRpcCallback(RpcCallback callback) {
    g_rpcCallback = callback;
}

void MqttHandler::setOnConnectCallback(OnConnectCallback callback) {
    g_onConnectCallback = callback;
}

void MqttHandler::setThresholdCallback(ThresholdCallback callback) {
    g_thresholdCallback = callback;
}

// Global callback for attributes and RPC
void mqttMessageCallback(char* topic, uint8_t* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    Serial.printf("[MQTT] Topic: %s\n", topic);
    Serial.printf("[MQTT] Payload: %s\n", message.c_str());
    
    String topicStr = String(topic);
    
    // Check if this is an attribute update
    if (topicStr == "v1/devices/me/attributes") {
        Serial.println("[MQTT] === ATTRIBUTE UPDATE RECEIVED ===");
        
        JsonDocument doc;
        if (deserializeJson(doc, message) == DeserializationError::Ok) {
            // Log all attributes received
            for (JsonPair kv : doc.as<JsonObject>()) {
                Serial.printf("[MQTT] Attribute: %s = ", kv.key().c_str());
                if (kv.value().is<const char*>()) {
                    Serial.println(kv.value().as<const char*>());
                } else if (kv.value().is<bool>()) {
                    Serial.println(kv.value().as<bool>() ? "true" : "false");
                } else if (kv.value().is<int>()) {
                    Serial.println(kv.value().as<int>());
                } else if (kv.value().is<float>()) {
                    Serial.println(kv.value().as<float>());
                } else {
                    Serial.println("[complex type]");
                }
            }
            
            // Handle control_mode attribute
            if (doc["control_mode"].is<bool>()) {
                bool isAuto = doc["control_mode"].as<bool>();
                Serial.printf("[MQTT] Control mode changed to: %s\n", isAuto ? "AUTO" : "MANUAL");
                
                if (g_controlModeCallback != nullptr) {
                    g_controlModeCallback(isAuto);
                }
            }
            
            // Handle threshold attributes (min_threshold, max_threshold)
            bool hasMinThreshold = doc["min_threshold"].is<float>() || doc["min_threshold"].is<int>();
            bool hasMaxThreshold = doc["max_threshold"].is<float>() || doc["max_threshold"].is<int>();
            
            if ((hasMinThreshold || hasMaxThreshold) && g_thresholdCallback != nullptr) {
                // Get current values or defaults
                float minThreshold = hasMinThreshold ? doc["min_threshold"].as<float>() : -1.0f;
                float maxThreshold = hasMaxThreshold ? doc["max_threshold"].as<float>() : -1.0f;
                
                Serial.printf("[MQTT] Threshold update: min=%.1f, max=%.1f\n", minThreshold, maxThreshold);
                g_thresholdCallback(minThreshold, maxThreshold);
            }
        }
        Serial.println("[MQTT] ================================");
    }
    // Check if this is an RPC request
    else if (topicStr.startsWith("v1/devices/me/rpc/request/")) {
        // Extract request ID from topic
        int requestId = topicStr.substring(26).toInt();
        Serial.println("[MQTT] === RPC REQUEST ===");
        Serial.printf("[MQTT] Request ID: %d\n", requestId);
        Serial.printf("[MQTT] Raw Payload: %s\n", message.c_str());
        
        JsonDocument doc;
        if (deserializeJson(doc, message) == DeserializationError::Ok) {
            // Print all JSON keys for debugging
            Serial.println("[MQTT] Parsed JSON:");
            for (JsonPair kv : doc.as<JsonObject>()) {
                Serial.printf("[MQTT]   Key: %s\n", kv.key().c_str());
            }
            
            const char* method = doc["method"].as<const char*>();
            Serial.printf("[MQTT] Method: %s\n", method);
            
            bool success = false;
            String responseMsg = "Unknown method";
            
            // Handle setRelay method
            if (strcmp(method, "setRelay") == 0) {
                int relay = doc["params"]["relay"].as<int>();
                bool state = doc["params"]["state"].as<bool>();
                Serial.printf("[MQTT] setRelay: relay=%d, state=%s\n", relay, state ? "ON" : "OFF");
                
                if (g_rpcCallback != nullptr) {
                    success = g_rpcCallback(method, relay, state);
                    responseMsg = success ? "OK" : "REJECTED (AUTO mode)";
                } else {
                    responseMsg = "No handler registered";
                }
            }
            // Handle getStatus method
            else if (strcmp(method, "getStatus") == 0) {
                success = true;
                responseMsg = "Status OK";
            }
            
            // Send response
            if (g_mqttHandler != nullptr) {
                g_mqttHandler->sendRpcResponse(requestId, success, responseMsg.c_str());
            }
        }
        Serial.println("[MQTT] ================================");
    }
}

void MqttHandler::update(const char* deviceName) {
    if (!_driver.isConnected()) {
        if (millis() - _lastReconnectAttempt > MQTT_RECONNECT_INTERVAL_MS) {
            _lastReconnectAttempt = millis();
            
            if (_token.length() > 0) {
                if (_driver.connect(deviceName, _token.c_str(), NULL)) {
                    Serial.println("[MQTT] Connected! Subscribing to topics...");
                    g_mqttHandler = this;
                    _driver.setCallback(mqttMessageCallback);
                    _driver.subscribe("v1/devices/me/attributes");
                    _driver.subscribe("v1/devices/me/rpc/request/+");
                    Serial.println("[MQTT] Subscribed to attributes and RPC");
                    
                    // Call onConnect callback
                    if (g_onConnectCallback != nullptr) {
                        g_onConnectCallback();
                    }
                }
            }
        }
    }
    _driver.loop();
}

bool MqttHandler::pushTelemetry(const TelemetryRecord& record) {
    return pushTelemetry(record.ph, record.temp, record.outputs, record.mode, record.errorCode);
}



bool MqttHandler::pushTelemetry(float ph, float temp, uint8_t outputs, uint8_t mode, uint8_t errorCode) {
    if (!_driver.isConnected()) return false;
    
    // Create JSON document matching the logic in original MqttModule.cpp
    JsonDocument doc;
    
    // Format pH to 2 decimal places as seen in original code
    doc["ph_value"] = (float)((int)(ph * 100 + 0.5)) / 100.0; 
    doc["temperature"] = temp;
    
    // Map relay status from mask
    doc["out_1_status"] = (outputs & 0x01) != 0;
    doc["out_2_status"] = (outputs & 0x02) != 0;
    doc["out_3_status"] = (outputs & 0x04) != 0;
    doc["out_4_status"] = (outputs & 0x08) != 0;
    
    // Additional logic from your new architecture
    doc["control_mode"] = (mode == 0) ? "AUTO" : "MANUAL";
    doc["error_code"] = errorCode;
    
    char buffer[256];
    serializeJson(doc, buffer);
    
    // Original MqttModule used "v1/devices/me/telemetry"
    // Ensure TB_TOPIC_TELEMETRY is defined as "v1/devices/me/telemetry"
    bool result = _driver.publish(TB_TOPIC_TELEMETRY, buffer);
    
    if (result) {
        Serial.println("[MQTT] Telemetry sent successfully.");
    }
    return result;
}

bool MqttHandler::isConnected() {
    return _driver.isConnected();
}

bool MqttHandler::sendRpcResponse(int requestId, bool success, const char* message) {
    if (!_driver.isConnected()) return false;
    
    JsonDocument doc;
    doc["success"] = success;
    doc["message"] = message;
    
    char buffer[128];
    serializeJson(doc, buffer);
    
    String responseTopic = String(TB_TOPIC_RPC_RESPONSE) + String(requestId);
    bool result = _driver.publish(responseTopic.c_str(), buffer);
    
    Serial.printf("[MQTT] RPC Response (ID: %d): %s\n", requestId, buffer);
    return result;
}

bool MqttHandler::pushAttribute(const char* key, bool value) {
    if (!_driver.isConnected()) return false;
    
    JsonDocument doc;
    doc[key] = value;
    
    char buffer[64];
    serializeJson(doc, buffer);
    
    bool result = _driver.publish(TB_TOPIC_ATTRIBUTES, buffer);
    
    if (result) {
        Serial.printf("[MQTT] Attribute pushed: %s = %s\n", key, value ? "true" : "false");
    }
    return result;
}

bool MqttHandler::pushAttribute(const char* key, float value) {
    if (!_driver.isConnected()) return false;
    
    JsonDocument doc;
    doc[key] = value;
    
    char buffer[64];
    serializeJson(doc, buffer);
    
    bool result = _driver.publish(TB_TOPIC_ATTRIBUTES, buffer);
    
    if (result) {
        Serial.printf("[MQTT] Attribute pushed: %s = %.2f\n", key, value);
    }
    return result;
}
