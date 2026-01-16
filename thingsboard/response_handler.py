"""
Response Handler Module - Handles receiving responses from ThingsBoard
"""

import json
from typing import Callable, Dict, Any, Optional
from enum import Enum


# ==========================================
# MQTT TOPICS FOR RESPONSES
# ==========================================
class ResponseTopic(Enum):
    """ThingsBoard response topics"""
    PROVISION_RESPONSE = "/provision/response"
    ATTRIBUTES_RESPONSE = "v1/devices/me/attributes/response/+"
    RPC_REQUEST = "v1/devices/me/rpc/request/+"
    ATTRIBUTES_PUSH = "v1/devices/me/attributes"


# ==========================================
# RESPONSE PACKET DEFINITIONS
# ==========================================
class ProvisionResponse:
    """Provision response from ThingsBoard"""
    
    def __init__(self, data: Dict[str, Any]):
        """
        Parse provision response
        
        Args:
            data: Raw JSON response from ThingsBoard
        """
        self.status = data.get("status")
        self.credentials_type = data.get("credentialsType")
        self.credentials_value = data.get("credentialsValue")  # Access Token
        self.error_msg = data.get("errorMsg")
    
    def is_success(self) -> bool:
        """Check if provisioning was successful"""
        return self.status == "SUCCESS"
    
    def get_token(self) -> Optional[str]:
        """Get access token if provisioning successful"""
        return self.credentials_value if self.is_success() else None
    
    def __repr__(self):
        return f"ProvisionResponse(status={self.status}, token={self.credentials_value})"


class AttributesResponse:
    """Attributes response from ThingsBoard"""
    
    def __init__(self, data: Dict[str, Any]):
        """
        Parse attributes response
        
        Args:
            data: Raw JSON response with attributes
        """
        self.shared = data.get("shared", {})
        self.client = data.get("client", {})
    
    def get_shared_attribute(self, key: str, default=None):
        """Get shared attribute value"""
        return self.shared.get(key, default)
    
    def get_client_attribute(self, key: str, default=None):
        """Get client attribute value"""
        return self.client.get(key, default)
    
    def __repr__(self):
        return f"AttributesResponse(shared={self.shared}, client={self.client})"


class RPCRequest:
    """RPC request from ThingsBoard"""
    
    def __init__(self, request_id: str, method: str, params: Dict[str, Any]):
        """
        Parse RPC request
        
        Args:
            request_id: Request ID for response
            method: RPC method name
            params: RPC parameters
        """
        self.request_id = request_id
        self.method = method
        self.params = params
    
    def __repr__(self):
        return f"RPCRequest(id={self.request_id}, method={self.method}, params={self.params})"


