#pragma once

#define __MEGA__
#ifdef __MEGA__
#define MEGA_PRINT(x) Serial.print(x)
#else
#define MEGA_PRINT(x)
#endif

#define USB_CONTROL
#ifdef USB_CONTROL
char usb_msg[9] = { 'r' };
int needRead = 1;
#endif
// #define CAN_ENTER
#define RTS_ENTER
// #define MASTER_ENTER

#define CH_TROTTLE 1
#define CH_ANGLE 2
#define CH_JUMP_R 6
#define CH_JUMP_L 7

//pwm uno 3,5,6,9,10,11
//pwm mega 2,3,4,5,6,7,8,9,10,11,12,13,44,45,46
#define pin_MoveRight 9
#define pin_MoveLeft 10

// 2-pos
#define pin_ReversRight 7
#define pin_ReversLeft 8

#define pin_UpRight 3
#define pin_DownRight 4
#define pin_UpLeft 5
#define pin_DownLeft 6

#define MIN_V 00
#define MAX_V 100

int left = 0;
int right = 0;
int16_t throttle = 0;
int16_t angle = 0;
uint8_t moveBack = 0;
int8_t deadZone = 0;

// Sbus decode
// Беззнаковые ломают абсолютное вычитание
// и получается какая то дичь
int16_t Smin = 174;
int16_t Smax = 1811;
int16_t Smid = Smin + ((Smax - Smin) / 2);

int16_t Smin_6pos = 191;
int16_t Smax_6pos = 1792;
int16_t Smid_6pos = Smin_6pos + ((Smax_6pos - Smin_6pos) / 2);

#define START_BYTE 0x0F
#define END_BYTE 0x00
#define MASK_11b 0x7FF
uint8_t currentByte = 0;
uint8_t lastByte = 0;
uint8_t isPacket = 0;

#define MASK_FAILSAFE 0x08
bool failSafe = 1;
unsigned long FS_TIME = 200;

#define MASK_LOSTFRAME 0x04
bool lostFrame = 1;
unsigned long lastTimeLost = 0;

unsigned long MOVE_TIME = 100;
unsigned long lastTimeMove = 0;

uint8_t buffer[30] = { 0 };
uint16_t channels[17] = { 0 };

byte data[8] = { 0x00, 0x01, 0x02, 0x03, 0xFF, 0xFF, 0xFF, 0xFF };

#define AXIS 6
#define REVERS 1
int arrayMove[AXIS] = { 0 };





// #include <stdint.h>
// const size_t SerialPrintf(const char *szFormat, ...) {
//   va_list argptr;
//   va_start(argptr, szFormat);
//   char *szBuffer = 0;
//   const size_t nBufferLength = vsnprintf(szBuffer, 0, szFormat, argptr) + 1;
//   if (nBufferLength == 1) return 0;
//   szBuffer = (char *)malloc(nBufferLength);
//   if (!szBuffer) return -nBufferLength;
//   vsnprintf(szBuffer, nBufferLength, szFormat, argptr);
//   Serial.print(szBuffer);
//   free(szBuffer);
//   return nBufferLength - 1;
// }