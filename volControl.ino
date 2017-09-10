#include <Cmd.h>
#include <IR.h>
#include <SPI.h>
#include <Mcp4261.h>

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
uint16_t volumeLevel = 0;
uint16_t balanceValue = 0;
float rAB_ohms = 50000.00; // 50Kohms
MCP4261 Mcp4261 = MCP4261( MCP4261_SLAVE_SELECT_PIN, rAB_ohms );

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
void redON (int arg_cnt, char **args) { digitalWrite(RED_LIGHT_PIN , HIGH); Serial.println("Red light ON");  }
void redOFF (int arg_cnt, char **args) { digitalWrite(RED_LIGHT_PIN , LOW); Serial.println("Red light OFF"); }
void relayTOGGLE (int arg_cnt, char **args) {
  if(LOW == digitalRead(RELAY_PIN)) { if(0 == relayToggleTimeout) { digitalWrite(RELAY_PIN , HIGH); } Serial.println("Relay ON"); }
  else { if(0 == relayToggleTimeout) { digitalWrite(RELAY_PIN , LOW); } Serial.println("Relay OFF"); }
  relayToggleTimeout = RELAYTOGGLE_TIMEOUT_START;
}
void volUp (int arg_cnt, char **args) {
  if(HIGH == digitalRead(RELAY_PIN)) { if(Mcp4261.scale > volumeLevel) { volumeLevel++; Mcp4261.wiper0(volumeLevel); } }
  Serial.print("Volume UP: "); Serial.println(volumeLevel);
}
void volDown (int arg_cnt, char **args) {
  if(HIGH == digitalRead(RELAY_PIN)) { if(0 < volumeLevel) { volumeLevel--; Mcp4261.wiper0(volumeLevel); } }
  Serial.print("Volume DOWN: "); Serial.println(volumeLevel);
}
void balLeft (int arg_cnt, char **args) {
  if(HIGH == digitalRead(RELAY_PIN)) { if(Mcp4261.scale > balanceValue) { balanceValue++; Mcp4261.wiper1(balanceValue); } }
  Serial.print("Balance Left: "); Serial.println(balanceValue);
}
void balRight (int arg_cnt, char **args) {
  if(HIGH == digitalRead(RELAY_PIN)) { if(0 < balanceValue) { balanceValue--; Mcp4261.wiper1(balanceValue); } }
  Serial.print("Balance Right: "); Serial.println(balanceValue);
}
void volControlEnablePrint(int arg_cnt, char **args) { volControl_printIsEnabled = true; Serial.println("volControl print enabled"); }
void volControlDisablePrint(int arg_cnt, char **args) { volControl_printIsEnabled = false; Serial.println("volControl print disabled"); }

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
  SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
  // First measure the the wiper resistance, called rW
  Mcp4261.wiper0_pos(0); // rAW = rW_ohms
  Mcp4261.wiper1_pos(0); // rAW = rW_ohms
  delay(5000);
  Mcp4261.scale = 100.0;
  Mcp4261.wiper0(10.0);
  balanceValue = Mcp4261.scale/2;
  Mcp4261.wiper1(balanceValue);

  cmdInit();

  cmdAdd("redON", "Red light ON", redON);
  cmdAdd("redOFF", "Red light OFF", redOFF);
  cmdAdd("relayTOGGLE", "Toggle relay", relayTOGGLE);
  cmdAdd("volUp", "Volume UP", volUp);
  cmdAdd("volDown", "Volume DOWN", volDown);
  cmdAdd("balLeft", "Balance Left", balLeft);
  cmdAdd("balRight", "Balance Right", balRight);
  cmdAdd("enablePrint", "Enable print", volControlEnablePrint);
  cmdAdd("disablePrint", "Disable print", volControlDisablePrint);
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
      if(0xD927 == ir.rxGetSamsungData()) {
        volUp(0, NULL);
      }
      else if(0x29D7 == ir.rxGetSamsungData()) {
        volDown(0, NULL);
      }
      else if(0x2BD9 == ir.rxGetSamsungData()) {
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

