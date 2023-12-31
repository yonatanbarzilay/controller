
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include"esp_gap_bt_api.h"
#include "esp_err.h"

#include <Bounce2.h>
#include <BleKeyboard.h>
BleKeyboard bleKeyboard("BarzilayRallyController");//("BT-Media", "ESP32-duke24", 74);

int switch_mode_btn = 18;

  
int c_play_pause_btn = 27;
int up_volume_up_btn = 4;
int down_volume_down_btn = 2;
int left_btn = 26;
int right_btn = 25;
int plus_next_track_btn = 32;
int minus_previous_track_btn = 12;

int last_switch_mode_state = 0;

#define REMOVE_BONDED_DEVICES 0   // <- Set to 0 to view all bonded devices addresses, set to 1 to remove

#define PAIR_MAX_DEVICES 9
uint8_t pairedDeviceBtAddr[PAIR_MAX_DEVICES][6];
char bda_str[18];


#define NUM_BUTTONS 7
const uint8_t BUTTON_PINS[NUM_BUTTONS] = {c_play_pause_btn, up_volume_up_btn, down_volume_down_btn, left_btn, right_btn, plus_next_track_btn, minus_previous_track_btn};

Bounce * buttons = new Bounce[NUM_BUTTONS];
unsigned long buttonPressTimeStampu;
unsigned long buttonPressTimeStampd;
void setup() {
  
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  pinMode(switch_mode_btn, INPUT_PULLUP);
  
  checkReset() ;
  bleKeyboard.begin();
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach( BUTTON_PINS[i] , INPUT_PULLUP  );       //setup the bounce instance for the current button
    buttons[i].interval(100);              // interval in ms
  }
}

void loop() {
  int switch_mode_state = digitalRead(switch_mode_btn);
  if (switch_mode_state != last_switch_mode_state) {
    last_switch_mode_state = switch_mode_state;
    Serial.print("New mode: ");
    Serial.println(switch_mode_state);
  }
  
  for (int i = 0; i < NUM_BUTTONS; i++)  {
    // Update the Bounce instance :
    buttons[i].update();
    // If it fell, flag the need to toggle the LED
  }
  if ( buttons[0].fell() ) {
    Serial.println("C or play/pause button pushed");
    if (bleKeyboard.isConnected()) {
      if (last_switch_mode_state) {
        Serial.println("Pressing the play/pause button via the Bluetooth keyboard");
        bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
      } else {
        Serial.println("Pressing the C key via the Bluetooth keyboard");
        bleKeyboard.write('C');
      }
    }                                                                                                              

    
                       
  }
  if ( buttons[1].fell() ) {
    Serial.println("up arrow or volume up button pushed");
    if (bleKeyboard.isConnected()) {
      if (last_switch_mode_state) {
        Serial.println("Pressing the volume up button via the Bluetooth keyboard");
        bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
      } else {
        Serial.println("Pressing the arrow up key via the Bluetooth keyboard");
        bleKeyboard.write(KEY_UP_ARROW);
      }
    }
  }
  if ( buttons[2].fell() ) {
    Serial.println("down arrow or volume down button pushed");
    if (bleKeyboard.isConnected()) {
      if (last_switch_mode_state) {
        Serial.println("Pressing the volume down button via the Bluetooth keyboard");
        bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
      } else {
        Serial.println("Pressing the arrow down key via the Bluetooth keyboard");
        bleKeyboard.write(KEY_DOWN_ARROW);
      }
    }
  }
  if ( buttons[3].fell() ) {
    Serial.println("left arrow button pushed");
    if (bleKeyboard.isConnected()) {
      if (!last_switch_mode_state) {
        Serial.println("Pressing the arrow left key via the Bluetooth keyboard");
        bleKeyboard.write(KEY_LEFT_ARROW);
      }
    }
  }
  if ( buttons[4].fell() ) {
    Serial.println("right arrow button pushed");
    if (bleKeyboard.isConnected()) {
      if (!last_switch_mode_state) {
        Serial.println("Pressing the arrow right key via the Bluetooth keyboard");
        bleKeyboard.write(KEY_RIGHT_ARROW);
      }
    }
  }
  if ( buttons[5].fell() ) {
    Serial.println("+ or next track button pushed");
    if (bleKeyboard.isConnected()) {
      if (last_switch_mode_state) {
        Serial.println("Pressing the vnext track button via the Bluetooth keyboard");
        bleKeyboard.write(KEY_MEDIA_NEXT_TRACK);
      } else {
        Serial.println("Pressing the + key via the Bluetooth keyboard");
        bleKeyboard.write('+');
      }
    }
  }
  if ( buttons[6].fell() ) {
    Serial.println("- or previous track button pushed");
    if (bleKeyboard.isConnected()) {
      if (last_switch_mode_state) {
        Serial.println("Pressing the previous track button via the Bluetooth keyboard");
        bleKeyboard.write(KEY_MEDIA_PREVIOUS_TRACK);
      } else {
        Serial.println("Pressing the - key via the Bluetooth keyboard");
        bleKeyboard.write('-');
      }
    }
  }
}

bool initBluetooth() {
  if (!btStart()) {
    Serial.println("Failed to initialize controller");
    return false;
  }

  if (esp_bluedroid_init() != ESP_OK) {
    Serial.println("Failed to initialize bluedroid");
    return false;
  }

  if (esp_bluedroid_enable() != ESP_OK) {
    Serial.println("Failed to enable bluedroid");
    return false;
  }
  return true;
}

char *bda2str(const uint8_t* bda, char *str, size_t size) {
  if (bda == NULL || str == NULL || size < 18) {
    return NULL;
  }
  sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
          bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
  return str;
}
void checkReset() {
  //initBluetooth();
  Serial.print("ESP32 bluetooth address: "); Serial.println(bda2str(esp_bt_dev_get_address(), bda_str, 18));
  // Get the numbers of bonded/paired devices in the BT module
  int count = esp_bt_gap_get_bond_device_num();
  if (!count) {
    Serial.println("No bonded device found.");
  } else {
    Serial.print("Bonded device count: "); Serial.println(count);
    if ( abs(count) > PAIR_MAX_DEVICES) {
      //count = PAIR_MAX_DEVICES;
      Serial.print("Reset bonded device count: "); Serial.println(count);
      REMOVE_BONDED_DEVICES == 1;
    }
    esp_err_t tError =  esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
    if (ESP_OK == tError) {
      for (int i = 0; i < count; i++) {
        Serial.print("Found bonded device # "); Serial.print(i); Serial.print(" -> ");
        Serial.println(bda2str(pairedDeviceBtAddr[i], bda_str, 18));
        if (REMOVE_BONDED_DEVICES) {
          esp_err_t tError = esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
          if (ESP_OK == tError) {
            Serial.print("Removed bonded device # ");
          } else {
            Serial.print("Failed to remove bonded device # ");
          }
          Serial.println(i);
        }
      }
    }
  }
}