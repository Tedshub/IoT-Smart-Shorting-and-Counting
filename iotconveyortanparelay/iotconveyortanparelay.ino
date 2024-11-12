// Import required libraries
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Arduino.h>

// Network credentials
const char* ssid = "ssid_iot";
const char* password = "uuuuuuuu";

const char* PARAM_INPUT_1 = "state";

// Define motor pins
#define RPWM_M1 18
#define LPWM_M1 19

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Flag to control conveyor movement
bool conveyorActive = false;

// HTML content for the web page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center; background-color: #0d1722; color: #fff; }
    h2 {font-size: 3.0rem;}
    h3 {font-size: 2.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .button {display: inline-block; padding: 10px 20px; font-size: 1.5rem; color: #fff; background-color: #2196F3; border: 2px solid #2196F3; border-radius: 40px; text-decoration: none; cursor: pointer; }
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>SSC</h2>
  <h3>Smart Sorting and Counting</h3>
  <h4>Klik untuk menyalakan konveyor</h4>
  <label class="switch">
    <input type="checkbox" onchange="toggleCheckbox(this)" id="1">
    <span class="slider"></span>
  </label>
  <br>
  <br>
  <br>
  <br>
  <a href="http://192.168.43.56/" class="button">Klik melihat jumlah produk yang dipilah</a>
<script>
function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ 
    xhr.open("GET", "/update?state=1", true); 
  } else { 
    xhr.open("GET", "/update?state=0", true); 
  }
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";

// Replaces placeholder with button section in your web page
String processor(const String& var) {
  return String();
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(9600);

  // Initialize motor pins
  pinMode(RPWM_M1, OUTPUT);
  pinMode(LPWM_M1, OUTPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/update?state=<inputMessage>
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    String inputMessage;
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      if (inputMessage == "1") {
        conveyorActive = true;
      } else {
        conveyorActive = false;
        analogWrite(RPWM_M1, 0);
        analogWrite(LPWM_M1, 0);
      }
    }
    request->send(200, "text/plain", "OK");
  });

  // Start server
  server.begin();
}

void loop() {
  if (conveyorActive) {
    int kecepatan = 100;

    // Berputar
    analogWrite(RPWM_M1, 0);
    analogWrite(LPWM_M1, kecepatan);
    Serial.println("Objek masuk");
    delay(900);

    // Diam
    analogWrite(RPWM_M1, 0);
    analogWrite(LPWM_M1, 0);
    Serial.println("Objek discan");
    delay(1000);

    // Berputar
    analogWrite(RPWM_M1, 0);
    analogWrite(LPWM_M1, kecepatan);
    Serial.println("Objek dipilah");
    delay(2300);

    // Diam
    analogWrite(RPWM_M1, 0);
    analogWrite(LPWM_M1, 0);
    Serial.println("Masukkan Objek");
    delay(2500);
  }
}