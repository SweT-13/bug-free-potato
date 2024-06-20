#ifndef MOVE_H
#define MOVE_H
// 0 - 2^16
#define SIDE_RIGHT 1
#define SIDE_LEFT -1
#define IS_ROLL 1
#define countAxis 6
// map axiss LT, LM, LB, RB, RM, RT
// map axiss  0,  1,  2,  3,  4,  5

#define AN_LT 0
#define AN_LB (countAxis / 2 - 1)
#define AN_RB (countAxis / 2)
#define AN_RT (countAxis - 1)

#define AV_LT (axiss[AN_LT])
#define AV_LB (axiss[AN_LB])
#define AV_RB (axiss[AN_RB])
#define AV_RT (axiss[AN_RT])

#define MAX_VALUE_ENABLE 0x00FE

void alignMiddle(const int side);
void movePitch(const int move, const int side);
void moveSide(const int move, const int side);
void moveRollPitch(const int roll, const int pitch, const int isRoll, const int PitchSide);
int getArrayMove(int *array, const int n);


#endif