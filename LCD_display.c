#include <stdint.h>
#include "SSD2119.h"
#include "inc/tm4c123gh6pm.h"
#include "pillslot.h"
#include "LCD_display.h"
#include "servo.h"

//int HR_COLOR[SCH_MAX_N][3] = {{250, 78, 78}, {134, 234, 78}};
int SLOT_COLOR[SLOT_MAX_X * SLOT_MAX_Y][3] = 
      {{0,0,255}, {0,255,0}, {0,255,255}, {255,0,0}, {255,0,255}, {255,255,0}};

//**********************Display**********************
void Draw_Hello(){
  LCD_ColorFill(HELLO_BG);
  LCD_SetTextColor(255,255,255);
  LCD_Goto(MAX_CHARS_X/2 - 6, MAX_CHARS_Y/2);
  printf("Hello User!");
}

void Draw_Header(char* str, int length){
  LCD_SetTextColor(255,255,255);
  int charX = MAX_CHARS_X/2 - length / 2;
  int charY = 1;
  LCD_DrawFilledRect(charX * CHAR_WD, charY * CHAR_HT - 2, 
                     length * CHAR_WD, CHAR_HT + 2, convertColor(0,0,0));
  LCD_Goto(charX, charY);
  printf("%s", str);
}

void px2Char(int* charX, int* charY, int x, int y, int charOffsetX, int charOffsetY){
  *charX = x / CHAR_WD + charOffsetX;
  *charY = y / CHAR_HT + charOffsetY;
}

void Draw_ButtonFrame(int charX, int charY, int length, int color){
  int x = charX * CHAR_WD;
  int y = charY * CHAR_HT - 2;
  LCD_DrawRect(x, y, CHAR_WD * length - 1, CHAR_HT + 3, color);
}

void Draw_Button(int charX, int charY, char* str, int length, int color){
  int x = charX * CHAR_WD;
  int y = charY * CHAR_HT - 2;
  LCD_DrawFilledRect(x, y, CHAR_WD * length, CHAR_HT + 4, color);
  LCD_Goto(charX, charY);
  printf("%s", str);
  Draw_ButtonFrame(charX, charY, length, 0xFFFF);
}

void Draw_ArrowButton(){
  LCD_DrawRect(BT_X, BT_UP_Y, BT_SIZE, BT_SIZE, 0xFFFF);
  LCD_DrawRect(BT_X, BT_DOWN_Y, BT_SIZE, BT_SIZE, 0xFFFF);
  
  int arrow[3][2] = {{BT_X + BT_SIZE / 2, BT_UP_Y + BT_SIZE / 3},
                     {BT_X + BT_SIZE / 3, BT_UP_Y + BT_SIZE * 2/3},
                     {BT_X + BT_SIZE * 2/3, BT_UP_Y + BT_SIZE * 2/3}};
  for(int i = 0; i < 3; i++){
    LCD_DrawLine(arrow[i][0], arrow[i][1], 
                 arrow[i+1>2? 0:i+1][0], arrow[i+1>2? 0:i+1][1], 0xFFFF);
    LCD_DrawLine(arrow[i][0], LCD_HEIGHT - arrow[i][1], 
                 arrow[i+1>2? 0:i+1][0], LCD_HEIGHT - arrow[i+1>2? 0:i+1][1], 0xFFFF);
  }
}

//format: 6:05
void printTime(int t){
  int hr = t / 60;
  int min = t % 60;
  printf("%d:", hr);
  if(min < 10){
    printf("0");
  }
  printf("%d", min);
}

//-------------------Draw Next-------------------
void Draw_Next(){
  LCD_ColorFill(SCH_BG);
  Draw_Header(" Next Alarm ", 12);
  LCD_DrawFilledRect(SLOT_OFFSET_X, SLOT_OFFSET_Y, 
                     LCD_WIDTH - SLOT_OFFSET_X * 2, 
                     LCD_HEIGHT - SLOT_OFFSET_Y * 2, 0x0000);
  int charX, charY;
  px2Char(&charX, &charY, SLOT_OFFSET_X, SLOT_OFFSET_Y, 1, 1);
  
  int nextPill[MAX_SLOT];
  int time;
  int size = getNextPill(nextPill, &time, 0);
  
  LCD_Goto(charX, charY++);
  printf("Time > ");
  printTime(time);
  for(int i = 0; i < size; i++){
    Pill pill = getPill(nextPill[i]);
    LCD_Goto(charX, charY++);
    printf(" %s", pill.pillName);
  }
  
  Draw_Button(SCH_BT_CHAR_X - 1, SCH_BT_CHAR_Y, " Pill Slots ", 12, 0x0000);
}

