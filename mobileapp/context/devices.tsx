import { fetchDevices } from "@/services/deviceApi";
import AsyncStorage from "@react-native-async-storage/async-storage";
import React, { createContext, useContext, useEffect, useMemo, useState } from "react";

export type Device = {
  id: number;
  name: string;
  isOn: boolean;
  ph: number;
  controlMode: "Auto" | "Manual";
  thingsBoardId?: string;
};

const initialDevices: Device[] = [
  { id: 1, name: "Device 1", isOn: false, ph: 7.0, controlMode: "Auto" },
  { id: 2, name: "Device 2", isOn: true, ph: 7.0, controlMode: "Auto" },
  { id: 3, name: "Device 3", isOn: false, ph: 7.0, controlMode: "Auto" },
  { id: 4, name: "Device 4", isOn: false, ph: 7.0, controlMode: "Auto" },
];

type DevicesContextValue = {
  devices: Device[];
  addDevice: () => void;
  resetDevices: () => void;
  updateDeviceName: (id: number, name: string) => void;
  deleteDevice: (id: number) => void;
  toggleControlMode: (id: number) => void;
  loadDevicesFromAPI: () => Promise<void>;
  isLoading: boolean;
};

const DevicesContext = createContext<DevicesContextValue | null>(null);

export function DevicesProvider({ children }: { children: React.ReactNode }) {
  const [devices, setDevices] = useState<Device[]>(initialDevices);
  const [isLoading, setIsLoading] = useState(false);

  // Load devices from AsyncStorage when component mounts
  useEffect(() => {
    const loadStoredDevices = async () => {
      try {
        const stored = await AsyncStorage.getItem("devices");
        if (stored) {
          setDevices(JSON.parse(stored));
        }
      } catch (error) {
        console.error("Failed to load devices from storage:", error);
      }
    };
    loadStoredDevices();
  }, []);

  // Save devices to AsyncStorage whenever they change
  useEffect(() => {
    const saveDevices = async () => {
      try {
        await AsyncStorage.setItem("devices", JSON.stringify(devices));
      } catch (error) {
        console.error("Failed to save devices:", error);
      }
    };
    saveDevices();
  }, [devices]);

  const loadDevicesFromAPI = async () => {
    setIsLoading(true);
    try {
      const token = await AsyncStorage.getItem("token");
      if (!token) {
        throw new Error("No authentication token found. Please login first.");
      }

      const response = await fetchDevices(token);
      
      // Convert ThingsBoard devices to local Device format
      const apiDevices: Device[] = response.data.map((tbDevice, index) => {
        const existingMaxId = devices.length > 0 ? Math.max(...devices.map((d) => d.id)) : 0;
        return {
          id: existingMaxId + index + 1,
          name: tbDevice.name,
          isOn: false,
          ph: 7.0,
          controlMode: "Manual" as const,
          thingsBoardId: tbDevice.id.id,
        };
      });

      // Add new devices to existing ones
      setDevices((prev) => [...prev, ...apiDevices]);
      
      return;
    } catch (error) {
      console.error("Failed to fetch devices from API:", error);
      throw error;
    } finally {
      setIsLoading(false);
    }
  };

  const value = useMemo<DevicesContextValue>(() => {
    return {
      devices,
      isLoading,
      loadDevicesFromAPI,
      addDevice: () => {
        const nextId =
          devices.length > 0 ? Math.max(...devices.map((d) => d.id)) + 1 : 1;
        setDevices([
          ...devices,
          {
            id: nextId,
            name: `Device ${nextId}`,
            isOn: false,
            ph: 7.0,
            controlMode: "Auto",
          },
        ]);
      },
      resetDevices: () => setDevices(initialDevices),
      updateDeviceName: (id, name) => {
        setDevices((prev) => prev.map((d) => (d.id === id ? { ...d, name } : d)));
      },
      deleteDevice: (id) => {
        setDevices((prev) => prev.filter((d) => d.id !== id));
      },
      toggleControlMode: (id) => {
        setDevices((prev) =>
          prev.map((device) =>
            device.id === id
              ? {
                  ...device,
                  controlMode: device.controlMode === "Auto" ? "Manual" : "Auto",
                }
              : device
          )
        );
      },
    };
  }, [devices, isLoading]);

  return <DevicesContext.Provider value={value}>{children}</DevicesContext.Provider>;
}

export function useDevices() {
  const ctx = useContext(DevicesContext);
  if (!ctx) {
    throw new Error("useDevices must be used within DevicesProvider");
  }
  return ctx;
}
