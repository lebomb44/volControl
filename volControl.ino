#include <Cmd.h>
#include <IR.h>
#include <SPI.h>
#include <McpDigitalPot.h>

/* *****************************
 *  Pin allocation
 * *****************************
 */
#define RED_LIGHT_PIN 6
#define RELAY_PIN 4

#define MCP4261_SLAVE_SELECT_PIN 3

/* *****************************
 *  Global variables
 * *****************************
 */
IR ir;
uint16_t relayToggleTimeout = 0;
#define RELAYTOGGLE_TIMEOUT_START 1000
#define VOLUME_CHANNEL 0
#define BALANCE_CHANNEL 1
McpDigitalPot Mcp4261 = McpDigitalPot( MCP4261_SLAVE_SELECT_PIN, 50000.0, 0.0 );

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
void redON (int arg_cnt, char **args) { digitalWrite(RED_LIGHT_PIN , HIGH); VOLCONTROL_PRINT(Serial.println("Red light ON");) }
void redOFF (int arg_cnt, char **args) { digitalWrite(RED_LIGHT_PIN , LOW); VOLCONTROL_PRINT(Serial.println("Red light OFF");) }
void relayTOGGLE (int arg_cnt, char **args) {
  if(LOW == digitalRead(RELAY_PIN)) { if(0 == relayToggleTimeout) { digitalWrite(RELAY_PIN , HIGH); VOLCONTROL_PRINT(Serial.println("Relay ON");) } }
  else { if(0 == relayToggleTimeout) { digitalWrite(RELAY_PIN , LOW); VOLCONTROL_PRINT(Serial.println("Relay OFF");) } }
  relayToggleTimeout = RELAYTOGGLE_TIMEOUT_START;
}
void volUp (int arg_cnt, char **args) {
  if(HIGH == digitalRead(RELAY_PIN)) { Mcp4261.setPosition(VOLUME_CHANNEL, constrainPos(((int)Mcp4261.getPosition(VOLUME_CHANNEL))-1)); }
  VOLCONTROL_PRINT(Serial.print("Volume UP: "); Serial.println(Mcp4261.getPosition(VOLUME_CHANNEL));)
}
void volDown (int arg_cnt, char **args) {
  if(HIGH == digitalRead(RELAY_PIN)) { Mcp4261.setPosition(VOLUME_CHANNEL, constrainPos(Mcp4261.getPosition(VOLUME_CHANNEL)+1)); }
  VOLCONTROL_PRINT(Serial.print("Volume DOWN: "); Serial.println(Mcp4261.getPosition(VOLUME_CHANNEL));)
}
void balLeft (int arg_cnt, char **args) {
  if(HIGH == digitalRead(RELAY_PIN)) { Mcp4261.setPosition(BALANCE_CHANNEL, constrainPos(Mcp4261.getPosition(BALANCE_CHANNEL)+1)); }
  VOLCONTROL_PRINT(Serial.print("Balance Left: "); Serial.println(Mcp4261.getPosition(BALANCE_CHANNEL));)
}
void balRight (int arg_cnt, char **args) {
  if(HIGH == digitalRead(RELAY_PIN)) { Mcp4261.setPosition(BALANCE_CHANNEL, constrainPos(((int)Mcp4261.getPosition(BALANCE_CHANNEL))-1)); }
  VOLCONTROL_PRINT(Serial.print("Balance Right: "); Serial.println(Mcp4261.getPosition(BALANCE_CHANNEL));)
}
void volControlEnablePrint(int arg_cnt, char **args) { volControl_printIsEnabled = true; Serial.println("volControl print enabled"); }
void volControlDisablePrint(int arg_cnt, char **args) { volControl_printIsEnabled = false; Serial.println("volControl print disabled"); }

unsigned int constrainPos(int pos) { return constrain(pos, 0, 256); }

void setup() {
  /* ****************************
   *  Pin configuration
   * ****************************
   */
  pinMode(RED_LIGHT_PIN, OUTPUT);
  digitalWrite(RED_LIGHT_PIN, LOW);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  redON(0, NULL);

  Serial.begin(115200);
  Serial.println("volControl Starting...");

  SPI.begin();
  Mcp4261.setPosition(VOLUME_CHANNEL, 250);
  Mcp4261.setPosition(BALANCE_CHANNEL, 128);

  cmdInit();
/*
  cmdAdd("enablePrint", "Enable print", volControlEnablePrint);
  cmdAdd("disablePrint", "Disable print", volControlDisablePrint);
  cmdAdd("redON", "Red light ON", redON);
  cmdAdd("redOFF", "Red light OFF", redOFF);
*/
  cmdAdd("relayTOGGLE", "Toggle relay", relayTOGGLE);
  cmdAdd("volUp", "Volume UP", volUp);
  cmdAdd("volDown", "Volume DOWN", volDown);
  cmdAdd("balLeft", "Balance Left", balLeft);
  cmdAdd("balRight", "Balance Right", balRight);
  cmdAdd("help", "List commands", cmdList);

  Serial.println("volControl Init done");

  redOFF(0, NULL);
}

void loop() {
  ir.run();
  if(true == ir.rxSamsungCodeIsReady()) {
    VOLCONTROL_PRINT( Serial.print(ir.rxGetSamsungCode(), HEX);Serial.print(" : "); )
    VOLCONTROL_PRINT( Serial.print(ir.rxGetSamsungManufacturer(), HEX);Serial.print("-"); )
    VOLCONTROL_PRINT( Serial.print(ir.rxGetSamsungData(), HEX);Serial.println(); )
    //delay(1);
    ir.purge();
    /* Check the authorized codes */
    if(0xF1F0 == ir.rxGetSamsungManufacturer()) {
      redON(0, NULL);
      if(0xD927 == ir.rxGetSamsungData()) { // A
        volUp(0, NULL);
      }
      else if(0x29D7 == ir.rxGetSamsungData()) { // B
        volDown(0, NULL);
      }
      else if(0x2BD5 == ir.rxGetSamsungData()) { // C
        relayTOGGLE(0, NULL);
      }
      else { VOLCONTROL_PRINT( Serial.print("Samsung command unknown: "); Serial.println(ir.rxGetSamsungData(), HEX); ) }
      redOFF(0, NULL);
    }
    else { VOLCONTROL_PRINT( Serial.print("Samsung manufacturer unknown: "); Serial.println(ir.rxGetSamsungManufacturer(), HEX); ) }
    ir.rxSamsungRelease();
  }

  /* Poll for new command line */
  cmdPoll();
  delay(1);
  if(0 < relayToggleTimeout) { relayToggleTimeout--; }
}

