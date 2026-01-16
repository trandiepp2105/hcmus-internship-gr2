"""
Test script using modularized ThingsBoard communication
Demonstrates sending attributes in rotation
"""

import json
import time
import sys
import paho.mqtt.client as mqtt
from paho.mqtt.enums import CallbackAPIVersion

# Import custom modules
from attributes import (
    AttributeSender,
    SetupModeCommand,
    ChangeOutputCommand,
    ChangeControlModeCommand
)
from telemetry import TelemetrySender, PHSensorTelemetry
from response_handler import ResponseHandler, ProvisionResponse

# ==========================================
# CONFIGURATION
# ==========================================
THINGSBOARD_HOST = "127.0.0.1"
PROVISION_KEY = "7b489653-2d36-45bd-9476-c7aa5b9ae5fc"
PROVISION_SECRET = "5BnZwv6WnuNbs6E45yNq"
DEVICE_NAME = "PH_CONTROLLER_001"
TOKEN_FILE = "token.txt"

# ==========================================
# PROVISIONER CLASS
# ==========================================
class Provisioner:
    """Handles device provisioning"""
    
    def __init__(self):
        self.client = mqtt.Client(CallbackAPIVersion.VERSION2)
        self.client.username_pw_set("provision")
        self.response_handler = ResponseHandler(self.client)
        self.is_success = False
        self.token = None
        
        # Register callback for provision response
        self.response_handler.on_provision_response(self._on_provision)
    
    def _on_provision(self, response: ProvisionResponse):
        """Callback when provision response received"""
        print(f"[PROVISION] Response: {response}")
        
        if response.is_success():
            self.token = response.get_token()
            print(f"[PROVISION] SUCCESS! Token: {self.token}")
            
            # Save token to file
            with open(TOKEN_FILE, "w") as f:
                f.write(self.token)
            
            self.is_success = True
        else:
            print(f"[PROVISION] FAILED: {response.error_msg}")
        
        self.client.disconnect()
    
    def run(self) -> bool:
        """Run provisioning process"""
        print("--- STARTING PROVISIONING ---")
        
        try:
            self.client.connect(THINGSBOARD_HOST, 1883, 60)
            
            # Subscribe to response
            self.response_handler.subscribe_provision_response()
            
            # Send provision request
            payload = {
                "deviceName": DEVICE_NAME,
                "provisionDeviceKey": PROVISION_KEY,
                "provisionDeviceSecret": PROVISION_SECRET
            }
            self.client.publish("/provision/request", json.dumps(payload))
            print(f"[PROVISION] Request sent for device: {DEVICE_NAME}")
            
            self.client.loop_forever()
            
        except Exception as e:
            print(f"[PROVISION] Connection error: {e}")
            return False
        
        return self.is_success


