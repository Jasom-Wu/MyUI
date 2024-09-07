/****************Copyright(c)***************

	Copyright (c) 2023 Jasom-Wu
	
	All rights reserved.
-------------------------------------------*/


#include "my_ui.h"
#include "codetab.h"
#include "string.h"
#include "bsp_uart.h"

/*==============移植修改部分==================
	1.三个按键的状态获取的引脚需要修改，KEY_LEFT、KEY_ENTER、KEY_RIGHT浮空输入
		KEY_LEFT_LOW、KEY_RIGHT_LOW、KEY_ENTER_LOW是因为硬件布局方便用于提供低电平信号（如果硬件上三个按键一端都接地就可以舍去）
	
	2.screenPutString、screenFill、screenInit宏定义需要替换成自己屏幕相应的驱动函数，具体的格式可以参考`oled.h`以及`oled.c`的定义
	
	3.UIGetTick 宏定义需要替换成定时器（或系统）时钟刻获取函数，在STM32中对应为`HAL_GetTick`函数
	
	4.SCREEN_WIDTH、SCREEN_HEIGHT用于定义屏幕的分辨率大小
	
	此外若移植到非51系统上，要删掉诸如sbit、idata、bdata、bit等关键字，避免报错
	
	祝移植顺利！		o(*￣▽￣*)ブ
*/
sbit KEY_LEFT = P2 ^ 4;
sbit KEY_RIGHT = P2 ^ 5;
sbit KEY_ENTER = P0 ^ 0;
sbit KEY_LEFT_LOW = P2^2;
sbit KEY_RIGHT_LOW = P2^7;
sbit KEY_ENTER_LOW = P0^2;

#define screenPutString OLED_ShowStr
#define screenFill      OLED_Fill
#define screenInit			oledInit

#define UIGetTick				getTick

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
//========================================

static idata Page_Typedef page_Main, page_Main2;
static idata Item_Typedef item_ShowRMS, item_ChangeGain, item_About, itemJump, itemNewPage;

static idata Page_Typedef *cur_page = NULL;
static idata Item_Typedef *cur_item = NULL;

static idata uint8_t index_y = 0;//游标指针的定义，其值为0-7
static idata ExecuteFunc running_func = NULL;//记录下当前活跃的活动回调函数，每一个菜单周期都执行一次

idata struct {
    uint8_t left: 2;//使用位域，节约内存
    uint8_t enter: 2;
    uint8_t right: 2;
    uint32_t enter_last_tick;
}mykeys = {0};

static bdata bit app_init_flag=0;//记录下ui初始化标记，用于初始化回调代码块执行一次
static bdata bit app_delete_flag=0;//记录ui删除的标记，用于删除前最后的特定代码块回调


static void ShowRMSFunc(void *_data);//活动回调函数申明

static void ChangeGainFunc(void *_data);

static void AboutFunc(void *_data);

static void itemInit(Item_Typedef *item, char *desc, ExecuteFunc func, void *userdata) {
  item->desc = desc;
  item->func = func;//赋予元素回调函数指针
  item->userdata = userdata;
  item->next_item = NULL;//记得要置空，链表检索需要判空！
  item->id = 1;//默认的id为1
  if (cur_item) {//如果不是page的第一个元素
    cur_item->next_item = item;//将自己的地址赋给上一个元素的next_item
    item->id = cur_item->id + 1;//在上一个元素的id上加一作为自己的id
  }
  cur_item = item;//需要这一句完成迭代的关系创建
}

