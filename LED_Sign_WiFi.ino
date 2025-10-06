/*
 * WiFi Controlled 10x10 LED Matrix Display
 * ESP32 Access Point Mode - Mobile Sign Control
 * 
 * Features:
 * - WiFi Access Point (connect directly to sign)
 * - Web interface for letter/number/symbol selection
 * - Color control
 * - Brightness control
 * - Effects: Static, Blink, Fade, Color Cycle with speed controls
 * - mDNS hostname: http://ledsign.local
 * 
 * REQUIRED LIBRARY: FastLED
 * Install via: Arduino IDE > Tools > Manage Libraries > Search "FastLED" > Install
 * 
 * SETUP:
 * 1. Upload this code to ESP32
 * 2. ESP32 will create WiFi network: "LED_SIGN_AP"
 * 3. Password: "sign1234"
 * 4. Connect to WiFi, open browser to: http://ledsign.local
 */

#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// ----- LED CONFIGURATION -----
#define LED_PIN       5
#define NUM_LEDS      100
#define MATRIX_WIDTH  10
#define MATRIX_HEIGHT 10
#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB

// ----- WIFI CONFIGURATION -----
const char* ssid = "LED_SIGN_AP";        // WiFi network name
const char* password = "sign1234";       // WiFi password (min 8 chars)

// ----- WEB SERVER -----
WebServer server(80);

// ----- LED ARRAY -----
CRGB leds[NUM_LEDS];

// ----- CURRENT SETTINGS -----
char currentChar = 'A';
CRGB currentColor = CRGB::White;
uint8_t currentBrightness = 200;
String currentEffect = "static";

// Message mode
String currentMessage = "";
bool messageMode = false;
int messageIndex = 0;
int messageSpeed = 1000;  // milliseconds per character
unsigned long lastMessageUpdate = 0;

// Effect timing and speeds
unsigned long lastEffectUpdate = 0;
uint8_t effectState = 0;
uint8_t hueOffset = 0;

// Effect speed settings (lower = faster)
int blinkSpeed = 500;      // milliseconds
int fadeSpeed = 20;        // milliseconds per step
int cycleSpeed = 50;       // milliseconds per hue step

// ----- CHARACTER PATTERNS (10x10) -----
#include "character_patterns.h"

