import { Stack, useLocalSearchParams, useRouter } from "expo-router";
import React, { useEffect, useState } from "react";
import { ActivityIndicator, Dimensions, FlatList, ScrollView, StyleSheet, View } from "react-native";
import { LineChart } from "react-native-chart-kit";
import {
  Button,
  Dialog,
  Divider,
  Portal,
  Switch,
  Text,
  useTheme
} from "react-native-paper";

import { useDevices } from "@/context/devices";
import { fetchDeviceTelemetry } from "@/services/deviceApi";
import AsyncStorage from "@react-native-async-storage/async-storage";

type Row = {
  ph: string;
  temp: string;
  o1: string;
  o2: string;
  o3: string;
  o4: string;
  time: string;
  ts: number;
};

export default function DeviceDetailsScreen() {
  const router = useRouter();
  const theme = useTheme();
  const { devices, deleteDevice, toggleControlMode } = useDevices();
  const params = useLocalSearchParams<{ id?: string }>();
  const deviceId = Number(params.id);

  const device = devices.find((d) => d.id === deviceId);

  const [deleteOpen, setDeleteOpen] = useState(false);
  const [o1, setO1] = useState(false);
  const [o2, setO2] = useState(false);
  const [o3, setO3] = useState(false);
  const [o4, setO4] = useState(false);
  const [rows, setRows] = useState<Row[]>([]);
  const [isLoading, setIsLoading] = useState(true);

  // Fetch telemetry data when screen loads and every 5 minutes
  useEffect(() => {
    const loadTelemetry = async () => {
      if (!device?.thingsBoardId) {
        console.log("⚠️ Device không có thingsBoardId:", device);
        setIsLoading(false);
        return;
      }

      try {
        const token = await AsyncStorage.getItem("token");
        if (!token) {
          console.log("⚠️ Không có token");
          setIsLoading(false);
          return;
        }

        console.log("🚀 Loading telemetry for device:", device.thingsBoardId);
        const data = await fetchDeviceTelemetry(token, device.thingsBoardId, 20);
        
        // Parse telemetry data into rows
        const phValues = data.ph_value || [];
        const tempValues = data.temperature || [];
        const out1Values = data.out_1_status || [];
        const out2Values = data.out_2_status || [];
        const out3Values = data.out_3_status || [];
        const out4Values = data.out_4_status || [];

        const parsedRows: Row[] = phValues.map((phItem, index) => {
          const timestamp = new Date(phItem.ts);
          const timeStr = `${timestamp.getHours().toString().padStart(2, "0")}:${timestamp
            .getMinutes()
            .toString()
            .padStart(2, "0")}:${timestamp.getSeconds().toString().padStart(2, "0")}`;

          return {
            ph: String(phItem.value),
            temp: tempValues[index]?.value ? String(tempValues[index].value) : "-",
            o1: out1Values[index]?.value === "true" ? "1" : "0",
            o2: out2Values[index]?.value === "true" ? "1" : "0",
            o3: out3Values[index]?.value === "true" ? "1" : "0",
            o4: out4Values[index]?.value === "true" ? "1" : "0",
            time: timeStr,
            ts: phItem.ts,
          };
        });

        setRows(parsedRows);

        // Update switch states from latest data
        if (parsedRows.length > 0) {
          const latest = parsedRows[0];
          setO1(latest.o1 === "1");
          setO2(latest.o2 === "1");
          setO3(latest.o3 === "1");
          setO4(latest.o4 === "1");
        }

        setIsLoading(false);
      } catch (error) {
        console.error("❌ Error loading telemetry:", error);
        setIsLoading(false);
      }
    };

    // Load immediately
    loadTelemetry();

    // Setup interval to reload every 5 minutes
    const intervalId = setInterval(loadTelemetry, 5 * 60 * 1000);

    // Cleanup interval on unmount
    return () => clearInterval(intervalId);
  }, [device?.thingsBoardId]);

  const isDark = theme.dark;

  if (!device) {
    return (
      <View style={[styles.screen, { backgroundColor: isDark ? "#181f2a" : "#fff" }]}>
        <Stack.Screen options={{ headerShown: false }} />
        <View style={styles.topBar}>
          <Button mode="text" onPress={() => router.back()} textColor={theme.colors.primary}>
            Back
          </Button>
        </View>
        <Text style={{ color: isDark ? "#fff" : "#111", padding: 16 }}>
          Không tìm thấy thiết bị.
        </Text>
      </View>
    );
  }

  return (
    <View style={[styles.screen, { backgroundColor: isDark ? "#0f1520" : "#fff" }]}>
      <Stack.Screen options={{ headerShown: false }} />

      <ScrollView showsVerticalScrollIndicator={false}>
        <View style={styles.topBar}>
        <Button mode="text" onPress={() => router.back()} textColor={theme.colors.primary}>
          Back
        </Button>

        <Text style={[styles.headerTitle, { color: isDark ? "#fff" : "#111" }]}>Device {device.id}</Text>

        <Button mode="text" onPress={() => setDeleteOpen(true)} textColor={theme.colors.error}>
          Delete
        </Button>
      </View>

      <View style={[styles.card, { backgroundColor: isDark ? "#232b3b" : "#f5f5f5" }]}> 
        <View style={styles.modeRow}>
          <Text style={[styles.modeLabel, { color: isDark ? "#fff" : "#111" }]}>Manual</Text>
          <Switch
            value={device.controlMode === "Auto"}
            onValueChange={() => toggleControlMode(device.id)}
            style={{ transform: [{ scaleX: 1.0 }, { scaleY: 1.0 }] }}
          />
          <Text style={[styles.modeLabel, { color: isDark ? "#fff" : "#111" }]}>Auto</Text>
        </View>

        <Divider style={{ opacity: 0.2 }} />

        <View style={styles.outputsRow}>
          <View style={styles.outputCol}>
            <Text style={[styles.outputLabel, { color: isDark ? "#fff" : "#111" }]}>Output 1</Text>
            <Switch value={o1} onValueChange={setO1} />
          </View>
          <View style={styles.outputCol}>
            <Text style={[styles.outputLabel, { color: isDark ? "#fff" : "#111" }]}>Output 2</Text>
            <Switch value={o2} onValueChange={setO2} />
          </View>
          <View style={styles.outputCol}>
            <Text style={[styles.outputLabel, { color: isDark ? "#fff" : "#111" }]}>Output 3</Text>
            <Switch value={o3} onValueChange={setO3} />
          </View>
          <View style={styles.outputCol}>
            <Text style={[styles.outputLabel, { color: isDark ? "#fff" : "#111" }]}>Output 4</Text>
            <Switch value={o4} onValueChange={setO4} />
          </View>
        </View>
      </View>

      <View style={[styles.card, { backgroundColor: isDark ? "#232b3b" : "#f5f5f5" }]}> 
        <Text style={[styles.sectionTitle, { color: theme.colors.primary }]}>Bảng 20 hàng dữ liệu từ API</Text>

        {isLoading ? (
          <View style={{ padding: 20, alignItems: "center" }}>
            <ActivityIndicator size="large" color={theme.colors.primary} />
            <Text style={{ color: isDark ? "#fff" : "#111", marginTop: 10 }}>Đang tải dữ liệu...</Text>
          </View>
        ) : rows.length === 0 ? (
          <View style={{ padding: 20, alignItems: "center" }}>
            <Text style={{ color: isDark ? "#fff" : "#111" }}>Không có dữ liệu</Text>
          </View>
        ) : (
          <>
            <View style={styles.tableHeader}>
              <Text style={[styles.th, { flex: 1.0 }]}>pH</Text>
              <Text style={[styles.th, { flex: 1.0 }]}>Temp</Text>
              <Text style={[styles.th, { flex: 0.6 }]}>O1</Text>
              <Text style={[styles.th, { flex: 0.6 }]}>O2</Text>
              <Text style={[styles.th, { flex: 0.6 }]}>O3</Text>
              <Text style={[styles.th, { flex: 0.6 }]}>O4</Text>
              <Text style={[styles.th, { flex: 1.2 }]}>Time</Text>
            </View>

            <FlatList
              data={rows}
              keyExtractor={(item) => String(item.ts)}
              scrollEnabled={false}
              renderItem={({ item, index }) => (
                <View
                  style={[
                    styles.tr,
                    {
                      backgroundColor:
                        index % 2 === 0
                          ? isDark
                            ? "rgba(255,255,255,0.03)"
                            : "rgba(0,0,0,0.03)"
                          : "transparent",
                    },
                  ]}
                >
                  <Text style={[styles.td, { flex: 1.0 }]}>{item.ph}</Text>
                  <Text style={[styles.td, { flex: 1.0 }]}>{item.temp}</Text>
                  <Text style={[styles.td, { flex: 0.6 }]}>{item.o1}</Text>
                  <Text style={[styles.td, { flex: 0.6 }]}>{item.o2}</Text>
                  <Text style={[styles.td, { flex: 0.6 }]}>{item.o3}</Text>
                  <Text style={[styles.td, { flex: 0.6 }]}>{item.o4}</Text>
                  <Text style={[styles.td, { flex: 1.2 }]}>{item.time}</Text>
                </View>
              )}
            />
          </>
        )}
      </View>

      {/* pH Chart */}
      <View style={[styles.chartCard, { backgroundColor: isDark ? "#232b3b" : "#f5f5f5" }]}>
        <Text style={[styles.chartTitle, { color: theme.colors.primary }]}>Biểu đồ pH theo thời gian</Text>
        {rows.length > 0 ? (
          <LineChart
            data={{
              labels: rows.slice(0, 10).reverse().map((r) => r.time.slice(0, 5)),
              datasets: [
                {
                  data: rows.slice(0, 10).reverse().map((r) => parseFloat(r.ph) || 0),
                },
              ],
            }}
            width={Dimensions.get("window").width - 48}
            height={220}
            chartConfig={{
              backgroundColor: isDark ? "#232b3b" : "#f5f5f5",
              backgroundGradientFrom: isDark ? "#232b3b" : "#f5f5f5",
              backgroundGradientTo: isDark ? "#1a2232" : "#e8e8e8",
              decimalPlaces: 1,
              color: (opacity = 1) => `rgba(111, 168, 255, ${opacity})`,
              labelColor: (opacity = 1) => (isDark ? `rgba(255, 255, 255, ${opacity})` : `rgba(0, 0, 0, ${opacity})`),
              style: {
                borderRadius: 16,
              },
              propsForDots: {
                r: "4",
                strokeWidth: "2",
                stroke: "#6fa8ff",
              },
            }}
            bezier
            style={{
              marginVertical: 8,
              borderRadius: 16,
            }}
          />
        ) : (
          <View style={{ padding: 20, alignItems: "center" }}>
            <Text style={{ color: isDark ? "#fff" : "#111" }}>Không có dữ liệu</Text>
          </View>
        )}
      </View>

      {/* Temperature Chart */}
      <View style={[styles.chartCard, { backgroundColor: isDark ? "#232b3b" : "#f5f5f5", marginTop: 12 }]}>
        <Text style={[styles.chartTitle, { color: theme.colors.primary }]}>Biểu đồ nhiệt độ theo thời gian</Text>
        {rows.length > 0 && rows.some(r => r.temp !== "-") ? (
          <LineChart
            data={{
              labels: rows.slice(0, 10).reverse().map((r) => r.time.slice(0, 5)),
              datasets: [
                {
                  data: rows.slice(0, 10).reverse().map((r) => r.temp !== "-" ? parseFloat(r.temp) : 0),
                },
              ],
            }}
            width={Dimensions.get("window").width - 48}
            height={220}
            chartConfig={{
              backgroundColor: isDark ? "#232b3b" : "#f5f5f5",
              backgroundGradientFrom: isDark ? "#232b3b" : "#f5f5f5",
              backgroundGradientTo: isDark ? "#1a2232" : "#e8e8e8",
              decimalPlaces: 1,
              color: (opacity = 1) => `rgba(255, 111, 111, ${opacity})`,
              labelColor: (opacity = 1) => (isDark ? `rgba(255, 255, 255, ${opacity})` : `rgba(0, 0, 0, ${opacity})`),
              style: {
                borderRadius: 16,
              },
              propsForDots: {
                r: "4",
                strokeWidth: "2",
                stroke: "#ff6f6f",
              },
            }}
            bezier
            style={{
              marginVertical: 8,
              borderRadius: 16,
            }}
          />
        ) : (
          <View style={{ padding: 20, alignItems: "center" }}>
            <Text style={{ color: isDark ? "#fff" : "#111" }}>Không có dữ liệu nhiệt độ</Text>
          </View>
        )}
      </View>

        <View style={{ height: 24 }} />
      </ScrollView>

      <Portal>
        <Dialog visible={deleteOpen} onDismiss={() => setDeleteOpen(false)}>
          <Dialog.Title>Xác nhận xóa</Dialog.Title>
          <Dialog.Content>
            <Text>Bạn có chắc chắn muốn xóa {device.name}?</Text>
          </Dialog.Content>
          <Dialog.Actions>
            <Button onPress={() => setDeleteOpen(false)}>Hủy</Button>
            <Button
              onPress={() => {
                deleteDevice(device.id);
                setDeleteOpen(false);
                router.back();
              }}
              textColor={theme.colors.error}
            >
              Xóa
            </Button>
          </Dialog.Actions>
        </Dialog>
      </Portal>
    </View>
  );
}

