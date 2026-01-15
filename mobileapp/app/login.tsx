import { Stack } from "expo-router";
import React from "react";

import LoginForm from "@/components/LoginForm";
import LoginLayout from "./layouts/LoginLayout";

export default function LoginScreen() {
  return (
    <>
      <Stack.Screen options={{ headerShown: false }} />
      <LoginLayout>
        <LoginForm />
      </LoginLayout>
    </>
  );
}
