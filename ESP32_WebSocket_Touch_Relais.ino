



/*
  #define ILI9341_DRIVER  2,4" et 2,8"
  
  #define TFT_MISO 12 //  T_OUT
  #define TFT_MOSI 13 //  T_DIN
  #define TFT_SCLK 14 //  T_SCLK
  #define TFT_CS   26 //  
  #define TFT_DC   25 //  
  #define TFT_LED  33 // 
  #define TFT_RST  -1 //  ESP32 board EN ou Vcc
  #define TOUCH_CS 27 //  T_CS
*/


#include   <SPI.h>
#include   <TFT_eSPI.h> //             ! version 2.5.34
            TFT_eSPI tft = TFT_eSPI();
#include   "U8g2_for_TFT_eSPI.h"
            U8g2_for_TFT_eSPI u8f; //  U8g2 font instance

#define     nbrEquipements 3
            TFT_eSPI_Button bouton[nbrEquipements];
            uint16_t nt_x;//  pour correction inversion axe X de touch ILI avec callibrage software

#include   <WiFi.h>
#include   <ESPAsyncWebServer.h>
#include   <AsyncTCP.h>
#include   <Preferences.h>

// Wi-Fi
const char* ssid = "Votre SSID";
const char* password = "Votre MDP";
        int wifitime, esprestart;

// Pins
const int relaisPins[3] = {15, 16, 17}; //  Lumière, Chauffage et Ventilation
const int buzzerPin = 32;

bool relaisStates[3] = {false, false, false};

// Libellés personnalisés
const char* labels[3] = {"Lumière", "Chauffage", "Ventilation"};

// Serveur WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Mémoire flash
Preferences preferences;

// HTML de l’interface
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html>
<head>
  <meta charset="utf-8">
  <title>Contrôle ESP32</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; margin-top: 40px; }
    .button {
      padding: 16px 32px;
      font-size: 22px;
      color: #fff;
      border: none;
      border-radius: 10px;
      margin: 12px;
      cursor: pointer;
      display: inline-block;
    }
    .on { background-color: red; }
    .off { background-color: green; }
  </style>
</head>
<body>
  <h1>Contrôle des Équipements</h1>
  <div>
    <button id="btn0" class="button">Lumière</button>
    <button id="btn1" class="button">Chauffage</button>
    <button id="btn2" class="button">Ventilation</button>
  </div>

  <script>
    const ws = new WebSocket(`ws://${location.host}/ws`);
    const buzzer = new Audio("data:audio/wav;base64,UklGRiQAAABXQVZFZm10IBAAAAABAAEAESsAACJWAAACABAAZGF0YQAAAAA=");
    const labels = ["Lumière", "Chauffage", "Ventilation"];

    for (let i = 0; i < 3; i++) {
         document.getElementById("btn" + i).addEventListener("click", () => {
            ws.send("toggle" + i);
            buzzer.play();
           });
        }

    ws.onmessage = (event) => {
      const [id, state] = event.data.split(":");
      const idx = parseInt(id);
      const btn = document.getElementById("btn" + idx);
      if (state === "ON") {
        btn.textContent = "Éteindre " + labels[idx];
        btn.classList.add("on");
        btn.classList.remove("off");
      } else {
        btn.textContent = "Allumer " + labels[idx];
        btn.classList.add("off");
        btn.classList.remove("on");
      }

    };
  </script>
</body>
</html>
)rawliteral";


//   Notifie l’état d’un relais
void notifyClients(int relaisIndex) {
  
     String msg = String(relaisIndex) + ":" + (relaisStates[relaisIndex] ? "ON" : "OFF");
     ws.textAll(msg);
     
}


//   Bascule un équipement
void toggleDevice(int idx) {
  
     if (idx < 0 || idx > 2) return;
         relaisStates[idx] = !relaisStates[idx];
         digitalWrite(relaisPins[idx], relaisStates[idx]);
         Serial.printf("Equipement %s etat: %s\n", labels[idx], relaisStates[idx] ? "ON" : "OFF");
         preferences.putBool(("relais" + String(idx)).c_str(), relaisStates[idx]);
         notifyClients(idx);
         tone(buzzerPin, 1000, 100);
         
}


