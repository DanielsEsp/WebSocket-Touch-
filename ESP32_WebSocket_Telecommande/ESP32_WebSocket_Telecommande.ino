



#include <WiFi.h>
#include <WebSocketsClient.h>

const char* ssid = "SSID";
const char* password = "PASS";

const char* websocket_host = "WebSocket_IP"; 
const int websocket_port = 80;
const char* websocket_path = "/ws";

const int ledPins[3] = {2, 4, 5};
const int buttonPins[3] = {25, 26, 27};

bool ledStates[3] = {false, false, false};
bool lastButtonStates[3] = {HIGH, HIGH, HIGH};

WebSocketsClient webSocket;


void sendToggle(int idx) {
    
     if (idx >= 0 && idx < 3) {
         String msg = "toggle" + String(idx);
         Serial.println("Envoi : " + msg);
         webSocket.sendTXT(msg);
        }
        
}


void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

     switch (type) {
       case WStype_DISCONNECTED:
            Serial.println("WebSocket déconnecté !");
      break;

      case WStype_CONNECTED:
           Serial.println("WebSocket connecté !");
      break;

       case WStype_TEXT: {
            String msg = String((char*)payload);
            Serial.println("Reçu : " + msg);
            int sep = msg.indexOf(':');
            if (sep > 0) {
                int idx = msg.substring(0, sep).toInt();
                String state = msg.substring(sep + 1);
                if (idx >= 0 && idx < 3) {
                    ledStates[idx] = (state == "ON");
                    digitalWrite(ledPins[idx], ledStates[idx] ? HIGH : LOW);
                    Serial.printf("LED %d => %s\n", idx, ledStates[idx] ? "ON" : "OFF");
                   }
               }
           } 
      break;

       default:
      break;
     }
     
}


void setup() {
  
     Serial.begin(115200);
     Serial.println("\nESP32 Client WebSocket");

     for (int i = 0; i < 3; i++) {
          pinMode(ledPins[i], OUTPUT);
          pinMode(buttonPins[i], INPUT_PULLUP);
          digitalWrite(ledPins[i], LOW);
         }

     Serial.printf("Connexion à %s", ssid);
     WiFi.begin(ssid, password);
     while (WiFi.status() != WL_CONNECTED) {
            Serial.print(".");
            delay(200);
           }
     Serial.println("\nConnecté !");
     Serial.print("IP locale : ");
     Serial.println(WiFi.localIP());

     webSocket.begin(websocket_host, websocket_port, websocket_path);
     webSocket.onEvent(webSocketEvent);
     webSocket.setReconnectInterval(5000);
}


void loop() {
  
     webSocket.loop();

     for (int i = 0; i < 3; i++) {
          bool reading = digitalRead(buttonPins[i]);
          if (lastButtonStates[i] == HIGH && reading == LOW) {
              sendToggle(i);
              delay(200);
             }

          lastButtonStates[i] = reading;
         }
         
}


//
