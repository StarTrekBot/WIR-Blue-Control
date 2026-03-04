/**********************************************************************
* WIR-Blue-Control
* ESP32 WLAN IR gesteuerte Bluetooth Fernbedienung für FireTV
*
* Copyright (c) 2026 Rüdiger Lauff
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
** You should have received a copy of the GNU General Public License
* along with this program. If not, see <https://www.gnu.org/licenses/>.
*
* --------------------------------------------------------------------
* Third-Party Components:
*
* - NimBLE-Arduino by h2zero	v1.4.3
* - IRremoteESP8266				v2.9.0
* - ArduinoOTA					v2.0.17
* - esp32 von Espressif Systes	v2.0.17
*---------------------------------------------------------------------
* 
*/

#define ENABLE_KEYBOARD_BLE 1

#include <WiFi.h>
#include <WebServer.h>

#include <ArduinoOTA.h>

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#include "keyboard_ble_hal_esp32.h"

#include "BleKeyboard.h"

#include "secrets.h"

#define IR_RECV_PIN 34
#define RELAY_PIN 32
#define LED_BUILTIN 2

String bleStatus = "Idle";
String bleLog = "";
bool bleConnected = false;
bool bleAdvertising = false;

bool advWindowActive = false;
unsigned long advStartTime = 0;
const unsigned long ADV_DURATION = 30000; // 30 Sekunden

bool tvOn = false;

IRrecv irrecv(IR_RECV_PIN);

decode_results results;

WebServer server(80);

void blinkLED() {
    #ifdef LED_BUILTIN
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(100);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    #endif
}

/**
 * Prüft ob derselbe Befehl zu schnell hintereinander gesendet wird.
 *
 * @param command    ID oder Code des Befehls (z.B. uint8_t oder uint16_t)
 * @param blockTime  Sperrzeit in Millisekunden
 *
 * @return true  -> Befehl darf ausgeführt werden
 * @return false -> Befehl wird blockiert (zu schnell wiederholt)
 */
bool checkCommandRate(uint16_t command, uint16_t blockTime)
{
    static uint16_t lastCommand = 0;
    static unsigned long lastTime = 0;

    unsigned long now = millis();

    // Wenn es ein anderer Befehl ist → sofort erlauben
    if (command != lastCommand)
    {
        lastCommand = command;
        lastTime = now;
        return true;
    }
    // Wenn gleicher Befehl aber Zeit abgelaufen → erlauben
    if (now - lastTime >= blockTime)
    {
        lastTime = now;
        return true;
    }
    // Gleicher Befehl und zu schnell
    return false;
}

void ActRelais(uint16_t command) {
  if (checkCommandRate(command, 500))
  {
    tvOn = !tvOn;
    #ifdef RELAY_PIN
    digitalWrite(RELAY_PIN, tvOn);
    #endif
  }
  if (tvOn)
  {
    startAdvertisingWindow();
  }
}

// ===== IR Key Mapping =====
// HIER stellst du deine Fernbedienung ein
// Command: 1=1 Command: 2=2 .....
void handleIR(decode_results *results)
{
  switch (results->command)
  {
    case 61: // POWER  Command: 61
      ActRelais(61);   // FireTV einschalten 
      break;
  }
  if (!keyboardBLE_isConnected_HAL())
    return;
  switch (results->command)
  {
    case 16: // VOL+  Command: 16
      if (checkCommandRate(results->command, 400)) consumerControlBLE_write_HAL(KEY_MEDIA_VOLUME_UP);
      break;

    case 17: // VOL-  Command: 17
      if (checkCommandRate(results->command, 400)) consumerControlBLE_write_HAL(KEY_MEDIA_VOLUME_DOWN);
      break;

    case 22: // LEFT Command: 22
      if (checkCommandRate(results->command, 400)) keyboardBLE_write_HAL(KEY_LEFT_ARROW);
      break;

    case 23: // RIGHT Command: 23
      if (checkCommandRate(results->command, 400)) keyboardBLE_write_HAL(KEY_RIGHT_ARROW);
      break;

    case 20: // UP Command: 20
      if (checkCommandRate(results->command, 400)) keyboardBLE_write_HAL(KEY_UP_ARROW);
      break;

    case 21: // DOWN Command: 21
      if (checkCommandRate(results->command, 400)) keyboardBLE_write_HAL(KEY_DOWN_ARROW);
      break;

    case 37: // OK Command: 37
      if (checkCommandRate(results->command, 400)) keyboardBLE_write_HAL(KEY_RETURN);
      break;

    case 31: // back  Command: 31
       if (checkCommandRate(results->command, 400)) keyboardBLE_write_HAL(KEY_BACKSPACE);
      break;

    case 53: // PLAY/PAUSE Command: 53 / Command: 48
      if (checkCommandRate(results->command, 400)) consumerControlBLE_write_HAL(KEY_MEDIA_PLAY_PAUSE);
      break;

    case 48: // PLAY/PAUSE Command: 53 / Command: 48
      if (checkCommandRate(results->command, 400)) consumerControlBLE_write_HAL(KEY_MEDIA_PLAY_PAUSE);
      break;

    case 15: // MUTE  Command: 15
      if (checkCommandRate(results->command, 400)) consumerControlBLE_write_HAL(KEY_MEDIA_MUTE);
      break;

    case 54: // Stop  Command: 54
      if (checkCommandRate(results->command, 400)) consumerControlBLE_write_HAL(KEY_MEDIA_STOP);
      break;

    case 52: // vor  Command: 52
      if (checkCommandRate(results->command, 400)) consumerControlBLE_write_HAL(KEY_MEDIA_FASTFORWARD);
      break;

    case 50: // zurück  Command: 50
      if (checkCommandRate(results->command, 400)) consumerControlBLE_write_HAL(KEY_MEDIA_REWIND);
      break;

    case 59: // Home  Command: 59
      if (checkCommandRate(results->command, 400)) consumerControlBLE_write_HAL(KEY_MEDIA_WWW_HOME);
      break;
  }
}

