#include <LibAPRS.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

// APRS Configulation
//#define CALL_SIGN E2xxxx 
//#define CALL_SIGN_SSID 2

#define ADC_REFERENCE REF_5V
#define OPEN_SQUELCH false

// OLED Configulation
#define I2C_ADDRESS 0x3C
#define RST_PIN -1
SSD1306AsciiAvrI2c oled;

// APRS Global Variable
boolean gotPacket = false;
AX25Msg incomingPacket;
uint8_t *packetData;


void setup() {
  // Set up serial port
  Serial.begin(115200);
  
  // Initialise APRS library - This starts the modem
  APRS_init(ADC_REFERENCE, OPEN_SQUELCH);
  //APRS_setCallsign(CALL_SIGN, CALL_SIGN_SSID);
  APRS_printSettings();
  Serial.print(F("Free RAM:     ")); Serial.println(freeMemory());

#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0

  oled.setFont(Adafruit5x7);
  oled.clear();
  oled.set1X();
  oled.println(F("APRS Decoder V1.0"));
}


void loop() {
  
  delay(500);
  processPacket();
}


void processPacket() {
  char sentence[150];
  if (gotPacket) {
    gotPacket = false;
    
    Serial.print(F("Received APRS packet. SRC: "));
    Serial.print(incomingPacket.src.call);
    Serial.print(F("-"));
    Serial.print(incomingPacket.src.ssid);
    Serial.print(F(". DST: "));
    Serial.print(incomingPacket.dst.call);
    Serial.print(F("-"));
    Serial.print(incomingPacket.dst.ssid);
    Serial.print(F(". Data: "));

    for (int i = 0; i < incomingPacket.len; i++) {
      Serial.write(incomingPacket.info[i]);
    }
    Serial.println("");

    oled.clear();
    oled.set2X();
    oled.print(incomingPacket.src.call);
    oled.print(F("-"));
    oled.print(incomingPacket.src.ssid);

    oled.println();
    oled.set1X();
    oled.println();
    for (int i = 0; i < incomingPacket.len; i++) {
      if(i%20==0) oled.println();
      oled.write(incomingPacket.info[i]);
    }
    oled.println("");

    free(packetData);

    // Serial.print(F("Free RAM: ")); Serial.println(freeMemory());
  }
}

void aprs_msg_callback(struct AX25Msg *msg) {
  if (!gotPacket) {
    gotPacket = true;

    memcpy(&incomingPacket, msg, sizeof(AX25Msg));

    if (freeMemory() > msg->len) {
      packetData = (uint8_t*)malloc(msg->len);
      memcpy(packetData, msg->info, msg->len);
      incomingPacket.info = packetData;
    } else {
      // We did not have enough free RAM to receive
      // this packet, so we drop it.
      gotPacket = false;
    }
  }
}
