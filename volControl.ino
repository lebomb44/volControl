/* *****************************
 *  VOLCONTROL 1.1.0
 *  USING:
 *   ARDULIBS 1.1.0
 */
#include <CnC.h>
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
uint32_t relayToggleTimeout = 0;
#define RELAYTOGGLE_TIMEOUT_START 10000
#define VOLUME_CHANNEL 0
#define BALANCE_CHANNEL 1
McpDigitalPot Mcp4261 = McpDigitalPot( MCP4261_SLAVE_SELECT_PIN, 50000.0, 0.0 );

uint32_t previousTime_1s = 0;
uint32_t currentTime = 0;

const char nodeName[] PROGMEM = "volcontrol";
const char sepName[] PROGMEM = " ";
const char hkName[] PROGMEM = "val";
const char cmdGetName[] PROGMEM = "get";
const char cmdSetName[] PROGMEM = "set";

const char pingName[] PROGMEM = "ping";
const char volName[] PROGMEM = "vol";
const char volUpName[] PROGMEM = "volup";
const char volDownName[] PROGMEM = "voldown";
const char volMuteName[] PROGMEM = "volmute";
const char volMaxName[] PROGMEM = "volmax";
const char balName[] PROGMEM = "bal";
const char balLeftName[] PROGMEM = "balleft";
const char balRightName[] PROGMEM = "balright";
const char onName[] PROGMEM = "on";
const char offName[] PROGMEM = "off";
const char powerName[] PROGMEM = "power";
const char debugName[] PROGMEM = "debug";

/* *****************************
 *  Command lines functions
 * *****************************
 */