// ----- SETUP -----
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n================================");
  Serial.println("WiFi LED Matrix Sign");
  Serial.println("================================\n");
  
  // Initialize LEDs
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(currentBrightness);
  FastLED.clear();
  FastLED.show();
  
  // Setup WiFi Access Point
  Serial.println("Starting Access Point...");
  WiFi.softAP(ssid, password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  // Setup mDNS responder
  if (MDNS.begin("ledsign")) {
    Serial.println("mDNS responder started");
    Serial.println("Access sign at: http://ledsign.local");
  } else {
    Serial.println("Error setting up mDNS responder!");
  }
  
  Serial.println("\nConnect to WiFi:");
  Serial.print("Network: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  Serial.println("\nThen open browser to:");
  Serial.println("  http://ledsign.local");
  Serial.print("  OR http://");
  Serial.println(IP);
  Serial.println("\n================================\n");
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/status", handleStatus);
  server.begin();
  
  // Display initial character
  displayCharacter(currentChar);
}

// ----- MAIN LOOP -----
void loop() {
  server.handleClient();
  
  // Handle message mode character cycling
  if (messageMode && currentMessage.length() > 0) {
    unsigned long currentTime = millis();
    if (currentTime - lastMessageUpdate > messageSpeed) {
      // Move to next character
      messageIndex++;
      if (messageIndex >= currentMessage.length()) {
        messageIndex = 0;
      }
      
      // Display current character in message
      currentChar = currentMessage.charAt(messageIndex);
      displayCharacter(currentChar);
      
      lastMessageUpdate = currentTime;
    }
  }
  
  updateEffect();
}

// ----- WEB PAGE -----
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>LED Sign Control</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Arial, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      padding: 20px;
      color: #333;
    }
    .container {
      max-width: 500px;
      margin: 0 auto;
      background: white;
      border-radius: 20px;
      padding: 30px;
      box-shadow: 0 20px 60px rgba(0,0,0,0.3);
    }
    h1 {
      text-align: center;
      color: #667eea;
      margin-bottom: 30px;
      font-size: 28px;
    }
    .section {
      margin-bottom: 25px;
      padding-bottom: 25px;
      border-bottom: 1px solid #eee;
    }
    .section:last-child { border-bottom: none; }
    label {
      display: block;
      font-weight: 600;
      margin-bottom: 10px;
      color: #555;
      font-size: 14px;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    input[type="text"], select {
      width: 100%;
      padding: 12px;
      border: 2px solid #e0e0e0;
      border-radius: 10px;
      font-size: 16px;
      transition: border 0.3s;
    }
    input[type="text"]:focus, select:focus {
      outline: none;
      border-color: #667eea;
    }
    input[type="color"] {
      width: 100%;
      height: 60px;
      border: none;
      border-radius: 10px;
      cursor: pointer;
    }
    input[type="range"] {
      width: 100%;
      height: 8px;
      border-radius: 5px;
      background: #e0e0e0;
      outline: none;
      -webkit-appearance: none;
    }
    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 24px;
      height: 24px;
      border-radius: 50%;
      background: #667eea;
      cursor: pointer;
    }
    .value-display {
      text-align: center;
      font-size: 20px;
      font-weight: bold;
      color: #667eea;
      margin-top: 10px;
    }
    .char-grid {
      display: grid;
      grid-template-columns: repeat(5, 1fr);
      gap: 10px;
      margin-top: 10px;
    }
    .char-btn {
      padding: 15px;
      border: 2px solid #e0e0e0;
      border-radius: 10px;
      background: white;
      font-size: 32px;
      cursor: pointer;
      transition: all 0.2s;
      display: flex;
      align-items: center;
      justify-content: center;
      min-height: 65px;
      color: #333;
    }
    .char-btn:hover {
      background: #667eea;
      color: white;
      transform: scale(1.05);
      border-color: #667eea;
    }
    .char-btn:active {
      transform: scale(0.95);
    }
    .mode-tabs {
      display: flex;
      gap: 10px;
      margin-bottom: 20px;
    }
    .mode-tab {
      flex: 1;
      padding: 12px;
      background: #f0f0f0;
      border: 2px solid #e0e0e0;
      border-radius: 10px;
      cursor: pointer;
      text-align: center;
      font-weight: 600;
      transition: all 0.3s;
      color: #666;
    }
    .mode-tab.active {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      border-color: #667eea;
    }
    .mode-content {
      display: none;
    }
    .mode-content.active {
      display: block;
    }
    button {
      width: 100%;
      padding: 15px;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      border: none;
      border-radius: 10px;
      font-size: 16px;
      font-weight: 600;
      cursor: pointer;
      transition: transform 0.2s;
      margin-top: 10px;
    }
    button:hover {
      transform: translateY(-2px);
      box-shadow: 0 5px 20px rgba(102, 126, 234, 0.4);
    }
    button:active {
      transform: translateY(0);
    }
    .status {
      text-align: center;
      padding: 15px;
      background: #f0f7ff;
      border-radius: 10px;
      margin-top: 20px;
      font-size: 14px;
      color: #667eea;
    }
    .current-display {
      text-align: center;
      font-size: 72px;
      font-weight: bold;
      padding: 20px;
      background: #f8f9fa;
      border-radius: 15px;
      margin-bottom: 20px;
      color: #667eea;
      min-height: 120px;
      display: flex;
      align-items: center;
      justify-content: center;
    }
    .speed-control {
      margin-top: 15px;
      padding: 15px;
      background: #f8f9fa;
      border-radius: 10px;
    }
    .speed-label {
      font-size: 12px;
      color: #666;
      margin-bottom: 8px;
      text-transform: none;
      font-weight: 500;
    }
    .hidden {
      display: none;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>🚦 LED Sign Control</h1>
    
    <div class="current-display" id="currentChar">A</div>
    
    <!-- Mode Selection -->
    <div class="mode-tabs">
      <div class="mode-tab active" onclick="switchMode('single')">Single Character</div>
      <div class="mode-tab" onclick="switchMode('message')">Message</div>
    </div>
    
    <!-- Single Character Mode -->
    <div id="singleMode" class="mode-content active">
      <div class="section">
        <label>Character / Symbol</label>
        <input type="text" id="charInput" maxlength="1" placeholder="Enter any character">
        <div class="char-grid">
          <button class="char-btn" onclick="setChar('❤')">❤</button>
          <button class="char-btn" onclick="setChar('★')">★</button>
          <button class="char-btn" onclick="setChar('✓')">✓</button>
          <button class="char-btn" onclick="setChar('✗')">✗</button>
          <button class="char-btn" onclick="setChar('→')">→</button>
          <button class="char-btn" onclick="setChar('←')">←</button>
          <button class="char-btn" onclick="setChar('↑')">↑</button>
          <button class="char-btn" onclick="setChar('↓')">↓</button>
          <button class="char-btn" onclick="setChar('◆')">◆</button>
          <button class="char-btn" onclick="setChar('●')">●</button>
        </div>
      </div>
    </div>
    
    <!-- Message Mode -->
    <div id="messageMode" class="mode-content">
      <div class="section">
        <label>Message (e.g., "I ❤ U")</label>
        <input type="text" id="messageInput" placeholder="Enter message..." maxlength="50">
        <div class="char-grid">
          <button class="char-btn" onclick="addToMessage('❤')">❤</button>
          <button class="char-btn" onclick="addToMessage('★')">★</button>
          <button class="char-btn" onclick="addToMessage('✓')">✓</button>
          <button class="char-btn" onclick="addToMessage('✗')">✗</button>
          <button class="char-btn" onclick="addToMessage('→')">→</button>
          <button class="char-btn" onclick="addToMessage('←')">←</button>
          <button class="char-btn" onclick="addToMessage('↑')">↑</button>
          <button class="char-btn" onclick="addToMessage('↓')">↓</button>
          <button class="char-btn" onclick="addToMessage('◆')">◆</button>
          <button class="char-btn" onclick="addToMessage('●')">●</button>
        </div>
        <div class="speed-control" style="margin-top: 15px;">
          <div class="speed-label">Display Speed per Character</div>
          <input type="range" id="messageSpeed" min="200" max="3000" value="1000" step="100" oninput="updateSpeedDisplay('message')">
          <div class="value-display" id="messageSpeedValue">1000ms</div>
        </div>
      </div>
    </div>
    
    <div class="section">
      <label>Color</label>
      <input type="color" id="colorPicker" value="#FFFFFF">
    </div>
    
    <div class="section">
      <label>Brightness</label>
      <input type="range" id="brightness" min="10" max="255" value="200" oninput="updateBrightnessDisplay()">
      <div class="value-display" id="brightnessValue">200</div>
    </div>
    
    <div class="section">
      <label>Effect</label>
      <select id="effect" onchange="updateEffectSpeedVisibility()">
        <option value="static">Static</option>
        <option value="blink">Blink</option>
        <option value="fade">Fade In/Out</option>
        <option value="cycle">Color Cycle</option>
      </select>
      
      <!-- Blink Speed Control -->
      <div id="blinkSpeedControl" class="speed-control hidden">
        <div class="speed-label">Blink Speed (slower ← → faster)</div>
        <input type="range" id="blinkSpeed" min="100" max="2000" value="500" step="50" oninput="updateSpeedDisplay('blink')">
        <div class="value-display" id="blinkSpeedValue">500ms</div>
      </div>
      
      <!-- Fade Speed Control -->
      <div id="fadeSpeedControl" class="speed-control hidden">
        <div class="speed-label">Fade Speed (slower ← → faster)</div>
        <input type="range" id="fadeSpeed" min="5" max="50" value="20" step="5" oninput="updateSpeedDisplay('fade')">
        <div class="value-display" id="fadeSpeedValue">20ms</div>
      </div>
      
      <!-- Cycle Speed Control -->
      <div id="cycleSpeedControl" class="speed-control hidden">
        <div class="speed-label">Cycle Speed (slower ← → faster)</div>
        <input type="range" id="cycleSpeed" min="10" max="200" value="50" step="10" oninput="updateSpeedDisplay('cycle')">
        <div class="value-display" id="cycleSpeedValue">50ms</div>
      </div>
    </div>
    
    <button onclick="updateSign()">✨ Update Sign</button>
    
    <div class="status" id="status">Ready</div>
  </div>

  <script>
    let currentMode = 'single';
    
    function switchMode(mode) {
      currentMode = mode;
      
      // Update tab styling
      document.querySelectorAll('.mode-tab').forEach(tab => {
        tab.classList.remove('active');
      });
      event.target.classList.add('active');
      
      // Show/hide mode content
      document.getElementById('singleMode').classList.remove('active');
      document.getElementById('messageMode').classList.remove('active');
      
      if (mode === 'single') {
        document.getElementById('singleMode').classList.add('active');
      } else {
        document.getElementById('messageMode').classList.add('active');
      }
    }
    
    function setChar(char) {
      document.getElementById('charInput').value = char;
      document.getElementById('currentChar').innerText = char;
    }
    
    function addToMessage(char) {
      const input = document.getElementById('messageInput');
      input.value += char;
      updateMessagePreview();
    }
    
    function updateMessagePreview() {
      const msg = document.getElementById('messageInput').value;
      if (msg.length > 0) {
        document.getElementById('currentChar').innerText = msg;
      }
    }
    
    function updateBrightnessDisplay() {
      document.getElementById('brightnessValue').innerText = 
        document.getElementById('brightness').value;
    }
    
    function updateSpeedDisplay(effect) {
      const value = document.getElementById(effect + 'Speed').value;
      document.getElementById(effect + 'SpeedValue').innerText = value + 'ms';
    }
    
    function updateEffectSpeedVisibility() {
      const effect = document.getElementById('effect').value;
      
      // Hide all speed controls
      document.getElementById('blinkSpeedControl').classList.add('hidden');
      document.getElementById('fadeSpeedControl').classList.add('hidden');
      document.getElementById('cycleSpeedControl').classList.add('hidden');
      
      // Show relevant speed control
      if (effect === 'blink') {
        document.getElementById('blinkSpeedControl').classList.remove('hidden');
      } else if (effect === 'fade') {
        document.getElementById('fadeSpeedControl').classList.remove('hidden');
      } else if (effect === 'cycle') {
        document.getElementById('cycleSpeedControl').classList.remove('hidden');
      }
    }
    
    function updateSign() {
      const color = document.getElementById('colorPicker').value;
      const brightness = document.getElementById('brightness').value;
      const effect = document.getElementById('effect').value;
      
      // Get speed values
      const blinkSpeed = document.getElementById('blinkSpeed').value;
      const fadeSpeed = document.getElementById('fadeSpeed').value;
      const cycleSpeed = document.getElementById('cycleSpeed').value;
      
      const r = parseInt(color.substr(1,2), 16);
      const g = parseInt(color.substr(3,2), 16);
      const b = parseInt(color.substr(5,2), 16);
      
      let url = '';
      
      if (currentMode === 'single') {
        const char = document.getElementById('charInput').value || 'A';
        url = `/set?mode=single&char=${encodeURIComponent(char)}&r=${r}&g=${g}&b=${b}&brightness=${brightness}&effect=${effect}&blinkSpeed=${blinkSpeed}&fadeSpeed=${fadeSpeed}&cycleSpeed=${cycleSpeed}`;
      } else {
        const message = document.getElementById('messageInput').value || 'HELLO';
        const messageSpeed = document.getElementById('messageSpeed').value;
        url = `/set?mode=message&message=${encodeURIComponent(message)}&messageSpeed=${messageSpeed}&r=${r}&g=${g}&b=${b}&brightness=${brightness}&effect=${effect}&blinkSpeed=${blinkSpeed}&fadeSpeed=${fadeSpeed}&cycleSpeed=${cycleSpeed}`;
      }
      
      document.getElementById('status').innerText = 'Updating...';
      
      fetch(url)
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerText = '✓ Updated!';
          setTimeout(() => {
            document.getElementById('status').innerText = 'Ready';
          }, 2000);
        })
        .catch(error => {
          document.getElementById('status').innerText = '✗ Error!';
        });
    }
    
    // Update character preview on input (single mode)
    document.getElementById('charInput').addEventListener('input', function(e) {
      const val = e.target.value;
      if(val) document.getElementById('currentChar').innerText = val;
    });
    
    // Update message preview on input (message mode)
    document.getElementById('messageInput').addEventListener('input', updateMessagePreview);
    
    // Initialize effect speed visibility
    updateEffectSpeedVisibility();
  </script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}

