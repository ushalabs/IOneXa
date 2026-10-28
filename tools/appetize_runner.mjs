/**
 * Appetize.io Provider Runner & Simulator Bridge
 * Part of IOneXa Native iOS Testing Architecture
 */

import fs from 'fs';
import path from 'path';

export class AppetizeProvider {
  constructor(apiToken = process.env.APPETIZE_API_TOKEN) {
    this.apiToken = apiToken;
    this.baseUrl = 'https://api.appetize.io/v1';
  }

  async checkAvailability() {
    try {
      const res = await fetch(`${this.baseUrl}/apps`, {
        method: 'GET',
        headers: this.apiToken ? { 'X-API-KEY': this.apiToken } : {}
      });
      if (res.status === 200) {
        const apps = await res.json();
        return { available: true, authenticated: true, appCount: apps.data?.length || 0 };
      } else if (res.status === 401) {
        return { available: true, authenticated: false, message: 'Appetize API reachable; API token required for programmatic uploads.' };
      }
      return { available: false, status: res.status };
    } catch (err) {
      return { available: false, error: err.message };
    }
  }

  async uploadApp(archivePath, platform = 'ios', note = 'IOneXa Native iOS Build') {
    if (!this.apiToken) {
      throw new Error('APPETIZE_API_TOKEN is required for direct API upload.');
    }
    if (!fs.existsSync(archivePath)) {
      throw new Error(`Archive not found at: ${archivePath}`);
    }

    const fileBuffer = fs.readFileSync(archivePath);
    const blob = new Blob([fileBuffer]);
    const formData = new FormData();
    formData.append('file', blob, path.basename(archivePath));
    formData.append('platform', platform);
    formData.append('note', note);

    const res = await fetch(`${this.baseUrl}/apps`, {
      method: 'POST',
      headers: {
        'X-API-KEY': this.apiToken
      },
      body: formData
    });

    if (!res.ok) {
      const errText = await res.text();
      throw new Error(`Upload failed (${res.status}): ${errText}`);
    }

    return await res.json();
  }

  getEmbedUrl(publicKeyOrBuildId, options = {}) {
    const {
      device = 'iphone15pro',
      osVersion = '17.0',
      scale = 75,
      autoplay = true,
      appearance = 'light'
    } = options;

    const params = new URLSearchParams({
      device,
      osVersion,
      scale: String(scale),
      autoplay: String(autoplay),
      appearance
    });

    return `https://appetize.io/embed/${publicKeyOrBuildId}?${params.toString()}`;
  }
}

// Standalone CLI execution
if (process.argv[1] && import.meta.url.endsWith(process.argv[1].replace(/\\/g, '/'))) {
  const provider = new AppetizeProvider();
  provider.checkAvailability().then(res => {
    console.log('[AppetizeProvider Status]', JSON.stringify(res, null, 2));
  });
}
