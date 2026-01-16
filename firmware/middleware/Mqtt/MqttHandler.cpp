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

void MqttHandler::update(const char* deviceName) {
    if (!_driver.isConnected()) {
        if (millis() - _lastReconnectAttempt > MQTT_RECONNECT_INTERVAL_MS) {
            _lastReconnectAttempt = millis();
            
            if (_token.length() > 0) {
                _driver.connect(deviceName, _token.c_str(), NULL);
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
