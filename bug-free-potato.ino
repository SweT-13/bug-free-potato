#include <Arduino.h>
#include "namespace.h"
#include "move.h"

int getChannels(int number, int ch_low, int ch_high, int8_t need_low = -100, int8_t need_high = 100) {
  if (number < 0 || 16 < number) {
    return 0;
  }
  return map(constrain((int)channels[number], ch_low, ch_high), ch_low, ch_high, need_low, need_high);
}

void setup() {
#ifdef __MEGA__
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial2.begin(100000);
  while (!Serial1) {
    Serial.println("rx no good");
  }
#else
  Serial.begin(100000);
  while (!Serial) {
    digitalWrite(13, HIGH);
  }
#endif

#ifdef MASTER_ENTER
  //pin init
#endif
  lostFrame = 1;
  failSafe = 1;
  buffer[23] = MASK_LOSTFRAME;

  delay(100);
}

void loop() {
#ifdef __MEGA__
  while (Serial2.available()) {
    currentByte = Serial2.read();
#else
  while (Serial.available()) {
    currentByte = Serial.read();
#endif
    if (isPacket > 24) {
      isPacket = 0;
      break;
    }
    if (isPacket) {
      buffer[isPacket++] = currentByte;
    }
    if (currentByte == START_BYTE && lastByte == END_BYTE) {
      buffer[isPacket] = currentByte;
      isPacket = 1;
    }
    lastByte = currentByte;
  }

  if (lostFrame && (millis() >= lastTimeLost)) {
    // millis - lastTimeLost >= FS_TIME
    failSafe = 1;
  }

  if (buffer[23] & MASK_LOSTFRAME) {
    if (!lostFrame) {
      lostFrame = 1;
      lastTimeLost = millis() + FS_TIME;
    }
  } else {
    if (failSafe) {
      failSafe = 0;
    }
    lostFrame = 0;
    setChannelsFromBuffer();
  }

  if (failSafe) {
    MEGA_PRINT("\n----Alarm----\n");
    throttle = left = right = 0;
  } else {
    {
      throttle = constrain((int)channels[CH_TROTTLE], Smin, Smax);
      angle = constrain((int)channels[CH_ANGLE], Smin, Smax);
      // moveBack = throttle < Smid ? 1 : 0;
      throttle = map((throttle - Smid), 0, Smax - Smid, MIN_V, MAX_V);
      left = throttle - ((angle < (Smid - deadZone)) ? map(abs(angle - Smid), 0, Smax - Smid, 0, throttle * 2) : 0);
      right = throttle - ((angle > (Smid + deadZone)) ? map(abs(angle - Smid), 0, Smax - Smid, 0, throttle * 2) : 0);
      // throttle = map(abs((throttle - Smid)), 0, Smax - Smid, MIN_V, MAX_V);
      // left = throttle - ((angle < (Smid - deadZone)) ? map(abs(angle - Smid), 0, Smax - Smid, 0, throttle * 2) : 0);
      // right = throttle - ((angle > (Smid + deadZone)) ? map(abs(angle - Smid), 0, Smax - Smid, 0, throttle * 2) : 0);
      // Serial.print(throttle), Serial.print(' '), Serial.print(left, HEX), Serial.print(' '), Serial.print(right, HEX), Serial.print(' '), Serial.print(abs(left), HEX), Serial.print(' '), Serial.println(abs(right), HEX);
      // Serial.print(angle), Serial.print(' '), Serial.print(moveBack), Serial.print(' '), Serial.print(throttle), Serial.print(' '), Serial.print(left), Serial.print(' '), Serial.println(right);
      // Serial.print(jumpL), Serial.print(' '), Serial.print(jumpR), Serial.println(' ');

      if (millis() >= lastTimeMove) {
        moveRollPitch(getChannels(3, Smin_6pos, Smax_6pos, -5, 5), getChannels(4, Smin_6pos, Smax_6pos, -5, 5), getChannels(6, Smin_6pos, Smax_6pos, 0, 1), getChannels(7, Smin_6pos, Smax_6pos, SIDE_LEFT, SIDE_RIGHT));
        lastTimeMove = millis() + MOVE_TIME;
      }
      // MEGA_PRINT('\n' + ' channel 3 ' + getChannels(3, Smin_6pos, Smax_6pos));
      // MEGA_PRINT(' channel 4 ' + getChannels(4, Smin_6pos, Smax_6pos));
      // MEGA_PRINT(' channel 6 ' + getChannels(6, Smin_6pos, Smax_6pos));
      // MEGA_PRINT(' channel 7 ' + getChannels(7, Smin_6pos, Smax_6pos) + '\n');
      getArrayMove(arrayMove, AXIS);
      // for (int i = 0; i < 6; i++) {
      //   Serial.print(arrayMove[i]);
      //   Serial.print(' ');
      // }
      // Serial.println();
    }
  }
  //write data
#ifdef USB_CONTROL
  int usb_i = 0;
  uint8_t currentUsbByte = 0;

  if (Serial.available() >= needRead) {
    usb_i = needRead != 1 ? 1 : 0;
    while (Serial.available()) {
      currentUsbByte = Serial.read();
      if (usb_i < 8 && currentUsbByte != '\n' && currentUsbByte != '\r') {
        usb_msg[usb_i] = currentUsbByte;
        usb_i++;
        if (usb_msg[0] == 'c' && usb_i == 1) {
          needRead = 6;
          if (Serial.available() < 6) {
            break;
          }
        } else {
          needRead = 1;
        }
      }
    }
    usb_msg[usb_i < 8 ? usb_i : 8] = 0x00;
    Serial.print(usb_msg);
    Serial.println(usb_i);
    Serial.flush();
  }
  if (usb_msg[0] == 'r') {
    // MEGA_PRINT("reciver mode \n");
    writeValue(arrayMove, left, right);
  } else if (usb_msg[0] == 's') {
    // MEGA_PRINT("stop mode \n");
    lostFrame = 0;
    failSafe = 0;
    writeValue(arrayMove, 0, 0);
  } else if (usb_msg[0] == 'q') {
    // MEGA_PRINT("quet mode \n");
    lostFrame = 0;
    failSafe = 0;
  } else if (usb_msg[0] == 'c' && usb_i <= 7) {
    // MEGA_PRINT("handle mode \n");
    lostFrame = 0;
    failSafe = 0;
    data[0] = constrain((hexFromAscii(usb_msg[1]) << 4) | hexFromAscii(usb_msg[2]), 0, 0x00FE);
    data[1] = constrain((hexFromAscii(usb_msg[3]) << 4) | hexFromAscii(usb_msg[4]), 0, 0x00FE);
    data[2] = constrain((hexFromAscii(usb_msg[5]) << 4) | hexFromAscii(usb_msg[6]), 0, 0x00FE);
    data[3] = 0xFF;
    Serial1.write(data, 4);
  } else {
    lostFrame = 0;
    failSafe = 0;
    int zeroArray[AXIS] = { 0 };
    writeValue(zeroArray, 0, 0);
  }
#else
  writeValue(arrayMove, left, right);
#endif
}


int setChannelsFromBuffer() {
  //channels[0] is not used to avoid dissonance
  channels[1] = (uint16_t)((buffer[1] | buffer[2] << 8) & MASK_11b);
  channels[2] = (uint16_t)((buffer[2] >> 3 | buffer[3] << 5) & MASK_11b);
  channels[3] = (uint16_t)((buffer[3] >> 6 | buffer[4] << 2 | buffer[5] << 10) & MASK_11b);
  channels[4] = (uint16_t)((buffer[5] >> 1 | buffer[6] << 7) & MASK_11b);
  channels[5] = (uint16_t)((buffer[6] >> 4 | buffer[7] << 4) & MASK_11b);
  channels[6] = (uint16_t)((buffer[7] >> 7 | buffer[8] << 1 | buffer[9] << 9) & MASK_11b);
  channels[7] = (uint16_t)((buffer[9] >> 2 | buffer[10] << 6) & MASK_11b);
  channels[8] = (uint16_t)((buffer[10] >> 5 | buffer[11] << 3) & MASK_11b);
  channels[9] = (uint16_t)((buffer[12] | buffer[13] << 8) & MASK_11b);
  channels[10] = (uint16_t)((buffer[13] >> 3 | buffer[14] << 5) & MASK_11b);
  channels[11] = (uint16_t)((buffer[14] >> 6 | buffer[15] << 2 | buffer[16] << 10) & MASK_11b);
  channels[12] = (uint16_t)((buffer[16] >> 1 | buffer[17] << 7) & MASK_11b);
}

int writeValue(const int* array, const int trottleLeft, const int trottleRight) {
#ifdef CAN_ENTER
  // Non work
  for (int i = 0; i < AXIS; i++) {
    data[0] = array[i] & 0x00FF;
    if (i < AXIS / 2) {
      data[1] = (abs(trottleLeft) & 0x007F) | ((trottleLeft < 0 ? 1 : 0) << 8);
    }
    if (i >= AXIS / 2) {
      data[1] = (abs(trottleRight) & 0x007F) | ((trottleRight < 0 ? 1 : 0) << 8);
    }
    byte sndStat = CAN0.sendMsgBuf(i, 0, 8, data);
    // if (sndStat == CAN_OK) {
    //   Serial.println("Message Sent Successfully!");
    // } else {
    //   Serial.println("Error Sending Message...");
    // }
  }
#endif
#ifdef __MEGA__
#ifdef RTS_ENTER
  for (int i = 0; i < AXIS; i++) {
    data[0] = i + 3;
    data[1] = array[i] & 0x00FF;
    if (i < AXIS / 2) {
      data[2] = (abs(trottleLeft) & 0x007F) | ((trottleLeft < 0 ? 1 : 0) << 7);
      // data[4] = (trottleLeft < 0 ? REVERS : !REVERS) & 0x00FF;
    }
    if (i >= AXIS / 2) {
      data[2] = (abs(trottleRight) & 0x007F) | ((trottleRight < 0 ? 1 : 0) << 7);
      // data[4] = (trottleRight < 0 ? REVERS : !REVERS) & 0x00FF;
    }
    data[3] = 0xFF;
    byte sndStat = Serial1.write(data, 4);
    // if (sndStat == CAN_OK) {
    //   Serial.println("Message Sent Successfully!");
    // } else {
    //   Serial.println("Error Sending Message...");
    // }
  }
#endif
#endif
#ifdef MASTER_ENTER
  //pin update
  // analogWrite(pin_MoveLeft, abs(trottleLeft));
  // digitalWrite(pin_ReversLeft, trottleLeft < 0 ? REVERS : !REVERS);
  // analogWrite(pin_MoveRight, abs(trottleRight));
  // digitalWrite(pin_ReversRight, trottleRight < 0 ? REVERS : !REVERS);
#endif
}

u8 hexFromAscii(byte s) {
  u8 tmp = 0;
  if (s < 0x30) {
    return 0x00;
  }
  //0-9
  if (0x30 <= s && s < 0x3A) {
    return s - 0x30;
  }
  if (0x3A <= s && s < 0x41) {
    return 0x00;
  }
  //A-F
  if (0x41 <= s && s < 0x47) {
    return s - 0x37;
  }
  if (0x47 <= s && s < 0x61) {
    return 0x00;
  }
  //a-f
  if (0x61 <= s && s < 0x67) {
    return s - 0x57;
  }
  if (0x67 <= s) {
    return 0x00;
  }
}