//   Gère les messages WebSocket
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  
     AwsFrameInfo *info = (AwsFrameInfo*)arg;
     if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
         data[len] = 0;
         String message = String((char*)data);

         if (message.startsWith("toggle")) {
             int idx = message.substring(6).toInt();
             
             if (idx >= 0 && idx < 3) {
                 relaisStates[idx] = !relaisStates[idx];
                 digitalWrite(relaisPins[idx], relaisStates[idx]);

                 if (idx == 0 && relaisStates[idx] == 0) { 
                     bouton[0].initButton(&tft, 160, 50, 300, 50, TFT_DARKGREY, TFT_GREEN, TFT_WHITE, "Lum. OFF", 2);
                     bouton[0].drawButton();
                    }
                 if (idx == 0 && relaisStates[idx] == 1) { 
                     bouton[0].initButton(&tft, 160, 50, 300, 50, TFT_DARKGREY, TFT_RED, TFT_WHITE, "Lum. ON", 2);
                     bouton[0].drawButton();
                    }
                 if (idx == 1 && relaisStates[idx] == 0) { 
                     bouton[1].initButton(&tft, 160, 120, 300, 50, TFT_DARKGREY, TFT_GREEN, TFT_WHITE, "Chauf. OFF", 2);
                     bouton[1].drawButton();
                    }
                 if (idx == 1 && relaisStates[idx] == 1) { 
                     bouton[1].initButton(&tft, 160, 120, 300, 50, TFT_DARKGREY, TFT_RED, TFT_WHITE, "Chauf. ON", 2);
                     bouton[1].drawButton();
                    }
                 if (idx == 2 && relaisStates[idx] == 0) { 
                     bouton[2].initButton(&tft, 160, 190, 300, 50, TFT_DARKGREY, TFT_GREEN, TFT_WHITE, "Vent. OFF", 2);
                     bouton[2].drawButton();
                    }
                 if (idx == 2 && relaisStates[idx] == 1) { 
                     bouton[2].initButton(&tft, 160, 190, 300, 50, TFT_DARKGREY, TFT_RED, TFT_WHITE, "Vent. ON", 2);
                     bouton[2].drawButton();
                    }  
        
                 preferences.putBool(("relais" + String(idx)).c_str(), relaisStates[idx]);
                 Serial.print("Equipement " +  String(labels[idx]) + "  >  "); Serial.println(String(relaisStates[idx] ? "ON" : "OFF"));            
                 notifyClients(idx);
                 tone(buzzerPin, 1000, 100);
              }
         }
     }
     
}


//   Événements WebSocket
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len) {
              
     if (type == WS_EVT_CONNECT) {
         for (int i = 0; i < 3; i++) {
              client->text(String(i) + ":" + (relaisStates[i] ? "ON" : "OFF"));
             }
        } else if (type == WS_EVT_DATA) {
                   handleWebSocketMessage(arg, data, len);
                  }
                  
}


void initWebSocket() {
  
     ws.onEvent(onEvent);
     server.addHandler(&ws);
     
}


void setup() {
  
     Serial.begin(115200); Serial.println("\n \n");

     tft.init();
     tft.setRotation(3); // Erreur Touch si rotation 1
     tft.begin();
     tft.fillScreen(TFT_BLACK);     
     u8f.begin(tft); 

     uint16_t calData[5];
     uint8_t calDataOK = 0;     
     calData[0] = 459; calData[1] = 3450; calData[2] = 399; calData[3] = 3364; calData[4] = 1; // ILI9341
     bouton[0].initButton(&tft, 160, 50, 300, 50, TFT_DARKGREY, TFT_TRANSPARENT, TFT_LIGHTGREY, "Lumiere", 2);
     bouton[0].drawButton();
     bouton[1].initButton(&tft, 160, 120, 300, 50, TFT_DARKGREY, TFT_TRANSPARENT, TFT_LIGHTGREY, "Chauffage", 2);
     bouton[1].drawButton();
     bouton[2].initButton(&tft, 160, 190, 300, 50, TFT_DARKGREY, TFT_TRANSPARENT, TFT_LIGHTGREY, "Ventilation", 2);
     bouton[2].drawButton();
        
     for (int i = 0; i < 3; i++) {
          pinMode(relaisPins[i], OUTPUT);
         }
     pinMode(buzzerPin, OUTPUT);

     preferences.begin("relaisPrefs", false);
     for (int i = 0; i < 3; i++) {
          relaisStates[i] = preferences.getBool(("relais" + String(i)).c_str(), false);
          digitalWrite(relaisPins[i], relaisStates[i] ? HIGH : LOW);
          Serial.printf("Equipement %s  etat: %s\n", labels[i], relaisStates[i] ? "ON" : "OFF");        
         
          if (i == 0 && relaisStates[i] == 0) { 
              bouton[0].initButton(&tft, 160, 50, 300, 50, TFT_DARKGREY, TFT_GREEN, TFT_WHITE, "Lum OFF", 2);
              bouton[0].drawButton();
             }
          if (i == 0 && relaisStates[i] == 1) { 
              bouton[0].initButton(&tft, 160, 50, 300, 50, TFT_DARKGREY, TFT_RED, TFT_WHITE, "Lum ON", 2);
              bouton[0].drawButton();
             }
          if (i == 1 && relaisStates[i] == 0) { 
              bouton[1].initButton(&tft, 160, 120, 300, 50, TFT_DARKGREY, TFT_GREEN, TFT_WHITE, "Chauf OFF", 2);
              bouton[1].drawButton();
             }
          if (i == 1 && relaisStates[i] == 1) { 
              bouton[1].initButton(&tft, 160, 120, 300, 50, TFT_DARKGREY, TFT_RED, TFT_WHITE, "Chauf ON", 2);
              bouton[1].drawButton();
             }
          if (i == 2 && relaisStates[i] == 0) { 
              bouton[2].initButton(&tft, 160, 190, 300, 50, TFT_DARKGREY, TFT_GREEN, TFT_WHITE, "Vent OFF", 2);
              bouton[2].drawButton();
             }
          if (i == 2 && relaisStates[i] == 1) { 
             bouton[2].initButton(&tft, 160, 190, 300, 50, TFT_DARKGREY, TFT_RED, TFT_WHITE, "Vent ON", 2);
             bouton[2].drawButton();
            }         
                  
       }

       WiFi.begin(ssid, password);
       Serial.print("Connexion Wi-Fi...");
       while (WiFi.status() != WL_CONNECTED) {
              wifitime ++;
              Serial.print("."); delay(100);
              if (wifitime > 20) { ESP.restart(); delay(50); }
             }
       Serial.println();
       Serial.print("Local IP "); Serial.print(WiFi.localIP()); Serial.println();
       Serial.print("Wifi RSSI="); Serial.println(WiFi.RSSI()); Serial.println();

       initWebSocket();
       server.on ("/", HTTP_GET, [](AsyncWebServerRequest *request) {
                  request->send_P(200, "text/html", index_html);
                 });

       server.begin();
     
}