//--------------Pill Slot--------------------
void printPill(int slotNum, int x, int y, int full){
  int charX, charY;
  px2Char(&charX, &charY, x, y, 1, 1);
  
  if(!full){
    LCD_Goto(charX, charY++);
    printf("%d", slotNum + 1);
  }
  
  Pill pill = getPill(slotNum);
  if(pill.enabled){
    LCD_Goto(charX, charY++);
    printf("Name: ");
    if(!full){
      char* ptr = pill.pillName;
      for(int i = 0; i < 8 && *ptr != 0; i++){
        printf("%c", *ptr++);
      }
    } else {
      printf(pill.pillName);
    }
    LCD_Goto(charX, charY++);
    printf("Amount: %d", pill.amount);
    
    if(full){
      LCD_Goto(charX, charY++);
      printf("Alarm Type: ");     
      printf("Time Set");
      LCD_Goto(charX, charY++);
      printf("Time List: ");
      
    }
    for(int i = 0; i < pill.timeSize; i++){
      if(!full && i >= 5){
        break;
      }
      LCD_Goto(charX + 1, charY++);
      printTime(pill.time[i]);
    }
    if(!full && pill.timeSize > 5){
      printf(" ...");
    }
  }
}

void Draw_SlotFrame(int slot, int slotX, int slotY, int slot_wd, int slot_ht){
  LCD_DrawFilledRect(slotX, slotY + slot_ht - SLOT_PAD, slot_wd, SLOT_PAD, 
          convertColor(SLOT_COLOR[slot][0], SLOT_COLOR[slot][1], SLOT_COLOR[slot][2]));
  LCD_DrawFilledRect(slotX, slotY, slot_wd, SLOT_PAD, 
          convertColor(SLOT_COLOR[slot][0], SLOT_COLOR[slot][1], SLOT_COLOR[slot][2]));
  LCD_DrawFilledRect(slotX, slotY, SLOT_PAD, slot_ht, 
          convertColor(SLOT_COLOR[slot][0], SLOT_COLOR[slot][1], SLOT_COLOR[slot][2]));
  LCD_DrawFilledRect(slotX + slot_wd - SLOT_PAD, slotY, SLOT_PAD, slot_ht, 
          convertColor(SLOT_COLOR[slot][0], SLOT_COLOR[slot][1], SLOT_COLOR[slot][2]));
}

void Draw_Slots(){
  LCD_ColorFill(SLOT_BG);
  Draw_Header(" Pill Slots ", 12);
  
  float slot_wd = SLOT_WD;
  float slot_ht = SLOT_HT;
  //int maxChar = (int)(slot_wd - SLOT_PAD * 2) / CHAR_WD;
  
  for(int y = 0; y < SLOT_MAX_Y; y++){
    for(int x = 0; x < SLOT_MAX_X; x++){
      int slotX = (int)(SLOT_OFFSET_X + x * (slot_wd + 1));
      int slotY = (int)(SLOT_OFFSET_Y + y * (slot_ht + 1));
      int slot = x + y * SLOT_MAX_X;
      
      /*LCD_DrawFilledRect(slotX, slotY, (int)slot_wd, (int)slot_ht, 
              convertColor(SLOT_COLOR[slot][0], SLOT_COLOR[slot][1], SLOT_COLOR[slot][2]));*/
      
      Draw_SlotFrame(slot, slotX, slotY, (int)slot_wd, (int)slot_ht);
      LCD_DrawFilledRect(slotX + SLOT_PAD, slotY + SLOT_PAD, 
                         (int)slot_wd - SLOT_PAD * 2, (int)slot_ht - SLOT_PAD * 2,
                         convertColor(0,0,0));
      printPill(slot, slotX + SLOT_PAD, slotY + SLOT_PAD, 0);
    }
  }
  
  //Draw Schedule button
  Draw_Button(SCH_BT_CHAR_X, SCH_BT_CHAR_Y, " Next Alarm ", 12, 0x0000);
}

//--------------------Pill---------------------
void Draw_PillCursor(int cursor, int color){
  LCD_DrawRect(SLOT_OFFSET_X, SLOT_OFFSET_Y + SLOT_PAD + cursor * CHAR_HT, 
               LCD_WIDTH - 2 * SLOT_OFFSET_X - BT_SIZE - 1, CHAR_HT, color);
}

void Draw_Pill(int slotNum){
  LCD_ColorFill(SLOT_BG);
  
  Draw_Header(" Slot   ", 8);
  LCD_Goto(MAX_CHARS_X/2 + 2, 1);
  printf("%d", slotNum + 1);
  
  LCD_DrawFilledRect(SLOT_OFFSET_X, SLOT_OFFSET_Y, 
                     LCD_WIDTH - SLOT_OFFSET_X * 2, LCD_HEIGHT - SLOT_OFFSET_Y * 2, 
                     convertColor(0,0,0));
  
  Pill pill = getPill(slotNum);
  printPill(slotNum, SLOT_OFFSET_X, SLOT_OFFSET_Y, 1);
  
  Draw_Button(BACK_BT_CHAR_X, BACK_BT_CHAR_Y, " Back ", 6, 0x0000);
}

