"""
Attributes Module - Handles sending device attributes/configuration to ThingsBoard
"""

import json
from typing import Dict, Any, Optional
from enum import IntEnum


# ==========================================
# MQTT ENDPOINT CONFIGURATION
# ==========================================
ATTRIBUTES_TOPIC = "v1/devices/me/attributes"


# ==========================================
# COMMAND TYPES
# ==========================================
class CommandType(IntEnum):
    """Command types for device control"""
    SETUP_MODE = 1          # Set min/max thresholds
    CHANGE_OUTPUT = 2       # Change output status
    CHANGE_CONTROL_MODE = 3 # Change automatic/manual mode


# ==========================================
# ATTRIBUTE PACKET DEFINITIONS
# ==========================================
class AttributePacket:
    """Base class for attribute packets"""
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert packet to dictionary"""
        raise NotImplementedError


class SetupModeCommand(AttributePacket):
    """Command to setup pH thresholds"""
    
    def __init__(self, min_threshold: float, max_threshold: float):
        """
        Initialize setup mode command
        
        Args:
            min_threshold: Minimum pH threshold (lower limit)
            max_threshold: Maximum pH threshold (upper limit)
        """
        self.command = CommandType.SETUP_MODE
        self.min_threshold = min_threshold
        self.max_threshold = max_threshold
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary"""
        return {
            "command": int(self.command),
            "min_threshold": self.min_threshold,
            "max_threshold": self.max_threshold
        }


class ChangeOutputCommand(AttributePacket):
    """Command to change output status"""
    
    def __init__(
        self,
        out_1_status: bool = False,
        out_2_status: bool = False,
        out_3_status: bool = False,
        out_4_status: bool = False
    ):
        """
        Initialize output change command
        
        Args:
            out_1_status: Output 1 status (True=ON, False=OFF)
            out_2_status: Output 2 status
            out_3_status: Output 3 status
            out_4_status: Output 4 status
        """
        self.command = CommandType.CHANGE_OUTPUT
        self.out_1_status = out_1_status
        self.out_2_status = out_2_status
        self.out_3_status = out_3_status
        self.out_4_status = out_4_status
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary"""
        return {
            "command": int(self.command),
            "out_1_status": self.out_1_status,
            "out_2_status": self.out_2_status,
            "out_3_status": self.out_3_status,
            "out_4_status": self.out_4_status
        }


class ChangeControlModeCommand(AttributePacket):
    """Command to change control mode"""
    
    def __init__(self, control_mode: bool):
        """
        Initialize control mode change command
        
        Args:
            control_mode: True for automatic, False for manual
        """
        self.command = CommandType.CHANGE_CONTROL_MODE
        self.control_mode = control_mode
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary"""
        return {
            "command": int(self.command),
            "control_mode": self.control_mode
        }


# ==========================================
# ATTRIBUTE SENDER
# ==========================================
class AttributeSender:
    """Handles sending attributes via MQTT"""
    
    def __init__(self, mqtt_client):
        """
        Initialize attribute sender
        
        Args:
            mqtt_client: paho.mqtt.client.Client instance
        """
        self.client = mqtt_client
        self.topic = ATTRIBUTES_TOPIC
    
    def send(self, packet: AttributePacket, qos: int = 1) -> bool:
        """
        Send attribute packet to ThingsBoard
        
        Args:
            packet: AttributePacket instance
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
            print(f"[ATTRIBUTES] Error sending: {e}")
            return False
    
    def send_raw(self, data: Dict[str, Any], qos: int = 1) -> bool:
        """
        Send raw dictionary as attributes
        
        Args:
            data: Dictionary with attribute data
            qos: MQTT Quality of Service
        
        Returns:
            bool: True if successful
        """
        try:
            payload = json.dumps(data)
            result = self.client.publish(self.topic, payload, qos=qos)
            return result.rc == 0
        except Exception as e:
            print(f"[ATTRIBUTES] Error sending raw: {e}")
            return False
    
    def send_setup_mode(self, min_threshold: float, max_threshold: float) -> bool:
        """
        Convenient method to send setup mode command
        
        Args:
            min_threshold: Minimum pH threshold
            max_threshold: Maximum pH threshold
        
        Returns:
            bool: True if successful
        """
        packet = SetupModeCommand(min_threshold, max_threshold)
        return self.send(packet)
    
    def send_output_control(self, outputs: Dict[int, bool]) -> bool:
        """
        Convenient method to send output control command
        
        Args:
            outputs: Dictionary mapping output number to status {1: True, 2: False, ...}
        
        Returns:
            bool: True if successful
        """
        packet = ChangeOutputCommand(
            out_1_status=outputs.get(1, False),
            out_2_status=outputs.get(2, False),
            out_3_status=outputs.get(3, False),
            out_4_status=outputs.get(4, False)
        )
        return self.send(packet)
    
    def send_control_mode(self, automatic: bool) -> bool:
        """
        Convenient method to send control mode command
        
        Args:
            automatic: True for automatic mode, False for manual
        
        Returns:
            bool: True if successful
        """
        packet = ChangeControlModeCommand(automatic)
        return self.send(packet)


# ==========================================
# HELPER FUNCTIONS
# ==========================================
def create_setup_command(min_ph: float, max_ph: float) -> SetupModeCommand:
    """Helper to create setup mode command"""
    return SetupModeCommand(min_ph, max_ph)


def create_output_command(outputs: Dict[int, bool]) -> ChangeOutputCommand:
    """Helper to create output control command"""
    return ChangeOutputCommand(
        out_1_status=outputs.get(1, False),
        out_2_status=outputs.get(2, False),
        out_3_status=outputs.get(3, False),
        out_4_status=outputs.get(4, False)
    )


def create_control_mode_command(automatic: bool) -> ChangeControlModeCommand:
    """Helper to create control mode command"""
    return ChangeControlModeCommand(automatic)