static void pageInit(Page_Typedef *page, char *title, Item_Typedef *first_item) {
  page->title = title;
  page->first_item = first_item;
  page->last_page = NULL;
  cur_item = NULL;//新的一页，头节点置空，以便生成首个item
}
static void OLED_ShowStrCenterAligned(const char *str,uint8_t y){
  uint8_t x_padding=0,len;
  len = strlen(str)*6;//居中算法
  if(SCREEN_WIDTH>=len)
    x_padding = (SCREEN_WIDTH - len)/2;
  screenPutString(x_padding,y,str);
}
static void pageJumper(void *tar_page) {//用于后一级page创建或者恢复前一级page的恢复
  Page_Typedef *new_page = (Page_Typedef *) tar_page;//强转获得跳转的目标page
  if (tar_page != NULL) {
		Item_Typedef * pt;
    uint8_t i;
    if(cur_page->last_page==new_page)//若为page回退则将当前的last_page置空
      cur_page->last_page = NULL;
    else //若为page创建则将当前page保存到新的page的last_page
      new_page->last_page = cur_page;//存储上一个page以便之后页面返回
    cur_page = new_page;
    cur_item = cur_page->first_item;
    running_func = NULL;//跳转到新的页面，不执行任何活动回调函数！
    screenFill(0x00,0,7);//清屏
    OLED_ShowStrCenterAligned(cur_page->title,0);//在第一行居中显示标题
    pt = cur_page->first_item;
    for(i =1;i<8;i++){//一个page最大仅能显示7个item
      if(pt==NULL)
        break;
      screenPutString(8,i,pt->desc);//逐个输出元素的标题
			pt = pt->next_item;
    }
    index_y = 0;//游标默认指向标题
  }
}

void MenuInit(void) {
  screenInit();//oled初始化操作
  pageInit(&page_Main, "Main", &item_ShowRMS);//先初始化page,需要传入第一个item
  itemInit(&item_ShowRMS, "ShowRMS", ShowRMSFunc, NULL);//第一个item
  itemInit(&item_ChangeGain, "ChangeGain", ChangeGainFunc, NULL);//第二个
  itemInit(&item_About, "About", AboutFunc, NULL);//第三个
	itemInit(&itemJump, "Jump to new Page", pageJumper, &page_Main2);//回调函数设成pageJumper功能就是跳转到新页面，后面参数为新页面对象的指针
	/*若要创建新的界面，必须先初始化一个page，以后的item才是它的子元素。
	pageInit(...)
	itemInit(...)
	itemInit(...)
	...
	*/
	
	pageInit(&page_Main2, "Main2", &itemNewPage);//创建一个新页面
	itemInit(&itemNewPage, "Jump to last Page", pageJumper, &page_Main);//单击跳回主页面，这个和长按推出实现功能一样
	
  pageJumper(&page_Main);//跳转至初始界面，激活菜单
}
void MenuKeyHandler() {
  KEY_LEFT = KEY_ENTER = KEY_RIGHT = 1;//先把输入端口拉高，检测低电平
	KEY_LEFT_LOW = KEY_RIGHT_LOW = KEY_ENTER_LOW =  0;//给各个输入端口提供低电平
  switch (mykeys.left) {//用switch语句构成按键的状态机
    case NONE: {
      if (KEY_LEFT == 0)//若采集到低电平说明按键按下
        mykeys.left = PRESSING;//按键按下
      break;
    }
    case PRESSING: {
      if (KEY_LEFT == 1)//在pressing状态下检测到输入电平为高表明按键完成了一次短按
        mykeys.left = CLICKED;
      break;
    }
  }
  switch (mykeys.right) {
    case NONE: {
      if (KEY_RIGHT == 0)
        mykeys.right = PRESSING;
      break;
    }
    case PRESSING: {
      if (KEY_RIGHT == 1)
        mykeys.right = CLICKED;
      break;
    }
  }
  switch (mykeys.enter) {
    case NONE: {
      if (KEY_ENTER == 0) {
        mykeys.enter = PRESSING;
        mykeys.enter_last_tick = UIGetTick();//当确认键按下，需要记录此时的时刻
      }
      break;
    }
    case PRESSING: {
      if (KEY_ENTER == 1) {//按键松开！
				//UIGetTick()获取系统心跳的时刻，若超前于刚被按下时刻900ms
        if (UIGetTick() - mykeys.enter_last_tick > 900){
					mykeys.enter = LONG_PRESSED;//触发长按
				}
        else{
          mykeys.enter = CLICKED;//若按下时间小于900ms则触发短按
        }
      }
      break;
    }
  }
}

