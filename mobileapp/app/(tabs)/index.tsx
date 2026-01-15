import { useRouter } from "expo-router";
import React, { useState } from "react";
import { FlatList, StyleSheet, View } from "react-native";
import {
  Button,
  Dialog,
  IconButton,
  Portal,
  Switch,
  Text,
  TextInput,
  useTheme,
} from "react-native-paper";

import { useDevices } from "@/context/devices";
import { useThemeContext } from "@/context/theme";

import AsyncStorage from "@react-native-async-storage/async-storage";
import { useEffect } from "react";

type Device = Parameters<
  ReturnType<typeof useDevices>["updateDeviceName"]
>[0] extends never
  ? never
  : import("@/context/devices").Device;

function DeviceManagerScreen({ isDark }: { isDark: boolean }) {
  const { setTheme, theme } = useThemeContext();
  
  useEffect(() => {
    const checkLogin = async () => {
      const token = await AsyncStorage.getItem("token");
      if (!token) {
        router.replace("/login");
      }
    };
    checkLogin();
  }, []);

  const router = useRouter();
  const {
    devices,
    addDevice,
    resetDevices,
    updateDeviceName,
    deleteDevice,
    toggleControlMode: toggleControlModeInStore,
    loadDevicesFromAPI,
    isLoading,
  } = useDevices();
  const [editId, setEditId] = useState<number | null>(null);
  const [editName, setEditName] = useState("");
  const [deleteId, setDeleteId] = useState<number | null>(null);
  const paperTheme = useTheme();

  const handleToggleControlMode = (id: number) => {
    toggleControlModeInStore(id);
  };

  const handleToggleTheme = () => {
    if (theme === "light") {
      setTheme("dark");
    } else if (theme === "dark") {
      setTheme("system");
    } else {
      setTheme("light");
    }
  };

  const handleEdit = (id: number) => {
    const device = devices.find((d) => d.id === id);
    if (device) {
      setEditId(id);
      setEditName(device.name);
    }
  };

  const handleEditSave = () => {
    if (editId !== null) {
      updateDeviceName(editId, editName);
    }
    setEditId(null);
    setEditName("");
  };

  const handleDelete = (id: number) => {
    setDeleteId(id);
  };

  const handleDeleteCancel = () => {
    setDeleteId(null);
  };

  const handleDeleteConfirm = () => {
    if (deleteId !== null) {
      deleteDevice(deleteId);
    }
    setDeleteId(null);
  };

  const handleReset = () => resetDevices();
  const handleAddDevice = () => addDevice();
  const handleRefresh = () => resetDevices();

  const handleLoadFromAPI = async () => {
    try {
      await loadDevicesFromAPI();
      alert("✅ Đã tải thiết bị từ API thành công!");
    } catch (error: any) {
      alert(`❌ Lỗi: ${error.message || "Không thể tải thiết bị từ API"}`);
    }
  };

  const handleOpenDetails = (device: Device) => {
    router.push({
      pathname: "/device/[id]",
      params: {
        id: String(device.id),
      },
    });
  };

  const renderItem = ({ item, index }: { item: Device; index: number }) => (
    <View style={styles.deviceCard}>
      <View style={{ flexDirection: "row", alignItems: "center" }}>
        <View style={styles.deviceIconBox}>
          <IconButton icon="chip" size={28} style={{ margin: 0 }} />
        </View>
        <View style={{ flex: 1 }}>
          <Text style={styles.deviceName}>{item.name}</Text>
          <Text style={styles.deviceId}>ID: {item.id}</Text>
        </View>
        <View style={{ flexDirection: "row", alignItems: "center", gap: 4 }}>
          <IconButton
            icon="chart-line"
            size={22}
            onPress={() => handleOpenDetails(item)}
          />
          <IconButton
            icon="pencil"
            size={22}
            onPress={() => handleEdit(item.id)}
          />
          <IconButton
            icon="delete"
            size={22}
            onPress={() => handleDelete(item.id)}
          />
        </View>
      </View>
      <View style={styles.deviceInfoRow}>
        <View style={{ flex: 1 }}>
          <Text style={[styles.deviceInfoText, { flexShrink: 1 }]}>
            pH Value:{" "}
            <Text style={{ fontWeight: "bold" }}>{item.ph.toFixed(2)}</Text>
          </Text>
        </View>
        <View style={styles.controlModeRow}>
          <Text style={{ marginRight: 8 }}>Manual</Text>
          <Switch
            value={item.controlMode === "Auto"}
            onValueChange={() => handleToggleControlMode(item.id)}
            color={paperTheme.colors.primary}
            style={{
              marginHorizontal: 4,
              transform: [{ scaleX: 0.9 }, { scaleY: 0.9 }],
            }}
          />
          <Text style={{ fontWeight: "bold", color: paperTheme.colors.primary }}>
            Auto
          </Text>
        </View>
      </View>
    </View>
  );

  return (
    <View style={{ flex: 1, backgroundColor: isDark ? "#181f2a" : "#fff" }}>
      <View style={styles.headerRow}>
        <Text style={styles.title}>IoT pH Sensor Management</Text>
        <View style={{ flexDirection: "row", alignItems: "center", gap: 4 }}>
          <IconButton
            icon={theme === "light" ? "white-balance-sunny" : theme === "dark" ? "moon-waning-crescent" : "brightness-auto"}
            size={22}
            onPress={handleToggleTheme}
          />
          <IconButton
            icon="login"
            size={22}
            onPress={() => router.push("/login")}
          />
        </View>
      </View>
      <View style={styles.buttonRow}>
        <Button
          mode="contained"
          icon="plus"
          style={styles.actionButton}
          onPress={handleAddDevice}
          labelStyle={{ color: "#6fa8ff", fontWeight: "bold" }}
          disabled={isLoading}
        >
          Add New Device
        </Button>
        <Button
          mode="contained"
          icon="cloud-download"
          style={styles.actionButton}
          onPress={handleLoadFromAPI}
          labelStyle={{ color: "#6fa8ff", fontWeight: "bold" }}
          loading={isLoading}
          disabled={isLoading}
        >
          Load from API
        </Button>
        <Button
          mode="contained"
          icon="refresh"
          style={styles.actionButton}
          onPress={handleRefresh}
          labelStyle={{ color: "#6fa8ff", fontWeight: "bold" }}
          disabled={isLoading}
        >
          Refresh Devices
        </Button>
      </View>
      <FlatList
        data={devices}
        keyExtractor={(item) => item.id.toString()}
        renderItem={renderItem}
        contentContainerStyle={{ padding: 12, paddingBottom: 32 }}
        showsVerticalScrollIndicator={false}
        ListEmptyComponent={
          <Text style={{ textAlign: "center", marginTop: 32 }}>
            No devices!
          </Text>
        }
      />

      {/* Dialog đổi tên */}
      <Portal>
        <Dialog visible={editId !== null} onDismiss={() => setEditId(null)}>
          <Dialog.Title>Đổi tên thiết bị</Dialog.Title>
          <Dialog.Content>
            <TextInput
              label="Tên mới"
              value={editName}
              onChangeText={setEditName}
              mode="outlined"
            />
          </Dialog.Content>
          <Dialog.Actions>
            <Button onPress={() => setEditId(null)}>Hủy</Button>
            <Button onPress={handleEditSave}>Lưu</Button>
          </Dialog.Actions>
        </Dialog>
      </Portal>

      {/* Dialog xác nhận xóa */}
      <Portal>
        <Dialog visible={deleteId !== null} onDismiss={handleDeleteCancel}>
          <Dialog.Title>Xác nhận xóa</Dialog.Title>
          <Dialog.Content>
            <Text>Bạn có chắc chắn muốn xóa thiết bị này?</Text>
          </Dialog.Content>
          <Dialog.Actions>
            <Button onPress={handleDeleteCancel}>Hủy</Button>
            <Button onPress={handleDeleteConfirm}>Xóa</Button>
          </Dialog.Actions>
        </Dialog>
      </Portal>
    </View>
  );
}