void ping_cmdGet(int arg_cnt, char **args) { cnc_print_cmdGet_u32(pingName, millis()); }
void vol_cmdSet (int arg_cnt, char **args) {
  if(4 == arg_cnt)
  {
    unsigned long volPer = strtoul(args[3], NULL, 10);
    if(HIGH == digitalRead(RELAY_PIN)) { Mcp4261.setPosition(VOLUME_CHANNEL, percent2position(volPer)); }
    cnc_print_cmdGet_tbd(debugName); cnc_Serial_get()->print("Volume "); cnc_Serial_get()->println(position2percent(Mcp4261.getPosition(VOLUME_CHANNEL))); cnc_Serial_get()->flush();
  }
}
void volUp_cmdSet (int arg_cnt, char **args) {
  if(HIGH == digitalRead(RELAY_PIN)) { Mcp4261.setPosition(VOLUME_CHANNEL, constrainPosition(((int)Mcp4261.getPosition(VOLUME_CHANNEL))-1)); }
  cnc_print_cmdGet_tbd(debugName); cnc_Serial_get()->print("Volume UP "); cnc_Serial_get()->println(position2percent(Mcp4261.getPosition(VOLUME_CHANNEL))); cnc_Serial_get()->flush();
}
void volDown_cmdSet (int arg_cnt, char **args) {
  if(HIGH == digitalRead(RELAY_PIN)) { Mcp4261.setPosition(VOLUME_CHANNEL, constrainPosition(Mcp4261.getPosition(VOLUME_CHANNEL)+1)); }
  cnc_print_cmdGet_tbd(debugName); cnc_Serial_get()->print("Volume DOWN "); cnc_Serial_get()->println(position2percent(Mcp4261.getPosition(VOLUME_CHANNEL))); cnc_Serial_get()->flush();
}
void volMute_cmdSet (int arg_cnt, char **args) {
  if(HIGH == digitalRead(RELAY_PIN)) { Mcp4261.setPosition(VOLUME_CHANNEL, constrainPosition(256)); }
  cnc_print_cmdGet_tbd(debugName); cnc_Serial_get()->print("Volume MUTE "); cnc_Serial_get()->println(position2percent(Mcp4261.getPosition(VOLUME_CHANNEL))); cnc_Serial_get()->flush();
}
void volMax_cmdSet (int arg_cnt, char **args) {
  if(HIGH == digitalRead(RELAY_PIN)) { Mcp4261.setPosition(VOLUME_CHANNEL, constrainPosition(0)); }
  cnc_print_cmdGet_tbd(debugName); cnc_Serial_get()->print("Volume MAX "); cnc_Serial_get()->println(position2percent(Mcp4261.getPosition(VOLUME_CHANNEL))); cnc_Serial_get()->flush();
}
void balLeft_cmdSet (int arg_cnt, char **args) {
  if(HIGH == digitalRead(RELAY_PIN)) { Mcp4261.setPosition(BALANCE_CHANNEL, constrainPosition(Mcp4261.getPosition(BALANCE_CHANNEL)+1)); }
  cnc_print_cmdGet_tbd(debugName); cnc_Serial_get()->print("Balance Left "); cnc_Serial_get()->println(position2percent(Mcp4261.getPosition(BALANCE_CHANNEL))); cnc_Serial_get()->flush();
}
void balRight_cmdSet (int arg_cnt, char **args) {
  if(HIGH == digitalRead(RELAY_PIN)) { Mcp4261.setPosition(BALANCE_CHANNEL, constrainPosition(((int)Mcp4261.getPosition(BALANCE_CHANNEL))-1)); }
  cnc_print_cmdGet_tbd(debugName); cnc_Serial_get()->print("Balance Right "); cnc_Serial_get()->println(position2percent(Mcp4261.getPosition(BALANCE_CHANNEL))); cnc_Serial_get()->flush();
}
void on_cmdSet (int arg_cnt, char **args) {
  digitalWrite(RELAY_PIN , HIGH); cnc_print_cmdGet_tbd(debugName); cnc_Serial_get()->println("Relay ON"); cnc_Serial_get()->flush();
}
void off_cmdSet (int arg_cnt, char **args) {
  digitalWrite(RELAY_PIN , LOW); cnc_print_cmdGet_tbd(debugName); cnc_Serial_get()->println("Relay OFF"); cnc_Serial_get()->flush();
}
void relayTOGGLE(void) {
  if(LOW == digitalRead(RELAY_PIN)) { if(0 == relayToggleTimeout) { digitalWrite(RELAY_PIN , HIGH); cnc_print_cmdGet_tbd(debugName); cnc_Serial_get()->println("Relay ON"); cnc_Serial_get()->flush(); } }
  else { if(0 == relayToggleTimeout) { digitalWrite(RELAY_PIN , LOW); cnc_print_cmdGet_tbd(debugName); cnc_Serial_get()->println("Relay OFF"); cnc_Serial_get()->flush(); } }
  relayToggleTimeout = RELAYTOGGLE_TIMEOUT_START;
}

void redON (void) { digitalWrite(RED_LIGHT_PIN , HIGH); cnc_print_cmdGet_tbd(debugName); cnc_Serial_get()->println("Red light ON"); cnc_Serial_get()->flush(); }
void redOFF(void) { digitalWrite(RED_LIGHT_PIN , LOW); cnc_print_cmdGet_tbd(debugName); cnc_Serial_get()->println("Red light OFF"); cnc_Serial_get()->flush(); }

unsigned int constrainPosition(int pos) { return constrain(pos, 0, 256); }
unsigned int position2percent(unsigned int position) { return map(constrainPosition(position), 0, 256, 100, 0); }
unsigned int constrainPercent(int per) { return constrain(per, 0, 100); }
unsigned int percent2position(unsigned int percent ) { return map(constrainPercent(percent), 0, 100, 256, 0); }

