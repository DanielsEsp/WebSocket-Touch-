



#include <ESP8266WiFi.h>
#include <espnow.h>

uint8_t broadcastAddress1[] = {0xA8, 0x48, 0xFA, 0xDC, 0x87, 0x8D}; //  lumière
uint8_t broadcastAddress2[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //  Chauffage
uint8_t broadcastAddress3[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //  Ventilation

// Structure example to send data
// Must match the receiver structure
typedef struct test_struct {
        String label;
          bool relais;
} test_struct;

// Create a struct_message called test to store variables to be sent
test_struct lumiere;
test_struct chauffage;
test_struct ventilation;

#define lumierePin 04 //      D2
#define chauffagePin 13 //    D7
#define ventilationPin 14 //  D5

   bool lumiState, old_lumiState = false;
   bool chaufState, old_chaufState = false;
   bool ventState, old_ventState = false;


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

 
void setup() {
  
  // Init Serial Monitor
     Serial.begin(115200); Serial.println();
 
  // Set device as a Wi-Fi Station
     WiFi.mode(WIFI_STA);
     WiFi.disconnect();

  // Init ESP-NOW
     if (esp_now_init() != 0) {
         Serial.println("Error initializing ESP-NOW");
         return;
        }

     esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
     esp_now_register_send_cb(OnDataSent);
  
  // Register peer
     esp_now_add_peer(broadcastAddress1, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
     esp_now_add_peer(broadcastAddress2, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
     esp_now_add_peer(broadcastAddress3, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);     

     pinMode(lumierePin, INPUT);
     pinMode(chauffagePin, INPUT);
     pinMode(ventilationPin, INPUT);

}

 
void loop() {

     bool lumiState = digitalRead(lumierePin); delay(50);
     if (lumiState != old_lumiState) {
         old_lumiState = lumiState ;
         lumiere.label = "lumiere";
         lumiere.relais = lumiState;
         esp_now_send(broadcastAddress1, (uint8_t *) &lumiere, sizeof(lumiere));               
        }

     bool chaufState = digitalRead(chauffagePin); delay(50);
     if (chaufState != old_chaufState) {
         old_chaufState = chaufState ;
         chauffage.label = "chauffage";
         chauffage.relais = chaufState;
//         esp_now_send(broadcastAddress2, (uint8_t *) &chauffage, sizeof(chauffage));               
        }    

     bool ventState = digitalRead(ventilationPin); delay(50);
     if (ventState != old_ventState) {
         old_ventState = ventState ;
         ventilation.label = "ventilat";
         ventilation.relais = ventState;
//         esp_now_send(broadcastAddress3, (uint8_t *) &ventilation, sizeof(ventilation));               
        }    

}


//