//--------------------Touch-------------------
//0 - 5: slot number, 6: schedule button, -1: ow
int buttonPressSlot(int x, int y){
  //Slot buttons
  int slot = 0;
  for(int j = SLOT_OFFSET_Y; j < LCD_HEIGHT - SLOT_OFFSET_Y; j += SLOT_HT + 1){
    for(int i = SLOT_OFFSET_X; i < LCD_WIDTH - SLOT_OFFSET_X; i += SLOT_WD + 1){
      if(x >= i && x < i + SLOT_WD && y >= j && y < j + SLOT_HT){
        return slot;
      }
      slot++;
    }
  }
  
  //Schedule button
  if(x >= SCH_BT_CHAR_X * CHAR_WD && x <= (SCH_BT_CHAR_X + 10) * CHAR_WD && 
     y >= SCH_BT_CHAR_Y * CHAR_HT - 2 && y <= (SCH_BT_CHAR_Y + 1) * CHAR_HT + 2){
    return 6;
  }
  return -1;
}

//0: up, 1: down, 2: bottom right(back), -1: ow
int buttonPressPill(int x, int y){
  if(x >= BT_X && x <= BT_X + BT_SIZE 
     && y >= SLOT_OFFSET_Y && y <= SLOT_OFFSET_Y + BT_SIZE){
    return 0;
  } else if(x >= BT_X && x <= BT_X + BT_SIZE
            && y >= LCD_HEIGHT - SLOT_OFFSET_Y - BT_SIZE && y <= LCD_HEIGHT - SLOT_OFFSET_Y){
    return 1;
  } else if(x >= BACK_BT_CHAR_X * CHAR_WD && x <= (BACK_BT_CHAR_X + 6) * CHAR_WD
            && y >= BACK_BT_CHAR_Y * CHAR_HT - 2 && y <= (BACK_BT_CHAR_Y + 1) * CHAR_HT + 2){
    return 2;
  }
  return -1;
}

int prevX = 0, prevY = 0;
int touch = 0;

void readCoor(int* touchX, int* touchY, int* touched){
  Touch_ReadX();
  Touch_ReadY();
  long coor = Touch_GetCoords();
  int x = coor >> 16;
  int y = coor & 0xFFFF;
  x = -x + 320;
  
  LCD_Goto(0,0);
  printf("(%d, %d)", x, y);
  
  int jump = (touch && 
            (prevX > x + OFF_SET_SWITCH || 
             prevX < x - OFF_SET_SWITCH || 
             prevY > y + OFF_SET_SWITCH || 
             prevY < y - OFF_SET_SWITCH));
  int pressing = x > VOID_X + VOID_OFFSET || x < VOID_X - VOID_OFFSET ||
      y > VOID_Y + VOID_OFFSET || y < VOID_Y - VOID_OFFSET; 
  if(pressing && !jump){
    touch = 1;
    prevX = x;
    prevY = y;
    *touchX = x;
    *touchY = y;
  } else {
    touch = 0;
    *touchX = 0;
    *touchY = 0;
  }
  
  *touched = touch;
  LCD_Goto(0,1);
  if(*touched){
    printf("Touch");
  } else {
    printf("     ");
  }
}

PAGE page = SLOT;
int slotNum = -1;

void set_page(PAGE input) {
  page = input;
}

void switchPage(int x, int y){
  int button;
  switch(page){
    case NEXT:
      button = buttonPressSlot(x, y);
      if(button == 6){
        page = SLOT;
        Draw_ButtonFrame(SCH_BT_CHAR_X - 1, SCH_BT_CHAR_Y, 12, convertColor(255,0,0));
        Draw_Slots(); 
      }
      break;
    case SLOT: 
      button = buttonPressSlot(x, y);
      if(button > -1 && button < 6){
        //page = PILL;
        //Draw_Pill(button);
        //slotNum = button;
        dispenseSlot(button);
      }
      
      else if(button == 6){
        page = NEXT;
        Draw_ButtonFrame(SCH_BT_CHAR_X, SCH_BT_CHAR_Y, 12, convertColor(255,0,0));
        Draw_Next();
      }
      break;
    case PILL: 
      button = buttonPressPill(x, y);
      if(button == 2){//back
        Draw_ButtonFrame(BACK_BT_CHAR_X, BACK_BT_CHAR_Y, 6, convertColor(255,0,0));
        page = SLOT;
        Draw_Slots();
      }
      break;
    default: page = NEXT; 
      Draw_Next();
      break;
  }
}

//---------------------main---------------------------
/*
int main() {
  LCD_Init();
  Touch_Init();
  //Timer0_Init();
  
  setSlot(0, "pedegreeeeeeeeiei", 10);
  setSlot(1, "me-o", 10);
  setSlot(2, "oreo", 5);
  setSlot(3, "kuy", 11);
  setSlot(4, "doop", 24);
  
  addTime(0, 6 * 60 + 30);
  addTime(0, 12 * 60);
  addTime(0, 18 * 60 + 59);
  addTime(0, 5);
  addTime(0, 259);
  addTime(0, 2333);
  //setPeriod(1, 4 * 60 + 10, 6 * 60, 18 * 60);
  //setPeriod(2, 5, 0, 60);
  addTime(4, 5);
  
  Draw_Slots();
  
  int x, y, touch;
  while(1) {
    readCoor(&x, &y, &touch);
    if(touch){
      switchPage(x, y);
    }
  }
}*/