#ifndef _MY_UI_H_
#define _MY_UI_H_

#include "main.h"
#include "oled.h"
#include "base_timer.h"
#include "bsp_task.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
typedef enum{ //按键的四种状态
    NONE=0,				//无操作
    CLICKED,			//短按
    LONG_PRESSED,	//长按
    PRESSING			//正在按下
}KeyState;
typedef void (*ExecuteFunc)(void *);//活动回调函数的类型定义
typedef struct Item{//一个Page页面下的元素结构体类型定义
    uint8_t id;			//编号，用于记录元素在page中的位置
    char *desc;			//描述信息，用于显示元素标题
    ExecuteFunc func; //活动回调函数指针，每个item都有一个
    void *userdata;		//指向用户数据，是活动回调函数参数
    struct Item *next_item;//指向下一个元素，构成链表结构
}Item_Typedef;
typedef struct Page{//页面Page结构体定义
    char *title;		//其标题，同时也是ui的主标题
    struct Item *first_item;//指向页面下的第一个元素
    struct Page *last_page;//指向上一个page，用于ui页面返回
}Page_Typedef;


void MenuInit(void);//菜单相关初始化，里面同时包含了OLED初始化
void MenuKeyHandler(void);//按键状态捕获函数，用于捕获按键状态
void MenuProcessHandler(void);//按键逻辑处理以及页面切换，这是是关键

#endif