import React, { useState } from 'react';
import {
  StyleSheet,
  Text,
  View,
  Pressable,
  TextInput,
  Alert,
  Platform,
  SafeAreaView,
  ScrollView,
  StatusBar
} from 'react-native';
import { PlatformIndicator } from './PlatformIndicator';

export default function App() {
  const [currentScreen, setCurrentScreen] = useState<'home' | 'details'>('home');
  const [counter, setCounter] = useState<number>(0);
  const [inputText, setInputText] = useState<string>('');
  const [networkStatus, setNetworkStatus] = useState<string>('Idle');
  const [errorSimulated, setErrorSimulated] = useState<boolean>(false);

  const triggerNativeAlert = () => {
    Alert.alert(
      'UIKit Native Alert',
      `Verified on ${Platform.OS} Simulator. Counter value: ${counter}`,
      [
        { text: 'Cancel', style: 'cancel' },
        { text: 'OK', onPress: () => console.log('[IOneXa-Fixture] Native alert acknowledged') }
      ]
    );
  };

  const testNetwork = async () => {
    setNetworkStatus('Requesting...');
    try {
      const res = await fetch('https://httpbin.org/get');
      if (res.ok) {
        setNetworkStatus('HTTP 200 OK (Remote network active)');
      } else {
        setNetworkStatus(`HTTP Error: ${res.status}`);
      }
    } catch (err: any) {
      setNetworkStatus(`Network Failure: ${err.message}`);
    }
  };

  if (errorSimulated) {
    throw new Error('IOneXa Controlled Negative Test: Intentional crash triggered by user.');
  }

  return (
    <SafeAreaView style={styles.safeArea}>
      <StatusBar barStyle="dark-content" />
      <ScrollView contentContainerStyle={styles.container}>
        {/* Header and Identity */}
        <View style={styles.headerBox}>
          <Text style={styles.title}>IOneXa Native iOS Verification Fixture</Text>
          <Text style={styles.subtitle}>Native Proof-of-Execution Gate</Text>
        </View>

        {/* Platform & Runtime Verification Badge */}
        <View style={[styles.badge, { borderColor: PlatformIndicator.color }]}>
          <Text style={styles.badgeLabel}>Detected Platform.OS:</Text>
          <Text style={styles.badgeValue}>{Platform.OS.toUpperCase()}</Text>
          <Text style={styles.badgeLabel}>Resolved Runtime Module:</Text>
          <Text style={[styles.badgeValue, { color: PlatformIndicator.color }]}>
            {PlatformIndicator.runtime}
          </Text>
          <Text style={styles.markerText}>Marker: {PlatformIndicator.marker}</Text>
        </View>

        {/* Screen 1: Home View */}
        {currentScreen === 'home' ? (
          <View style={styles.card}>
            <Text style={styles.sectionHeader}>Screen 1: Primary Controls</Text>

            {/* Interactive Pressable Counter */}
            <View style={styles.row}>
              <Text style={styles.counterText}>Taps: {counter}</Text>
              <Pressable
                style={({ pressed }) => [styles.btnPrimary, pressed && styles.btnPressed]}
                onPress={() => setCounter(c => c + 1)}
                testID="btn-increment"
              >
                <Text style={styles.btnText}>+ Increment</Text>
              </Pressable>
            </View>

            {/* Native Text Input (Opens genuine iOS Keyboard) */}
            <View style={styles.inputGroup}>
              <Text style={styles.inputLabel}>Native iOS Keyboard Test:</Text>
              <TextInput
                style={styles.textInput}
                placeholder="Tap here to open iOS keyboard..."
                placeholderTextColor="#8E8E93"
                value={inputText}
                onChangeText={setInputText}
                autoCorrect={false}
                testID="input-test"
              />
              <Text style={styles.inputFeedback}>
                Observed Text: {inputText ? `"${inputText}"` : '(empty)'}
              </Text>
            </View>

            {/* Native UIKit Alert Trigger */}
            <Pressable
              style={({ pressed }) => [styles.btnSecondary, pressed && styles.btnPressed]}
              onPress={triggerNativeAlert}
              testID="btn-alert"
            >
              <Text style={styles.btnSecondaryText}>Trigger UIKit Alert.alert()</Text>
            </Pressable>

            {/* Navigation Button */}
            <Pressable
              style={({ pressed }) => [styles.btnNavigate, pressed && styles.btnPressed]}
              onPress={() => setCurrentScreen('details')}
              testID="btn-nav-to-details"
            >
              <Text style={styles.btnText}>Navigate to Screen 2 &rarr;</Text>
            </Pressable>
          </View>
        ) : (
          /* Screen 2: Details & Diagnostics */
          <View style={styles.card}>
            <Text style={styles.sectionHeader}>Screen 2: Persistent State & Network</Text>
            <Text style={styles.persistentNotice}>
              Persistent Counter Value from Screen 1: <Text style={styles.bold}>{counter}</Text>
            </Text>

            {/* Network Verification */}
            <View style={styles.networkBox}>
              <Text style={styles.networkLabel}>Network Probe:</Text>
              <Text style={styles.networkStatus}>{networkStatus}</Text>
              <Pressable
                style={({ pressed }) => [styles.btnPrimary, pressed && styles.btnPressed]}
                onPress={testNetwork}
              >
                <Text style={styles.btnText}>Send HTTP Request</Text>
              </Pressable>
            </View>

            {/* Controlled Negative Crash Test */}
            <Pressable
              style={({ pressed }) => [styles.btnDanger, pressed && styles.btnPressed]}
              onPress={() => setErrorSimulated(true)}
            >
              <Text style={styles.btnDangerText}>Trigger Negative Crash Test</Text>
            </Pressable>

            {/* Back Navigation */}
            <Pressable
              style={({ pressed }) => [styles.btnNavigate, pressed && styles.btnPressed]}
              onPress={() => setCurrentScreen('home')}
            >
              <Text style={styles.btnText}>&larr; Back to Screen 1</Text>
            </Pressable>
          </View>
        )}

        <Text style={styles.footer}>IOneXa Native iOS Test Bed &bull; Not Expo Web</Text>
      </ScrollView>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  safeArea: { flex: 1, backgroundColor: '#F2F2F7' },
  container: { padding: 20, alignItems: 'center' },
  headerBox: { marginBottom: 16, alignItems: 'center' },
  title: { fontSize: 20, fontWeight: '700', color: '#1C1C1E', textAlign: 'center' },
  subtitle: { fontSize: 13, color: '#8E8E93', marginTop: 4 },
  badge: {
    width: '100%',
    backgroundColor: '#FFFFFF',
    borderWidth: 2,
    borderRadius: 12,
    padding: 14,
    marginBottom: 16
  },
  badgeLabel: { fontSize: 11, fontWeight: '600', color: '#8E8E93', textTransform: 'uppercase' },
  badgeValue: { fontSize: 15, fontWeight: '700', color: '#1C1C1E', marginBottom: 8 },
  markerText: { fontSize: 12, fontWeight: '600', color: '#007AFF', fontFamily: Platform.OS === 'ios' ? 'Menlo' : 'monospace' },
  card: {
    width: '100%',
    backgroundColor: '#FFFFFF',
    borderRadius: 14,
    padding: 16,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 1 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
    elevation: 2,
    marginBottom: 16
  },
  sectionHeader: { fontSize: 16, fontWeight: '600', color: '#1C1C1E', marginBottom: 14 },
  row: { flexDirection: 'row', justifyContent: 'space-between', alignItems: 'center', marginBottom: 16 },
  counterText: { fontSize: 18, fontWeight: '600', color: '#1C1C1E' },
  btnPrimary: { backgroundColor: '#007AFF', paddingVertical: 10, paddingHorizontal: 16, borderRadius: 8 },
  btnSecondary: { backgroundColor: '#E5E5EA', paddingVertical: 12, borderRadius: 8, alignItems: 'center', marginBottom: 12 },
  btnSecondaryText: { color: '#007AFF', fontWeight: '600', fontSize: 15 },
  btnNavigate: { backgroundColor: '#5856D6', paddingVertical: 12, borderRadius: 8, alignItems: 'center' },
  btnDanger: { backgroundColor: '#FF3B3015', borderWidth: 1, borderColor: '#FF3B30', paddingVertical: 10, borderRadius: 8, alignItems: 'center', marginBottom: 12 },
  btnDangerText: { color: '#FF3B30', fontWeight: '600' },
  btnPressed: { opacity: 0.75 },
  btnText: { color: '#FFFFFF', fontWeight: '600', fontSize: 15 },
  inputGroup: { marginBottom: 16 },
  inputLabel: { fontSize: 13, fontWeight: '500', color: '#3A3A3C', marginBottom: 6 },
  textInput: {
    height: 44,
    borderWidth: 1,
    borderColor: '#C7C7CC',
    borderRadius: 8,
    paddingHorizontal: 12,
    fontSize: 15,
    backgroundColor: '#FAFAFA'
  },
  inputFeedback: { fontSize: 12, color: '#8E8E93', marginTop: 4 },
  persistentNotice: { fontSize: 14, color: '#3A3A3C', marginBottom: 14 },
  bold: { fontWeight: '700', color: '#007AFF' },
  networkBox: { backgroundColor: '#F2F2F7', padding: 12, borderRadius: 8, marginBottom: 14 },
  networkLabel: { fontSize: 12, color: '#8E8E93', marginBottom: 2 },
  networkStatus: { fontSize: 13, fontWeight: '500', color: '#1C1C1E', marginBottom: 8 },
  footer: { fontSize: 11, color: '#C7C7CC', marginTop: 12 }
});