void setup() {
  /* ****************************
   *  Pin configuration
   * ****************************
   */
  pinMode(RED_LIGHT_PIN, OUTPUT);
  digitalWrite(RED_LIGHT_PIN, LOW);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  cncInit(nodeName, &Serial);
  cnc_Serial_get()->begin(115200);
  cnc_hkName_set(hkName);
  cnc_cmdGetName_set(cmdGetName);
  cnc_cmdSetName_set(cmdSetName);
  cnc_sepName_set(sepName);
  cnc_print_cmdGet_tbd(debugName); cnc_Serial_get()->println("Starting..."); cnc_Serial_get()->flush();

  redON();

  SPI.begin();
  Mcp4261.setPosition(VOLUME_CHANNEL, 250);
  Mcp4261.setPosition(BALANCE_CHANNEL, 128);

  cnc_cmdGet_Add(pingName, ping_cmdGet);
  cnc_cmdSet_Add(volName, vol_cmdSet);
  cnc_cmdSet_Add(volUpName, volUp_cmdSet);
  cnc_cmdSet_Add(volDownName, volDown_cmdSet);
  cnc_cmdSet_Add(volMuteName, volMute_cmdSet);
  cnc_cmdSet_Add(volMaxName, volMax_cmdSet);
  cnc_cmdSet_Add(balLeftName, balLeft_cmdSet);
  cnc_cmdSet_Add(balRightName, balRight_cmdSet);
  cnc_cmdSet_Add(onName, on_cmdSet);
  cnc_cmdSet_Add(offName, off_cmdSet);

  redOFF();
  previousTime_1s = millis();

  cnc_print_cmdGet_tbd(debugName); cnc_Serial_get()->println("Started"); cnc_Serial_get()->flush();
}

void loop() {
  currentTime = millis(); cncPoll();
  /* HK @ 1.0Hz */
  if((uint32_t)(currentTime - previousTime_1s) >= 1000) {
    cnc_print_hk_bool(powerName, digitalRead(RELAY_PIN)); cncPoll(); cncPoll();
    cnc_print_hk_u32(volName, position2percent(Mcp4261.getPosition(VOLUME_CHANNEL))); cncPoll();
    cnc_print_hk_u32(balName, Mcp4261.getPosition(BALANCE_CHANNEL)); cncPoll();
    previousTime_1s = currentTime;
  }

  ir.run();
  if(true == ir.rxSamsungCodeIsReady()) {
    cnc_print_cmdGet_tbd(debugName);
    cnc_Serial_get()->print(ir.rxGetSamsungCode(), HEX); cnc_Serial_get()->print(" : ");
    cnc_Serial_get()->print(ir.rxGetSamsungManufacturer(), HEX); cnc_Serial_get()->print("-");
    cnc_Serial_get()->print(ir.rxGetSamsungData(), HEX); cnc_Serial_get()->println(); cnc_Serial_get()->flush();
    //delay(1);
    ir.purge();
    /* Check the authorized codes */
    if(0xF1F0 == ir.rxGetSamsungManufacturer()) {
      redON();
      if(0x0FF1 == ir.rxGetSamsungData()) { // A
        volUp_cmdSet(0, NULL);
      }
      else if(0x17E9 == ir.rxGetSamsungData()) { // B
        volDown_cmdSet(0, NULL);
      }
      else if(0x1FE1 == ir.rxGetSamsungData()) { // C
        relayTOGGLE();
      }
      else { cnc_print_cmdGet_tbd(debugName); cnc_Serial_get()->print("Samsung command unknown "); cnc_Serial_get()->println(ir.rxGetSamsungData(), HEX); cnc_Serial_get()->flush(); }
      redOFF();
    }
    else { cnc_print_cmdGet_tbd(debugName); cnc_Serial_get()->print("Samsung manufacturer unknown "); cnc_Serial_get()->println(ir.rxGetSamsungManufacturer(), HEX); cnc_Serial_get()->flush(); }
    ir.rxSamsungRelease();
  }

  /* Poll for new command line */
  cncPoll();
  //delay(1);
  if(0 < relayToggleTimeout) { relayToggleTimeout--; }
}
