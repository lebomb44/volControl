#include <Cmd.h>
#include <IR.h>
#include <SPI.h>

/* *****************************
 *  Pin allocation
 * *****************************
 */
#define GREEN_LIGHT_PIN 13

#define NRF24_CE_pin   9
#define NRF24_CSN_pin  10
#define NRF24_MOSI_pin 11
#define NRF24_MISO_pin 12
#define NRF24_SCK_pin  13

/* *****************************
 *  Global variables
 * *****************************
 */
IR ir;

/* *****************************
 *  Debug Macros
 * *****************************
 */
bool volControl_printIsEnabled = true;
#define VOLCONTROL_PRINT(m) if(true == volControl_printIsEnabled) { m }

/* *****************************
 *  Command lines functions
 * *****************************
 */
/* Light */
void greenON (int arg_cnt, char **args) { digitalWrite(GREEN_LIGHT_PIN , HIGH); Serial.println("Green light ON"  );  }
void greenOFF (int arg_cnt, char **args) { digitalWrite(GREEN_LIGHT_PIN , LOW); Serial.println("Green light OFF" ); }
void volUp (int arg_cnt, char **args) { Serial.println("Volume UP"  );  }
void volDown (int arg_cnt, char **args) { Serial.println("Volume DOWN" ); }
void volControlEnablePrint(int arg_cnt, char **args) { volControl_printIsEnabled = true; Serial.println("volControl print enabled"); }
void volControlDisablePrint(int arg_cnt, char **args) { volControl_printIsEnabled = false; Serial.println("volControl print disabled"); }

void setup() {
  /* ****************************
   *  Pin configuration
   * ****************************
   */
  pinMode(GREEN_LIGHT_PIN, OUTPUT);
  //digitalWrite(GREEN_LIGHT_PIN, HIGH);

  Serial.begin(115200);
  Serial.println("volControl Starting...");

  cmdInit();

  cmdAdd("greenON", "Green light ON", greenON);
  cmdAdd("greenOFF", "Green light OFF", greenOFF);
  cmdAdd("volUp", "Volume UP", volUp);
  cmdAdd("volDown", "Volume DOWN", volDown);
  cmdAdd("enablePrint", "Enable print", volControlEnablePrint);
  cmdAdd("disablePrint", "Disable print", volControlDisablePrint);
  cmdAdd("help", "List commands", cmdList);

  Serial.println("volControl Init done");

  delay(1000);
  digitalWrite(GREEN_LIGHT_PIN, HIGH);
}

void loop() {
  static uint16_t volumeLevel = 0;

  ir.run();
  if(true == ir.rxSamsungCodeIsReady()) {
    digitalWrite(GREEN_LIGHT_PIN , LOW);
    VOLCONTROL_PRINT( Serial.print(ir.rxGetSamsungCode(), HEX);Serial.print(" : "); )
    VOLCONTROL_PRINT( Serial.print(ir.rxGetSamsungManufacturer(), HEX);Serial.print("-"); )
    VOLCONTROL_PRINT( Serial.print(ir.rxGetSamsungData(), HEX);Serial.println(); )
    //delay(1);
    ir.purge();
    /* Check the authorized codes */
    if(0xF1F0 == ir.rxGetSamsungManufacturer()) {
      if(0xD927 == ir.rxGetSamsungData()) {
        if(1023 > volumeLevel) { volumeLevel++; }
        VOLCONTROL_PRINT( Serial.print("Volume UP: "); Serial.println(volumeLevel); )
      }
      else if(0x29D7 == ir.rxGetSamsungData()) {
        if(0 < volumeLevel) { volumeLevel--; }
        VOLCONTROL_PRINT( Serial.print("Volume DOWN: "); Serial.println(volumeLevel); )
      }
      else { VOLCONTROL_PRINT( Serial.print("Samsung command unknown: "); Serial.println(ir.rxGetSamsungData(), HEX); ) }
    }
    else { VOLCONTROL_PRINT( Serial.print("Samsung manufacturer unknown: "); Serial.println(ir.rxGetSamsungManufacturer(), HEX); ) }
    ir.rxSamsungRelease();
    digitalWrite(GREEN_LIGHT_PIN , HIGH);
  }

  if(0 == volumeLevel) { }
  else { }
  /* Poll for new command line */
  cmdPoll();
}

