#ifndef MQTT_DRIVER_H
#define MQTT_DRIVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

/**
 * @class MqttDriver
 * @brief Low-level MQTT driver wrapping PubSubClient
 */
class MqttDriver {
public:
    MqttDriver();
    
    void init(const char* server, uint16_t port);
    
    /**
     * @brief Connect to broker
     * @param clientId Client identifier
     * @param username Username (for ThingsBoard, this is the token)
     * @param password Password (NULL for ThingsBoard)
     */
    bool connect(const char* clientId, const char* username = NULL, const char* password = NULL);
    
    void disconnect();
    bool isConnected();
    
    /**
     * @brief Publish message to topic
     */
    bool publish(const char* topic, const char* payload);
    
    /**
     * @brief Subscribe to topic
     */
    bool subscribe(const char* topic);
    
    /**
     * @brief Set callback for incoming messages
     */
    void setCallback(MQTT_CALLBACK_SIGNATURE);
    
    /**
     * @brief Must be called in loop to process MQTT
     */
    void loop();
    
    /**
     * @brief Get client state for debugging
     */
    int getState();

private:
    WiFiClient _wifiClient;
    PubSubClient _mqtt;
    const char* _server;
    uint16_t _port;
};

#endif
