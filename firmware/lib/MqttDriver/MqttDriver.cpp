#include "MqttDriver.h"

MqttDriver::MqttDriver() : _mqtt(_wifiClient) {
    _server = NULL;
    _port = 1883;
}

void MqttDriver::init(const char* server, uint16_t port) {
    _server = server;
    _port = port;
    _mqtt.setServer(_server, _port);
    Serial.printf("[MQTT] Driver initialized: %s:%d\n", _server, _port);
}

bool MqttDriver::connect(const char* clientId, const char* username, const char* password) {
    if (_server == NULL) {
        Serial.println("[MQTT] Error: Server not set");
        return false;
    }
    
    Serial.printf("[MQTT] Connecting as '%s'...\n", clientId);
    bool result = _mqtt.connect(clientId, username, password);
    
    if (result) {
        Serial.println("[MQTT] Connected!");
    } else {
        Serial.printf("[MQTT] Connection failed, rc=%d\n", _mqtt.state());
    }
    return result;
}

void MqttDriver::disconnect() {
    _mqtt.disconnect();
}

bool MqttDriver::isConnected() {
    return _mqtt.connected();
}

bool MqttDriver::publish(const char* topic, const char* payload) {
    return _mqtt.publish(topic, payload);
}

bool MqttDriver::subscribe(const char* topic) {
    return _mqtt.subscribe(topic);
}

void MqttDriver::setCallback(MQTT_CALLBACK_SIGNATURE) {
    _mqtt.setCallback(callback);
}

void MqttDriver::loop() {
    _mqtt.loop();
}

int MqttDriver::getState() {
    return _mqtt.state();
}