// ----- HANDLE SETTINGS UPDATE -----
void handleSet() {
  // Check mode
  if (server.hasArg("mode")) {
    String mode = server.arg("mode");
    if (mode == "message") {
      messageMode = true;
      if (server.hasArg("message")) {
        String rawMessage = server.arg("message");
        currentMessage = processMessage(rawMessage);
        messageIndex = 0;
        lastMessageUpdate = millis();
        
        Serial.print("Received message: ");
        Serial.println(rawMessage);
        Serial.print("Processed to: ");
        for (int i = 0; i < currentMessage.length(); i++) {
          Serial.print((int)currentMessage.charAt(i));
          Serial.print(" ");
        }
        Serial.println();
      }
      if (server.hasArg("messageSpeed")) {
        messageSpeed = server.arg("messageSpeed").toInt();
      }
    } else {
      messageMode = false;
      currentMessage = "";
    }
  }
  
  // Handle single character
  if (server.hasArg("char") && !messageMode) {
    String charStr = server.arg("char");
    if (charStr.length() > 0) {
      // Handle UTF-8 symbols by detecting common patterns
      if (charStr.indexOf("❤") >= 0 || charStr.indexOf("\xE2\x99\xA4") >= 0) {
        currentChar = 1;  // Use special code for heart
      } else if (charStr.indexOf("★") >= 0 || charStr.indexOf("\xE2\x98\x85") >= 0) {
        currentChar = 2;  // Use special code for star
      } else if (charStr.indexOf("✓") >= 0 || charStr.indexOf("\xE2\x9C\x93") >= 0) {
        currentChar = 3;  // Use special code for check
      } else if (charStr.indexOf("✗") >= 0 || charStr.indexOf("\xE2\x9C\x97") >= 0) {
        currentChar = 4;  // Use special code for cross
      } else if (charStr.indexOf("→") >= 0 || charStr.indexOf("\xE2\x86\x92") >= 0) {
        currentChar = 5;  // Use special code for arrow right
      } else if (charStr.indexOf("←") >= 0 || charStr.indexOf("\xE2\x86\x90") >= 0) {
        currentChar = 6;  // Use special code for arrow left
      } else if (charStr.indexOf("↑") >= 0 || charStr.indexOf("\xE2\x86\x91") >= 0) {
        currentChar = 7;  // Use special code for arrow up
      } else if (charStr.indexOf("↓") >= 0 || charStr.indexOf("\xE2\x86\x93") >= 0) {
        currentChar = 8;  // Use special code for arrow down
      } else if (charStr.indexOf("◆") >= 0 || charStr.indexOf("\xE2\x97\x86") >= 0) {
        currentChar = 9;  // Use special code for diamond
      } else if (charStr.indexOf("●") >= 0 || charStr.indexOf("\xE2\x97\x8F") >= 0) {
        currentChar = 10;  // Use special code for circle
      } else {
        currentChar = charStr.charAt(0);
      }
    }
  }
  
  if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {
    uint8_t r = server.arg("r").toInt();
    uint8_t g = server.arg("g").toInt();
    uint8_t b = server.arg("b").toInt();
    currentColor = CRGB(r, g, b);
  }
  
  if (server.hasArg("brightness")) {
    currentBrightness = server.arg("brightness").toInt();
    FastLED.setBrightness(currentBrightness);
  }
  
  if (server.hasArg("effect")) {
    currentEffect = server.arg("effect");
    effectState = 0;
  }
  
  // Update effect speeds
  if (server.hasArg("blinkSpeed")) {
    blinkSpeed = server.arg("blinkSpeed").toInt();
  }
  
  if (server.hasArg("fadeSpeed")) {
    fadeSpeed = server.arg("fadeSpeed").toInt();
  }
  
  if (server.hasArg("cycleSpeed")) {
    cycleSpeed = server.arg("cycleSpeed").toInt();
  }
  
  // Display character (if not in message mode, message mode handles its own display)
  if (!messageMode) {
    displayCharacter(currentChar);
  } else {
    // Start message display immediately
    if (currentMessage.length() > 0) {
      currentChar = currentMessage.charAt(0);
      displayCharacter(currentChar);
    }
  }
  
  Serial.println("Settings Updated:");
  if (messageMode) {
    Serial.print("  Mode: MESSAGE - \"");
    Serial.print(server.arg("message"));
    Serial.println("\"");
    Serial.print("  Message Speed: ");
    Serial.print(messageSpeed);
    Serial.println("ms/char");
    Serial.print("  Message Length: ");
    Serial.println(currentMessage.length());
  } else {
    Serial.println("  Mode: SINGLE CHARACTER");
    Serial.print("  Character code: ");
    Serial.println((int)currentChar);
  }
  Serial.print("  Color: RGB(");
  Serial.print(currentColor.r);
  Serial.print(",");
  Serial.print(currentColor.g);
  Serial.print(",");
  Serial.print(currentColor.b);
  Serial.println(")");
  Serial.print("  Brightness: ");
  Serial.println(currentBrightness);
  Serial.print("  Effect: ");
  Serial.println(currentEffect);
  Serial.println();
  
  server.send(200, "text/plain", "OK");
}

