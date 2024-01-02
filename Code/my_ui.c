#include "my_ui.h"
#include "codetab.h"
#include "string.h"

sbit KEY_LEFT = P3 ^ 1;
sbit KEY_ENTER = P3 ^ 0;
sbit KEY_RIGHT = P3 ^ 2;

static idata Page_Typedef page_Main;
static idata Item_Typedef item_ShowRMS, item_ChangeGain, item_About;

static idata Page_Typedef *cur_page = NULL;
static idata Item_Typedef *cur_item = NULL;

static idata uint8_t index_y = 0;
static idata ExecuteFunc running_func = NULL;

idata struct {
    uint8_t left: 2;
    uint8_t enter: 2;
    uint8_t right: 2;
    uint32_t enter_last_tick;
}mykeys = {0};

static bdata bit app_init_flag=0;
static bdata bit app_delete_flag=0;
static void ShowRMSFunc(void *_data);

static void ChangeGainFunc(void *_data);

static void AboutFunc(void *_data);

static void itemInit(Item_Typedef *item, char *desc, ExecuteFunc func, void *userdata) {
  item->desc = desc;
  item->func = func;
  item->userdata = userdata;
  item->next_item = NULL;
  item->id = 1;
  if (cur_item) {
    cur_item->next_item = item;
    item->id = cur_item->id + 1;
  }
  cur_item = item;
}

static void pageInit(Page_Typedef *page, char *title, Item_Typedef *first_item) {
  page->title = title;
  page->first_item = first_item;
  page->last_page = NULL;
  cur_item = NULL;//�µ�һҳ��ͷ�ڵ��ÿ�
}
static void OLED_ShowStrCenterAligned(const char *str,uint8_t y){
  uint8_t x_padding=0,len;
  len = strlen(str)*6;//�����㷨
  if(SCREEN_WIDTH>=len)
    x_padding = (SCREEN_WIDTH - len)/2;
  OLED_ShowStr(x_padding,y,1,str);
}
static void pageJumper(void *tar_page) {
  Page_Typedef *new_page = (Page_Typedef *) tar_page;
  if (tar_page != NULL) {
		Item_Typedef * pt;
    uint8_t i;
    if(cur_page->last_page==new_page)//��Ϊpage�����򽫵�ǰ��last_page�ÿ�
      cur_page->last_page = NULL;
    else //��Ϊpage�����򽫵�ǰpage���浽�µ�page��last_page
      new_page->last_page = cur_page;//�洢��һ��page�Ա㷵��
    cur_page = new_page;
    cur_item = cur_page->first_item;
    running_func = NULL;
    OLED_CLS(0);
    OLED_ShowStrCenterAligned(cur_page->title,0);
    pt = cur_page->first_item;
    for(i =1;i<8;i++){
      if(pt==NULL)
        break;
      OLED_ShowStr(8,i,1,pt->desc);
			pt = pt->next_item;
    }
    index_y = 0;
  }
}

void MenuInit(void) {
  oledInit();
  OLED_ShowStr(16, 1, 1, "JASOM-WU");
  OLED_ShowStr(10, 2, 1, "2023.11.25");
  delay_ms(500);
  OLED_CLS(0);

  pageInit(&page_Main, "MAIN", &item_ShowRMS);
  itemInit(&item_ShowRMS, "ShowRMS", ShowRMSFunc, NULL);
  itemInit(&item_ChangeGain, "ChangeGain", ChangeGainFunc, NULL);
  itemInit(&item_About, "About", AboutFunc, NULL);

  pageJumper(&page_Main);//��ת����ʼ���棬����˵�
}
void MenuKeyHandler() {
  KEY_LEFT = KEY_ENTER = KEY_RIGHT = 1;
  switch (mykeys.left) {
    case NONE: {
      if (KEY_LEFT == 0)
        mykeys.left = PRESSING;
      break;
    }
    case PRESSING: {
      if (KEY_LEFT == 1)
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
        mykeys.enter_last_tick = getTick();
      }
      break;
    }
    case PRESSING: {
      if (KEY_ENTER == 1) {
        if (getTick() - mykeys.enter_last_tick > 900){
					mykeys.enter = LONG_PRESSED;
				}
        else{
          mykeys.enter = CLICKED;
        }
      }
      break;
    }
  }
}

void MenuProcessHandler() {
  if (mykeys.enter == CLICKED) {
    if(cur_item){
      if(running_func == NULL){
        OLED_CLS(0);
        running_func = cur_item->func;
        app_init_flag = 1;//����app����ʱ��ʼ�������
      }
    }
  } else if (mykeys.enter == LONG_PRESSED) {
    if(cur_page){
      if(running_func == NULL)
        pageJumper(cur_page->last_page);
      else{
        pageJumper(cur_page);
        app_delete_flag = 1;//����appɾ����Ļص������
      }

    }
  }
  if (mykeys.left == CLICKED) {
    Item_Typedef * pt = cur_page->first_item;
    while (pt) {
      if ((cur_item != cur_page->first_item && pt->next_item == cur_item) ||
          (cur_item == cur_page->first_item && pt->next_item == NULL))
        break;
      pt = pt->next_item;
    }
    cur_item = pt;
  }
  if (mykeys.right == CLICKED) {
    if (cur_item) {
      if (cur_item->next_item == NULL)
        cur_item = cur_page->first_item;
      else
        cur_item = cur_item->next_item;
    }
  }
  if (running_func) {
    running_func(cur_item->userdata);
  }
  if (cur_item) {
    if (index_y != cur_item->id) {
      if(index_y!=0)
        OLED_DrawBMP(0, index_y, 8, index_y+1, NULL);
      OLED_DrawBMP(0,cur_item->id, 8,cur_item->id+1,BMP_INDEX_8X8);
      index_y = cur_item->id;
    }
  }
  if(mykeys.enter!=PRESSING)mykeys.enter = NONE;
  if(mykeys.right!=PRESSING)mykeys.right = NONE;
  if(mykeys.left!=PRESSING)mykeys.left = NONE;
}

//Appliance��
static void ShowRMSFunc(void *_data) {
  static uint8_t ticks=0;
  float volt;
  if(app_init_flag==1){
    app_init_flag = 0;
    OLED_ShowStrCenterAligned(item_ShowRMS.desc,0);
  }
  if(++ticks*TASK_MENU_CORE_MS_PER_TICK>=200){
		uint16_t adc;
		adc = TM7705_ReadAdc(2);
    volt = getRMS(256);
    OLED_ShowStr(30,3,1,"%6.1fmV",volt);
    ticks = 0;
  }
}

static void ChangeGainFunc(void *_data) {

}

static void AboutFunc(void *_data) {

}