void loop() {
  
     ws.cleanupClients();

//   Gestion tactile
     uint16_t t_x = 0, t_y = 0; // coordonnées touchées par l'utilisateur

     boolean pressed = tft.getTouch(&t_x, &t_y); // vrai si contact avec l'écran
     nt_x = 320 - t_x; // compense inversion de x de touch ILI9341 2,4" et 2,8"

//   On vérifie si la position du contact correspond à celle d'un bouton
     for (uint8_t numero = 0; numero < nbrEquipements; numero++) {
          if (pressed && bouton[numero].contains(nt_x, t_y)) {
              bouton[numero].press(true);  
             } else {
              bouton[numero].press(false);  
             }
         }

//   Vérifions maintenant si l'état d'un des boutons a changé
     for (uint8_t numero = 0; numero < nbrEquipements; numero++) {

       // si le bouton vient d'être pressé...
          if (bouton[numero].justPressed()) {

       //...puis on fait ce que l'utilisateur a demandé:

            switch (numero) {
                    case 0:    // premier bouton
                    Serial.println("Lumière");
                    toggleDevice(0);
                    if (relaisStates[0] == 0) { 
                        bouton[0].initButton(&tft, 160, 50, 300, 50, TFT_DARKGREY, TFT_GREEN, TFT_WHITE, "Lum OFF", 2);
                        bouton[0].drawButton();
                       }
                    if (relaisStates[0] == 1) { 
                        bouton[0].initButton(&tft, 160, 50, 300, 50, TFT_DARKGREY, TFT_RED, TFT_WHITE, "Lum ON", 2);
                        bouton[0].drawButton();
                       }
                    break;
                    case 1:    // deuxième bouton
                    Serial.println("Chauffage");
                    toggleDevice(1);
                    if (relaisStates[1] == 0) { 
                        bouton[1].initButton(&tft, 160, 120, 300, 50, TFT_DARKGREY, TFT_GREEN, TFT_WHITE, "Chauf OFF", 2);
                        bouton[1].drawButton();
                       }
                    if (relaisStates[1] == 1) { 
                        bouton[1].initButton(&tft, 160, 120, 300, 50, TFT_DARKGREY, TFT_RED, TFT_WHITE, "Chauf ON", 2);
                        bouton[1].drawButton();
                       }
                    break;
                    case 2:    // troisième bouton
                    Serial.println("Ventilation");
                    toggleDevice(2);
                    if (relaisStates[2] == 0) { 
                        bouton[2].initButton(&tft, 160, 190, 300, 50, TFT_DARKGREY, TFT_GREEN, TFT_WHITE, "Vent OFF", 2);
                        bouton[2].drawButton();
                       }
                    if (relaisStates[2] == 1) { 
                        bouton[2].initButton(&tft, 160, 190, 300, 50, TFT_DARKGREY, TFT_RED, TFT_WHITE, "Vent ON", 2);
                        bouton[2].drawButton();
                       }  
                    break;
                   }

             delay(10); // anti-rebond
            }
         } 
          
}


//
