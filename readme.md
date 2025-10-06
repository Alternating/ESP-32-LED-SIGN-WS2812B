# WiFi Controlled LED Matrix Sign System

A WiFi-controlled 70×70 inch LED matrix display system for creating large-format road signs and displays. Control letters, numbers, symbols, and scrolling messages from your phone or computer.

![LED Sign Control](https://img.shields.io/badge/Platform-ESP32-blue)
![Status](https://img.shields.io/badge/Status-Active-success)
![License](https://img.shields.io/badge/License-MIT-green)

## 📋 Table of Contents

- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Wiring Guide](#wiring-guide)
- [Software Setup](#software-setup)
- [Usage](#usage)
- [Web Interface](#web-interface)
- [Future Upgrades](#future-upgrades)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## ✨ Features

### Current Features (v1.0)

- **WiFi Access Point Mode** - Connect directly to the sign from any device
- **Web-Based Control** - Mobile-friendly interface at `http://ledsign.local`
- **Two Display Modes:**
  - **Single Character** - Display any letter (A-Z), number (0-9), or symbol
  - **Message Mode** - Scrolling messages like "I ❤ U" with adjustable speed
- **Full Color Control** - RGB color picker for unlimited colors
- **Brightness Control** - Adjustable from 10-255
- **Visual Effects:**
  - Static - Normal display
  - Blink - On/off with adjustable speed
  - Fade - Smooth fade in/out
  - Color Cycle - Rainbow effect
- **10 Built-in Symbols:**
  - ❤ Heart
  - ★ Star
  - ✓ Check
  - ✗ Cross
  - → ← ↑ ↓ Arrows
  - ◆ Diamond
  - ● Circle
- **mDNS Support** - Easy access via `http://ledsign.local` (no IP address needed)

## 🔧 Hardware Requirements

### Single Sign Setup (Current)

| Component | Specification | Quantity |
|-----------|--------------|----------|
| ESP32 Development Board | ESP32 DevKit or similar | 1 |
| WS2812B LED Strip/Pixels | Individual addressable LEDs | 100 |
| Power Supply | 5V 10A (minimum) | 1 |
| MDF/Plywood Board | 70" × 70" (1778mm × 1778mm) | 1 |
| Wire | 18 AWG (power), 22 AWG (data) | As needed |
| Mounting Hardware | Hot glue, clips, or brackets | - |

### LED Matrix Specifications

- **Grid Size:** 10×10 (100 LEDs total)
- **Physical Size:** 70" × 70"
- **LED Spacing:** 7" between centers
- **Wiring Pattern:** Serpentine (zigzag rows)
- **Total Power Draw:** ~4A at full white brightness

## 🔌 Wiring Guide

### Basic Connections

```
ESP32 Pin 5 (GPIO5) ──────► LED Data In (First LED)

5V Power Supply (+) ──────► LED Strip VCC (Multiple injection points)
5V Power Supply (-) ──────► LED Strip GND + ESP32 GND (Common ground!)
```

### Power Injection Points

For optimal performance, inject 5V power at these locations:
- **Row 1** (Top) - Primary power input
- **Row 4** (Middle-top) - Prevents voltage drop
- **Row 7** (Middle-bottom) - Ensures consistent brightness
- **Row 10** (Bottom) - Optional, recommended for full brightness

### Serpentine Wiring Pattern

```
LED Numbers (Data flow):
0 ─> 1 ─> 2 ─> 3 ─> 4 ─> 5 ─> 6 ─> 7 ─> 8 ─> 9      Row 0
19 <─ 18 <─ 17 <─ 16 <─ 15 <─ 14 <─ 13 <─ 12 <─ 11 <─ 10     Row 1
20 ─> 21 ─> 22 ─> 23 ─> 24 ─> 25 ─> 26 ─> 27 ─> 28 ─> 29     Row 2
...and so on
```

## 💻 Software Setup

### Required Libraries

Install via Arduino IDE Library Manager:

1. **FastLED** by Daniel Garcia
   - Go to `Tools > Manage Libraries`
   - Search for "FastLED"
   - Click Install

### ESP32 Board Support

If not already installed:

1. Go to `File > Preferences`
2. Add to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to `Tools > Board > Boards Manager`
4. Search "ESP32" and install "ESP32 by Espressif Systems"

### Upload Instructions

1. **Open** `LED_Sign_WiFi.ino` in Arduino IDE
2. **Place** `character_patterns.h` in the same folder
3. **Select Board:** `Tools > Board > ESP32 Dev Module`
4. **Select Port:** `Tools > Port > [Your ESP32 COM Port]`
5. **Upload:** Click the Upload button
6. **Monitor:** Open Serial Monitor (115200 baud) to see connection info

### WiFi Configuration

Default settings in code:

```cpp
const char* ssid = "LED_SIGN_AP";        // WiFi network name
const char* password = "sign1234";       // Password (8+ characters)
```

To customize, edit these values before uploading.

## 🚀 Usage

### First Time Setup

1. **Power on** the ESP32
2. **Connect** to WiFi network `LED_SIGN_AP` (password: `sign1234`)
3. **Open browser** to `http://ledsign.local` (or IP shown in Serial Monitor)
4. **Start controlling** your sign!

### Single Character Mode

1. Click **"Single Character"** tab
2. Type any letter/number or click a symbol button
3. Adjust **color**, **brightness**, and **effect**
4. Click **"✨ Update Sign"**

### Message Mode

1. Click **"Message"** tab
2. Type your message (e.g., "HELLO")
3. Click symbol buttons to insert symbols
4. Adjust **Display Speed** slider (how long each character shows)
5. Set **color**, **brightness**, and **effect**
6. Click **"✨ Update Sign"**

The message will cycle through each character automatically!

## 🌐 Web Interface

### Control Panel Sections

#### Character/Symbol Selection
- Text input for typing characters
- 10 quick-select symbol buttons
- Real-time preview display

#### Color Control
- Full RGB color picker
- Supports any color combination

#### Brightness Control
- Slider: 10-255
- Live value display
- Lower brightness = longer battery life

#### Effects
- **Static** - Normal display
- **Blink** - Toggles on/off (speed: 100-2000ms)
- **Fade** - Smooth fade in/out (speed: 5-50ms)
- **Color Cycle** - Rainbow rotation (speed: 10-200ms)

Each effect has its own speed control that appears when selected.

### Accessing the Interface

**Primary Method:** `http://ledsign.local`

**Backup Method:** Direct IP address (shown in Serial Monitor)
- Usually `http://192.168.4.1`

## 🔮 Future Upgrades

### Multi-Sign System (v2.0 - Planned)

Expand to **11 independent signs** controlled by a single ESP32, allowing you to display full words and phrases.

#### Planned Features

- **11 GPIO Pins** - Each controlling one 10×10 LED sign
- **Word Mode** - Type entire words (e.g., "GRAND OPENING")
- **Individual Control** - Set each sign separately or all at once
- **Synchronized Effects** - All signs can share the same effect
- **Memory Presets** - Save common words/phrases
- **API Control** - RESTful API for integration with other systems

#### Hardware Requirements (Multi-Sign)

| Component | Quantity |
|-----------|----------|
| ESP32 Board | 1 |
| 10×10 LED Signs | 11 |
| Total LEDs | 1,100 |
| Power Supply | 5V 60-100A |
| GPIO Pins Used | 11 |

#### Recommended GPIO Pins for 11 Signs

```cpp
const int ledPins[11] = {
  5, 18, 19, 21, 22, 23,   // First 6 signs
  25, 26, 27, 32, 33       // Next 5 signs
};
```

#### Example: "GRAND OPENING"

```
Sign 1: G    Sign 5: D    Sign 9:  I
Sign 2: R    Sign 6:      Sign 10: N
Sign 3: A    Sign 7: O    Sign 11: G
Sign 4: N    Sign 8: P
```

#### Power Distribution

With 11 signs (1,100 LEDs):
- **Peak Power:** 66A at full white brightness
- **Typical Usage:** 30-40A with colors and effects
- **Recommended:** 5V 80A power supply with proper bus bars
- **Power Injection:** Every sign needs its own power connection

### Other Future Enhancements

- [ ] Mobile app (iOS/Android)
- [ ] DMX512 control support
- [ ] Scheduling (time-based display changes)
- [ ] Animation editor
- [ ] Weather integration
- [ ] MQTT support for IoT integration
- [ ] Battery backup system
- [ ] Outdoor weatherproof enclosure design
- [ ] Custom character/logo creator

## 🔧 Troubleshooting

### LEDs Not Lighting

**Check:**
- Power supply is 5V and sufficient amperage (10A minimum)
- Common ground between ESP32 and power supply
- Data wire connected to GPIO 5
- Power injection points are connected

**Try:**
- Reduce brightness to 50 to test
- Upload the chasing light test code
- Check for loose connections

### Can't Access Web Interface

**`http://ledsign.local` not working:**
- Use direct IP address (check Serial Monitor)
- Ensure you're connected to `LED_SIGN_AP` WiFi
- Try rebooting the ESP32
- Clear browser cache

**Wrong WiFi password:**
- Default is `sign1234` (8 characters minimum)
- Check Serial Monitor for correct SSID

### Symbols Not Displaying Correctly

**Issue:** Symbols show as blank or wrong characters

**Solution:**
- Use the quick-select buttons instead of typing
- If typing, use the exact UTF-8 symbols: ❤ ★ ✓ ✗ → ← ↑ ↓ ◆ ●
- Ensure `character_patterns.h` is in the same folder

### Voltage Drop / Dim LEDs

**Symptoms:** Last rows dimmer than first rows

**Solution:**
- Add more power injection points
- Use thicker wire (18 AWG minimum for power)
- Check all ground connections
- Reduce overall brightness

### Message Mode Not Cycling

**Check:**
- Message mode tab is selected (should be highlighted blue)
- Message has been entered
- "Update Sign" button clicked
- Check Serial Monitor for error messages

## 📝 Configuration Options

### Changing WiFi Settings

Edit in code before uploading:

```cpp
const char* ssid = "YOUR_NETWORK_NAME";
const char* password = "YOUR_PASSWORD";
```

### Changing Default Display

```cpp
char currentChar = 'A';                  // Starting character
CRGB currentColor = CRGB::White;         // Starting color
uint8_t currentBrightness = 200;         // Starting brightness (10-255)
String currentEffect = "static";         // Starting effect
```

### Adjusting LED Pin

```cpp
#define LED_PIN 5  // Change to your preferred GPIO
```

### Matrix Size

Current system is designed for 10×10. To change:

```cpp
#define MATRIX_WIDTH  10
#define MATRIX_HEIGHT 10
#define NUM_LEDS      100  // WIDTH × HEIGHT
```

**Note:** Character patterns are designed for 10×10 and will need updating for different sizes.

## 🤝 Contributing

Contributions welcome! Areas for improvement:

- Additional character patterns
- New visual effects
- UI/UX enhancements
- Documentation improvements
- Testing and bug reports

Please submit pull requests or open issues on GitHub.

## 📄 License

This project is licensed under the MIT License - see LICENSE file for details.

## 🙏 Acknowledgments

- **FastLED Library** by Daniel Garcia
- **ESP32 Arduino Core** by Espressif Systems
- Inspired by large-format LED signage for commercial applications

## 📞 Support

For questions, issues, or feature requests:
- Open an issue on GitHub
- Check existing issues for solutions
- Refer to troubleshooting section above

---

**Current Version:** 1.0.0  
**Last Updated:** 2025  
**Hardware Platform:** ESP32  
**Display Size:** 10×10 LED Matrix (70" × 70")

**Made for creating eye-catching road signs and displays** 🚦✨