# ==========================================
# RESPONSE HANDLER
# ==========================================
class ResponseHandler:
    """Handles all responses from ThingsBoard"""
    
    def __init__(self, mqtt_client):
        """
        Initialize response handler
        
        Args:
            mqtt_client: paho.mqtt.client.Client instance
        """
        self.client = mqtt_client
        self.provision_callback: Optional[Callable] = None
        self.attributes_callback: Optional[Callable] = None
        self.rpc_callback: Optional[Callable] = None
        
        # Set up MQTT message handler
        self.client.on_message = self._on_message
    
    def _on_message(self, client, userdata, msg):
        """Internal MQTT message handler"""
        try:
            topic = msg.topic
            payload = msg.payload.decode()
            data = json.loads(payload)
            
            # Route to appropriate handler
            if topic == ResponseTopic.PROVISION_RESPONSE.value:
                self._handle_provision_response(data)
            elif topic.startswith("v1/devices/me/attributes/response"):
                self._handle_attributes_response(data)
            elif topic.startswith("v1/devices/me/rpc/request"):
                request_id = topic.split("/")[-1]
                self._handle_rpc_request(request_id, data)
            elif topic == "v1/devices/me/attributes":
                self._handle_attributes_push(data)
            else:
                print(f"[RESPONSE] Unknown topic: {topic}")
                
        except Exception as e:
            print(f"[RESPONSE] Error handling message: {e}")
    
    def _handle_provision_response(self, data: Dict[str, Any]):
        """Handle provision response"""
        response = ProvisionResponse(data)
        
        if self.provision_callback:
            self.provision_callback(response)
        else:
            print(f"[RESPONSE] Provision: {response}")
    
    def _handle_attributes_response(self, data: Dict[str, Any]):
        """Handle attributes response"""
        response = AttributesResponse(data)
        
        if self.attributes_callback:
            self.attributes_callback(response)
        else:
            print(f"[RESPONSE] Attributes: {response}")
    
    def _handle_rpc_request(self, request_id: str, data: Dict[str, Any]):
        """Handle RPC request from server"""
        method = data.get("method")
        params = data.get("params", {})
        
        rpc_request = RPCRequest(request_id, method, params)
        
        if self.rpc_callback:
            # Let callback handle and return response
            response_data = self.rpc_callback(rpc_request)
            self._send_rpc_response(request_id, response_data)
        else:
            print(f"[RESPONSE] RPC Request: {rpc_request}")
            # Send default response
            self._send_rpc_response(request_id, {"status": "ok"})
    
    def _handle_attributes_push(self, data: Dict[str, Any]):
        """Handle pushed attributes from server"""
        if self.attributes_callback:
            response = AttributesResponse({"shared": data})
            self.attributes_callback(response)
        else:
            print(f"[RESPONSE] Attributes pushed: {data}")
    
    def _send_rpc_response(self, request_id: str, response_data: Dict[str, Any]):
        """Send RPC response back to ThingsBoard"""
        topic = f"v1/devices/me/rpc/response/{request_id}"
        payload = json.dumps(response_data)
        self.client.publish(topic, payload)
    
    # ==========================================
    # CALLBACK REGISTRATION
    # ==========================================
    def on_provision_response(self, callback: Callable[[ProvisionResponse], None]):
        """
        Register callback for provision responses
        
        Args:
            callback: Function that takes ProvisionResponse as parameter
        """
        self.provision_callback = callback
    
    def on_attributes_response(self, callback: Callable[[AttributesResponse], None]):
        """
        Register callback for attributes responses
        
        Args:
            callback: Function that takes AttributesResponse as parameter
        """
        self.attributes_callback = callback
    
    def on_rpc_request(self, callback: Callable[[RPCRequest], Dict[str, Any]]):
        """
        Register callback for RPC requests
        
        Args:
            callback: Function that takes RPCRequest and returns response dict
        """
        self.rpc_callback = callback
    
    # ==========================================
    # SUBSCRIPTION METHODS
    # ==========================================
    def subscribe_provision_response(self):
        """Subscribe to provision response topic"""
        self.client.subscribe(ResponseTopic.PROVISION_RESPONSE.value)
        print(f"[RESPONSE] Subscribed to {ResponseTopic.PROVISION_RESPONSE.value}")
    
    def subscribe_attributes_response(self):
        """Subscribe to attributes response topic"""
        self.client.subscribe(ResponseTopic.ATTRIBUTES_RESPONSE.value)
        print(f"[RESPONSE] Subscribed to attributes responses")
    
    def subscribe_rpc_requests(self):
        """Subscribe to RPC request topic"""
        self.client.subscribe(ResponseTopic.RPC_REQUEST.value)
        print(f"[RESPONSE] Subscribed to RPC requests")
    
    def subscribe_attributes_push(self):
        """Subscribe to pushed attributes from server"""
        self.client.subscribe(ResponseTopic.ATTRIBUTES_PUSH.value)
        print(f"[RESPONSE] Subscribed to attribute pushes")
    
    def subscribe_all(self):
        """Subscribe to all response topics"""
        self.subscribe_provision_response()
        self.subscribe_attributes_response()
        self.subscribe_rpc_requests()
        self.subscribe_attributes_push()
