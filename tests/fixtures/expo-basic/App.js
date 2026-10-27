import React, { useState } from 'react';
import { StatusBar } from 'expo-status-bar';
import { StyleSheet, Text, View, TextInput, TouchableOpacity, Image, ScrollView } from 'react-native';

export default function App() {
  const [counter, setCounter] = useState(0);
  const [text, setText] = useState('');
  const [submittedText, setSubmittedText] = useState('World');

  return (
    <ScrollView contentContainerStyle={styles.container}>
      <StatusBar style="auto" />
      <View style={styles.card}>
        <Image
          source={require('./assets/icon.png')}
          style={styles.logo}
          accessibilityLabel="IOneXa Logo"
        />
        <Text style={styles.title}>IOneXa Expo Fixture</Text>
        <Text style={styles.subtitle}>Expo Web Preview — Not an iOS Runtime</Text>

        {/* Counter Section */}
        <View style={styles.section}>
          <Text style={styles.sectionTitle}>Interactive State</Text>
          <Text style={styles.counterValue}>Clicks: {counter}</Text>
          <View style={styles.buttonRow}>
            <TouchableOpacity
              style={styles.button}
              onPress={() => setCounter((c) => c + 1)}
              testID="increment-button"
            >
              <Text style={styles.buttonText}>+ Increment</Text>
            </TouchableOpacity>
            <TouchableOpacity
              style={[styles.button, styles.secondaryButton]}
              onPress={() => setCounter(0)}
              testID="reset-button"
            >
              <Text style={[styles.buttonText, styles.secondaryButtonText]}>Reset</Text>
            </TouchableOpacity>
          </View>
        </View>

        {/* Form Section */}
        <View style={styles.section}>
          <Text style={styles.sectionTitle}>Form Input</Text>
          <Text style={styles.greeting}>Hello, {submittedText || 'there'}!</Text>
          <TextInput
            style={styles.input}
            placeholder="Type your name..."
            placeholderTextColor="#888"
            value={text}
            onChangeText={setText}
            testID="name-input"
          />
          <TouchableOpacity
            style={styles.button}
            onPress={() => {
              setSubmittedText(text);
              setText('');
            }}
            testID="submit-button"
          >
            <Text style={styles.buttonText}>Update Greeting</Text>
          </TouchableOpacity>
        </View>

        {/* Runtime Diagnostics Info */}
        <View style={styles.infoBox}>
          <Text style={styles.infoText}>Platform: Web (Metro Bundler)</Text>
          <Text style={styles.infoText}>Build: v0.1 Feasibility Spike</Text>
          <Text style={styles.infoText}>Status: Live Interactive</Text>
        </View>
      </View>
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  container: {
    flexGrow: 1,
    backgroundColor: '#0f172a',
    alignItems: 'center',
    justifyContent: 'center',
    padding: 20,
  },
  card: {
    width: '100%',
    maxWidth: 400,
    backgroundColor: '#1e293b',
    borderRadius: 16,
    padding: 24,
    alignItems: 'center',
    borderWidth: 1,
    borderColor: '#334155',
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 4 },
    shadowOpacity: 0.3,
    shadowRadius: 8,
  },
  logo: {
    width: 64,
    height: 64,
    borderRadius: 12,
    marginBottom: 12,
  },
  title: {
    fontSize: 22,
    fontWeight: '700',
    color: '#f8fafc',
    marginBottom: 4,
  },
  subtitle: {
    fontSize: 12,
    color: '#94a3b8',
    marginBottom: 20,
    fontStyle: 'italic',
  },
  section: {
    width: '100%',
    backgroundColor: '#0f172a',
    borderRadius: 12,
    padding: 16,
    marginBottom: 16,
    borderWidth: 1,
    borderColor: '#1e293b',
  },
  sectionTitle: {
    fontSize: 14,
    fontWeight: '600',
    color: '#38bdf8',
    marginBottom: 8,
    textTransform: 'uppercase',
    letterSpacing: 0.5,
  },
  counterValue: {
    fontSize: 20,
    fontWeight: '700',
    color: '#f8fafc',
    marginBottom: 12,
    textAlign: 'center',
  },
  buttonRow: {
    flexDirection: 'row',
    justifyContent: 'center',
    gap: 12,
  },
  button: {
    backgroundColor: '#3b82f6',
    paddingVertical: 10,
    paddingHorizontal: 16,
    borderRadius: 8,
    alignItems: 'center',
  },
  secondaryButton: {
    backgroundColor: '#334155',
  },
  buttonText: {
    color: '#ffffff',
    fontWeight: '600',
    fontSize: 14,
  },
  secondaryButtonText: {
    color: '#cbd5e1',
  },
  greeting: {
    fontSize: 16,
    fontWeight: '500',
    color: '#e2e8f0',
    marginBottom: 8,
    textAlign: 'center',
  },
  input: {
    backgroundColor: '#1e293b',
    borderWidth: 1,
    borderColor: '#475569',
    borderRadius: 8,
    padding: 10,
    color: '#f8fafc',
    fontSize: 14,
    marginBottom: 12,
  },
  infoBox: {
    width: '100%',
    padding: 12,
    backgroundColor: '#020617',
    borderRadius: 8,
    borderLeftWidth: 3,
    borderLeftColor: '#38bdf8',
  },
  infoText: {
    fontSize: 11,
    color: '#64748b',
    lineHeight: 18,
  },
});
