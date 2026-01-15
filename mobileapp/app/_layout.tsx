import {
  DarkTheme as NavigationDarkTheme,
  DefaultTheme as NavigationDefaultTheme,
  ThemeProvider,
} from '@react-navigation/native';
import { Stack } from 'expo-router';
import { StatusBar } from 'expo-status-bar';
import 'react-native-reanimated';

import {
  MD3DarkTheme as PaperDarkTheme,
  MD3LightTheme as PaperDefaultTheme,
  Provider as PaperProvider,
} from 'react-native-paper';

import { DevicesProvider } from '@/context/devices';
import { ThemeProvider as CustomThemeProvider, useThemeContext } from '@/context/theme';

export const unstable_settings = {
  // Đặt màn hình đăng nhập là route đầu tiên khi mở app
  anchor: 'login',
};

function RootLayoutInner() {
  const { isDark } = useThemeContext();
  const paperTheme = isDark ? PaperDarkTheme : PaperDefaultTheme;
  const navigationTheme = isDark ? NavigationDarkTheme : NavigationDefaultTheme;

  return (
    <ThemeProvider value={navigationTheme}>
      <PaperProvider theme={paperTheme}>
        <DevicesProvider>
          <Stack initialRouteName="login">
            <Stack.Screen name="login" options={{ headerShown: false }} />
            <Stack.Screen name="(tabs)" options={{ headerShown: false }} />
            <Stack.Screen
              name="modal"
              options={{ presentation: 'modal', title: 'Modal' }}
            />
          </Stack>
          <StatusBar style="auto" />
        </DevicesProvider>
      </PaperProvider>
    </ThemeProvider>
  );
}

export default function RootLayout() {
  return (
    <CustomThemeProvider>
      <RootLayoutInner />
    </CustomThemeProvider>
  );
}
