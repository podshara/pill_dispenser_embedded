#ifndef __LCD_DISPLAY_H_
#define __LCD_DISPLAY_H_

#define LCD_HEIGHT              240
#define LCD_WIDTH               320
#define MAX_CHARS_X             53
#define MAX_CHARS_Y             26
#define CHAR_WD                 6
#define CHAR_HT                 9

#define SCH_OFFSET_X            10
#define SCH_OFFSET_Y            40
#define SCH_HT                  90
#define SCH_MAX                 6

#define SLOT_OFFSET_X           17
#define SLOT_OFFSET_Y           40
#define SLOT_MAX_X              3
#define SLOT_MAX_Y              2
#define SLOT_PAD                3
#define SLOT_WD                 (LCD_WIDTH - SLOT_OFFSET_X * 2.0) / SLOT_MAX_X
#define SLOT_HT                 (LCD_HEIGHT - SLOT_OFFSET_Y * 2.0) / SLOT_MAX_Y
#define SCH_BT_CHAR_X           (MAX_CHARS_X / 2 - 5)
#define SCH_BT_CHAR_Y           (MAX_CHARS_Y - 2)

#define BT_SIZE                 40
#define BT_X                    (LCD_WIDTH - SLOT_OFFSET_X - BT_SIZE)
#define BT_UP_Y                 SLOT_OFFSET_Y
#define BT_DOWN_Y               (LCD_HEIGHT - SLOT_OFFSET_Y - BT_SIZE)
#define BACK_BT_CHAR_X          (MAX_CHARS_X * 2 / 3 - 3)
#define BACK_BT_CHAR_Y          (MAX_CHARS_Y - 2)

#define SCH_BG                  convertColor(255, 253, 204)
#define SLOT_BG                 convertColor(255, 253, 204)
#define HELLO_BG                convertColor(93, 136, 204)

#define VOID_X                  140
#define VOID_Y                  150

#define VOID_OFFSET             20
#define OFF_SET_SWITCH          30

#define F_CPU                   16000000

//command
#define LCD_READY 0
#define UPDATE_LCD 1


typedef enum {
  NEXT,
  SLOT,
  PILL
} PAGE;

void set_page(PAGE input);

void Draw_Hello();

void px2Char(int* charX, int* charY, int x, int y, int charOffsetX, int charOffsetY);

void Draw_ButtonFrame(int charX, int charY, int length, int color);

void Draw_Button(int charX, int charY, char* str, int length, int color);

void printTime(int t);

//-----------------Draw Page--------------------
//Draw next alarm
void Draw_Next();//in progress

//Draw concise pill details in slots
void Draw_Slots();

//Draw pill details full screen
void Draw_Pill(int slotNum);

void switchPage(int x, int y);

//-----------------Touch----------------------
//return which button is pressed in slot page
int buttonPressSlot(int x, int y);

int buttonPressPill(int x, int y);

void readCoor(int* X, int* Y, int* touch);

#endif