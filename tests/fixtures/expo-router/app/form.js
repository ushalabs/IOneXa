import React, { useState } from 'react';
import { View, Text, TextInput, TouchableOpacity, StyleSheet, Switch } from 'react-native';
import { useRouter } from 'expo-router';

export default function FormScreen() {
  const [name, setName] = useState('');
  const [email, setEmail] = useState('');
  const [agree, setAgree] = useState(false);
  const [error, setError] = useState('');
  const [success, setSuccess] = useState(false);
  const router = useRouter();

  const handleSubmit = () => {
    if (!name.trim()) {
      setError('Name is required');
      return;
    }
    if (!email.includes('@')) {
      setError('Valid email is required');
      return;
    }
    setError('');
    setSuccess(true);
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>Form Validation Test</Text>

      {error ? <Text style={styles.errorText} testID="form-error">{error}</Text> : null}
      {success ? <Text style={styles.successText} testID="form-success">Submission successful!</Text> : null}

      <TextInput
        style={styles.input}
        placeholder="Full Name"
        placeholderTextColor="#64748b"
        value={name}
        onChangeText={setName}
        testID="input-name"
      />

      <TextInput
        style={styles.input}
        placeholder="Email Address"
        placeholderTextColor="#64748b"
        value={email}
        onChangeText={setEmail}
        keyboardType="email-address"
        autoCapitalize="none"
        testID="input-email"
      />

      <View style={styles.switchRow}>
        <Text style={styles.label}>Accept Terms</Text>
        <Switch
          value={agree}
          onValueChange={setAgree}
          trackColor={{ false: '#334155', true: '#3b82f6' }}
          thumbColor={agree ? '#ffffff' : '#94a3b8'}
          testID="switch-terms"
        />
      </View>

      <TouchableOpacity style={styles.submitBtn} onPress={handleSubmit} testID="btn-submit">
        <Text style={styles.btnText}>Submit Form</Text>
      </TouchableOpacity>

      <TouchableOpacity style={styles.linkBtn} onPress={() => router.back()}>
        <Text style={styles.linkText}>&larr; Back</Text>
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
  errorText: {
    color: '#ef4444',
    marginBottom: 12,
    textAlign: 'center',
    fontWeight: '500',
  },
  successText: {
    color: '#22c55e',
    marginBottom: 12,
    textAlign: 'center',
    fontWeight: '500',
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
  switchRow: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    marginBottom: 20,
    paddingHorizontal: 4,
  },
  label: {
    color: '#cbd5e1',
    fontSize: 15,
  },
  submitBtn: {
    backgroundColor: '#3b82f6',
    paddingVertical: 14,
    borderRadius: 8,
    alignItems: 'center',
    marginBottom: 12,
  },
  btnText: {
    color: '#ffffff',
    fontWeight: '600',
    fontSize: 16,
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
