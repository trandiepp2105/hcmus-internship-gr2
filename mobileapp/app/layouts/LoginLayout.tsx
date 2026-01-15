import React from "react";
import { StyleSheet, View } from "react-native";

export default function LoginLayout({ children }: { children: React.ReactNode }) {
  return (
    <View style={styles.container}>
      <View style={styles.box}>{children}</View>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: "#f0f2f5",
    justifyContent: "center",
    alignItems: "center",
  },
  box: {
    padding: 32,
    borderRadius: 8,
    backgroundColor: "#fff",
    minWidth: 320,
    elevation: 4,
  },
});