const styles = StyleSheet.create({
  headerRow: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "center",
    paddingHorizontal: 8,
  },
  controlModeRow: {
    flex: 1.5,
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "flex-end",
    flexWrap: "nowrap",
    minWidth: 120,
  },
  title: {
    fontSize: 20,
    fontWeight: "bold",
    textAlign: "center",
    marginTop: 32,
    marginBottom: 16,
    color: "#fff",
    letterSpacing: 0.5,
  },
  buttonRow: {
    flexDirection: "row",
    justifyContent: "center",
    alignItems: "center",
    gap: 4,
    marginBottom: 16,
    paddingHorizontal: 8, // Thêm dòng này để tránh tràn lề
  },
  actionButton: {
    borderRadius: 24,
    marginHorizontal: 5,
    paddingHorizontal: 0, // Để mặc định hoặc giảm lại
    backgroundColor: "#232b3b",
    flex: 1, // Thêm dòng này để nút tự co giãn đều nhau
    minWidth: 0, // Đảm bảo không bị giới hạn min width
    maxWidth: 180, // GIỚI HẠN CHIỀU RỘNG MỖI NÚT
  },
  deviceCard: {
    backgroundColor: "#232b3b",
    borderRadius: 12,
    padding: 12,
    marginBottom: 14,
    borderWidth: 1,
    borderColor: "#3a4253",
    shadowColor: "#000",
    shadowOpacity: 0.1,
    shadowRadius: 4,
    elevation: 2,
  },
  deviceIconBox: {
    backgroundColor: "#181f2a",
    borderRadius: 8,
    padding: 2,
    marginRight: 12,
  },
  deviceName: {
    fontWeight: "bold",
    fontSize: 16,
    color: "#fff",
  },
  deviceId: {
    color: "#b0b8c9",
    fontSize: 13,
  },
  deviceInfoRow: {
    flexDirection: "row",
    alignItems: "center",
    marginTop: 8,
    flexWrap: "nowrap",
    justifyContent: "space-between",
    minHeight: 32,
  },
  deviceInfoText: {
    color: "#fff",
    fontSize: 14,
    marginRight: 8,
  },
});

export default function IndexScreen() {
  const theme = useTheme();
  return <DeviceManagerScreen isDark={theme.dark} />;
}
