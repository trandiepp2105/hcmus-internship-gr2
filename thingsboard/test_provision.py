import json
import time
import sys
import paho.mqtt.client as mqtt
from paho.mqtt.enums import CallbackAPIVersion

# ==========================================
# 1. CONFIGURATION (EDIT THIS SECTION)
# ==========================================
THINGSBOARD_HOST = "127.0.0.1"      # Docker host IP (or localhost)
PROVISION_KEY    = "7b489653-2d36-45bd-9476-c7aa5b9ae5fc"     # Key from Device Profile
PROVISION_SECRET = "5BnZwv6WnuNbs6E45yNq"  # Secret from Device Profile
DEVICE_NAME      = "PH_CONTROLLER_004" 
TOKEN_FILE       = "token.txt"      # File to save the received token

# ==========================================
# 2. CLASS: PROVISIONER (GETS THE TOKEN)
# ==========================================
class Provisioner:
    """Handles the initial registration to get the Access Token."""
    
    def __init__(self):
        self.client = mqtt.Client(CallbackAPIVersion.VERSION2)
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.is_success = False

    def on_connect(self, client, userdata, flags, rc, properties=None):
        if rc == 0:
            print("[PROVISION] Connected to Server. Sending request...")
            
            # 1. Subscribe to listen for the answer
            client.subscribe("/provision/response")
            
            # 2. Send the request (Payload)
            payload = {
                "deviceName": DEVICE_NAME,
                "provisionDeviceKey": PROVISION_KEY,
                "provisionDeviceSecret": PROVISION_SECRET
            }
            client.publish("/provision/request", json.dumps(payload))
        else:
            print(f"[PROVISION] Connection failed! Result code: {rc}")

    def on_message(self, client, userdata, msg):
        print(f"[PROVISION] Received response on topic: {msg.topic}")
        data = json.loads(msg.payload.decode())
        
        # Print full response for debugging
        print(f"[PROVISION] Full response: {json.dumps(data, indent=2)}")
        
        if data.get("status") == "SUCCESS":
            token = data.get("credentialsValue")
            print(f"[PROVISION] SUCCESS! Token received: {token}")
            
            # Save token to file
            with open(TOKEN_FILE, "w") as f:
                f.write(token)
            
            self.is_success = True
        else:
            error_msg = data.get('errorMsg', 'Unknown error')
            print(f"[PROVISION] FAILED. Server error: {error_msg}")
            print(f"[PROVISION] Status: {data.get('status')}")
            print(f"[PROVISION] Response data: {data}")
        
        # Disconnect immediately after finishing
        client.disconnect()

    def run(self):
        print("--- STARTING PROVISIONING ---")
        # IMPORTANT: Username must be "provision" for this step
        self.client.username_pw_set("provision")
        
        try:
            self.client.connect(THINGSBOARD_HOST, 1883, 60)
            self.client.loop_forever() # Blocks here until disconnect() is called
        except Exception as e:
            print(f"[PROVISION] Connection error: {e}")
            return False
        
        return self.is_success

# ==========================================
# 3. CLASS: DEVICE (OPERATIONAL MODE)
# ==========================================
class Device:
    """Represents the actual device sending data."""
    
    def __init__(self):
        self.client = mqtt.Client(CallbackAPIVersion.VERSION2)
        self.client.on_connect = self.on_connect

    def on_connect(self, client, userdata, flags, rc, properties=None):
        if rc == 0:
            print("[DEVICE] Connected successfully to ThingsBoard!")
        else:
            print(f"[DEVICE] Connection failed (Invalid Token?): {rc}")

    def run(self):
        print("\n--- STARTING DEVICE OPERATION ---")
        
        # 1. Load token from file
        try:
            with open(TOKEN_FILE, "r") as f:
                token = f.read().strip()
        except FileNotFoundError:
            print("[DEVICE] No token found! Please run provisioning first.")
            return

        if not token:
            print("[DEVICE] Token file is empty!")
            return

        # 2. Connect using the Token
        print(f"[DEVICE] Connecting with Token: {token}...")
        self.client.username_pw_set(token)
        self.client.connect(THINGSBOARD_HOST, 1883, 60)
        
        # 3. Start background loop and send data
        self.client.loop_start() 
        
        try:
            print("[DEVICE] Sending attributes every 5 seconds. Press Ctrl+C to stop.\n")
            
            # Define 3 types of command packets
            commands = [
                {
                    "command": 1,
                    "min_threshold": 6.0,
                    "max_threshold": 9.0
                },
                {
                    "command": 2,
                    "out_1_status": True,    # true is ON, false is OFF
                    "out_2_status": False,   # true is ON, false is OFF
                    "out_3_status": False,   # true is ON, false is OFF
                    "out_4_status": False    # true is ON, false is OFF
                },
                {
                    "command": 3,
                    "control_mode": True     # true is automatic, false is manual
                }
            ]
            
            command_index = 0
            
            while True:
                # Get current command in rotation
                data = commands[command_index]
                
                # Publish to attributes topic
                self.client.publish("v1/devices/me/attributes", json.dumps(data))
                
                # Print with command type description
                cmd_desc = {
                    1: "Setup Mode (min/max threshold)",
                    2: "Change Output Status",
                    3: "Change Control Mode"
                }
                
                print(f"[{time.strftime('%H:%M:%S')}] Sent Command {data['command']}: {cmd_desc[data['command']]}")
                print(f"  Data: {json.dumps(data, indent=2)}\n")
                
                # Rotate to next command
                command_index = (command_index + 1) % 3
                
                time.sleep(5)  # Send every 5 seconds
                
        except KeyboardInterrupt:
            print("\n[DEVICE] Stopping...")
            self.client.loop_stop()

# ==========================================
# 4. MAIN EXECUTION
# ==========================================
if __name__ == "__main__":
    # Step 1: Check if we already have a token
    has_token = False
    try:
        with open(TOKEN_FILE, "r") as f:
            token_content = f.read().strip()
            if len(token_content) > 0:
                has_token = True
                print(f"[INFO] Token found in {TOKEN_FILE}: {token_content}")
    except FileNotFoundError:
        pass

    # Step 2: If no token, try provisioning or manual input
    if not has_token:
        print("\n[INFO] No token found. Attempting provisioning...")
        prov = Provisioner()
        success = prov.run()
        
        if not success:
            print("\n[WARNING] Provisioning failed. This may be because the device already exists.")
            print("[INFO] If the device was already created, you need to use its Access Token.")
            print("\nYou can find the token in ThingsBoard UI:")
            print("  1. Go to Devices")
            print("  2. Click on 'PH_CONTROLLER_001'")
            print("  3. Click 'Copy access token' button")
            
            # Option to manually input token
            manual_token = input("\nEnter the Access Token manually (or press Enter to exit): ").strip()
            
            if manual_token:
                print(f"[INFO] Saving token to {TOKEN_FILE}")
                with open(TOKEN_FILE, "w") as f:
                    f.write(manual_token)
                has_token = True
            else:
                print("Program exited. No token provided.")
                sys.exit(1)
    else:
        print(f"[INFO] Using existing token. Skipping provisioning.")

    # Step 3: Run the actual Device
    device = Device()
    device.run()