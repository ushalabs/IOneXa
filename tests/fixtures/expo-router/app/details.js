import React from 'react';
import { View, Text, StyleSheet, Image, TouchableOpacity } from 'react-native';
import { useLocalSearchParams, useRouter } from 'expo-router';

export default function Details() {
  const params = useLocalSearchParams();
  const router = useRouter();

  return (
    <View style={styles.container}>
      <Image
        source={require('../assets/splash-icon.png')}
        style={styles.splash}
        resizeMode="contain"
      />
      <Text style={styles.header}>Details Screen</Text>
      <Text style={styles.paramLabel}>
        Param: <Text style={styles.paramValue}>{params.item || 'Default'}</Text>
      </Text>

      <TouchableOpacity
        style={styles.backButton}
        onPress={() => router.back()}
        testID="details-back-btn"
      >
        <Text style={styles.backText}>&larr; Back to Home</Text>
      </TouchableOpacity>
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
  splash: {
    width: 100,
    height: 100,
    marginBottom: 20,
  },
  header: {
    fontSize: 20,
    fontWeight: '700',
    color: '#f8fafc',
    marginBottom: 12,
  },
  paramLabel: {
    fontSize: 15,
    color: '#94a3b8',
    marginBottom: 24,
  },
  paramValue: {
    color: '#38bdf8',
    fontWeight: '600',
  },
  backButton: {
    backgroundColor: '#334155',
    paddingVertical: 10,
    paddingHorizontal: 20,
    borderRadius: 8,
  },
  backText: {
    color: '#e2e8f0',
    fontWeight: '500',
  },
});