const styles = StyleSheet.create({
  screen: {
    flex: 1,
    paddingHorizontal: 12,
    paddingTop: 12,
  },
  topBar: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "space-between",
    paddingHorizontal: 4,
    paddingBottom: 8,
  },
  headerTitle: {
    fontSize: 18,
    fontWeight: "700",
  },
  card: {
    borderRadius: 14,
    padding: 12,
    marginTop: 12,
    borderWidth: 1,
    borderColor: "rgba(255,255,255,0.10)",
  },
  modeRow: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "center",
    gap: 10,
    paddingVertical: 6,
  },
  modeLabel: {
    fontSize: 16,
    fontWeight: "600",
  },
  outputsRow: {
    flexDirection: "row",
    justifyContent: "space-between",
    marginTop: 10,
  },
  outputCol: {
    alignItems: "center",
    flex: 1,
  },
  outputLabel: {
    fontSize: 13,
    marginBottom: 6,
    opacity: 0.9,
  },
  sectionTitle: {
    fontSize: 14,
    fontWeight: "700",
    marginBottom: 10,
    textAlign: "center",
  },
  tableHeader: {
    flexDirection: "row",
    paddingVertical: 8,
    borderTopWidth: 1,
    borderBottomWidth: 1,
    borderColor: "rgba(255,255,255,0.12)",
  },
  th: {
    color: "rgba(255,255,255,0.85)",
    fontWeight: "700",
    fontSize: 12,
    textAlign: "center",
  },
  tr: {
    flexDirection: "row",
    paddingVertical: 8,
    borderBottomWidth: StyleSheet.hairlineWidth,
    borderBottomColor: "rgba(255,255,255,0.08)",
  },
  td: {
    color: "rgba(255,255,255,0.85)",
    fontSize: 12,
    textAlign: "center",
  },
  chartCard: {
    borderRadius: 14,
    padding: 12,
    marginTop: 12,
    borderWidth: 1,
    borderColor: "rgba(255,255,255,0.10)",
  },
  chartTitle: {
    fontSize: 14,
    fontWeight: "700",
    marginBottom: 10,
    textAlign: "center",
  },
});