void MenuProcessHandler() {
  if (mykeys.enter == CLICKED) {//确认键被短按后
    if(cur_item){
      if(running_func == NULL){//当前没有活动回调函数，只可能是创建或者删除ui操作
        screenFill(0x00,0,7);//清屏
        running_func = cur_item->func;
        app_init_flag = 1;//用于app启动时初始化代码块
				if(running_func!=pageJumper)
					OLED_ShowStrCenterAligned(cur_item->desc,0);
				running_func(cur_item->userdata);//立即回调app以执行app初始化代码块
				app_init_flag = 0;//复位app初始化标志
      }
    }
  } else if (mykeys.enter == LONG_PRESSED) {//确认键长按只可能是回退ui操作
    if(cur_page){
      if(running_func == NULL)//表明当前ui停在page页面
        pageJumper(cur_page->last_page);//直接转到上一个页面
      else {//如果ui当前停在元素的活动回调阶段，则回退到元素自己的page里
				app_delete_flag = 1;//用于app删除后的回调代码块
				running_func(cur_item->userdata);//立即回调app以执行app删除回调代码块
				app_delete_flag = 0;
        pageJumper(cur_page);
      }
    }
  }
  if(running_func == NULL){//函数为空时才允许游标检索，否则在活动回调里短按左右键出现游标
    if (mykeys.left == CLICKED) {
      Item_Typedef * pt = cur_page->first_item;
			/*分两种情况寻找上一个元素：
				1.当前元素不是首元素：那只需要找到持有自己指针的元素即可
				2.当前元素是首元素：上一个元素就是末尾元素了，故只需要找到下一个元素指针为空的元素
			*/
      while (pt) {
        if ((cur_item != cur_page->first_item && pt->next_item == cur_item) ||
            (cur_item == cur_page->first_item && pt->next_item == NULL))
          break;
        pt = pt->next_item;
      }
      cur_item = pt;//将找到的元素作为当前元素
    }
    if (mykeys.right == CLICKED) {//找下一个元素
      if (cur_item) {
        if (cur_item->next_item == NULL)//最后一个元素则下一个元素为头元素
          cur_item = cur_page->first_item;
        else
          cur_item = cur_item->next_item;
      }
    }
  }
  if (running_func) {//若为pageJumper回调函数则只在这个函数中执行一次，因为里面对running_func置空了
    running_func(cur_item->userdata);
  }
  if (cur_item) {//若page没元素，则游标不会出现
    if (index_y != cur_item->id) {//时刻检测当前游标位置，以更新
      if(index_y!=0)//不刷新标题
        OLED_DrawBMP(0, index_y, 8, index_y+1, NULL);//清除原来的游标
      OLED_DrawBMP(0,cur_item->id, 8,cur_item->id+1,BMP_INDEX_8X8);//在新的位置上画游标
      index_y = cur_item->id;//将新的id作为当前游标位置
    }
  }
	//以下是按键状态捕获与该函数衔接的关键，处理完按键事务后除了pressing状态的其他状态下按键都要复位
  if(mykeys.enter!=PRESSING)mykeys.enter = NONE;
  if(mykeys.right!=PRESSING)mykeys.right = NONE;
  if(mykeys.left!=PRESSING)mykeys.left = NONE;
}



//Appliance	定义应用回调函数↓
static idata uint8_t PGA_Gain=1;

static void ShowRMSFunc(void *_data) {
  static uint8_t ticks=0;
  float RMS;
  if(app_init_flag==1){
		PGA_Gain = getGain();
		return;
  }
  if(++ticks*TASK_MENU_CORE_MS_PER_TICK>=200){
		uint16_t adc;
		float DC_volt;
		adc = TM7705_ReadAdc();
    RMS = getRMS(256,PGA_Gain,&DC_volt);
		screenPutString(24,2, "DC: %6.1fmV",DC_volt);
    screenPutString(18,4, "RMS: %6.1fmV",RMS);
		UART_SendBuf((uint8_t *)&RMS,4);
    ticks = 0;
  }
}

static void ChangeGainFunc(void *_data) {
  if(app_init_flag==1){
    PGA_Gain = getGain();
		return;
  }
  if (mykeys.left == CLICKED) {
		PGA_Gain = PGA_Gain==0?7:PGA_Gain-1;
  }
  if (mykeys.right == CLICKED) {
		PGA_Gain = PGA_Gain==7?0:PGA_Gain+1;
  }
	screenPutString(42,3, "X %3bu",(uint8_t)(0x01<<PGA_Gain));
	if(app_delete_flag==1){
		setGain(PGA_Gain);
	}

}

static void AboutFunc(void *_data) {
	if(app_init_flag==1){
    OLED_DrawBMP(49,2,79,6,BMP_AVATAR_32X30);
		OLED_ShowStrCenterAligned("Author:JasomWu",7);
		return;
  }
}
