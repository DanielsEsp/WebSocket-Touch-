



#include <ESP8266WiFi.h>
#include <espnow.h>

// Structure example to receive data
// Must match the sender structure
typedef struct test_struct {
        String label;
          bool relais;
} test_struct;

// Create a struct_message called test to store variables to be sent
test_struct lumiere;
//test_struct chauffage;
//test_struct ventilation;

   String equipement;
     bool lumiState = false;
#define relaisPin 14 //  D5
   

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
     esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
     esp_now_register_recv_cb(OnDataRecv);

     pinMode(relaisPin, OUTPUT); digitalWrite(relaisPin, LOW);
  
}


void loop() {
  
}


//
