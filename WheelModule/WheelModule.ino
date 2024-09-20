#include <mcp_can.h>
#include <SPI.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];

#define BIT_DOWN 0b0100
#define BIT_UP 0b1000
#define BIT_OFF 0b0000

#define deadLvlZone 5
#define maxLVL 13203

#define CAN0_INT 3  // Set INT
MCP_CAN CAN0(4);    // Set CS

uint8_t pcId = 0;
volatile unsigned int time = 0;
volatile int16_t curLVL[2] = { maxLVL };
int16_t tmp = 0;
int16_t needLVL = 0;
unsigned long times = 0;

void LinerInter() {
  curLVL[0] = constrain(curLVL[0] + curLVL[1], 0, maxLVL);
}

void setup() {
  Serial.begin(115200);
  // DDRB &= ~(0b111);
  pcId = (PINB & 0b0111);
  // pcId = digitalRead(2) << 2 | digitalRead(1) << 1 | digitalRead(0);
  Serial.println(pcId);
  if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK) Serial.println(F("Init Succ"));
  else Serial.println(F("Err Init"));

  CAN0.setMode(MCP_NORMAL);
  pinMode(CAN0_INT, INPUT);

  attachInterrupt(1, LinerInter, CHANGE);

  DDRC = 0b00001100;
  PORTC = BIT_OFF;
  curLVL[0] = maxLVL;
  curLVL[1] = -1;
  PORTC = BIT_DOWN;
  for (; curLVL[0] != tmp;) {
    tmp = curLVL[0];
    times = millis() + 1000;
    while (times > millis()) {
    }
    // Serial.print(tmp);
    // Serial.print(" ");
    // Serial.println(curLVL[0]);
  }
  PORTC = BIT_OFF;
  curLVL[0] = 0;
  curLVL[1] = 1;
}

void loop() {
  if (!digitalRead(CAN0_INT))  // If CAN0_INT pin is low, read receive buffer
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);  // Read data: len = data length, buf = data byte(s)
    if (rxId == pcId) {
      needLVL = ((rxBuf[1] << 8) | rxBuf[2]) == -1 ? curLVL[0] : constrain(((rxBuf[1] << 8) | rxBuf[2]), 0, maxLVL);
      // Serial.println(needLVL);
    }
  }
  updateLvlAxis();
  // if (times < millis()) {
  //   times = millis() + 15000;
  //   Serial.println(curLVL[0]);
  // }
}

void updateLvlAxis() {
  if (needLVL - deadLvlZone <= curLVL[0] && curLVL[0] <= needLVL + deadLvlZone) {
    PORTC = BIT_OFF;
    // Serial.println("OFF");
  } else {
    if (curLVL[0] < needLVL) {
      PORTC = BIT_UP;
      curLVL[1] = 1;
      // Serial.println("UP");
    } else {
      PORTC = BIT_DOWN;
      curLVL[1] = -1;
      // Serial.println("DOWN");
    }
  }
}
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
