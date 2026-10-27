import React from 'react';
import { View, Text, StyleSheet, TouchableOpacity, Image } from 'react-native';
import { Link } from 'expo-router';

export default function Home() {
  return (
    <View style={styles.container}>
      <Image
        source={require('../assets/icon.png')}
        style={styles.logo}
        accessibilityLabel="App Logo"
      />
      <Text style={styles.title}>Expo Router Workbench</Text>
      <Text style={styles.subtitle}>Feature Matrix & Navigation Test Fixture</Text>

      <View style={styles.navGroup}>
        <Link href="/details?item=WorkbenchFeature" asChild>
          <TouchableOpacity style={styles.button} testID="nav-details-btn">
            <Text style={styles.buttonText}>Go to Details Screen</Text>
          </TouchableOpacity>
        </Link>

        <Link href="/form" asChild>
          <TouchableOpacity style={styles.button} testID="nav-form-btn">
            <Text style={styles.buttonText}>Open Form & Validation</Text>
          </TouchableOpacity>
        </Link>

        <Link href="/storage" asChild>
          <TouchableOpacity style={styles.button} testID="nav-storage-btn">
            <Text style={styles.buttonText}>Test Local Persistence</Text>
          </TouchableOpacity>
        </Link>
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#0f172a',
    alignItems: 'center',
    justifyContent: 'center',
    padding: 24,
  },
  logo: {
    width: 80,
    height: 80,
    marginBottom: 16,
    borderRadius: 16,
  },
  title: {
    fontSize: 22,
    fontWeight: 'bold',
    color: '#f8fafc',
    marginBottom: 6,
  },
  subtitle: {
    fontSize: 14,
    color: '#94a3b8',
    marginBottom: 24,
  },
  navGroup: {
    width: '100%',
    maxWidth: 320,
    gap: 12,
  },
  button: {
    backgroundColor: '#3b82f6',
    paddingVertical: 12,
    paddingHorizontal: 20,
    borderRadius: 10,
    alignItems: 'center',
  },
  buttonText: {
    color: '#ffffff',
    fontWeight: '600',
    fontSize: 15,
  },
});