// ----- STATUS ENDPOINT -----
void handleStatus() {
  String json = "{";
  json += "\"char\":\"" + String(currentChar) + "\",";
  json += "\"brightness\":" + String(currentBrightness) + ",";
  json += "\"effect\":\"" + currentEffect + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

// ----- DISPLAY CHARACTER -----
void displayCharacter(char c) {
  const bool* pattern = getCharacterPattern(c);
  
  if (pattern == nullptr) {
    Serial.print("Character not found: ");
    Serial.println(c);
    return;
  }
  
  FastLED.clear();
  
  for (int row = 0; row < MATRIX_HEIGHT; row++) {
    for (int col = 0; col < MATRIX_WIDTH; col++) {
      int bitmapIndex = row * MATRIX_WIDTH + col;
      
      int ledIndex;
      if (row % 2 == 0) {
        ledIndex = row * MATRIX_WIDTH + col;
      } else {
        ledIndex = row * MATRIX_WIDTH + (MATRIX_WIDTH - 1 - col);
      }
      
      if (pattern[bitmapIndex]) {
        leds[ledIndex] = currentColor;
      }
    }
  }
  
  FastLED.show();
}

// ----- CONVERT UTF-8 SYMBOL IN MESSAGE TO CHARACTER CODE -----
char convertSymbolToCode(String str, int &offset) {
  // Check for UTF-8 multi-byte sequences
  if (str.indexOf("❤", offset) == offset || str.indexOf("\xE2\x99\xA4", offset) == offset) {
    offset += 3; // UTF-8 symbols are typically 3 bytes
    return 1; // Heart
  }
  if (str.indexOf("★", offset) == offset || str.indexOf("\xE2\x98\x85", offset) == offset) {
    offset += 3;
    return 2; // Star
  }
  if (str.indexOf("✓", offset) == offset || str.indexOf("\xE2\x9C\x93", offset) == offset) {
    offset += 3;
    return 3; // Check
  }
  if (str.indexOf("✗", offset) == offset || str.indexOf("\xE2\x9C\x97", offset) == offset) {
    offset += 3;
    return 4; // Cross
  }
  if (str.indexOf("→", offset) == offset || str.indexOf("\xE2\x86\x92", offset) == offset) {
    offset += 3;
    return 5; // Arrow right
  }
  if (str.indexOf("←", offset) == offset || str.indexOf("\xE2\x86\x90", offset) == offset) {
    offset += 3;
    return 6; // Arrow left
  }
  if (str.indexOf("↑", offset) == offset || str.indexOf("\xE2\x86\x91", offset) == offset) {
    offset += 3;
    return 7; // Arrow up
  }
  if (str.indexOf("↓", offset) == offset || str.indexOf("\xE2\x86\x93", offset) == offset) {
    offset += 3;
    return 8; // Arrow down
  }
  if (str.indexOf("◆", offset) == offset || str.indexOf("\xE2\x97\x86", offset) == offset) {
    offset += 3;
    return 9; // Diamond
  }
  if (str.indexOf("●", offset) == offset || str.indexOf("\xE2\x97\x8F", offset) == offset) {
    offset += 3;
    return 10; // Circle
  }
  
  // Regular ASCII character
  offset += 1;
  return str.charAt(offset - 1);
}

// ----- PROCESS MESSAGE TO HANDLE UTF-8 SYMBOLS -----
String processMessage(String msg) {
  String processed = "";
  int i = 0;
  while (i < msg.length()) {
    char c = convertSymbolToCode(msg, i);
    processed += c;
  }
  return processed;
}

// ----- UPDATE EFFECTS -----
void updateEffect() {
  unsigned long currentTime = millis();
  
  if (currentEffect == "static") {
    // No animation needed
    return;
  }
  
  if (currentEffect == "blink") {
    if (currentTime - lastEffectUpdate > blinkSpeed) {
      effectState = !effectState;
      if (effectState) {
        displayCharacter(currentChar);
      } else {
        FastLED.clear();
        FastLED.show();
      }
      lastEffectUpdate = currentTime;
    }
  }
  
  else if (currentEffect == "fade") {
    if (currentTime - lastEffectUpdate > fadeSpeed) {
      static int fadeDirection = 1;
      static int fadeBrightness = currentBrightness;
      
      fadeBrightness += fadeDirection * 5;
      
      if (fadeBrightness >= currentBrightness) {
        fadeBrightness = currentBrightness;
        fadeDirection = -1;
      } else if (fadeBrightness <= 20) {
        fadeBrightness = 20;
        fadeDirection = 1;
      }
      
      FastLED.setBrightness(fadeBrightness);
      FastLED.show();
      lastEffectUpdate = currentTime;
    }
  }
  
  else if (currentEffect == "cycle") {
    if (currentTime - lastEffectUpdate > cycleSpeed) {
      hueOffset += 2;
      
      for (int i = 0; i < NUM_LEDS; i++) {
        if (leds[i]) {
          leds[i] = CHSV(hueOffset, 255, 255);
        }
      }
      
      FastLED.show();
      lastEffectUpdate = currentTime;
    }
  }
}
