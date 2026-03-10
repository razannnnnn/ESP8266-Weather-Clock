# 🕐 ESP8266 Weather Clock

Jam digital berbasis ESP8266 dengan tampilan OLED 128x64, informasi cuaca real-time dari OpenWeatherMap, dan animasi slide antar halaman.

![Platform](https://img.shields.io/badge/Platform-ESP8266-blue)
![Display](https://img.shields.io/badge/Display-OLED%20128x64-green)
![License](https://img.shields.io/badge/License-MIT-yellow)

---

## ✨ Fitur

- 🕐 **Jam digital** dengan tanggal dan hari dalam Bahasa Indonesia
- 🌤️ **Cuaca real-time** dari OpenWeatherMap (suhu, deskripsi, forecast)
- 📅 **Forecast 3 hari** ke depan dengan ikon cuaca
- 📶 **WiFiManager** — setup WiFi via hotspot tanpa hardcode SSID/password
- 🔄 **Animasi slide** antar halaman otomatis (powered by OLEDDisplayUi)
- 🔁 **Auto update cuaca** setiap 20 menit
- 🌏 **Timezone WIB (GMT+7)** dan bahasa Indonesia

---

## 🛠️ Hardware

| Komponen | Keterangan |
|---|---|
| ESP8266 | NodeMCU v1.0 / ESP-12E |
| OLED Display | 128x64, I2C, SSD1306 |
| Kabel | SDA → D2, SCL → D1 |
| Power | USB / 3.3V |

### Wiring

```
ESP8266        OLED
-------        ----
D1      →      SCL
D2      →      SDA
3.3V    →      VCC
GND     →      GND
```

---

## 📦 Library yang Dibutuhkan

Install semua via **Arduino Library Manager**:

| Library | Author |
|---|---|
| ESP8266 OLED Driver for SSD1306 | ThingPulse |
| ESP8266 Weather Station | ThingPulse |
| Json Streaming Parser | ThingPulse |
| WiFiManager | tzapu |

> **Board:** ESP8266 by ESP8266 Community (via Board Manager)
> URL: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`

---

## ⚙️ Konfigurasi

Buka file `.ino` dan sesuaikan bagian settings:

```cpp
// API Key OpenWeatherMap
String OPEN_WEATHER_MAP_APP_ID = "YOUR_API_KEY_HERE";

// Koordinat lokasi (contoh: Kota Blitar)
float OPEN_WEATHER_MAP_LOCATION_LAT = -8.0955;
float OPEN_WEATHER_MAP_LOCATION_LON = 112.1608;

// Bahasa deskripsi cuaca
String OPEN_WEATHER_MAP_LANGUAGE = "id";

// Interval update cuaca (detik)
const int UPDATE_INTERVAL_SECS = 20 * 60; // 20 menit
```

### Cara Mendapatkan API Key OpenWeatherMap

1. Daftar gratis di [openweathermap.org](https://openweathermap.org)
2. Login → klik nama profil → **My API Keys**
3. Copy API key default atau buat baru
4. Tunggu ±10-30 menit hingga aktif

---

## 🚀 Cara Upload

1. Install semua library yang dibutuhkan
2. Buka sketch di Arduino IDE
3. Pilih board **NodeMCU 1.0 (ESP-12E Module)**
4. Upload ke ESP8266
5. Saat pertama boot, ESP8266 akan membuat hotspot **`Clock-Setup`**
6. Connect ke hotspot tersebut dari HP/laptop
7. Buka browser → `192.168.4.1`
8. Pilih WiFi dan masukkan password
9. ESP8266 akan otomatis reboot dan terhubung

> ⏱️ Portal WiFi timeout setelah **3 menit** jika tidak ada input, lalu auto reboot.

---

## 📁 Struktur File

```
WeatherStationDemo/
└── WeatherStationDemo.ino   # Sketch utama
└── README.md                # Dokumentasi ini
```

---

## 🐛 Troubleshooting

| Masalah | Solusi |
|---|---|
| Layar terbalik | Pastikan `display.flipScreenVertically()` dipanggil setelah `ui.init()` |
| Cuaca tidak muncul | Cek API key dan koneksi internet |
| Jam salah | Pastikan `TZ 7` untuk WIB (GMT+7) |
| WiFi tidak tersimpan | Tekan reset lama untuk clear WiFi credentials |
| Library tidak ditemukan | Install semua library ThingPulse via Library Manager |

---

## 📄 Lisensi

MIT License — bebas digunakan dan dimodifikasi.

---

## 🙏 Credit

- **[ThingPulse](https://thingpulse.com)** — ESP8266 Weather Station & OLED library
- **[OpenWeatherMap](https://openweathermap.org)** — Data cuaca real-time
- **[tzapu](https://github.com/tzapu/WiFiManager)** — WiFiManager library
- **[Claude AI](https://claude.ai) by Anthropic** — Assisted in development, debugging, and code integration

---

*Made with ❤️ by **Razan***
