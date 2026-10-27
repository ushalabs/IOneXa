import React, { useState, useEffect } from 'react';
import { View, Text, TextInput, TouchableOpacity, StyleSheet } from 'react-native';
import { useRouter } from 'expo-router';

// Simple web-compatible storage helper with memory fallback
const StorageHelper = {
  getItem: (key) => {
    try {
      if (typeof window !== 'undefined' && window.localStorage) {
        return window.localStorage.getItem(key);
      }
    } catch (e) {
      // Fallback
    }
    return null;
  },
  setItem: (key, value) => {
    try {
      if (typeof window !== 'undefined' && window.localStorage) {
        window.localStorage.setItem(key, value);
      }
    } catch (e) {
      // Fallback
    }
  },
  removeItem: (key) => {
    try {
      if (typeof window !== 'undefined' && window.localStorage) {
        window.localStorage.removeItem(key);
      }
    } catch (e) {
      // Fallback
    }
  },
};

export default function StorageScreen() {
  const [keyInput, setKeyInput] = useState('user_pref');
  const [valInput, setValInput] = useState('');
  const [storedVal, setStoredVal] = useState('');
  const [status, setStatus] = useState('');
  const router = useRouter();

  useEffect(() => {
    const val = StorageHelper.getItem(keyInput);
    if (val) setStoredVal(val);
  }, [keyInput]);

  const handleSave = () => {
    StorageHelper.setItem(keyInput, valInput);
    setStoredVal(valInput);
    setStatus('Saved key "' + keyInput + '" successfully');
    setValInput('');
  };

  const handleClear = () => {
    StorageHelper.removeItem(keyInput);
    setStoredVal('');
    setStatus('Cleared key "' + keyInput + '"');
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>Local Persistence Test</Text>

      {status ? <Text style={styles.statusText} testID="storage-status">{status}</Text> : null}

      <View style={styles.displayCard}>
        <Text style={styles.cardLabel}>Stored Value for [{keyInput}]:</Text>
        <Text style={styles.cardValue} testID="stored-value">
          {storedVal || '<empty>'}
        </Text>
      </View>

      <TextInput
        style={styles.input}
        placeholder="Key"
        placeholderTextColor="#64748b"
        value={keyInput}
        onChangeText={setKeyInput}
        testID="input-key"
      />

      <TextInput
        style={styles.input}
        placeholder="Value to persist"
        placeholderTextColor="#64748b"
        value={valInput}
        onChangeText={setValInput}
        testID="input-value"
      />

      <View style={styles.btnRow}>
        <TouchableOpacity style={[styles.btn, styles.saveBtn]} onPress={handleSave} testID="btn-save">
          <Text style={styles.btnText}>Persist</Text>
        </TouchableOpacity>
        <TouchableOpacity style={[styles.btn, styles.clearBtn]} onPress={handleClear} testID="btn-clear">
          <Text style={styles.btnText}>Remove</Text>
        </TouchableOpacity>
      </View>

      <TouchableOpacity style={styles.linkBtn} onPress={() => router.back()}>
        <Text style={styles.linkText}>&larr; Back to Home</Text>
      </TouchableOpacity>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#0f172a',
    padding: 24,
    justifyContent: 'center',
  },
  title: {
    fontSize: 22,
    fontWeight: 'bold',
    color: '#f8fafc',
    marginBottom: 20,
    textAlign: 'center',
  },
  statusText: {
    color: '#38bdf8',
    marginBottom: 12,
    textAlign: 'center',
  },
  displayCard: {
    backgroundColor: '#1e293b',
    padding: 16,
    borderRadius: 8,
    marginBottom: 16,
    borderWidth: 1,
    borderColor: '#334155',
  },
  cardLabel: {
    color: '#94a3b8',
    fontSize: 13,
    marginBottom: 6,
  },
  cardValue: {
    color: '#f8fafc',
    fontSize: 16,
    fontWeight: '600',
  },
  input: {
    backgroundColor: '#1e293b',
    borderWidth: 1,
    borderColor: '#334155',
    borderRadius: 8,
    padding: 12,
    color: '#ffffff',
    fontSize: 15,
    marginBottom: 14,
  },
  btnRow: {
    flexDirection: 'row',
    gap: 12,
    marginBottom: 16,
  },
  btn: {
    flex: 1,
    paddingVertical: 12,
    borderRadius: 8,
    alignItems: 'center',
  },
  saveBtn: {
    backgroundColor: '#3b82f6',
  },
  clearBtn: {
    backgroundColor: '#ef4444',
  },
  btnText: {
    color: '#ffffff',
    fontWeight: '600',
    fontSize: 15,
  },
  linkBtn: {
    alignItems: 'center',
    paddingVertical: 8,
  },
  linkText: {
    color: '#94a3b8',
    fontSize: 14,
  },
});
