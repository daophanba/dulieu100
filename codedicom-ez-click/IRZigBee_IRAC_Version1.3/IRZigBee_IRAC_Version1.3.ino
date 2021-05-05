//Version 1.1 ( Using IRAC )
extern "C" {
#include "user_interface.h"
}
#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <assert.h>
//#include <IRrecv.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <string>
#include <FS.h>
//#include <C:\Users\ADMIN\Desktop\ARDUNIO\IRZigBee_IRAC_\JsonGenerator.h>

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <vfs.h>
#include <SPI.h>
#include <FS.h>
#include <IRsend.h>


#define led_status_pin 2
int ir_send_pin = 12 ;
IRac irac(ir_send_pin);
const int buttonPin = 5;
//////////RECV
const uint16_t kRecvPin = 13;
const uint16_t kCaptureBufferSize = 1024;
const uint8_t kTimeout = 50;
const uint16_t kMinUnknownSize = 12;
const uint8_t kTolerancePercentage = kTolerance;
#define LEGACY_TIMING_INFO false
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
void setup() {
  //system_update_cpu_freq(160);
  pinMode(led_status_pin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  SPIFFS.begin();
  Serial.begin(115200);
  Serial.setTimeout(50);
  //system_update_cpu_freq(SYS_CPU_160MHZ);
  WiFiOff();
  ///save
  Serial.println("111");
  sqlite3 *db1;
  int rc;
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  String create_table = "CREATE TABLE save_data_1 ( DICOM INTEGER , content);";
  // Serial.print(insert_info);
  //Serial.printf("\n");
  // remove existing file (AT;4)
  sqlite3_initialize();
  // Open databases
  File db_file_obj_1;
  vfs_set_spiffs_file_obj(&db_file_obj_1);
  if (db_open("/FLASH/save_data_1.db", &db1))
    return;
  rc = db_exec(db1, create_table.c_str() );
  if (rc != SQLITE_OK) {
    sqlite3_close(db1);
    return;
  }
  //////////RECV
  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50);
  // Perform a low level sanity checks that the compiler performs bit field
  // packing as we expect and Endianness is as we expect.
  assert(irutils::lowLevelSanityCheck() == 0);
  Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", kRecvPin);
#if DECODE_HASH
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif
  irrecv.setTolerance(kTolerancePercentage);
  irrecv.enableIRIn(true);
}
String id_saving;
String only_raw_code(const decode_results * const results) {  // Get raw code
  String output = "";  // Reserve some space for the string to reduce heap fragmentation.
  output.reserve(1536);  // 1.5KB should cover most cases.
  // Dump data
  for (uint16_t i = 1; i < results->rawlen; i++) {
    uint32_t usecs;
    for (usecs = results->rawbuf[i] * kRawTick; usecs > UINT16_MAX;
         usecs -= UINT16_MAX) {
      //output +=  uint64ToString(UINT16_MAX)  ;
    }
    output += ';' + uint64ToString(usecs, 10) ;
    //if (i < results->rawlen - 1)
    // output += kCommaSpaceStr;            // ',' not needed on the last one
  }
  // End declaration
  // Comment
  output += F("\n");
  return output;
}
decode_results results;
// Variables will change:
int ledState = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 300;
void led_status() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(led_status_pin, ledState);
  }
}
int check = 0 ;
void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()) {
    //Serial.println("start");
    while (Serial.available() ) {
      check = Get_info(Serial.readString());
    }
  }
  if (check == 2 ) {
    if ( irrecv.decode(&results)) {
      save_data_file(id_saving, only_raw_code(&results));
    }
  }
}
