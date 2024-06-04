#include <Arduino.h>
#include "move.h"

// #define max(a,b) ((a) < (b) ? (b) : (a))
// #define min(a,b) ((a) > (b) ? (b) : (a))
// #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

// map axiss LT, LM, LB, RB, RM, RT
// map axiss  0,  1,  2,  3,  4,  5
int axiss[countAxis] = { 0 };

int curLVL[6][2] = {0};
const int deadLvlZone = 1;
const int stepLvl = 1;

// Принимаю значение стороны (side) которую нужно выровнять \
   По умолчанию выравниваются обе стороны индивидуально
// Беру максимум и минимум на стороне, и промежуточным осям \
   присваиваю значение с рассчитаным шагом
// Что возвращать не придумал
void alignMiddle(const int side = 0) {
  if (side == 0 || side == SIDE_RIGHT) {
    //Align middle axiss right side
    int stepVal = (max(AV_RT, AV_RB) - min(AV_RT, AV_RB)) / (countAxis / 2 - 2 + 1);
    for (int i = AN_RB + 1; i < AN_RT; i++) {
      axiss[i] = min(AV_RT, AV_RB) + stepVal * (i - AN_RB);
    }
  }
  if (side == 0 || side == SIDE_LEFT) {
    //Align middle axiss left side
    int stepVal = (max(AV_LT, AV_LB) - min(AV_LT, AV_LB)) / (countAxis / 2 - 2 + 1);
    for (int i = AN_LT + 1; i < AN_LB; i++) {
      axiss[i] = min(AV_LT, AV_LB) + stepVal * i;
    }
  }
  return;
}

// Принимаю значение (move) и сторону (side) которой нужно изменить расположение по питчу
// Сначало меняется значения передней оси
// Когда она упрется в свой максимум начнет изменяться задняя ось
// Что возвращать не придумал
void movePitch(const int move = 0, const int side = 0) {
  if (side == 0 || side == SIDE_RIGHT) {
    // if (AV_RT == 0 || AV_LT == MAX_VALUE_ENABLE) {
    AV_RB = constrain(AV_RB + move, 0, MAX_VALUE_ENABLE);
    // } else {
    AV_RT = constrain(AV_RT + (-move), 0, MAX_VALUE_ENABLE);
    // }
    alignMiddle(SIDE_RIGHT);
  }
  if (side == 0 || side == SIDE_LEFT) {
    // if (AV_LT == 0 || AV_LT == MAX_VALUE_ENABLE) {
    AV_LB = constrain(AV_LB + move, 0, MAX_VALUE_ENABLE);
    // } else {
    AV_LT = constrain(AV_LT + (-move), 0, MAX_VALUE_ENABLE);
    // }
    alignMiddle(SIDE_LEFT);
  }
  return;
}

// Принимаю значение (move) и сторону (side) которой нужно изменить расположение по питчу
// Для всех осей на стороне происходят изменения на полученную дельту (move)
// Что возвращать не придумал
void moveSide(const int move = 0, const int side = 0) {
  if (side == 0 || side == SIDE_RIGHT) {
    for (int i = AN_RB; i <= AN_RT; i++) {
      axiss[i] = constrain(axiss[i] + move, 0, MAX_VALUE_ENABLE);
    }
  }
  if (side == 0 || side == SIDE_LEFT) {
    for (int i = AN_LT; i <= AN_LB; i++) {
      axiss[i] = constrain(axiss[i] + move, 0, MAX_VALUE_ENABLE);
    }
  }
  return;
}

// Принимает значения по ролу (roll) и питчу (pitch) и выполняет соответствующие функции \
   при установке флага isRoll корректируются обе стороны по роллу \
   при задании PitchSide [SIDE_RIGHT/SIDE_LEFT] корректируются выбранная сторона по питчу
// Что возвращать не придумал
void moveRollPitch(const int roll, const int pitch, const int isRoll = 0, const int PitchSide = 0) {
  if (isRoll) {
    moveSide(-roll, SIDE_RIGHT);
    moveSide(roll, SIDE_LEFT);
    if (roll != 0)
      movePitch(pitch, constrain(roll, -1, 1));
  } else {
    moveSide(roll);
  }
  if (PitchSide != 0) {
    movePitch(pitch, PitchSide);
  } else {
    movePitch(pitch, constrain(roll, -1, 1));
  }
  return;
}

// Принимаю указатель на массив (array) в который нужно поместить значения и его длинну (n)
// Возвращает количество перезаписаных элементов
int getArrayMove(int *array, const int n) {
  //returning the number of records
  if (array == 0 || n < countAxis) {
    return 0;
  }
  for (int i = 0; i < countAxis; i++) {
    array[i] = axiss[i];
  }
  return countAxis;
}

int curLVL[6][2] = {0};
const int deadLvlZone = 1;
const int stepLvl = 1;

int inter()
{
  curLVL[0][0] += stepLvl * curLVL[0][1];
}


void setLvlAxis(const int n, const int needLVL, const int pin_MoveUp, const int pin_MoveDown)
{
  // !!! STUPID FUNCTION, NO INPUT VALIDATION !!!
  if (needLVL - deadLvlZone <= curLVL[n][0] && curLVL[n][0] <= needLVL + deadLvlZone)
  {
    digitalWrite(pin_MoveUp, LOW);
    digitalWrite(pin_MoveDown, LOW);
    // curLVL[n][1] = 0;
  }
  else
  {
    if (curLVL[n][0] < needLVL)
    {
      digitalWrite(pin_MoveUp, HIGH);
      digitalWrite(pin_MoveDown, LOW);
      curLVL[n][1] = 1;
    }
    else
    {
      digitalWrite(pin_MoveUp, LOW);
      digitalWrite(pin_MoveDown, HIGH);
      curLVL[n][1] = -1;
    }
  }
}