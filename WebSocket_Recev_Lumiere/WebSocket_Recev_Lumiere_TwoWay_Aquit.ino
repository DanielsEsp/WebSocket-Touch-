



#include <ESP8266WiFi.h>
#include <espnow.h>

// Structure example to receive data
// Must match the sender structure
typedef struct send_struct {
        String label;
          bool relais;
} send_struct;


// Create a struct_message called test to store variables to be sent
send_struct lumiere;
//test_struct chauffage;
//test_struct ventilation;

uint8_t broadcastAddress[] = {0xAC, 0x0B, 0xFB, 0xCF, 0x99, 0xD8}; //  Sender
typedef struct recev_struct {
          bool aquit;
} recev_struct; 

recev_struct confirmation;
   bool restore = true; //  flag pour demande de renvoi relaisStates

#define relaisPin 14 //  D5
   String equipement;
     bool lumiState = false;
   

//   Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  
     char macStr[18];
     Serial.print("Packet to:");
     snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
     mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
     Serial.print(macStr);
     Serial.print(" send status: ");
     if (sendStatus == 0) {
         Serial.println("Delivery success");
        } else {
         Serial.println("Delivery fail");
        }
        
}


// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  
     memcpy(&lumiere, incomingData, sizeof(lumiere));
     Serial.print("Bytes received: ");
     Serial.println(len);
     Serial.print("Equipement ");
     Serial.print(lumiere.label);
     Serial.print(" > état (0 = OFF / 1 = ON) ");
     Serial.println(lumiere.relais);
     lumiState = lumiere.relais;
     Serial.println();
     digitalWrite(relaisPin, lumiState);
     
}

 
void setup() {
  
  // Initialize Serial Monitor
     Serial.begin(115200); Serial.println(); Serial.println();
  
  // Set device as a Wi-Fi Station
     WiFi.mode(WIFI_STA);
     WiFi.disconnect();

  // Init ESP-NOW
     if (esp_now_init() != 0) {
         Serial.println("Error initializing ESP-NOW");
         return;
        }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
     esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
     esp_now_register_recv_cb(OnDataRecv);
     esp_now_register_send_cb(OnDataSent);

  // Register peer
     esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
     
     pinMode(relaisPin, OUTPUT); digitalWrite(relaisPin, LOW);

     if (restore == true) {
         confirmation.aquit = true;
         esp_now_send(broadcastAddress, (uint8_t *) &confirmation, sizeof(confirmation));
         restore = false;      
        }
  
}


void loop() {
  
}


//
