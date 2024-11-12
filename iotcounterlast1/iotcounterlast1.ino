#include <Arduino.h>
#include <ESP32Servo.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"

// Definisikan pin untuk sensor ultrasonik biru dan servo
#define TRIG_PIN_BLUE 18
#define ECHO_PIN_BLUE 19
#define SERVO_PIN 32

// Variabel untuk menyimpan hitungan objek
int blueCount1 = 0; // Untuk jarak 6-5 cm
int blueCount2 = 0; // Untuk jarak 4-3 cm
int blueCount3 = 0; // Untuk jarak 2-1 cm

Servo servo; // Objek servo

// Variabel WiFi dan server
const char* ssid = "ssid_iot";
const char* password = "uuuuuuuu";
AsyncWebServer server(80);

// Fungsi untuk mengukur jarak menggunakan sensor ultrasonik
float measureDistance(int trigPin, int echoPin) {
    long duration;
    float distance;
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = (duration / 2.0) / 29.1; // Menghitung jarak dalam cm
    return distance;
}

// Fungsi untuk mengirimkan data hitungan sebagai JSON
String getCounts() {
  String json = "{";
  json += "\"blue1\":" + String(blueCount1) + ",";
  json += "\"blue2\":" + String(blueCount2) + ",";
  json += "\"blue3\":" + String(blueCount3);
  json += "}";
  return json;
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
      font-family: Arial;
      display: inline-block;
      text-align: center;
      background-color: #0d1722;
      color: #fff;
    }
    h2 { font-size: 3.0rem; }
    h3 { font-size: 2.0rem; }
    p { font-size: 3.0rem; }
    body {
      max-width: 600px;
      margin: 0px auto;
      padding-bottom: 25px;
    }
    .button {
      display: inline-block;
      padding: 10px 20px;
      font-size: 1.5rem;
      color: #fff;
      background-color: #2196F3;
      border: 2px solid #2196F3;
      border-radius: 40px;
      text-decoration: none;
      cursor: pointer;
    }
  </style>
</head>
<body>
  <h2>SSC</h2>
  <h3>(Smart Sorting and Counting)</h3>
  <h3>Jumlah Kemasan</h3>
  <h4>Kemasan Kotak = <span id="blueCount1">0</span></h4>
  <h4>Kemasan Silinder = <span id="blueCount2">0</span></h4>
  <h4>Kemasan Segi Enam = <span id="blueCount3">0</span></h4>
  
  <script>
    setInterval(function() {
      getCounts();
    }, 1000); // Update every second

    function getCounts() {
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var counts = JSON.parse(this.responseText);
          document.getElementById("blueCount1").innerText = counts.blue1;
          document.getElementById("blueCount2").innerText = counts.blue2;
          document.getElementById("blueCount3").innerText = counts.blue3;
        }
      };
      xhr.open("GET", "/counts", true);
      xhr.send();
    }

    function resetCounts() {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/reset", true);
      xhr.send();
    }
  </script>
  <a href="#" class="button" onclick="resetCounts()">Reset Perhitungan</a>
  <br>
  <br>
  <br>
  <a href="http://192.168.43.216/" class="button">Kembali</a>
</body>
</html>
)rawliteral";

void setup() {
    Serial.begin(9600);

    // Set pin mode untuk sensor biru
    pinMode(TRIG_PIN_BLUE, OUTPUT);
    pinMode(ECHO_PIN_BLUE, INPUT);

    // Attach servo ke pin yang ditentukan
    servo.attach(SERVO_PIN);

    // Inisialisasi WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.println(WiFi.localIP());

    // Inisialisasi server
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });

    server.on("/counts", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "application/json", getCounts());
    });

    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
        blueCount1 = 0;
        blueCount2 = 0;
        blueCount3 = 0;
        request->send(200, "text/plain", "Counts reset");
    });

    server.begin();
}

void loop() {
    float distanceBlue = measureDistance(TRIG_PIN_BLUE, ECHO_PIN_BLUE);

    // Tampilkan nilai jarak pada Serial Monitor dengan satu angka di belakang koma
    Serial.print("Jarak: ");
    Serial.print(distanceBlue, 1); // Menampilkan jarak dengan satu angka di belakang koma
    Serial.println(" cm");

    // Hitung objek untuk sensor biru sesuai dengan jarak yang spesifik
    if (distanceBlue >= 10.3 && distanceBlue <= 10.7) {
        blueCount1++;
        servo.write(180); // Gerakkan servo ke 180 derajat
        delay(500); // Delay perhitungan 0,5 detik
    } else if (distanceBlue >= 9 && distanceBlue <= 10.1) {
        blueCount2++;
        servo.write(130); // Gerakkan servo ke 130 derajat
        delay(500); // Delay perhitungan 0,5 detik
    } else if (distanceBlue >= 8 && distanceBlue <= 8.8) {
        blueCount3++;
        servo.write(45); // Gerakkan servo ke 48 derajat
        delay(500); // Delay perhitungan 0,5 detik
    } else {
        Serial.println("Tidak ada objek");
    }

    // Tampilkan hasil hitungan di Serial Monitor
    Serial.print("Kemasan Kotak : ");
    Serial.println(blueCount1);

    Serial.print("Kemasan Silinder : ");
    Serial.println(blueCount2);

    Serial.print("Kemasan Segienam : ");
    Serial.println(blueCount3);

    delay(1000); // Interval untuk pembacaan data
}
