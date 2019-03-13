#ifndef __PILLSLOT_H_
#define __PILLSLOT_H_

#define MAX_SLOT        6
#define MAX_TIMESET     10
#define NAME_LN         32

typedef struct {
  int time[MAX_TIMESET];
  int timeSize;
  int amount;
  int enabled;
  char pillName[NAME_LN]; 
} Pill;

//Add new pill to the slot. Amount specifies how many pills per time. 
//Returns 0 if slot is occupied, 1 if sucessful.
int setSlot(int slotNum, char pillName[NAME_LN], int amount);

void removeSlot(int slotNum);

void setTime(int slotNum, int timeset[MAX_TIMESET], int timeSize);

void setName(int slotNum, char pillName[NAME_LN]);

void setAmount(int slotNum, int amount);

//Add new alarm for the pill in slotNum.
int addTime(int slotNum, int t);

//decrease pill amount by 1;
int dispensePill(int slotNum);

//Returns pill details.
Pill getPill(int slotNum);

//return next alarms
int getNextPill(int nextPill[MAX_SLOT], int* time, int currTime);

#endif