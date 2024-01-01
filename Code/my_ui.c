#include "my_ui.h"
#include "codetab.h"
#include "string.h"

sbit KEY_LEFT = P3 ^ 1;
sbit KEY_ENTER = P3 ^ 0;
sbit KEY_RIGHT = P3 ^ 2;

idata Page_Typedef page_Main;
idata Item_Typedef item_ShowRMS, item_ChangeGain, item_About;

Page_Typedef *cur_page = NULL;
Item_Typedef *cur_item = NULL;


struct {
    uint8_t left: 2;
    uint8_t enter: 2;
    uint8_t right: 2;
    uint16_t enter_last_tick;
} keys_state;

static void ShowRMSFunc(void *userdata);

static void ChangeGainFunc(void *userdata);

static void AboutFunc(void *userdata);

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
  cur_page = page;
  cur_item = NULL;//新的一页，头节点置空
}

static void pageJumper(void *tar_page) {
  Page_Typedef *new_page = (Page_Typedef *) tar_page;
  if (tar_page != NULL) {
    if(cur_page->last_page==new_page)//若为page回退则将当前的last_page置空
      cur_page->last_page = NULL;
    else //若为page创建则将当前page保存到新的page的last_page
      new_page->last_page = cur_page;//存储上一个page以便返回
    cur_page = new_page;
    cur_item = cur_page->first_item;
  }
}

void MenuInit(void) {
  oledInit();
  OLED_ShowStr(8, 1, 2, "JASOM-WU");
  OLED_ShowStr(0, 3, 2, "2023.11.25");
  delay_ms(500);
  OLED_CLS(0);

  pageInit(&page_Main, "MAIN", NULL);
  itemInit(&item_ShowRMS, "ShowRMS", ShowRMSFunc, NULL);
  itemInit(&item_ChangeGain, "ChangeGain", ChangeGainFunc, NULL);
  itemInit(&item_About, "About", AboutFunc, NULL);

  cur_page = &page_Main;//指定初始页
  cur_item = &item_ShowRMS;//指定初始条目
}

void MenuKeyHandler() {
  KEY_LEFT = KEY_ENTER = KEY_RIGHT = 1;
  switch (keys_state.left) {
    case NONE: {
      if (KEY_LEFT == 0)
        keys_state.left = PRESSING;
      break;
    }
    case PRESSING: {
      if (KEY_LEFT == 1)
        keys_state.right = CLICKED;
      break;
    }
  }
  switch (keys_state.right) {
    case NONE: {
      if (KEY_RIGHT == 0)
        keys_state.right = PRESSING;
      break;
    }
    case PRESSING: {
      if (KEY_RIGHT == 1)
        keys_state.right = CLICKED;
      break;
    }
  }
  switch (keys_state.enter) {
    case NONE: {
      if (KEY_ENTER == 0) {
        keys_state.enter = PRESSING;
        keys_state.enter_last_tick = getTick();
      }
      break;
    }
    case PRESSING: {
      if (KEY_ENTER == 1) {
        if (getTick() - keys_state.enter_last_tick > 800)
          keys_state.enter = LONG_PRESSED;
        else
          keys_state.enter = CLICKED;
      }
      break;
    }
  }
}

void MenuProcessHandler() {
  static uint8_t index_y = 0;
	static ExecuteFunc running_func = NULL;
  if (keys_state.enter == CLICKED) {
    keys_state.enter = NONE;
    if(cur_item)
      running_func = cur_item->func;
  } else if (keys_state.enter == LONG_PRESSED) {
    keys_state.enter = NONE;
    if(cur_page)
      pageJumper(cur_page->last_page);
  }
  if (keys_state.left == CLICKED) {
    Item_Typedef * pt = cur_page->first_item;
    keys_state.left = NONE;
    while (pt) {
      if ((cur_item != cur_page->first_item && pt->next_item == cur_item) ||
          (cur_item == cur_page->first_item && pt->next_item == NULL))
        break;
      pt = pt->next_item;
    }
    cur_item = pt;
  }
  if (keys_state.right == CLICKED) {
    keys_state.right = NONE;
    if (cur_item) {
      if (cur_item->next_item == NULL)
        cur_item = cur_page->first_item;
      else
        cur_item = cur_item->next_item;
    }
  }
  if (running_func) {
    running_func(cur_item->userdata);
    if(running_func==pageJumper){
			uint8_t x_padding=0;
      uint8_t len = strlen(cur_page->title)*6;
			Item_Typedef * pt = cur_page->first_item;
      running_func = NULL;
      OLED_CLS(0);
			if(SCREEN_WIDTH>=len)
        x_padding = (SCREEN_WIDTH - len)/2;
      OLED_ShowStr(x_padding,0,1,cur_page->title);
      for(len =len;len<8;len++){
        if(pt==NULL)
          break;
        OLED_ShowStr(8,len,1,pt->desc);
      }
      index_y = 0;
    }
  }
  if (cur_item) {
    if (index_y != cur_item->id) {
      if(index_y!=0)
        OLED_DrawBMP(0, index_y, 8, index_y, NULL);
      OLED_DrawBMP(0,cur_item->id, 8,cur_item->id,BMP_INDEX_8X8);
      index_y = cur_item->id;
    }
  }
}

//Appliance↓
static void ShowRMSFunc(void *userdata) {
  userdata = NULL;
}

static void ChangeGainFunc(void *userdata) {

}

static void AboutFunc(void *userdata) {

}
