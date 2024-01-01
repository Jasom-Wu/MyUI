#ifndef _MY_UI_H_
#define _MY_UI_H_

#include "main.h"
#include "oled.h"
#include "base_timer.h"


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
typedef enum{
    NONE=0,
    CLICKED,
    LONG_PRESSED,
    PRESSING
}KeyState;
typedef void (*ExecuteFunc)(void *);
typedef struct Item{
    uint8_t id;
    char *desc;
    ExecuteFunc func;
    void *userdata;
    struct Item *next_item;

}Item_Typedef;
typedef struct Page{
    char *title;
    struct Item *first_item;
    struct Page *last_page;
}Page_Typedef;

typedef struct KeysGroup {
    uint8_t left: 2;
    uint8_t enter: 2;
    uint8_t right: 2;
    uint32_t enter_last_tick;
} KeysGroup_Typedef;

extern KeysGroup_Typedef mykeys;

void MenuInit(void);
void MenuKeyHandler(void);
void MenuProcessHandler(void);

#endif