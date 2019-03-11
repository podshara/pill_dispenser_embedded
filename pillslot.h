#ifndef __PILLSLOT_H_
#define __PILLSLOT_H_

#define MAX_SLOT        6
#define MAX_TIMESET     10

typedef struct {
  int time[MAX_TIMESET];
  int timeSize;
  int amount;
  int enabled;
  char* pillName; 
} Pill;

//Add new pill to the slot. Amount specifies how many pills per time. 
//Returns 0 if slot is occupied, 1 if sucessful.
int setSlot(int slotNum, char* pillName, int amount);

void removeSlot(int slotNum);

//Add new alarm for the pill in slotNum.
int addTime(int slotNum, int t);

//decrease pill amount by 1;
int dispensePill(int slotNum);

//Returns pill details.
Pill getPill(int slotNum);

//return number of alarms
int getNextPill(int nextPill[MAX_SLOT], int* time, int currTime);

#endif