# ==========================================
# DEVICE CLASS
# ==========================================
class Device:
    """Represents the operational device"""
    
    def __init__(self, token: str):
        self.token = token
        self.client = mqtt.Client(CallbackAPIVersion.VERSION2)
        self.client.username_pw_set(token)
        
        # Initialize senders
        self.attribute_sender = AttributeSender(self.client)
        self.telemetry_sender = TelemetrySender(self.client)
        
        # Initialize response handler
        self.response_handler = ResponseHandler(self.client)
        self.response_handler.on_rpc_request(self._on_rpc_request)
        
        self.client.on_connect = self._on_connect
    
    def _on_connect(self, client, userdata, flags, rc, properties=None):
        """Callback when connected"""
        if rc == 0:
            print("[DEVICE] Connected successfully to ThingsBoard!")
            # Subscribe to RPC requests from server
            self.response_handler.subscribe_rpc_requests()
        else:
            print(f"[DEVICE] Connection failed: {rc}")
    
    def _on_rpc_request(self, rpc_request):
        """Handle RPC requests from server"""
        print(f"[DEVICE] RPC Request: {rpc_request.method}")
        print(f"[DEVICE] RPC Params: {rpc_request.params}")
        
        # Return response
        return {"status": "ok", "result": "Command received"}
    
    def run_attribute_rotation(self):
        """Send attributes in rotation every 5 seconds"""
        print("\n--- STARTING ATTRIBUTE ROTATION ---")
        print("[DEVICE] Sending attributes every 5 seconds. Press Ctrl+C to stop.\n")
        
        try:
            self.client.connect(THINGSBOARD_HOST, 1883, 60)
            self.client.loop_start()
            
            # Define 3 command packets
            commands = [
                SetupModeCommand(min_threshold=6.0, max_threshold=9.0),
                ChangeOutputCommand(
                    out_1_status=True,
                    out_2_status=False,
                    out_3_status=False,
                    out_4_status=False
                ),
                ChangeControlModeCommand(control_mode=True)
            ]
            
            command_descriptions = {
                1: "Setup Mode (min/max threshold)",
                2: "Change Output Status",
                3: "Change Control Mode"
            }
            
            command_index = 0
            
            while True:
                # Get current command
                packet = commands[command_index]
                
                # Send attribute
                success = self.attribute_sender.send(packet)
                
                if success:
                    data = packet.to_dict()
                    cmd_type = data['command']
                    print(f"[{time.strftime('%H:%M:%S')}] ✓ Sent Command {cmd_type}: {command_descriptions[cmd_type]}")
                    print(f"  Data: {json.dumps(data, indent=2)}\n")
                else:
                    print(f"[{time.strftime('%H:%M:%S')}] ✗ Failed to send attribute\n")
                
                # Rotate to next command
                command_index = (command_index + 1) % 3
                
                time.sleep(5)
                
        except KeyboardInterrupt:
            print("\n[DEVICE] Stopping...")
            self.client.loop_stop()
            self.client.disconnect()
    
    def run_telemetry_test(self):
        """Send telemetry data every 5 seconds"""
        print("\n--- STARTING TELEMETRY TEST ---")
        print("[DEVICE] Sending telemetry every 5 seconds. Press Ctrl+C to stop.\n")
        
        try:
            self.client.connect(THINGSBOARD_HOST, 1883, 60)
            self.client.loop_start()
            
            while True:
                # Create telemetry packet
                packet = PHSensorTelemetry(
                    ph_value=7.2,
                    temperature=28.5,
                    out_1_status=True,
                    out_2_status=False,
                    out_3_status=False,
                    out_4_status=False,
                    error_code=0
                )
                
                # Send telemetry
                success = self.telemetry_sender.send(packet)
                
                if success:
                    print(f"[{time.strftime('%H:%M:%S')}] ✓ Sent telemetry")
                    print(f"  Data: {json.dumps(packet.to_dict(), indent=2)}\n")
                else:
                    print(f"[{time.strftime('%H:%M:%S')}] ✗ Failed to send telemetry\n")
                
                time.sleep(5)
                
        except KeyboardInterrupt:
            print("\n[DEVICE] Stopping...")
            self.client.loop_stop()
            self.client.disconnect()


# ==========================================
# MAIN EXECUTION
# ==========================================
if __name__ == "__main__":
    # Step 1: Check if token exists
    has_token = False
    token = None
    
    try:
        with open(TOKEN_FILE, "r") as f:
            token = f.read().strip()
            if len(token) > 0:
                has_token = True
                print(f"[INFO] Token found: {token}\n")
    except FileNotFoundError:
        pass
    
    # Step 2: Provision if no token
    if not has_token:
        print("[INFO] No token found. Attempting provisioning...\n")
        prov = Provisioner()
        success = prov.run()
        
        if not success:
            print("\n[WARNING] Provisioning failed!")
            print("[INFO] If device already exists, get token from ThingsBoard UI:")
            print("  1. Go to Devices")
            print("  2. Click on 'PH_CONTROLLER_001'")
            print("  3. Click 'Copy access token' button\n")
            
            token = input("Enter Access Token (or press Enter to exit): ").strip()
            
            if token:
                print(f"[INFO] Saving token to {TOKEN_FILE}")
                with open(TOKEN_FILE, "w") as f:
                    f.write(token)
            else:
                print("Program exited.")
                sys.exit(1)
        else:
            token = prov.token
    
    # Step 3: Run device
    device = Device(token)
    
    # Choose mode
    print("\n--- SELECT MODE ---")
    print("1. Send Attributes (rotation)")
    print("2. Send Telemetry")
    choice = input("Enter choice (1 or 2): ").strip()
    
    if choice == "1":
        device.run_attribute_rotation()
    elif choice == "2":
        device.run_telemetry_test()
    else:
        print("Invalid choice. Defaulting to attribute rotation.")
        device.run_attribute_rotation()
