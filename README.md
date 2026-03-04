# WIR-Blue-Control
ESP32 WLAN + IR gesteuerte Bluetooth Fernbedienung für Amazon Fire TV.
---
## DE Deutsch
WIR-Blue-Control ist eine Open-Source ESP32 Firmware, die:
- IR-Signale einer beliebigen Fernbedienung empfängt
- Diese in Bluetooth HID Signale umwandelt
- Einen Amazon Fire TV steuert
- Über eine Weboberfläche konfiguriert werden kann
- OTA-Updates unterstützt
### Funktionen
- IR → BLE Tastatur / Consumer Control Mapping
- 30 Sekunden Advertising-Fenster
- Automatischer Advertising-Start bei:
- Reboot
- Verbindungsverlust
- IR Steuerung 
- Einschalten des Relais
- Web-Interface mit Statusanzeige
- OTA Firmware Update
- Relay-Steuerung (TV Ein/Aus)
### Hardware
- ESP32 Dev Module
- IR Empfänger (z.B. VS1838B)
- Relais Modul
### Verwendete Bibliotheken- NimBLE-Arduino
- IRremoteESP8266
- ArduinoOTA
### Lizenz
Dieses Projekt steht unter der GNU GPL v3 Lizenz.

Teile oder Konzepte basieren auf dem OMOTE Projekt (GPLv3).
https://github.com/OMOTE-Community/esp32-mqtt-keyboard
Die ursprünglichen Copyrights verbleiben bei den jeweiligen Autoren.
---
---
## GB English
WIR-Blue-Control is an open-source ESP32 firmware that:
- Receives IR signals from any remote control
- Converts them into Bluetooth HID signals
- Controls an Amazon Fire TV
- Provides a web interface
- Supports OTA firmware updates
### Features
- IR → BLE keyboard / consumer control mapping
- 30-second advertising window logic
- Automatic advertising start on:
- Reboot
- Connection loss
- IR control
- Relay activation
- Web-based status interface
- OTA firmware update
- Relay control (TV power)
### Hardware
- ESP32 Dev Module
- IR receiver (e.g. VS1838B)
- Relay module
### License
This
project is licensed under the GNU General Public License v3.

Parts and/or concepts are derived from the OMOTE project (GPLv3).
https://github.com/OMOTE-Community/esp32-mqtt-keyboard
Original copyrights remain with the respective authors.

