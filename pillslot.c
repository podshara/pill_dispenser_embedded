#include <stdlib.h>
#include "pillslot.h"

Pill pills[MAX_SLOT];
int slotSize = 0;

int setSlot(int slotNum, char pillName[NAME_LN], int amount){
  for(int i = 0; i < NAME_LN; i++){
    pills[slotNum].pillName[i] = pillName[i];
  }
  pills[slotNum].amount = amount;
  pills[slotNum].enabled = 1;
  slotSize++;
  return 1;
}

void setName(int slotNum, char pillName[NAME_LN]){
  for(int i = 0; i < NAME_LN; i++){
    pills[slotNum].pillName[i] = pillName[i];
  }
  pills[slotNum].enabled = 1;
}

void setAmount(int slotNum, int amount){
  pills[slotNum].amount = amount;
}

void removeSlot(int slotNum){
  if(pills[slotNum].enabled){
    pills[slotNum].enabled = 0;
    slotSize--;
  }
}

int compare( const void* a, const void* b){
   int int_a = * ( (int*) a );
   int int_b = * ( (int*) b );

   if ( int_a == int_b ) return 0;
   else if ( int_a < int_b ) return -1;
   else return 1;
}

void setTime(int slotNum, int timeset[MAX_TIMESET], int timeSize){
  if(pills[slotNum].enabled){
    pills[slotNum].timeSize = timeSize;
    for(int i = 0; i < timeSize; i++){
      pills[slotNum].time[i] = timeset[i];
    }
    qsort(pills[slotNum].time, timeSize, sizeof(int), compare);
  }
}

int addTime(int slotNum, int t){
  if(pills[slotNum].enabled){
    int thisTimeSize = pills[slotNum].timeSize;
    if(thisTimeSize >= MAX_TIMESET){
      //print no more alarm allowed
      return 0;
    }
    pills[slotNum].time[thisTimeSize] = t;
    qsort(pills[slotNum].time, thisTimeSize + 1, sizeof(int), compare);
    pills[slotNum].timeSize++;
    return 1;
  }
  return 0;
}

//not used
/*void setPeriod(int slotNum, int period, int startTime, int endTime){
  if(pills[slotNum].enabled){
    pills[slotNum].period = period;
    pills[slotNum].time[0] = startTime;
    pills[slotNum].time[1] = endTime;
  }
}*/

int dispensePill(int slotNum){
  if(!pills[slotNum].enabled | pills[slotNum].amount <= 0){
    return 0;
  }
  pills[slotNum].amount--;
  return 1;
}

Pill getPill(int slotNum){
  return pills[slotNum];
}

int getNextPill(int nextPill[MAX_SLOT], int* time, int currTime){
  int minTime = 24 * 60;
  int index = 0;
  for(int i = 0; i < MAX_SLOT; i++){
    Pill pill = getPill(i); 
    if(pill.enabled){
      for(int j = 0; j < pill.timeSize; j++){
        if(pill.time[j] > currTime && pill.time[j] < minTime){
          nextPill[0] = i;
          minTime = pill.time[j];
          index = 1;
        } else if(pill.time[j] == minTime){
          nextPill[index++] = i;
        }
      }
    }
  }
  *time = minTime;
  return index;
  
}

int getSlotSize(){
  return slotSize;
}