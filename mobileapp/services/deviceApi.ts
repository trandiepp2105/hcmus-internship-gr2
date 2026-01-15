const BASE_URL = "http://192.168.1.13:8080/api";

export interface ThingsBoardDevice {
  id: {
    entityType: string;
    id: string;
  };
  createdTime: number;
  name: string;
  type: string;
  label: string | null;
  deviceProfileId: {
    entityType: string;
    id: string;
  };
  tenantId: {
    entityType: string;
    id: string;
  };
  customerId: {
    entityType: string;
    id: string;
  };
}

export interface DevicesResponse {
  data: ThingsBoardDevice[];
  totalPages: number;
  totalElements: number;
  hasNext: boolean;
}

export async function fetchDevices(
  token: string,
  type: string = "PH_CONTROLLER",
  pageSize: number = 10,
  page: number = 0
): Promise<DevicesResponse> {
  const response = await fetch(
    `${BASE_URL}/tenant/devices?type=${type}&pageSize=${pageSize}&page=${page}`,
    {
      method: "GET",
      headers: {
        "Content-Type": "application/json",
        "X-Authorization": `Bearer ${token}`,
      },
    }
  );

  if (!response.ok) {
    throw new Error(`Failed to fetch devices: ${response.status}`);
  }

  return response.json();
}

export interface TelemetryData {
  [key: string]: Array<{
    ts: number;
    value: string | number;
  }>;
}

export async function fetchDeviceTelemetry(
  token: string,
  deviceId: string,
  limit: number = 20
): Promise<TelemetryData> {
  const startTs = 1766646366384;
  const endTs = 1766750400384;

  const keys = "ph_value,control_mode,active,out_1_status,out_2_status,out_3_status,out_4_status,alarms";
  
  const url = `${BASE_URL}/plugins/telemetry/DEVICE/${deviceId}/values/timeseries?keys=${keys}&startTs=${startTs}&endTs=${endTs}&limit=${limit}`;

  console.log("🔍 Fetching telemetry from:", url);

  const response = await fetch(url, {
    method: "GET",
    headers: {
      "Content-Type": "application/json",
      "X-Authorization": `Bearer ${token}`,
    },
  });

  if (!response.ok) {
    throw new Error(`Failed to fetch telemetry: ${response.status}`);
  }

  const data = await response.json();
  console.log("📊 Telemetry data received:", JSON.stringify(data, null, 2));
  
  return data;
}