void ExeCommand(decode_results *results) {
  int count = results->rawlen;
  if (results->decode_type == UNKNOWN) {
    Serial.print("Unknown encoding: ");
  } else if (results->decode_type == NEC) {
    Serial.print("Decoded NEC: ");
  } else if (results->decode_type == SONY) {
    Serial.print("Decoded SONY: ");
  } else if (results->decode_type == RC5) {
    Serial.print("Decoded RC5: ");
    blinkLED();
    handleIR(results);
  } else if (results->decode_type == RC6) {
    Serial.print("Decoded RC6: ");
  } else if (results->decode_type == PANASONIC) {
    Serial.print("Decoded PANASONIC - Address: ");
    Serial.print(results->address, HEX);
    Serial.print(" Value: ");
  } else if (results->decode_type == LG) {
    Serial.print("Decoded LG: ");
  } else if (results->decode_type == JVC) {
    Serial.print("Decoded JVC: ");
  } else if (results->decode_type == AIWA_RC_T501) {
    Serial.print("Decoded AIWA RC T501: ");
  } else if (results->decode_type == WHYNTER) {
    Serial.print("Decoded Whynter: ");
  }
  Serial.printf("Command: %d  Value: %d  Value HEX: %X  ", results->command, results->value, results->value);  // command: 61  Repead: 0  Value: 1981  Address: 0  Typ: 30
  Serial.println();
}

void setupIR() {
  irrecv.enableIRIn();
}

void loopIR() {
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    ExeCommand(&results);
     irrecv.resume();
  }
}

void announceBLE(std::string message) {
    bleLog += String(message.c_str()) + "\n";
    if (bleLog.length() > 2000)
        bleLog.remove(0, 1000);   // Log begrenzen
    if (message.find("Connected") != std::string::npos)
        bleConnected = true;
    if (message.find("Disconnected") != std::string::npos)
        bleConnected = false;
    if (message.find("Advertising") != std::string::npos)
        bleAdvertising = true;
}

void startAdvertisingWindow()
{
    Serial.println("Advertising gestartet (30s Fenster)");

    keyboardBLE_startAdvertisingForAll_HAL();

    advStartTime = millis();
    advWindowActive = true;
}

void checkBLEConnection()
{
    // Wenn Advertising läuft und 30 Sekunden vorbei → stoppen
    if (advWindowActive && millis() - advStartTime > ADV_DURATION)
    {
        Serial.println("Advertising Zeit abgelaufen → Stop");
        keyboardBLE_stopAdvertising_HAL();
        advWindowActive = false;
    }
    // Wenn Verbindung verloren geht → neues Fenster starten
    static bool lastConnected = false;
    bool nowConnected = keyboardBLE_isConnected_HAL();
    if (lastConnected && !nowConnected)
    {
        Serial.println("Verbindung verloren → Advertising Fenster starten");
        startAdvertisingWindow();
    }
    lastConnected = nowConnected;
}

void setup() {
    Serial.begin(115200);
    #ifdef LED_BUILTIN
      pinMode(LED_BUILTIN, OUTPUT);    // OnBoardLed ESP32 Dev Module
      digitalWrite(LED_BUILTIN, LOW);  // LED aus 
    #endif  
    #ifdef RELAY_PIN
      pinMode(RELAY_PIN, OUTPUT);    // Board Relais Esp32
      digitalWrite(RELAY_PIN, LOW);  // TV aus 
    #endif
    setupWiFi();
    delay(500); 
    setupWeb();
    setupIR();
    delay(500);
    init_keyboardBLE_HAL();
    delay(500);   // BLE Stack stabilisieren
    set_announceBLEmessage_cb_HAL(announceBLE);
    startAdvertisingWindow();
    ArduinoOTA.onStart([]() {
      //save();                       //Einkommentieren wenn Werte vor dem Update gesichert werden sollen
    });
    ArduinoOTA.begin();
    Serial.println("System ready");
}

void loop() {
    // 1️⃣ OTA zuerst (sehr wichtig!)
    ArduinoOTA.handle();
    // 2️⃣ Webserver
    server.handleClient();
    // 3️⃣ WiFi Stabilität prüfen
    checkWiFiConnection();
    // 4️⃣ BLE Status lesen
    bleConnected   = keyboardBLE_isConnected_HAL();
    bleAdvertising = keyboardBLE_isAdvertising_HAL();
    checkBLEConnection();
    // 5️⃣ IR Logik
    loopIR();
    // 6️⃣ LED Anzeige
    #ifdef LED_BUILTIN
        digitalWrite(LED_BUILTIN, tvOn);
    #endif
    yield();   // wichtig bei viel BLE + WiFi
}
