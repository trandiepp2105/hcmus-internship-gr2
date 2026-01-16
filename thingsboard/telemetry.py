"""
Telemetry Module - Handles sending time-series data to ThingsBoard
"""

import json
import time
from typing import Dict, Any, Optional


# ==========================================
# MQTT ENDPOINT CONFIGURATION
# ==========================================
TELEMETRY_TOPIC = "v1/devices/me/telemetry"


# ==========================================
# TELEMETRY PACKET DEFINITIONS
# ==========================================
class TelemetryPacket:
    """Base class for telemetry packets"""
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert packet to dictionary"""
        raise NotImplementedError


class PHSensorTelemetry(TelemetryPacket):
    """pH Sensor telemetry data packet"""
    
    def __init__(
        self,
        ph_value: float,
        temperature: float,
        out_1_status: bool = False,
        out_2_status: bool = False,
        out_3_status: bool = False,
        out_4_status: bool = False,
        error_code: int = 0,
        include_timestamp: bool = True
    ):
        """
        Initialize pH sensor telemetry packet
        
        Args:
            ph_value: pH value (0-14)
            temperature: Temperature in Celsius
            out_1_status: Output 1 status (pump/relay)
            out_2_status: Output 2 status
            out_3_status: Output 3 status
            out_4_status: Output 4 status
            error_code: Error code (0 = no error)
            include_timestamp: Include timestamp in milliseconds
        """
        self.ph_value = ph_value
        self.temperature = temperature
        self.out_1_status = out_1_status
        self.out_2_status = out_2_status
        self.out_3_status = out_3_status
        self.out_4_status = out_4_status
        self.error_code = error_code
        self.include_timestamp = include_timestamp
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary for JSON serialization"""
        data = {
            "ph_value": self.ph_value,
            "temperature": self.temperature,
            "out_1_status": self.out_1_status,
            "out_2_status": self.out_2_status,
            "out_3_status": self.out_3_status,
            "out_4_status": self.out_4_status,
            "error_code": self.error_code
        }
        
        if self.include_timestamp:
            data["ts"] = int(time.time() * 1000)  # Milliseconds
        
        return data


# ==========================================
# TELEMETRY SENDER
# ==========================================
class TelemetrySender:
    """Handles sending telemetry data via MQTT"""
    
    def __init__(self, mqtt_client):
        """
        Initialize telemetry sender
        
        Args:
            mqtt_client: paho.mqtt.client.Client instance
        """
        self.client = mqtt_client
        self.topic = TELEMETRY_TOPIC
    
    def send(self, packet: TelemetryPacket, qos: int = 1) -> bool:
        """
        Send telemetry packet to ThingsBoard
        
        Args:
            packet: TelemetryPacket instance
            qos: MQTT Quality of Service (0, 1, or 2)
        
        Returns:
            bool: True if publish successful, False otherwise
        """
        try:
            data = packet.to_dict()
            payload = json.dumps(data)
            result = self.client.publish(self.topic, payload, qos=qos)
            
            return result.rc == 0  # MQTT_ERR_SUCCESS
        except Exception as e:
            print(f"[TELEMETRY] Error sending: {e}")
            return False
    
    def send_raw(self, data: Dict[str, Any], qos: int = 1) -> bool:
        """
        Send raw dictionary as telemetry
        
        Args:
            data: Dictionary with telemetry data
            qos: MQTT Quality of Service
        
        Returns:
            bool: True if successful
        """
        try:
            payload = json.dumps(data)
            result = self.client.publish(self.topic, payload, qos=qos)
            return result.rc == 0
        except Exception as e:
            print(f"[TELEMETRY] Error sending raw: {e}")
            return False


# ==========================================
# HELPER FUNCTIONS
# ==========================================
def create_ph_telemetry(
    ph_value: float,
    temperature: float,
    outputs: Optional[Dict[int, bool]] = None,
    error_code: int = 0
) -> PHSensorTelemetry:
    """
    Helper function to create pH telemetry packet
    
    Args:
        ph_value: pH value
        temperature: Temperature
        outputs: Dictionary mapping output number to status {1: True, 2: False, ...}
        error_code: Error code
    
    Returns:
        PHSensorTelemetry instance
    """
    outputs = outputs or {}
    
    return PHSensorTelemetry(
        ph_value=ph_value,
        temperature=temperature,
        out_1_status=outputs.get(1, False),
        out_2_status=outputs.get(2, False),
        out_3_status=outputs.get(3, False),
        out_4_status=outputs.get(4, False),
        error_code=error_code
    )
