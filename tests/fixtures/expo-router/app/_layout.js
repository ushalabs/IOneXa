import React from 'react';
import { Stack } from 'expo-router';

export default function Layout() {
  return (
    <Stack
      screenOptions={{
        headerStyle: { backgroundColor: '#1e293b' },
        headerTintColor: '#38bdf8',
        headerTitleStyle: { fontWeight: 'bold' },
      }}
    >
      <Stack.Screen name="index" options={{ title: 'Home' }} />
      <Stack.Screen name="details" options={{ title: 'Details' }} />
      <Stack.Screen name="form" options={{ title: 'Form Input' }} />
      <Stack.Screen name="storage" options={{ title: 'Local Persistence' }} />
    </Stack>
  );
}
