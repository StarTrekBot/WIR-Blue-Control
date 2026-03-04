
unsigned long lastWifiCheck = 0;
const unsigned long wifiCheckInterval = 60000; // 60 Sekunden

void setupWiFi()  {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    blinkLED();
    delay(250);
    Serial.print(".");
    if (millis() > 10000) {
      Serial.print("\nVerbindung zum AP fehlgeschlagen\n\n");
      ESP.restart();
    }
  }
  //WiFi.setAutoReconnect(true);  // neu
  //WiFi.persistent(true);  // aktiviert standardmäßig Modem Power Save..Das verhindert, dass der ESP nach einem kurzen Router-Reconnect „halb verbunden“ bleibt.
  //WiFi.setSleep(false); // aktiviert standardmäßig Modem Power Save. Dabei schaltet das WLAN ständig aus → Router hält Pakete → TCP läuft fest.
  //WiFi.setTxPower(WIFI_POWER_19_5dBm);  //Das erhöht nicht nur die Reichweite — es verhindert vor allem, dass der ESP beim Kanalwechsel „stecken bleibt“.
}

void checkWiFiConnection() {
  if (millis() - lastWifiCheck < wifiCheckInterval) return;
  lastWifiCheck = millis();
  // Noch verbunden?
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WLAN verloren -> reconnect");
    WiFi.disconnect();
    WiFi.reconnect();
    return;
  }
  // WICHTIG: echter Verbindungstest (Gateway ping)
  IPAddress gateway = WiFi.gatewayIP();
  WiFiClient test;
  test.setTimeout(1000);
  if (!test.connect(gateway, 80)) {
    Serial.println("Gateway nicht erreichbar -> WLAN neu starten");
    WiFi.disconnect(true, true);   // löscht kaputte Sessions
    delay(1000);
    setupWiFi();
  } else {
    test.stop();
  }
} 